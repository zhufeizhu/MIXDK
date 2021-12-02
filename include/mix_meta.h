#ifndef MIX_META_H
#define MIX_META_H

#include "mix_bit.h"

typedef struct free_segment{
    size_t size;    //当前segment的总体的大小
    size_t block_num;
    size_t head; //当前segment的起始地址     
    mix_bitmap* bitmap;   //描述当前segment的bitmap信息
}free_segment_t;

void mix_init_free_segment(int fragment_num);

size_t get_next_free_segment(int idx, io_task_t* task);

size_t mix_get_buffer(int offset);

#endif