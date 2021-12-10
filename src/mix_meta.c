#include "mix_meta.h"
#include "mix_bitmap.h"
#include "mix_hash.h"
#include "mix_bloom_filter.h"

#include <stdbool.h>

static free_segment_t* segments;
static mix_hash_t* mix_hash; 

bool mix_init_free_segment(int fragment_num, int block_num){
    segments = (free_segment_t*)malloc(fragment_num * sizeof(free_segment_t));
    if(segments == NULL){
        return false;
    }

    for(int i = 0; i < fragment_num; i++){
        segments[i].bitmap = mix_init_bitmap(block_num);
        
        segments[i].
    }

};


//获取重定向后写的位置
size_t get_next_free_segment(int idx, io_task_t* task){
    free_segment_t segment = segments[idx];

    size_t next = segment.next;
    int bitx = 0;
    int bity = 0;
    __uint8_t mask = 0;
    do{
        bitx = next/8;
        bity = next%8;
        mask = 1 << bity;
    }while(!(segment.bitmap[bitx] & mask) && (next = (next+1)%segment.block_num)>=0);//直到找到符合要求的block块
    segment.bitmap[bitx] |= mask;
    segment.next = next + 1;
    return segment.head + segment.next * BLOCK_SZIE;
}

void mix_init_free_segment(int fragment_num);


/**
 * @brief 将给定的offset添加到bloom filter中
 * 
 * @param offset 重定向得到的offset
 */
void mix_set_buffer_for_bloom_filter(size_t offset){

}

/**
 * @brief 将给定的offset添加到hash table中
 * 
 * @param offset 重定向得到的offset
 */
void mix_set_buffer_for_hash_table(size_t offset){

}

/**
 * @brief 获取指定idx的segment的空闲块 并返回对应的offset
 * 
 * @param idx 
 * @param task 
 * @return size_t 
 */
size_t mix_get_next_free_segment(int idx, io_task_t* task);

/**
 * @brief mix_get_buffer的子查询 从bloom_filter中查询指定的offset是否不存在
 * 
 * @param offset 
 * @return size_t 
 */
size_t mix_get_buffer_by_bloom_filter(size_t offset);

/**
 * @brief mix_get_buffer的子查询 从hash_table中查询给定的offset所指向的key是否存在
 * 
 * @param offset 
 * @return size_t 
 */
size_t mix_get_buffer_by_hash_table(size_t offset);

/**
 * @brief 查询buffer中是否有指定offset的block 如果有则返回其在buffer中的offset 
   查询分为三个阶段
   1. 在对应的bloom filter中找 可以直接判断其是否不在buffer中
   2. 在全局的hash table中找 可以查询其所在的buffer中的偏移
   3. 将查询到的结果返回
 * 
 * @param offset 需要查询的block的offset
 * @return size_t 查询到的其在buffer中的offset 如果没有找到则返回-1
 */
size_t mix_get_buffer(int offset){

}