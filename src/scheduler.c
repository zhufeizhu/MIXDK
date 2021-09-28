#include "scheduler.h"

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "ssd_queue.h"
#include "nvm_queue.h"
#include "mix_bit.h"

#define NVM_TASK 1
#define SSD_TASK 2
#define UNDEF_TASK 3

static scheduler_ctx_t *sched_ctx = NULL;

static size_t nvm_size = 4 * 1024 * 1024;  //1 G
static size_t ssd_size = 128 * 1024 * 1024; //128G

int mix_wait_for_task_completed(int index){
    pthread_mutex_lock(sched_ctx->ctx_mutex[index]);

    while(mix_test_bit(index,sched_ctx->lock_bitmap)){
        pthread_cond_wait(sched_ctx->completation_conds[index],sched_ctx->ctx_mutex[index]);
    }

    pthread_mutex_unlock(sched_ctx->ctx_mutex[index]);
}

static void mix_task_completed(io_task_t* task){
    int index = task->task_index;
    assert(task != NULL);
    assert(index >= 0 && index < sched_ctx->max_current);
    
    pthread_mutex_lock(sched_ctx->ctx_mutex[index]);
    // pthread_mutex_lock(sched_ctx->bitmap_lock);
    mix_clear_bit(index,sched_ctx->lock_bitmap);

    task->original_task->ret = task->ret;
    // pthread_mutex_unlock(sched_ctx->bitmap_lock);
    pthread_cond_signal(sched_ctx->completation_conds[index]);
    pthread_mutex_unlock(sched_ctx->ctx_mutex[index]);
    
}

// static void mix_task_failed(io_task_t* task){
//     int index = task->task_index;
//     assert(index >= 0 && index < sched_ctx->max_current);
    
//     pthread_mutex_lock(sched_ctx->ctx_mutex[index]);
//     mix_clear_bit(index,sched_ctx->lock_bitmap);
//     task->ret = TASK_SUCCEED;
//     pthread_cond_signal(sched_ctx->completation_conds[index]);
//     pthread_mutex_unlock(sched_ctx->ctx_mutex[index]);
// }

/**
 * 分配
 * 
**/
static int mix_alloc_completed_lock()
{
    assert(sched_ctx != NULL);

    pthread_mutex_lock(sched_ctx->bitmap_lock);
    int first_zero_bit = -1;
    for (int i = 0; i < sched_ctx->max_current; i++) {
        if(!mix_test_bit(i,sched_ctx->lock_bitmap)){
            mix_set_bit(i,sched_ctx->lock_bitmap);
            first_zero_bit = i;
            break;
        }
    }
    pthread_mutex_unlock(sched_ctx->bitmap_lock);
    return first_zero_bit;
}

int mix_post_task_to_io(io_task_t *task)
{
    __uint8_t index = task->task_index;
    // mix_alloc_completed_lock();
    // task->task_index = index;
    int l = 0;
    if (index >= 0) {
        task->on_task_completed = mix_task_completed;
        pthread_spin_lock(sched_ctx->schedule_queue_lock);
        l = mix_enqueue(sched_ctx->submit_queue, task, 1);//传递指针
        pthread_spin_unlock(sched_ctx->schedule_queue_lock);
    }
    return (l > 0)?index:-1;;
}

static inline int schedule(io_task_t *task)
{
    if (task->offset <= nvm_size && task->offset >= 0) {
        return NVM_TASK;
    }
    
    if (task->offset > nvm_size && task->offset <= (nvm_size + ssd_size)) {
        task->offset -= nvm_size;
        return SSD_TASK;
    }

    return UNDEF_TASK;
}

static void scheduler(void *arg)
{
    int len = 0;
    while (1) {
        io_task_t* io_task = malloc(TASK_SIZE);
        pthread_spin_lock(sched_ctx->schedule_queue_lock);
        len = mix_dequeue(sched_ctx->submit_queue, io_task, 1);
        pthread_spin_unlock(sched_ctx->schedule_queue_lock);
        if (len < 1) {
            free(io_task);
            continue;
        }

        switch (schedule(io_task)) {
            case NVM_TASK:
            {
                len = mix_post_task_to_nvm(io_task);
                break;
            };
            case SSD_TASK:
            {
                len = mix_post_task_to_ssd(io_task);
                break;
            };
            default:
                io_task->ret = -1;
                io_task->on_task_completed(io_task);
                break;
        }

        if(len < 0) {
            io_task->ret = -1;
            io_task->on_task_completed(io_task);
        }
    }
}

int mix_init_scheduler(unsigned int size, unsigned int esize, int max_current)
{
    pthread_t scheduler_thread;
    sched_ctx = malloc(sizeof(scheduler_ctx_t));

    sched_ctx->submit_queue = mix_queue_init(size, esize);
    sched_ctx->complete_queue = mix_queue_init(4*size,esize);
    sched_ctx->max_current = max_current;
    sched_ctx->completation_conds = malloc(sizeof(pthread_cond_t *) * max_current);
    for (int i = 0; i < max_current; i++) {
        sched_ctx->completation_conds[i] = malloc(sizeof(pthread_cond_t));
        pthread_cond_init(sched_ctx->completation_conds[i],NULL);
    }

    sched_ctx->ctx_mutex = malloc(sizeof(pthread_mutex_t*) * max_current);
    for(int i = 0; i < max_current; i++){
        sched_ctx->ctx_mutex[i] = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(sched_ctx->ctx_mutex[i],NULL);
    }

    sched_ctx->bitmap_lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(sched_ctx->bitmap_lock,NULL);
    sched_ctx->schedule_queue_lock = malloc(sizeof(pthread_mutex_t));
    pthread_spin_init(sched_ctx->schedule_queue_lock, 1);

    sched_ctx->lock_bitmap = malloc(max_current / 8);
    memset(sched_ctx->lock_bitmap, 0, max_current / 8);

    assert(mix_init_ssd_queue(size,esize) == 0);

    assert(mix_init_nvm_queue(size,esize) == 0);

    if (pthread_create(&scheduler_thread, NULL, (void*)scheduler, NULL)) {
        printf("create scheduler failed\n");
        return -1;
    }
    return 0;
}