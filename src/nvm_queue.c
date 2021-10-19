#include "nvm_queue.h"

#include <pthread.h>
#include <stdio.h>
#include <assert.h>

#include "nvm.h"

#define NVM_QUEUE_NUM 4
static mix_queue_t** nvm_queue;

/**
 * @param dst:读取的目标地址
 * @param len:读取的长度
 * @param offset:读取的偏移
**/
static inline size_t mix_read_from_nvm(void* dst, size_t len, size_t offset,size_t flags){
    return mix_nvm_read(dst,len,offset,flags);
}

/**
 * @param src:写入的内容
 * @param len:写入的长度
 * @param offset:写入的偏移
**/
static inline size_t mix_write_to_nvm(void* src, size_t len, size_t offset,size_t flags){
    return mix_nvm_write(src,len,offset,flags);
}

static void mix_submit_to_nvm(void* arg){
    int i = *(int*)arg;
    //printf("init nvm queue %d\n",i);
    int len = 0;
    int ret = 0;
    mix_queue_t* nvm_queue_ = nvm_queue[i];
    while(1){
        io_task_t* task = malloc(TASK_SIZE);
        len = mix_dequeue(nvm_queue_,task,1);
        if (len < 1) {
            free(task);
            continue;
        }

        size_t op_code = task->opcode & (MIX_READ | MIX_WRITE);

        switch (op_code) {
            case MIX_READ:
            {
                ret = mix_read_from_nvm(task->buf,task->len,task->offset,task->opcode);
                break;
            };
            case MIX_WRITE:
            {
                ret = mix_write_to_nvm(task->buf,task->len,task->offset,task->opcode);
                break;
            };
            default:
                break;
        }
        task->ret = ret;
        task->on_task_completed(task);
        free(task);
    }
    return;
}
int indexs[NVM_QUEUE_NUM];

int mix_init_nvm_queue(unsigned int size, unsigned int esize){
    nvm_queue = malloc(NVM_QUEUE_NUM * sizeof(mix_queue_t*));
    for(int i = 0; i < NVM_QUEUE_NUM; i++){
        nvm_queue[i] = mix_queue_init(size,esize);
    }

    if(mix_nvm_init()){
        return -1;
    }
    pthread_t pid[NVM_QUEUE_NUM];
    
    for(int i = 0; i < NVM_QUEUE_NUM; i++)  indexs[i] = i;

    for(int i = 0; i < NVM_QUEUE_NUM; i++){
        if(pthread_create(&pid[i],NULL,(void*)mix_submit_to_nvm,(void*)(indexs + i))){
            printf("create ssd queue failed\n");
            return -1;
        }
    }
    
    return 0;
}

int pre_nvm_ind = 0;

int retry_time = 0;

/**
 * task入队到nvm_queue中 nvm_queue的个数在编译器确定
 * 目前入队的策略是循环遍历
**/
int mix_post_task_to_nvm(io_task_t* task){
    int l = 0;
    while(l == 0){
        l= mix_enqueue(nvm_queue[pre_nvm_ind],task,1);
        pre_nvm_ind = (pre_nvm_ind + 1)%NVM_QUEUE_NUM;
        // if(l > 0){
        //     break;
        // }else{
        //     retry_time++;
        //     printf("nvm retry time is %d\n",retry_time);
        // }
        //printf("%d\n",pre_nvm_ind);
    }
    return l;
}