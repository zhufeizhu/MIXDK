#include "mock_ssd.h"

#include <stdio.h>

int mix_mock_ssd_init(){
    return 0;
}

unsigned int mix_mock_ssd_read(void* dst, unsigned int len, unsigned int offset){
    printf("type: read ssd, len: %u, offset: %u\n",len,offset);
    return 0;
}

unsigned int mix_mock_ssd_write(void* src, unsigned int len, unsigned int offset){
    printf("type: write ssd, len: %u, offset: %u\n",len,offset);
    return 0;
}