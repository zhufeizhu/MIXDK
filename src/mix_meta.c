#include "mix_meta.h"

#include <stdlib.h>

#include "mix_hash.h"
#include "mix_log.h"
#include "mix_queue.h"
#include "mix_task.h"
#include "nvm.h"
#include "ssd.h"

/**
 * @brief 申请并初始化segment
 *
 * @param size 每个segment的大小
 */
static bool mix_free_segment_init(free_segment_t* segment, uint32_t size) {
    if (segment == NULL)
        return false;
    segment->bitmap = mix_bitmap_init(size / BLOCK_SIZE);
    segment->block_num = size / BLOCK_SIZE;
    segment->size = size;
    segment->used_block_num = 0;
    segment->segment_lock = malloc(sizeof(pthread_rwlock_t));
    pthread_rwlock_init(segment->segment_lock,NULL);
    return true;
};

/**
 * @brief 申请并初始化metadata 包括全局的hash 全局的bloom_filter
 *全局的free_segment等
 *
 **/
mix_metadata_t* mix_metadata_init(uint32_t block_num) {
    pthread_t pid;
    mix_metadata_t* meta_data = malloc(sizeof(mix_metadata_t));
    if (meta_data == NULL) {
        mix_log("mix_init_metadata", "malloc for metadata failed");
        return NULL;
    }

    meta_data->block_num = block_num;
    meta_data->size = block_num * BLOCK_SIZE;
    meta_data->per_block_num = block_num / SEGMENT_NUM;

    size_t per_segment_size = BLOCK_SIZE * meta_data->per_block_num;

    for (int i = 0; i < SEGMENT_NUM; i++) {
        meta_data->hash[i] = mix_hash_init(100);
        meta_data->bloom_filter[i] =
            mix_counting_bloom_filter_init(meta_data->per_block_num, 0.01);
        if (!mix_free_segment_init(&(meta_data->segments[i]),
                                   per_segment_size)) {
            mix_log("mix_init_metadata", "init free segment faield");
            return NULL;
        }
    }

    return meta_data;
}

/**
 * @brief 释放segment申请的内存
 *
 * @param segment
 */
void mix_free_free_segment(free_segment_t* segment) {
    if (segment == NULL)
        return;
    mix_bitmap_free(segment->bitmap);
}

/**
 * @brief 释放meta_data申请的内存
 *
 * @param meta_data
 */
void mix_metadata_free(mix_metadata_t* meta_data) {
    for (int i = 0; i < SEGMENT_NUM; i++) {
        mix_free_free_segment(&(meta_data->segments[i]));
        mix_hash_free(meta_data->hash[i]);
        mix_counting_bloom_filter_free(meta_data->bloom_filter[i]);
    }

    free(meta_data);
}

/**
 * @brief 为task获取free_segment中下一个空闲块 返回的值代表该块在buffer中的偏移
 *
 * @return 不存在空闲块时返回-1 存在时返回其在buffer中的偏移
 **/
