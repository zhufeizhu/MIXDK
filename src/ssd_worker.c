#include "ssd_worker.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "mixdk.h"
#include "ssd.h"

#define SSD_QUEUE_NUM 4
#define SSD_WORKER_NUM 4

static mix_queue_t** ssd_queue;
static atomic_bool queue_empty[SSD_QUEUE_NUM];
//设置队列任务的大小 测试发现当block的大小超过12k时 性能将不再随着block的
//大小而增加 后面需要再详细测试一下
static const int stripe_size = 4;
static atomic_int completed_ssd_write_block_num = 0;
static io_task_t ssd_task[SSD_QUEUE_NUM];
static pthread_mutex_t ssd_mutex[SSD_QUEUE_NUM];
static io_task_t post_task;

/**
 * 从ssd中读数据
 **/
static inline int mix_read_from_ssd(void* dst,
                                    size_t len,
                                    size_t offset,
                                    size_t flags) {
    return mix_ssd_read(dst, len, offset, flags);
}

/**
 * 向ssd中写数据
 **/
static inline int mix_write_to_ssd(void* src,
                                   size_t len,
                                   size_t offset,
                                   size_t flags,int idx) {
    return mix_ssd_write(src, len, offset, flags, idx);
}

atomic_int mix_get_completed_ssd_write_block_num() {
    return completed_ssd_write_block_num;
}

static inline void mix_ssd_write_block_completed(int nblock) {
    atomic_fetch_add_explicit(&completed_ssd_write_block_num,nblock,memory_order_relaxed);
}

io_task_t* get_task_from_ssd_queue(int idx) {
    int len = mix_dequeue(ssd_queue[idx], &ssd_task[idx], 1);
    if (len) {
        queue_empty[idx] = false;
        return &ssd_task[idx];
    } else {
        queue_empty[idx] = true;
        return NULL;
    }
}

atomic_int_fast32_t ssd_task_num = 0;

static void ssd_worker(void* arg) {
    int idx = *(int*)arg;
    printf("ssd worker %d init\n",idx);
    int len = 0;
    int ret = 0;
    io_task_t* task = NULL;
    while (1) {
        task = get_task_from_ssd_queue(idx);
        if (task == NULL) {
            //printf("empty task\n");
            continue;
        }
        //printf("ssd task %ld\n",ssd_task_num++);
        //printf("start %d\n",idx);
        size_t op_code = task->opcode & (MIX_READ | MIX_WRITE);

        switch (op_code) {
            case MIX_READ: {
                ret = mix_read_from_ssd(task->buf, task->len, task->offset,
                                        task->opcode);
                atomic_fetch_add_explicit(task->ret,ret,memory_order_relaxed);
                break;
            };
            case MIX_WRITE: {
                // printf("mix write %d\n",local_time);
                // local_time++;
                ret = mix_write_to_ssd(task->buf, task->len, task->offset,
                                       task->opcode,idx);
                
                //ret = task->len;
                mix_ssd_write_block_completed(ret);
                break;
            }
            default:
                ret = 0;
                break;
        }
        //printf("end %d\n",idx);
        //free(task);
    }
    return;
}

ssd_info_t* mix_ssd_worker_init(unsigned int size, unsigned int esize) {
    ssd_info_t* ssd_info = NULL;

    pthread_t pid = 0;
    if ((ssd_info = mix_ssd_init()) == NULL) {
        return NULL;
    }

    ssd_queue = malloc(sizeof(mix_queue_t*)*SSD_QUEUE_NUM);
    if(ssd_queue == NULL) {
        return NULL;
    }

    for(int i = 0; i < SSD_QUEUE_NUM; i++){
        ssd_queue[i] = mix_queue_init(size, esize);
        pthread_mutex_init(&ssd_mutex[i],NULL);
        if(ssd_queue[i] == NULL) {
            return NULL; //未做内存释放工作
        }
    }

    int *idxs = malloc(4);
    for(int i = 0; i < SSD_WORKER_NUM; i++){
        idxs[i] = i;
        if (pthread_create(&pid, NULL, (void*)ssd_worker, (void*)(idxs + i))) {
            printf("create ssd queue failed\n");
            free(ssd_info);
            return NULL;
        }
    }
    
    return ssd_info;
}

/**
 * Scheduler调用该接口将请求post到ssd的任务队列中
 * version 0.1.0版本只有一个ssd的任务队列
 **/

int mix_post_task_to_ssd(io_task_t* task, int idx) {
    while (!mix_enqueue(ssd_queue[idx], task, 1));
    return 1;
}

atomic_bool mix_ssd_queue_is_empty(int idx) {
    return queue_empty[idx];
}