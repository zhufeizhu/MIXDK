#include "nvm_queue.h"

#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "nvm.h"
#include "mix_hash.h"


#define NVM_QUEUE_NUM 4
#define BLOCK_SZIE 4096

static mix_queue_t** nvm_queue;
_Atomic int completed_nvm_task_num = 0;

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

static inline void mix_nvm_task_completed(io_task_t* task){
    completed_nvm_task_num++;
    //if(completed_nvm_task_num > 500000) printf("completed nvm task num is %d\n",completed_nvm_task_num);
    if(task->flag == NULL) return;
    atomic_store(task->flag,true);
}

atomic_int mix_get_completed_nvm_task_num(){
    return completed_nvm_task_num;
}

static atomic_int task_num = 0;

<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
static void mix_submit_to_nvm(void* arg){
    int i = *(int*)arg;
    //printf("init nvm queue %d\n",i);
    int len = 0;
    int ret = 0;
    mix_queue_t* nvm_queue_ = nvm_queue[i];
    io_task_t* task = malloc(TASK_SIZE);
    while(1){
        len = mix_dequeue(nvm_queue_,task,1);
        if (len == 0) {
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
                if (task->redirect == 1){
                    //需要重新确定下写的位置
                    task->offset = get_next_free_segment(i,task);
                }

                ret = mix_write_to_nvm(task->buf,task->len,task->offset,task->opcode);
                break;
            };
            default:{
                ret = 0;
                break;
            }
        }
        task->ret = ret;
        mix_nvm_task_completed(task);
    }
    return;
}

int indexs[NVM_QUEUE_NUM];

nvm_info_t* mix_init_nvm_queue(unsigned int size, unsigned int esize){
    nvm_info_t* nvm_info = mix_nvm_init();
    if(nvm_info == NULL){
        return NULL;
    }

    nvm_queue = malloc(NVM_QUEUE_NUM * sizeof(mix_queue_t*));
    if(nvm_queue == NULL){
        perror("alloc memory for mix queue failed\n");
    }

    mix_hash = mix_hash_init();

    for(int i = 0; i < NVM_QUEUE_NUM; i++){
        nvm_queue[i] = mix_queue_init(size,esize);
    }
    nvm_info->queue_num = NVM_QUEUE_NUM;

    pthread_t pid[NVM_QUEUE_NUM];
    for(int i = 0; i < NVM_QUEUE_NUM; i++)  indexs[i] = i;

    for(int i = 0; i < NVM_QUEUE_NUM; i++){
        if(pthread_create(&pid[i],NULL,(void*)mix_submit_to_nvm,(void*)(indexs + i))){
            printf("create ssd queue failed\n");
            free(nvm_info);
            return NULL;
        }
    }

    return nvm_info;
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
        if(task->queue_idx < 0){
            l= mix_enqueue(nvm_queue[pre_nvm_ind],task,1);
            pre_nvm_ind = (pre_nvm_ind + 1)%NVM_QUEUE_NUM;
        }else{
            l = mix_enqueue(nvm_queue[task->queue_idx],task,1);
        }
    }
    return l;
}