int mix_get_next_free_block(mix_metadata_t* meta_data, int idx) {
    if (meta_data->segments[idx].migration) {
        return -1;
    }

    if(meta_data->segments[idx].used_block_num == meta_data->per_block_num) return -1;

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
bool mix_write_redirect_block(mix_metadata_t* meta_data,
                              int idx,
                              uint32_t offset,
                              int bit) {
    uint32_t value = bit + idx * meta_data->per_block_num;
    //将对应的bitmap设置为dirty
    if (!mix_bitmap_set_bit(meta_data->segments[idx].bitmap,bit)) {
        return false;
    }

    //将key-value保存到hash中
    mix_hash_put(meta_data->hash[idx], offset, value);

    //将key保存到bloom filter中
    mix_counting_bloom_filter_add(meta_data->bloom_filter[idx], offset);

    meta_data->segments[idx].used_block_num++;
    if (meta_data->segments[idx].used_block_num ==
        meta_data->segments[idx].block_num) {
        meta_data->migration = true;
    }
    return false;
}

/**
 * @brief 释放task占用的free_segment中块
 * 整个流程包含3步
 * 1. 将key从bloom filter中移除
 * 2. 将key-value从hash中移除
 * 3. 将对应的bitmap设置为clean
 * 需要保证以上为原子操作
 **/
void mix_clear_blocks(mix_metadata_t* meta_data, io_task_t* task) {
    for (int i = 0; i < task->len; i++) {
        for (int j = 0; j < SEGMENT_NUM; j++) {
            // 查询是否在bloom_filter中
            if (!mix_counting_bloom_filter_test(meta_data->bloom_filter[j],
                                                task->offset + i)) {
                continue;
            }

            // 查询是否在hash中
            int value = mix_hash_get(meta_data->hash[j], task->offset + i);
            if (value == -1) {
                continue;
            }

            //将key从bloom filter中移除
            mix_counting_bloom_filter_remove(meta_data->bloom_filter[j],
                                             task->offset + i);

            mix_hash_delete(meta_data->hash[j], task->offset + i);

            //将对应的bitmap设置为clean
            int bit = value % meta_data->per_block_num;
            mix_bitmap_clear_bit(meta_data->segments[j].bitmap, bit);
            meta_data->segments[j].used_block_num--;
            break;
        }
    }
}

/**
 * @brief 判断offset指代的block是否在buffer中
 *
 * @return 如果在则返回对应的偏移 如果不在则返回-1
 **/
int mix_buffer_block_test(mix_metadata_t* meta_data, uint32_t offset) {
    int value = -1;
    for (int i = 0; i < SEGMENT_NUM; i++) {
        if (!mix_counting_bloom_filter_test(meta_data->bloom_filter[i],
                                            offset)) {
            continue;
        }

        int value = mix_hash_get(meta_data->hash[i], offset);
        if (value == -1) {
            continue;
        }

        int bit = value % meta_data->per_block_num;
        if (!mix_bitmap_test_bit(meta_data->segments[i].bitmap, bit)) {
            value = -1;
            continue;
        }
        break;
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
inline bool mix_in_migration(mix_metadata_t* meta_data, int idx) {
    return meta_data->segments[idx].migration = true;
}

void migrate_from_buffer_to_ssd(uint32_t src, uint32_t dst) {
    char buf[4096];
    mix_buffer_read(buf,src,0);
    mix_ssd_write(buf,1,dst,0);
}

/**
 * @brief 将指定idx中buffer的数据迁入到ssd中
 *
 * @param idx
 */
void mix_migrate(mix_metadata_t* meta_data, int idx) {
    meta_data->hash[idx]->hash_list_node_entry = NULL;
    meta_data->hash[idx]->hash_node_entry_idx = 0;
    for (int i = 0; i < meta_data->per_block_num; i++) {
        mix_kv_t kv = mix_hash_get_entry(meta_data->hash[idx]);
        if (kv.key == (uint32_t)-1 && kv.value == (uint32_t)-1)
            continue;
        migrate_from_buffer_to_ssd(kv.value, kv.key);
        mix_counting_bloom_filter_remove(meta_data->bloom_filter[idx] ,kv.key);
        mix_bitmap_clear_bit(meta_data->segments[idx].bitmap, kv.value);
        meta_data->segments[idx].used_block_num--;
    }
}

inline bool mix_segment_migrating(mix_metadata_t* meta_data, int idx) {
    return meta_data->segments[idx].migration;
}

inline bool mix_buffer_migrating(mix_metadata_t* meta_data) {
    return meta_data->segments[0].migration &&
           meta_data->segments[1].migration &&
           meta_data->segments[2].migration && meta_data->segments[3].migration;
}

inline void mix_segment_migration_begin(mix_metadata_t* meta_data, int idx) {
    meta_data->segments[idx].migration = true;
}

inline void mix_segment_migration_end(mix_metadata_t* meta_data, int idx) {
    meta_data->segments[idx].migration = false;
}

inline bool mix_has_free_block(mix_metadata_t* meta_data, int idx) {
    return meta_data->segments[idx].used_block_num <
           meta_data->segments[idx].block_num;
}