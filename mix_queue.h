#ifndef MIX_QUEUE_H_
#define MIX_QUEUE_H_

#include <stdlib.h>
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

#define MIX_WRITE 1
#define MIX_READ 1<<1

typedef struct mix_queue{
    unsigned int in;
    unsigned int out;
    unsigned int mask;
    unsigned int esize;
    int (*write)(void*,unsigned int, unsigned int);
    int (*read)(void*,unsigned int, unsigned int);
    void* data;
}mix_queue_t;

#define A 1

// typedef struct general_task{
    
//     char padding[9];//保证长度是2的整次幂
// } __attribute__((packed)) general_task_t;

typedef struct io_task{
    char* buf;
    size_t len;
    size_t offset;
    u_int8_t opcode;
    u_int16_t ret_index;
    char padding[9];//保证结果是2的整次幂 目前是32
} __attribute__((packed)) io_task_t;

mix_queue_t* mix_queue_init(unsigned int size, unsigned int esize);

unsigned int mix_enqueue(mix_queue_t* queue, const void* src,unsigned int len);

unsigned int mix_dequeue(mix_queue_t* queue,void* dst, unsigned int len);

void mix_queue_free(mix_queue_t* queue);

#endif //MIXDK_MIX_QUEUE