#ifndef MIX_SCHEDULER_H
#define MIX_SCHEDULER_H

#include "mix_queue.h"

#include <pthread.h>
#include <stdatomic.h>

#include "mix_task.h"
#include "nvm.h"
#include "ssd.h"

#define READ_AHEAD 0
#define WRITE_AHEAD 1
typedef struct scheduler_metadata {
    size_t saddr;      //起始地址 存放在nvm中
    size_t size;       //元数据的大小
    size_t nvm_size;   // nvm的存储大小
    size_t ssd_size;   // ssd的存储大小
    size_t nvm_saddr;  // nvm数据区的起始偏移
    size_t ssd_saddr;  // ssd数据去的起始偏移
} scheduler_metadata_t;

typedef struct scheduler_ctx {
    mix_queue_t* submit_queue;
    int max_current;
    pthread_cond_t** completation_conds;
    pthread_mutex_t** ctx_mutex;
    pthread_mutex_t* bitmap_lock;
    pthread_spinlock_t* schedule_queue_lock;
    scheduler_metadata_t* metadata;  //
    ssd_info_t* ssd_info;
    nvm_info_t* nvm_info;
    buffer_info_t* buffer_info;
} scheduler_ctx_t;

int mix_init_scheduler(unsigned int, unsigned int, int);

int mix_post_task_to_io(io_task_t*);

int mix_wait_for_task_completed(atomic_bool*);

size_t get_completed_task_num();

#endif  // MIX_SCHEDULER_H