#ifndef MIXDK_NVM_H
#define MIXDK_NVM_H

#include "mixdk.h"


typedef struct nvm_info{
    void* nvm_buffer; //nvm的内存
    size_t block_num; 
    size_t nvm_capacity;
    int block_size;    
    int queue_num;
}nvm_info_t;

nvm_info_t* mix_nvm_init();

size_t mix_nvm_read(void*,size_t, size_t,size_t);

size_t mix_nvm_write(void*,size_t,size_t,size_t);

#endif