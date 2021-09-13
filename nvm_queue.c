#include "nvm_queue.h"

#include "mock_nvm.h"

static mix_queue_t* nvm_queue;

/**
 * @param dst:读取的目标地址
 * @param len:读取的长度
 * @param offset:读取的偏移
**/
static inline int mix_read_from_nvm(void* dst, unsigned int len, unsigned int offset){
    return mix_mock_nvm_read(dst,len,offset);
}

/**
 * @param src:写入的内容
 * @param len:写入的长度
 * @param offset:写入的偏移
**/
static inline int mix_write_to_nvm(void* src, unsigned int len, unsigned int offset){
    return mix_mock_nvm_write(src,len,offset);
}

static void* mix_submit_to_nvm(void* arg){
    int len = 0;
    int ret = 0;
    while(1){
        io_task_t* task = malloc(TASK_SIZE);
        len = mix_dequeue(nvm_queue,task,1);
        if (len < 1) {
            free(task);
            continue;
        }

        switch (task->opcode) {
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
        if (ret) {
            task->on_task_succeed(task->task_index);
        } else {
            task->on_task_failed(task->task_index);
        }
    }
}

int mix_init_nvm_queue(unsigned int size, int esize){
    nvm_queue = mix_queue_init(size,esize);
    pthread_t pid;
    if(pthread_create(&pid,NULL,mix_submit_to_nvm,NULL)){
        printf("create ssd queue failed\n");
        return -1;
    }

    pthread_join(pid,NULL);
    return 0;
}