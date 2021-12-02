#include "mix_meta.h"
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

size_t mix_get_buffer(size_t offset){
    
}