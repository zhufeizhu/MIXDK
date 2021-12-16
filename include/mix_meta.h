#ifndef MIX_META_H
#define MIX_META_H

#include <stdint.h>
#include <stdbool.h>

#include "mix_bitmap.h"
#include "mix_hash.h"
#include "mix_bloom_filter.h"
#include "mix_task.h"

#define SEGMENT_NUM 4

typedef struct free_segment{
    size_t size;    //当前segment的总体的大小
    uint32_t block_num;     //当前segment的block的个数
    uint32_t head;      //当前segment的起始地址     
    mix_bitmap_t* bitmap;   //描述当前segment的bitmap信息
}free_segment_t;

typedef struct mix_metadata{
    uint32_t offset;      //元数据区的偏移
    uint32_t size;        //元数据区的大小
    uint32_t block_num;     //元数据区block的个数
    uint32_t per_block_num;     //元数据区每个segment的block的个数
    mix_hash_t* hash;   //全局的hash
    mix_counting_bloom_filter_t* bloom_filter;  //全局的bloom_filter
    free_segment_t segments[SEGMENT_NUM];     //四个free_segment
}mix_metadata_t;

bool mix_init_metadata(uint32_t offset,uint32_t block_num);

bool mix_init_free_segment(free_segment_t* segment,uint32_t offset, uint32_t block_num);

uint32_t mix_get_next_free_block(int idx);

void mix_clear_block(int idx, io_task_t* task);

int mix_free_free_segment(free_segment_t* segments);

bool mix_bloom_filter_test(uint32_t offset);

bool mix_write_redirect_block(int idx, uint32_t offset, int bit);

#endif