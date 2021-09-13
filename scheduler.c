#include "scheduler.h"

#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "mix_queue.h"
#include "ssd.h"
#include "nvm.h"

#define NVM_TASK 1
#define SSD_TASK 2
#define UNDEF_TASK 3

static scheduler_ctx_t* sched_ctx;

pthread_spin_lock* completation_locks;

static void* lock_bitmap;

static int nvm_size = 4 * 1024 * 1024 * 1024;//1 G
static int ssd_size = 128 * 1024 * 1024 * 1024; //128G

int mix_post_task_to_io(io_task_t* task){
    int ret = 0;
    ret = mix_alloc_completation_lock();
    task->ret_index = ret;

    if(ret >= 0){
        mix_enqueue(sched_ctx->io_queue,task,1);
    }
    return ret;
}

static inline int schedule(io_task_t* task){
    if(task->offset <= nvm_size && task->offset >= 0){
        return NVM_TASK;
    }else if (task->offset > nvm_size && task->offset <= (nvm_size + ssd_size)){
        return SSD_TASK;
    }else{
        return UNDEF_TASK;
    }
}

static void* scheduler(void* arg){
    int len = 0;
    int ret = 0;
    while(1){
        io_task_t* io_task = malloc(sizeof(io_task_t));
        len = mix_dequeue(io_queue,io_task,1);
        if(len < 1){
            free(io_task);
            continue;
        }

        switch(schedule(io_task)){
            case NVM_TASK:{
                io_task->ret_index = 
                mix_post_task_to_nvm();
            };
            case SSD_TASK:{
                mix_post_task_to_ssd();
            };     
        }
    } 
}


int mix_init_scheduler(unsigned int size, unsigned int esize, int max_current){
    sched_ctx = malloc(sizeof(scheduler_ctx_t));

    sched_ctx->io_queue = mix_queue_init(size,esize);
    sched_ctx->max_current = max_current;
    sched_ctx->completation_conds = malloc(sizeof(pthread_cond_t*)*max_current);
    for(int i = 0; i < max_current; i++){
        sched_ctx->completation_conds[i] = malloc(sizeof(pthread_cond_t));
    }
    sched_ctx->lock_bitmap = malloc(max_current/8);
    memset(sched_ctx->lock_bitmap,0,max_current/8);
    pthread_spin_init(sched_ctx->ctx_lock,NULL);
    pthread_t scheduler_thread;

    if(pthread_create(&scheduler_thread,NULL,scheduler,NULL)){
        printf("create scheduler failed\n");
        return -1;
    }
}

int mix_alloc_completation_lock(){
    static_assert(sched_ctx!=NULL);
    
    pthread_spin_lock(sched_ctx->ctx_lock);
    int first_zero_bit = -1;
    char* bitmap = (char*)(sched_ctx->lock_bitmap);
    for(int i = 0; i < sched_ctx->max_current/8; i++){
        char bits = bitmap[i];
        char mask = 0x00000001;
        for(int j = 0; j < 8; j++){
            if(bits&mask == 0){
                first_zero_bit = i*8+j;
                bitmap[i] = bitmap[i]|mask;
                goto out;
            }else{
                mask = mask<<1;
            }
        }
    }
out:
    pthread_spin_unlock(sched_ctx->ctx_lock);
    return first_zero_bit;
}


pthread_cond_t* mix_get_cond(int index){
    return sched_ctx[index];
}
