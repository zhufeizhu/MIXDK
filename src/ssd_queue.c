#include "ssd_queue.h"

#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "mock_ssd.h"

static mix_queue_t* ssd_queue;

#define BLOCK_SIZE 4096

/**
 * 从ssd中读数据
**/
static inline int mix_read_from_ssd(void* dst,unsigned int len, unsigned int offset){
    return mix_mock_ssd_read(dst,len,offset);
}


/**
 * 向ssd中写数据
**/
static inline int mix_write_to_ssd(void* src,unsigned int len, unsigned int offset){
    return mix_mock_ssd_write(src,len,offset);
}

static void mix_submit_to_ssd(void* arg){
    int len = 0;
    int ret = 0;
    while(1){
        io_task_t* task = malloc(TASK_SIZE);
        len = mix_dequeue(ssd_queue,task,1);
        if(!len){
            free(task);
            continue;
        }

        switch(task->opcode){
            case MIX_READ:
            {
                ret = mix_write_to_ssd(task->buf,task->len,task->offset);
                break;
            };
            case MIX_WRITE:
            {
                ret = mix_read_from_ssd(task->buf,task->len,task->offset);
                break;
            }
        }
        if(ret){
            task->on_task_succeed(task);
        }else{
            task->on_task_failed(task);
        }
    }
    return;
}

int mix_init_ssd_queue(unsigned int size, unsigned int esize){
    ssd_queue = mix_queue_init(size,esize);
    pthread_t pid;
    if(pthread_create(&pid,NULL,(void*)mix_submit_to_ssd,NULL)){
        printf("create ssd queue failed\n");
        return -1;
    }
    return 0;
}

/**
 * Scheduler调用该接口将请求post到ssd的任务队列中
 * version 0.1.0版本只有一个ssd的任务队列
**/
int mix_post_task_to_ssd(io_task_t* task){
    assert(task != NULL);
    int len = mix_enqueue(ssd_queue,task,1);
    return len;
}

