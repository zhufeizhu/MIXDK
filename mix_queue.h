#ifndef MIXDK_MIX_QUEUE_H_
#define MIXDK_MIX_QUEUE_H_


#include "request.h"

#include <stdio.h>

#define EINVAL -1
#if 1
#define smp_wmb()	__sync_synchronize()
#else
#define smp_wmb()       wmb()

 #ifdef CONFIG_UNORDERED_IO
 #define wmb()   asm volatile("sfence" ::: "memory")
 #else
 #define wmb()   asm volatile("" ::: "memory")
 #endif
#endif

#define TASK_SIZE (sizeof(struct io_task))

typedef struct mix_queue{
    unsigned int in;
    unsigned int out;
    unsigned int mask;
    unsigned int esize;
    void* data; 
}mix_queue_t;

struct io_task{
    char* buf;
    size_t len;
    size_t block;
    size_t offset;
    /*flag[0]:请求类型 0为读 1为写*/
    /*flag[1]:请求介质:0为pm 1为ssd*/
    uint8_t flag; 
}io_task_t;

int mix_queue_init(mix_queue_t* queue,void* buffer,unsigned int size, unsigned int esize);

unsigned int mix_enqueue(mix_queue_t* queue, const void* src,unsigned int len,unsigned int off);

void mix_dequeue(mix_queue_t* queue,void* dst, unsigned int len,unsigned int off);

void mix_queue_free(mix_queue_t* queue);

#endif //MIXDK_MIX_QUEUE