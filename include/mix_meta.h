#ifndef MIX_META_H
#define MIX_META_H

#include <stdint.h>

#include "mix_bitmap.h"
#include "mix_hash.h"
#include "mix_bloom_filter.h"

typedef struct free_segment{
    size_t size;    //当前segment的总体的大小
    uint32_t block_num;
    size_t head; //当前segment的起始地址     
    mix_bitmap_t* bitmap;   //描述当前segment的bitmap信息
}free_segment_t;

typedef struct{
    size_t size;        //元数据区的大小
    mix_hash_t* hash;   //全局的hash
    mix_counting_bloom_filter_t* bloom_filter;  //全局的bloom_filter
    free_segment_t segments[4];     //四个free_segment
}mix_metadata_t;

free_segment_t* mix_init_free_segment(int size);

int mix_free_free_segment(free_segment_t* segment);



#endif