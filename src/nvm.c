#include "nvm.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

static nvm_info_t* nvm_info;


int mix_nvm_init(){
    nvm_info = malloc(sizeof(nvm_info_t));
    nvm_info->block_size = 4096;
    nvm_info->block_num = 1024*256*10;//10GB
    
    int raw_nvm_fd = open("/dev/pmem0",O_RDWR);
    nvm_info->nvm_buffer = mmap(NULL,1024*1024*1024,PROT_READ|PROT_WRITE,raw_nvm_fd,MAP_SHARED,0);

    if(!nvm_info->nvm_buffer){
        return -1;
    }

    return 0;
}

unsigned int mix_nvm_read(void* dst, unsigned int len, unsigned int offset){
    unsigned int l = 0;
    if((len + offset) > nvm_info->block_num * nvm_info->block_size){
        l = nvm_info->block_num * nvm_info->block_size - offset;
    }else{
        l = len;
    }
    
    memcpy(dst,nvm_info->nvm_buffer+offset,l);
    
    return l;
}

unsigned int mix_nvm_write(void* src, unsigned int len, unsigned int offset){
    unsigned int l = 0;
    if((len + offset) > nvm_info->block_num * nvm_info->block_size){
        l = nvm_info->block_num * nvm_info->block_size - offset;
    }else{
        l = len;
    }

    memcpy(nvm_info->nvm_buffer+offset,src,l);
    
    return l;
}