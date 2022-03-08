#ifndef MIXDK_NVM_H
#define MIXDK_NVM_H

#include "mixdk.h"

#include <stdint.h>
typedef struct nvm_info {
    void* nvm_addr;  // nvm的内存起始地址
    size_t block_num;
    size_t per_block_num;
    size_t nvm_capacity;
    int block_size;
    uint8_t queue_num;
} nvm_info_t;

typedef struct buffer_info {
    void* buffer_addr;  // buffer的内存起始地址
    void* meta_addr;    // buffer的字描述块地址
    size_t block_num;
    size_t buffer_capacity;
} buffer_info_t;

typedef struct buffer_meta {
    size_t status;     //状态位
    size_t timestamp;  //时间戳
    size_t offset;     //目标地址
    size_t flags;      //保留位
} __attribute__((packed)) buffer_meta_t;

nvm_info_t* mix_nvm_init();

buffer_info_t* mix_buffer_init();

void mix_mmap(nvm_info_t* nvm_info, buffer_info_t* buffer_info);

size_t mix_nvm_read(void*, size_t, size_t, size_t);

size_t mix_nvm_write(void*, size_t, size_t, size_t);

size_t mix_buffer_write(void* src, size_t, size_t, size_t);

size_t mix_buffer_read(void* dst, size_t dst_block, size_t flags);

void mix_buffer_clear(size_t dst_block);

void mix_buffer_get_meta(buffer_meta_t*, int);

#endif