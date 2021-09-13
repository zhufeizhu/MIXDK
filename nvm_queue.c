#include "nvm_queue.h"

static mix_queue_t* nvm_queue;

/**
 * @param task:完成的任务
 * 当前任务完成，写回对应的完成队列中
**/
static void mix_task_completed(io_task_t* task){
    return;
}

/**
 * @param dst:读取的目标地址
 * @param len:读取的长度
 * @param offset:读取的偏移
**/
static inline int mix_read_from_nvm(void* dst, unsigned int len, unsigned int offset){

}

/**
 * @param src:写入的内容
 * @param len:写入的长度
 * @param offset:写入的偏移
**/
static inline int mix_write_to_nvm(void* src, unsigned int len, unsigned int offset){
    
}

static void* mix_submit_to_nvm(void* arg){
    int len = 0;
    int ret = 0;
    while(1){
        io_task_t* task = malloc(TASK_SIZE);
        len = mix_dequeue(nvm_queue,task,1);
        if(len < 1){
            free(task);
            continue;
        }

        switch(task->opcode){
            case MIX_READ:{
                ret = mix_read_from_nvm(task->buf,task->len,task->offset);
                break;
            };
            case MIX_WRITE:{
                ret = mix_write_to_nvm(task->buf,task->len,task->offset);
                break;
            };
            default:
                break;
        }

        mix_task_completed(task);
    }
}

int mix_init_nvm_queue(unsigned int size, int esize){
    
}