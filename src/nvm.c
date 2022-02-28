#include "nvm.h"

#include <assert.h>
#include <emmintrin.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

#include "mix_log.h"

#define FLUSH_ALIGN 64
#define uintptr_t unsigned long long
#define BLOCK_SIZE 4096
#define META_SIZE sizeof(buffer_meta_t)

static nvm_info_t* nvm_info;
static buffer_info_t* buffer_info;

void mix_nvm_free(nvm_info_t* nvm_info) {
    if (nvm_info == NULL)
        return;
    free(nvm_info);
}

void mix_buffer_free(buffer_info_t* buffer_info) {
    if (buffer_info == NULL)
        return;
    free(buffer_info);
}

void mix_mmap(nvm_info_t* nvm_info, buffer_info_t* buffer_info) {
    int raw_nvm_fd = open("/dev/pmem0", O_RDWR);
    void* mmap_addr =
        mmap(NULL, nvm_info->nvm_capacity + buffer_info->buffer_capacity,
             PROT_READ | PROT_WRITE, MAP_SHARED, raw_nvm_fd, 0);

    if (mmap_addr == MAP_FAILED) {
        mix_log("mix_mmap", "mmap pmem0 failed");
        mix_buffer_free(buffer_info);
        mix_nvm_free(nvm_info);
        return;
    }

    nvm_info->nvm_addr = mmap_addr;
    buffer_info->meta_addr = mmap_addr + nvm_info->nvm_capacity;
    buffer_info->buffer_addr =
        buffer_info->meta_addr + buffer_info->block_num * sizeof(buffer_meta_t);
    return;
}

nvm_info_t* mix_nvm_init() {
    nvm_info = malloc(sizeof(nvm_info_t));
    if (nvm_info == NULL) {
        mix_log("mix_nvm_init", "malloc for nvm info failed");
        return NULL;
    }

    nvm_info->block_size = 4096;
    nvm_info->block_num = 16 * 1024 * 1024;
    nvm_info->queue_num = 4;
    nvm_info->per_block_num = nvm_info->block_num / nvm_info->queue_num;
    nvm_info->nvm_capacity = nvm_info->block_num * BLOCK_SIZE;  // nvm 64G
    nvm_info->nvm_addr = NULL;
    return nvm_info;
}

buffer_info_t* mix_buffer_init() {
    buffer_info = malloc(sizeof(buffer_info_t));
    if (buffer_info == NULL) {
        mix_log("mix_buffer_init", "malloc for buffer info failed");
        return NULL;
    }

    buffer_info->block_num = 4 * 1024;  //总共分成4块 一块有空间 一共占用8m
    buffer_info->buffer_capacity =
        (size_t)buffer_info->block_num * (4096 + sizeof(buffer_meta_t));
    buffer_info->meta_addr = NULL;
    buffer_info->buffer_addr = NULL;
    return buffer_info;
}

static inline void mix_clflush(const void* addr) {
    _mm_clflush(addr);
}

static inline void mix_ntstore32(int* dst, int n) {
    _mm_stream_si32(dst, n);
}

static inline void mix_ntstore64(long long* dst, unsigned long n) {
    _mm_stream_si64(dst, n);
}

static inline void mix_ntstorenx32(const void* dst,
                                   const void* src,
                                   size_t len) {
    for (size_t i = 0; i < len; i = i + 4) {
        mix_ntstore32((int*)(dst + i), *(int*)(src + i));
    }
}

static inline void mix_ntstorenx64(const void* dst,
                                   const void* src,
                                   size_t len) {
    for (size_t i = 0; i < len; i = i + 8) {
        mix_ntstore64((long long*)(dst + i), *(long long*)(src + i));
    }
}

static inline void mix_clflushopt(const void* addr) {
    asm volatile(".byte 0x66 clflush %0" : "+m"(*(volatile char*)(addr)));
}

static inline void mix_flushopt(const void* addr, size_t len) {
    uintptr_t uptr;

    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
         uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
        mix_clflush((char*)uptr);
    }
}

static inline void mix_flush(const void* addr, size_t len) {
    uintptr_t uptr;

    for (uptr = (uintptr_t)addr & ~(FLUSH_ALIGN - 1);
         uptr < (uintptr_t)addr + len; uptr += FLUSH_ALIGN) {
        mix_clflush((char*)uptr);
    }
}

size_t mix_nvm_read(void* dst, size_t len, size_t offset, size_t flags) {
    // size_t l = 0;
    // if ((len + offset) > nvm_info->block_num) {
    //     l = nvm_info->block_num - offset;
    // } else {
    //     l = len;
    // }
    //_mm_lfence();
    mix_ntstorenx64(dst, nvm_info->nvm_addr + offset * BLOCK_SIZE,
                    len * BLOCK_SIZE);
    // printf("[len] %d [offset] %d\n",l,offset);

    return len;
}

static _Atomic size_t local_time = 0;

size_t mix_nvm_write(void* src, size_t len, size_t offset, size_t flags) {
    //printf("write %lld task\n",local_time++);
    mix_ntstorenx32(nvm_info->nvm_addr + offset * BLOCK_SIZE, src, len * BLOCK_SIZE);
    //memcpy(nvm_info->nvm_addr + offset * BLOCK_SIZE, src, len * BLOCK_SIZE);
    // printf("nvm task local time is %lld\n",local_time++);
    //printf("[%lld]:[len] %lld [offset] %lld\n",local_time++,len,offset);

    return len;
}

size_t mix_buffer_read(void* src, size_t dst_block, size_t flags) {
    buffer_meta_t meta;

    mix_ntstorenx32(src, buffer_info->buffer_addr + dst_block * BLOCK_SIZE,
                    BLOCK_SIZE);
    mix_ntstorenx32(&meta, buffer_info->meta_addr + META_SIZE * dst_block,
                    META_SIZE);
    return BLOCK_SIZE;
}

size_t mix_buffer_write(void* src,
                        size_t src_block,
                        size_t dst_block,
                        size_t flags) {
    
    // struct timespec start, end;
    // clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    // buffer_meta_t meta;
    // meta.flags = flags;
    // meta.status = 1;
    // meta.timestamp = 0;  //暂时不用
    // meta.offset = src_block;

    // mix_ntstorenx64(buffer_info->buffer_addr + dst_block * BLOCK_SIZE, src,
    //                 BLOCK_SIZE);
    // mix_ntstorenx64(buffer_info->meta_addr + META_SIZE * dst_block, &meta,
    //                 META_SIZE);
    //printf("[%lld]:[len] %d [offset] %lld\n",local_time++,1,src_block);
    // clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    // printf("data time is %lu us\n", (end.tv_sec - start.tv_sec) * 1000000 +
    //                                (end.tv_nsec - start.tv_nsec) / 1000);
    // clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    // clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    // printf("meta time is %lu ns\n",
    //                                (end.tv_nsec - start.tv_nsec));
    return BLOCK_SIZE;
}

void mix_buffer_clear(size_t dst_block) {
    size_t status = 0;
    mix_ntstorenx32(buffer_info->buffer_addr + dst_block * BLOCK_SIZE, &status,
                    sizeof(size_t));
}

void mix_buffer_record(size_t src_block,
                        size_t dst_block,
                        size_t flags){
    buffer_meta_t meta;
    meta.flags = flags;
    meta.status = 1;
    meta.timestamp = 0;  //暂时不用
    meta.offset = src_block;
    mix_ntstorenx32(buffer_info->meta_addr + META_SIZE * dst_block, &meta,
                     META_SIZE);
}