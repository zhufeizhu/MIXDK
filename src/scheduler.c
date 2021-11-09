#include "scheduler.h"

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "mixdk.h"
#include "ssd_queue.h"
#include "nvm_queue.h"
#include "mix_bit.h"

#define NVM_TASK 1
#define SSD_TASK 2
#define UNDEF_TASK 3

static scheduler_ctx_t *sched_ctx = NULL;

static size_t nvm_size = (size_t)128 * 1024 * 1024 * 1024;  //128 G
static size_t ssd_size = (size_t)1024 * 1024 * 1024 * 1024; //1T

int mix_wait_for_task_completed(atomic_bool* flag){
    while(atomic_load(flag) == false){};
    return 0;
}

// /**
//  * 任务完成时的回调函数 这里只需要在读完成时回调即可
// **/
// static inline void mix_task_completed(io_task_t* task){
//     assert(task != NULL);
//     if(task->flag == NULL) return;
    
//     atomic_store(task->flag,true);
//     completed_task_num++;
// }

size_t get_completed_task_num(){
    return mix_get_completed_nvm_task_num() + mix_get_completed_ssd_task_num();
}

static atomic_int task_num = 0;

int mix_post_task_to_io(io_task_t *task)
{
    int l = 0;
    while(l == 0){
        l = mix_enqueue(sched_ctx->submit_queue, task, 1);
    }
    
    return l;
}

static inline int schedule(io_task_t *task)
{
    if ((size_t)task->offset < nvm_size && task->offset >= 0) {
        // printf("nvm task\n");
        return NVM_TASK;
    }
    
    if ((size_t)task->offset >= nvm_size && (size_t)task->offset < (nvm_size + ssd_size)) {
        task->offset -= nvm_size;
        // printf("ssd task\n");
        return SSD_TASK;
    }
    //printf("[task offset]:%llu\n [nvm_size]:%llu\n [ssd_size]:%llu\n",(size_t)task->offset,nvm_size,ssd_size);
    return UNDEF_TASK;
}

static void scheduler(void *arg)
{
    int len = 0;
    io_task_t* io_task = malloc(TASK_SIZE);
    while (1) {
        //pthread_spin_lock(sched_ctx->schedule_queue_lock);
        len = mix_dequeue(sched_ctx->submit_queue, io_task, 1);
        //pthread_spin_unlock(sched_ctx->schedule_queue_lock);
        if (len == 0) {
            //printf("empty\n");
            continue;
        }

        switch (schedule(io_task)) {
            case NVM_TASK:
            {
                //printf("post task num is %d\n",task_num++);
                mix_post_task_to_nvm(io_task);
                break;
            };
            case SSD_TASK:
            {
                mix_post_task_to_ssd(io_task);
                break;
            };
            default:{
                //printf("post error\n");
                break;
            }
        }
    }
}

int mix_init_scheduler(unsigned int size, unsigned int esize, int max_current)
{
    pthread_t scheduler_thread;
    sched_ctx = malloc(sizeof(scheduler_ctx_t));

    sched_ctx->submit_queue = mix_queue_init(size, esize);
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

    sched_ctx->bitmap_lock = malloc(max_current / 8);
    memset(sched_ctx->bitmap_lock, 0, max_current / 8);

    ssd_info_t* ssd_info = mix_init_ssd_queue(size,esize);
    assert(ssd_info != NULL);
    nvm_info_t* nvm_info = mix_init_nvm_queue(size,esize);
    assert(nvm_info != NULL);
    sched_ctx->ssd_info = ssd_info;
    sched_ctx->nvm_info = nvm_info;

    sched_ctx->metadata = malloc(sizeof(scheduler_metadata_t));
    assert(sched_ctx->metadata != NULL);

    sched_ctx->metadata->nvm_size = (size_t)128 * 1024 * 1024 * 1024;  //128 G
    sched_ctx->metadata->ssd_size = (size_t)1024 * 1024 * 1024 * 1024;  //1 T

    sched_ctx->metadata->size = (sched_ctx->metadata->nvm_size + sched_ctx->metadata->ssd_size) / 2048;//8byte->4kb
    sched_ctx->metadata->ssd_saddr = 0;
    sched_ctx->metadata->nvm_saddr = sched_ctx->metadata->size;

    if (pthread_create(&scheduler_thread, NULL, (void*)scheduler, NULL)) {
        printf("create scheduler failed\n");
        return -1;
    }
    return 0;
}