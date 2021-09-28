#include "ssd.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


ssd_info_t* ssd_info;

int mix_ssd_init(){
    ssd_info = malloc(sizeof(ssd_info_t));
    ssd_info->block_num = 1024 * 1024;
    ssd_info->block_size = SSD_BLOCK_SIZE;
    ssd_info->ssd_fd = open("/dev/sdb",O_RDWR);
    if(ssd_info->ssd_fd < 0){
        perror("mix_ssd_init");
        return -1;
    }
    ssd_info->ssd_capacity = ssd_info->block_num * SSD_BLOCK_SIZE;
    return 0;
}

size_t mix_ssd_read(void* dst, size_t len, size_t offset){
    size_t l = len;
    if((len + offset) > ssd_info->ssd_capacity){
        l = ssd_info->ssd_capacity - offset;
    }

    int n = pread(ssd_info->ssd_fd,dst,l,offset);
    if(n <= 0){
        perror("mix_ssd_read");
        return 0;
    }
    return n;
}

size_t mix_ssd_write(void* src, size_t len, size_t offset){
    size_t l = len;
    if((len + offset) > ssd_info->ssd_capacity){
        l = ssd_info->ssd_capacity - offset;
    }

    int n = pwrite(ssd_info->ssd_fd,src,l,offset);
    if(n <= 0){
        perror("mix_ssd_write");
        return 0;
    }
    printf("offset is %d, len is %d\n",offset ,l);
    return n;
}