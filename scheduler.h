#include "mix_queue.h"

#include <pthread.h>

typedef struct scheduler_ctx{
    mix_queue_t* io_queue;
    int max_current;
    pthread_cond_t* completation_conds;
    pthread_spin_lock_t* ctx_lock;
    void* lock_bitmap;
}scheduler_ctx_t;

int mix_post_task_to_io(io_task_t*);

pthread_cond_t* mix_get_cond(int);