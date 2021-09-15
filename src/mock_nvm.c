#include "mock_nvm.h"

#include <stdio.h>

int mix_mock_nvm_init(){
    return 0;
}

unsigned int mix_mock_nvm_read(void* dst, unsigned int len, unsigned int offset){
    printf("type: read nvm, len: %u, offset: %u\n",len,offset);
    return 0;
}

unsigned int mix_mock_nvm_write(void* src, unsigned int len, unsigned int offset){
    printf("type: write nvm, len: %u, offset: %u\n",len,offset);
    return 0;
}