#include "mock_ssd.h"

#include <stdio.h>

int mix_mock_ssd_init(){

}

unsigned int mix_mock_ssd_read(void* dst, unsigned int len, unsigned int offset){
    printf("type: read ssd, len: %d, offset: %d\n");
}

unsigned int mix_mock_ssd_write(void* src, unsigned int len, unsigned int offset){
    printf("type: write ssd, len: %d, offset: %d\n");
}