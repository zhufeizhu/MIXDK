#include "ssd.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


ssd_info_t* ssd_info;

/**
 * 打开裸设备进行读写
**/
ssd_info_t* mix_ssd_init(){
    ssd_info = malloc(sizeof(ssd_info_t));
    ssd_info->block_num = 1024 * 1024 * 1024;
    ssd_info->block_size = SSD_BLOCK_SIZE;
    ssd_info->ssd_fd = open("/dev/sdb",O_RDWR);
    if(ssd_info->ssd_fd < 0){
        free(ssd_info);
        perror("mix_ssd_init");
        return NULL;
    }
    ssd_info->ssd_capacity = (size_t)1024 * 1024 * 1024 * 1024;
    return ssd_info;
}

size_t mix_ssd_read(void* dst, size_t len, size_t offset,size_t flags){
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

static size_t local_time = 0;

size_t mix_ssd_write(void* src, size_t len, size_t offset,size_t flag){
    size_t l = len;
    if((len + offset) > ssd_info->ssd_capacity){
        l = ssd_info->ssd_capacity - offset;
    }

    int n = pwrite(ssd_info->ssd_fd,src,l,offset);
    //printf("mix ssd write: len:%llu, offset:%llu\n",len,offset);


    if(n <= 0){
        perror("mix_ssd_write");
        return 0;
    }

    if(flag & MIX_SYNC){
        local_time++;
        //printf("%llu sync\n",local_time);
        fsync(ssd_info->ssd_fd);
        //fsync(ssd_info->ssd_fd);
    }
    
    return n;
}