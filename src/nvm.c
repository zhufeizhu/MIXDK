#include "nvm.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <emmintrin.h>

#define FLUSH_ALIGN 64
#define uintptr_t unsigned long long

static nvm_info_t* nvm_info;

nvm_info_t* mix_nvm_init(){
    nvm_info = malloc(sizeof(nvm_info_t));
    assert(nvm_info != NULL);

    nvm_info->block_size = 4096;
    nvm_info->nvm_capacity = (size_t)128 * 1024 * 1024 * 1024;
    nvm_info->block_num = 32 * 1024 * 1024;
    
    int raw_nvm_fd = open("/dev/pmem0",O_RDWR);
    nvm_info->nvm = mmap(NULL,nvm_info->nvm_capacity,PROT_READ|PROT_WRITE,MAP_SHARED,raw_nvm_fd,0);
    if(nvm_info->nvm == MAP_FAILED){
        perror("mix_nvm_init");

        return NULL;
    }
    return nvm_info;
}



static inline void mix_clflush(const void* addr){
    _mm_clflush(addr);
}

static inline void mix_ntstore32(int* dst, int n){
    _mm_stream_si32(dst,n);
}

static inline void mix_ntstore64(long long* dst,unsigned long n){
    _mm_stream_si64(dst,n);
}

static inline void mix_ntstorenx32(const void* dst, const void* src, size_t len){
    for(size_t i = 0; i < len; i = i + 4){
        mix_ntstore32((int*)(dst + i),*(int*)(src + i));
    }
}

static inline void mix_ntstorenx64(const void* dst, const void* src, size_t len){
    for(size_t i = 0; i < len; i = i + 8){
        mix_ntstore64((long long*)(dst + i),*(long long*)(src + i));
    }
}

static inline void mix_clflushopt(const void* addr){
    asm volatile(".byte 0x66 clflush %0" : "+m"\
        (*(volatile char*)(addr)));
}

static inline void mix_flushopt(const void* addr, size_t len){
    uintptr_t uptr;

    for(uptr = (uintptr_t)addr & ~(FLUSH_ALIGN -1); uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN){
        mix_clflush((char*)uptr);
    }
}

static inline void mix_flush(const void* addr, size_t len){
    uintptr_t uptr;

    for(uptr = (uintptr_t)addr & ~(FLUSH_ALIGN -1); uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN){
        mix_clflush((char*)uptr);
    }
}

size_t mix_nvm_read(void* dst, size_t len, size_t offset,size_t flags){
    size_t l = 0;
    if((len + offset) > nvm_info->nvm_capacity){
        l = nvm_info->nvm_capacity - offset;
    }else{
        l = len;
    }
    
    //_mm_lfence();
    mix_ntstorenx32(dst,nvm_info->nvm+offset,l);
    // printf("[len] %d [offset] %d\n",l,offset);

    return l;
}

//static size_t local_time = 0;

size_t mix_nvm_write(void* src, size_t len, size_t offset,size_t flags){
    size_t l = 0;
    if((len + offset) > nvm_info->nvm_capacity){
        l = nvm_info->nvm_capacity - offset;
    }else{
        l = len;
    }
    //_mm_sfence();
    mix_ntstorenx32(nvm_info->nvm+offset,src,l);

    //printf("nvm task local time is %d\n",local_time++);
    //printf("[%d]:[len] %d [offset] %d\n",local_time++,l,offset);

    return l;
}

size_t mix_buffer_read(void* src, size_t len, size_t offset, size_t flags){

}

size_t mix_buffer_write(void* dst, size_t len, size_t offset, size_t flags){

}