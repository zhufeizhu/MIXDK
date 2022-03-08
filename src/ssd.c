#include "ssd.h"

#define __USE_GNU 1
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

ssd_info_t* ssd_info;
char* memalign_src = NULL;
char* memalign_dst = NULL;

/**
 * 打开裸设备进行读写
 **/
ssd_info_t* mix_ssd_init() {
    ssd_info = malloc(sizeof(ssd_info_t));
    ssd_info->block_num = 100 * 1024 * 1024;
    ssd_info->block_size = SSD_BLOCK_SIZE;
    ssd_info->ssd_fd = open("/dev/nvme0n1", O_RDWR|O_DIRECT);
    if (ssd_info->ssd_fd < 0) {
        free(ssd_info);
        perror("mix_ssd_init");
        return NULL;
    }
    ssd_info->ssd_capacity = (size_t)400 * 1024 * 1024 * 1024;
    memalign_src = valloc(4*4*SSD_BLOCK_SIZE); //最大的64k的对齐空间 预分配好64k的空间
    memalign_dst = valloc(4*4*SSD_BLOCK_SIZE);
    if(memalign_src == NULL || memalign_dst == NULL){
        perror("malloc align memory failed!");
        close(ssd_info->ssd_fd);
        free(ssd_info);
        return NULL;
    }
    return ssd_info;
}

size_t mix_ssd_read(void* dst, size_t len, size_t offset, size_t flags) {
    size_t off = offset * SSD_BLOCK_SIZE;
    size_t idx = (offset&15)/4;
    size_t l = len * SSD_BLOCK_SIZE;

    int n = pread(ssd_info->ssd_fd, memalign_dst + idx * 4 * SSD_BLOCK_SIZE, l, off);
    memcpy(dst,memalign_dst + idx * 4 * SSD_BLOCK_SIZE,l);
    if ( n <= 0) {
        perror("mix_ssd_read");
        return 0;
    }
    return len;
}

static size_t local_time = 0;

size_t mix_ssd_write(void* src, size_t len, size_t offset, size_t flag) {
    size_t off = offset * SSD_BLOCK_SIZE;
    size_t idx = (offset&15)/4;
    size_t l = len * SSD_BLOCK_SIZE;

    memcpy(memalign_src + idx * 4 * SSD_BLOCK_SIZE,src,l);
    int n = pwrite(ssd_info->ssd_fd, memalign_src + idx * 4 * SSD_BLOCK_SIZE, l,off);
    //printf("mix ssd write: len:%llu, offset:%llu\n",l,off);
    //free(memalign_src);
    if (n <= 0) {
        perror("mix_ssd_write");
        return 0;
    }

    return len;
}