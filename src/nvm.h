#ifndef MIXDK_NVM_H
#define MIXDK_NVM_H

#include <stdlib.h>

typedef struct nvm_info{
    void* nvm_buffer;
    unsigned long long block_num;
    int block_size;
}nvm_info_t;

int mix_nvm_init();

unsigned int mix_nvm_read(void*,unsigned int, unsigned int);

unsigned int mix_nvm_write(void*,unsigned int,unsigned int);

#endif