#ifndef MIXDK_SSD_H
#define MIXDK_SSD_H
#include <stdlib.h>

#define SSD_BLOCK_SIZE 4096


typedef struct ssd_info{
    size_t block_num;
    int block_size;
    size_t ssd_capacity;
    int ssd_fd;
}ssd_info_t;

int mix_ssd_init();

size_t mix_ssd_read(void* dst, size_t len, size_t offset);

size_t mix_ssd_write(void* src, size_t len, size_t offset);

#endif