#include <stdio.h>
#include <pthread.h>

#include "mix_queue.h"

static mix_queue_t* ssd_queue;

#define BLOCK_SIZE 4096

static void* mix_submit_to_ssd(void* arg){
    while(1){
        io_task_t* task = malloc(TASK_SIZE);
        mix_dequeue(ssd_queue,task,1);
        if(task->opcode & MIX_WRITE){
            mix_write_to_ssd(task->buf,task->len,task->block*BLOCK_SIZE + task->offset);
        }else if(task->opcode & MIX_READ){
            mix_read_from_ssd(task->buf,task->len,task->block*BLOCK_SIZE + task->offset);
        }
        mix_task_completed();
    }   
}

/**
 * 从ssd中读数据
**/
static inline int mix_read_from_ssd(void* dst,unsigned int len, unsigned int offset){

}


/**
 * 向ssd中写数据
**/
static inline int mix_write_to_ssd(void* src,unsigned int len, unsigned int offset){

}

int mix_init_ssd_queue(unsigned int size, unsigned int esize){
    ssd_queue = mix_queue_init(size,esize);
    pthread_t pid;
    if(pthread_create(&pid,NULL,mix_submit_to_ssd,NULL)){
        printf("create ssd queue failed\n");
        return -1;
    }

    return 0;
}



/**
 * Scheduler调用该接口将请求post到ssd的任务队列中
 * version0.1.0版本只有一个ssd的任务队列
**/
int mix_post_task_to_ssd(io_task_t* task){
    static_assert(task != NULL);
}

