#ifndef MIXDK_NVM_H
#define MIXDK_NVM_H

#include "mixdk.h"
typedef struct nvm_info{
    void* nvm_addr;     //nvm的内存起始地址
    size_t block_num; 
    size_t nvm_capacity;
    int block_size;
}nvm_info_t;

typedef struct buffer_info{
    void* buffer_addr;      //buffer的内存起始地址
    void* meta_addr;        //buffer的字描述块地址
    size_t block_num;
    size_t buffer_capacity;
}buffer_info_t;

typedef struct buffer_meta{
    size_t status;      //状态位
    size_t timestamp;   //时间戳
    size_t offset;      //目标地址
    size_t flags;       //保留位
}buffer_meta_t;

nvm_info_t* mix_nvm_init();

buffer_info_t* mix_buffer_init();

size_t mix_nvm_read(void*,size_t, size_t,size_t);

size_t mix_nvm_write(void*,size_t,size_t,size_t);

size_t mix_buffer_write(void*, size_t, size_t, size_t);

size_t mix_buffer_read(void*, size_t, size_t, size_t);

#endif