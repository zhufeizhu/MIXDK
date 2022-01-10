#include "ssd.h"

#define __USE_GNU 1
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

ssd_info_t* ssd_info;

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
    return ssd_info;
}

size_t mix_ssd_read(void* dst, size_t len, size_t offset, size_t flags) {
    size_t l = len;
    if ((len + offset) > ssd_info->ssd_capacity) {
        l = ssd_info->ssd_capacity - offset;
    }

    char* memalign_dst = valloc(l*SSD_BLOCK_SIZE);
    int n = pread(ssd_info->ssd_fd, memalign_dst, l * SSD_BLOCK_SIZE,
                  offset * SSD_BLOCK_SIZE);
    memcpy(dst,memalign_dst,l*SSD_BLOCK_SIZE);
    free(memalign_dst);
    if (n <= 0) {
        perror("mix_ssd_read");
        return 0;
    }
    return n;
}

static size_t local_time = 0;

size_t mix_ssd_write(void* src, size_t len, size_t offset, size_t flag) {
    size_t l = len;
    if ((len + offset) > ssd_info->block_num) {
        l = ssd_info->block_num - offset;
    }

    char* memalign_src = valloc(l*SSD_BLOCK_SIZE);
    memcpy(memalign_src,src,l*SSD_BLOCK_SIZE);

    int n = pwrite(ssd_info->ssd_fd, memalign_src, l * SSD_BLOCK_SIZE,
                   offset * SSD_BLOCK_SIZE);
    printf("mix ssd write: len:%llu, offset:%llu\n",len,offset);

    free(memalign_src);
    if (n <= 0) {
        perror("mix_ssd_write");
        return 0;
    }

    if (flag & MIX_SYNC) {
        local_time++;
        // printf("%llu sync\n",local_time);
        fsync(ssd_info->ssd_fd);
        // fsync(ssd_info->ssd_fd);
    }

    return n;
}