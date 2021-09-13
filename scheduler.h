#ifndef MIX_SCHEDULER_H
#define MIX_SCHEDULER_H

#include "mix_queue.h"
#include <pthread.h>

#include "mix_task.h"

typedef struct scheduler_ctx{
    mix_queue_t* io_queue;
    int max_current;
    pthread_cond_t** completation_conds;
    pthread_spinlock_t* ctx_lock;
    pthread_mutex_t** ctx_mutex;
    void* lock_bitmap;
}scheduler_ctx_t;

int mix_init_scheduler(unsigned int, unsigned int, int);

int mix_post_task_to_io(io_task_t*);

int mix_wait_for_task_completed(int);

#endif //MIX_SCHEDULER_H