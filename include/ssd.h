#ifndef MIXDK_SSD_H
#define MIXDK_SSD_H
#include <stdlib.h>

#include "mixdk.h"

#define SSD_BLOCK_SIZE 4096

typedef struct ssd_info {
    size_t block_num;
    int block_size;
    size_t ssd_capacity;
    int ssd_fd;
} ssd_info_t;

ssd_info_t* mix_ssd_init();

size_t mix_ssd_read(void*, size_t, size_t, size_t);

size_t mix_ssd_write(void*, size_t, size_t, size_t);

#endif