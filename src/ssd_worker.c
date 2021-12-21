#include "ssd_worker.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "mixdk.h"
#include "ssd.h"

static mix_queue_t* ssd_queue;
static atomic_bool queue_empty;
static atomic_int completed_ssd_task_num = 0;
static io_task_t ssd_task;

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
                                   size_t flags) {
    return mix_ssd_write(src, len, offset, flags);
}

atomic_int mix_get_completed_ssd_task_num() {
    return completed_ssd_task_num;
}

static inline void mix_ssd_task_completed(io_task_t* task) {
    completed_ssd_task_num++;
    if (task->flag == NULL)
        return;
    atomic_store(task->flag, true);
}

io_task_t* get_task_from_ssd_queue() {
    int len = mix_dequeue(ssd_queue, &ssd_task, 1);
    if (len) {
        queue_empty = true;
    } else {
        queue_empty = false;
    }
}

static void ssd_worker(void* arg) {
    int len = 0;
    int ret = 0;
    io_task_t* task = NULL;
    while (1) {
        len = mix_dequeue(ssd_queue, task, 1);
        if (!len) {
            continue;
        }

        size_t op_code = task->opcode & (MIX_READ | MIX_WRITE);

        switch (op_code) {
            case MIX_READ: {
                ret = mix_read_from_ssd(task->buf, task->len, task->offset,
                                        task->opcode);
                break;
            };
            case MIX_WRITE: {
                // printf("mix write %d\n",local_time);
                // local_time++;
                ret = mix_write_to_ssd(task->buf, task->len, task->offset,
                                       task->opcode);
                break;
            }
            default:
                ret = 0;
                break;
        }
        task->ret = ret;
        mix_ssd_task_completed(task);
        free(task);
    }
    return;
}

ssd_info_t* mix_ssd_worker_init(unsigned int size, unsigned int esize) {
    ssd_info_t* ssd_info = NULL;

    pthread_t pid = 0;
    if ((ssd_info = mix_ssd_init()) == NULL) {
        return NULL;
    }

    ssd_queue = mix_queue_init(size, esize);

    if (pthread_create(&pid, NULL, (void*)ssd_worker, NULL)) {
        printf("create ssd queue failed\n");
        free(ssd_info);
        return NULL;
    }
    return ssd_info;
}

/**
 * Scheduler调用该接口将请求post到ssd的任务队列中
 * version 0.1.0版本只有一个ssd的任务队列
 **/

int mix_post_task_to_ssd(io_task_t* task) {
    int len = 0;
    while (len == 0) {
        len = mix_enqueue(ssd_queue, task, 1);
    }
    return len;
}

atomic_bool mix_ssd_queue_is_empty() {
    return queue_empty;
}