#ifndef MIXDK_NVM_H
#define MIXDK_NVM_H

#include <stdlib.h>

typedef struct nvm_info{
    void* nvm_buffer;
    size_t block_num;
    int block_size;
}nvm_info_t;

int mix_nvm_init();

size_t  mix_nvm_read(void*,size_t, size_t);

size_t mix_nvm_write(void*,size_t,size_t);

#endif