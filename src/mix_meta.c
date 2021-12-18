#include "mix_meta.h"

#include <stdlib.h>

#include "mix_log.h"
#include "mix_queue.h"
#include "mix_task.h"

static mix_metadata_t* meta_data;

/**
 * @brief 申请并初始化metadata 包括全局的hash 全局的bloom_filter
 *全局的free_segment等
 *
 **/
bool mix_init_metadata(uint32_t block_num) {
    pthread_t pid;
    meta_data = malloc(sizeof(mix_metadata_t));
    if (meta_data == NULL) {
        mix_log("mix_init_metadata", "malloc for metadata failed");
        return false;
    }

    meta_data->block_num = block_num;
    meta_data->size = block_num * BLOCK_SIZE;
    meta_data->per_block_num = block_num / SEGMENT_NUM;
    meta_data->bloom_filter = mix_init_counting_bloom_filter(block_num, 0.01);
    if (meta_data->bloom_filter == NULL) {
        return false;
    }
    meta_data->hash = mix_init_hash(20);
    if (meta_data->hash == NULL) {
        return false;
    }

    size_t per_segment_size = BLOCK_SIZE * meta_data->per_block_num;

    for (int i = 0; i < SEGMENT_NUM; i++) {
        if (!mix_init_free_segment(&(meta_data->segments[i]),
                                   per_segment_size)) {
            mix_log("mix_init_metadata", "init free segment faield");
            return false;
        }
    }

    return true;
}

void mix_free_metadata() {
    for (int i = 0; i < SEGMENT_NUM; i++) {
        mix_free_free_segment(&(meta_data->segments[i]));
    }
    mix_free_hash(meta_data->hash);
    mix_free_counting_bloom_filter(meta_data->bloom_filter);
    free(meta_data);
}

/**
 * @brief 申请并初始化segment
 *
 * @param size 每个segment的大小
 */
bool mix_init_free_segment(free_segment_t* segment, uint32_t size) {
    if (segment == NULL)
        return false;
    segment->bitmap = mix_bitmap_init(size / BLOCK_SIZE);
    segment->block_num = size / BLOCK_SIZE;
    segment->size = size;
    return true;
};

void mix_free_free_segment(free_segment_t* segment) {
    if (segment == NULL)
        return;
    mix_bitmap_free(segment->bitmap);
}

/**
 * @brief 为task获取free_segment中下一个空闲块 返回的值代表该块在buffer中的偏移
 **/
uint32_t mix_get_next_free_block(int idx) {
    //从free_segment中申请空闲块
    return idx * meta_data->per_block_num +
           mix_bitmap_next_zero_bit(meta_data->segments[idx].bitmap);
}

/**
 * @brief 整个流程包含3步来设置元数据
 * 1. 将对应的bitmap设置为dirty
 * 2. 将key-value保存到hash中
 * 3. 将key保存到bloom filter中
 * 通过自描述来保证原子性
 *
 * @return true表示当前idx已满
 *         false表示当前idx未满
 **/
bool mix_write_redirect_block(int idx, uint32_t offset, int bit) {
    uint32_t value = bit + idx * meta_data->per_block_num;
    //将对应的bitmap设置为dirty
    if (!mix_bitmap_set_bit(bit, meta_data->segments[idx].bitmap)) {
        return false;
    }

    //将key-value保存到hash中
    mix_hash_put(meta_data->hash, offset, value);

    //将key保存到bloom filter中
    mix_counting_bloom_filter_add(meta_data->bloom_filter, offset);

    meta_data->segments[idx].used_block_num++;
    if (meta_data->segments[idx].used_block_num ==
        meta_data->segments[idx].block_num) {
        meta_data->migration = true;
    }
    return true;
}

/**
 * @brief 释放task占用的free_segment中块
 * 整个流程包含3步
 * 1. 将key从bloom filter中移除
 * 2. 将key-value从hash中移除
 * 3. 将对应的bitmap设置为clean
 * 需要保证以上为原子操作
 **/
void mix_clear_blocks(io_task_t* task) {
    for (int i = 0; i < task->len; i++) {
        // 查询是否在bloom_filter中
        if (!mix_counting_bloom_filter_test(meta_data->bloom_filter,
                                            task->offset + i)) {
            continue;
        }

        // 查询是否在hash中
        int value = mix_hash_get(meta_data->hash, task->offset + i);
        if (value == -1) {
            continue;
        }

        //将key从bloom filter中移除
        mix_counting_bloom_filter_remove(meta_data->bloom_filter,
                                         task->offset + i);

        mix_hash_delete(meta_data->hash, task->offset + i);

        //将对应的bitmap设置为clean
        int idx = value / meta_data->per_block_num;
        int bit = value % meta_data->per_block_num;
        mix_bitmap_clear_bit(bit, meta_data->segments[idx].bitmap);
        meta_data->segments[idx].used_block_num--;
    }
}

/**
 * @brief 判断offset指代的block是否在buffer中
 *
 * @return 如果在则返回对应的偏移 如果不在则返回-1
 **/
int mix_buffer_block_test(uint32_t offset) {
    if (!mix_counting_bloom_filter_test(meta_data->bloom_filter, offset)) {
        return -1;
    }

    int value = mix_hash_get(meta_data->hash, offset);
    if (value == -1) {
        return -1;
    }

    int idx = value / meta_data->per_block_num;
    int bit = value % meta_data->per_block_num;
    if (!mix_bitmap_test_bit(bit, meta_data->segments[idx].bitmap)) {
        return -1;
    }

    return value;
}

/**
 * @brief 判断当前segment是否还有空闲块
 *
 * @param idx segment的下标
 * @return true 有空闲块
 * @return false 无空闲块
 */
bool mix_has_free_block(int idx) {
    return meta_data->segments[idx].migration;
}