#include "mix_meta.h"

#include <stdlib.h>

#include "mix_log.h"

static mix_metadata_t* meta_data;

/**
 * @brief 申请并初始化metadata 包括全局的hash 全局的bloom_filter 全局的free_segment等
 * 
**/
bool mix_init_metadata(uint32_t offset,uint32_t block_num){
    meta_data = malloc(sizeof(mix_metadata_t));
    if(meta_data == NULL){
        mix_log("mix_init_metadata","malloc for metadata failed");
        return false;
    }

    meta_data->block_num = block_num;
    meta_data->size = block_num * BLOCK_SIZE;
    meta_data->offset = offset;
    meta_data->per_block_num = block_num / SEGMENT_NUM;
    meta_data->bloom_filter = mix_init_counting_bloom_filter(block_num,0.01);
    if(meta_data->bloom_filter == NULL){
        return false;
    }
    meta_data->hash = mix_init_hash(20);
    if(meta_data->hash == NULL){
        return false;
    }
    
    size_t per_segment_size = BLOCK_SIZE * meta_data->per_block_num;

    for(int i = 0; i < SEGMENT_NUM; i++){
        if(!mix_init_free_segment(&(meta_data->segments[i]),offset + i * per_segment_size, per_segment_size)){
            mix_log("mix_init_metadata","init free segment faield");
            return false;
        }
    }
    return true;
}

/**
 * @brief 申请并初始化segment
 * 
 * @param size 每个segment的大小
 */
bool mix_init_free_segment(free_segment_t* segment, uint32_t offset, uint32_t size){
    if(segment == NULL)  return false;
    segment->bitmap = mix_bitmap_init(size/BLOCK_SIZE);
    segment->block_num = size / BLOCK_SIZE;
    segment->size = size;
    segment->head = offset;
    return true;
};

/**
 * @brief 为task获取free_segment中下一个空闲块
 * 整个流程包含4步
 * 1. 从free_segment中申请空闲块
 * 2. 将对应的bitmap设置为dirty
 * 3. 将key-value保存到hash中
 * 4. 将key保存到bloom filter中
 * 需要保证以上为原子操作 
**/
uint32_t mix_get_next_free_block(int idx, io_task_t* task){
    //从free_segment中申请空闲块
    int bit = mix_bitmap_next_zero_bit(meta_data->segments[idx].bitmap);
    if(bit == -1){
        return 0;
    }

    uint32_t value = bit + idx * meta_data->per_block_num;
    //将对应的bitmap设置为dirty
    if(!mix_bitmap_set_bit(bit,meta_data->segments[idx].bitmap)){
        return 0;
    }

    //将key-value保存到hash中
    mix_hash_put(meta_data->hash,task->offset,value);

    //将key保存到bloom filter中
    mix_counting_bloom_filter_add(meta_data->bloom_filter,task->offset);

    return value + meta_data->offset;
}

/**
 * @brief 释放task占用的free_segment中块
 * 整个流程包含3步
 * 1. 将key从bloom filter中移除
 * 2. 将key-value从hash中移除
 * 3. 将对应的bitmap设置为clean 
 * 需要保证以上为原子操作 
**/
void mix_clear_block(int idx, io_task_t* task){
    //将key从bloom filter中移除
    mix_counting_bloom_filter_remove(meta_data->bloom_filter,task->offset);

    //将对应的bitmap设置为clean
    uint32_t value = mix_hash_get(meta_data->hash, task->offset);
    if(value == -1){
        return;
    }

    int bit = value - meta_data->offset - idx * meta_data->per_block_num;

    mix_bitmap_clear_bit(bit,meta_data->segments[idx].bitmap);

    return;
}

io_task_v
