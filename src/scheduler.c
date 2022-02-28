#include "scheduler.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mix_log.h"
#include "mix_meta.h"
#include "mix_task.h"
#include "mixdk.h"
#include "nvm_worker.h"
#include "ssd_worker.h"

//数据定义区
static scheduler_ctx_t* sched_ctx = NULL;
static const size_t threshold = 4096;

static io_task_t* p_task;

int mix_wait_for_task_completed(atomic_bool* flag) {
    while (atomic_load(flag) == false) {
    };
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

size_t get_completed_task_num() {
    return mix_get_completed_nvm_task_num() + mix_get_completed_ssd_task_num();
}

static atomic_int task_num = 0;
static atomic_int retry_time = 0;

int mix_post_task_to_io(io_task_t* task) {
    int len = 0;
    while (len == 0) {
        len = mix_enqueue(sched_ctx->submit_queue, task, 1);
    }

    return len;
}

/**
 * @brief 根据传入的task 将其拆分成多个task(io_task_v)
 * 需要将其中的每个task进行处理
 *
 * @param task 用户传入的io操作
 * @return io_task_t** 对用户传入的io操作的拆分 针对跨nvm和ssd的数据
 */
static inline io_task_t* handle_task(io_task_t* task) {
    //当前task的offset都在nvm的范围内
    if ((size_t)(task->offset + task->len) < sched_ctx->nvm_info->block_num) {
        // printf("nvm task\n");
        // 还未考虑跨区的问题
        int queue_idx1 = (task->offset / 2 )% 4;
        int queue_idx2 =
            ((task->offset + task->len) / 2 )%  4;
        if (queue_idx1 == queue_idx1) {
            task->queue_idx = queue_idx1;
            task->type = NVM_TASK;
            return NULL;
        } else {
            task->queue_idx = queue_idx1;
            task->type = NVM_TASK;
            task->len =
                task->len - (task->offset % sched_ctx->nvm_info->per_block_num);

            p_task->queue_idx = queue_idx2;
            p_task->type = NVM_TASK;
            p_task->len =
                (task->len + task->offset) % sched_ctx->nvm_info->per_block_num;
            p_task->buf = task->buf + p_task->len * BLOCK_SIZE;
            return p_task;
        }
    }

    //当前task的offset都在ssd的范围内
    if ((size_t)task->offset >= sched_ctx->nvm_info->block_num) {
        task->offset -= sched_ctx->nvm_info->block_num;
        task->type = SSD_TASK;
        return NULL;
    }

    //当前task跨过了nvm和ssd两个区域 基本上不会存在
    if ((size_t)task->offset < sched_ctx->nvm_info->block_num &&
        (size_t)(task->offset + task->len) > sched_ctx->nvm_info->block_num) {
        size_t len1 = sched_ctx->nvm_info->block_num - (size_t)task->offset;
        size_t len2 =
            (size_t)(task->offset + task->len) - sched_ctx->nvm_info->block_num;

        task->len = len1;
        task->type = NVM_TASK;

        p_task->len = len2;
        p_task->offset = task->offset + len1;
        p_task->type = SSD_TASK;
        p_task->buf = task->buf + len1 * BLOCK_SIZE;
        return p_task;
    }

    // printf("[task offset]:%llu\n [nvm_size]:%llu\n
    // [ssd_size]:%llu\n",(size_t)task->offset,nvm_size,ssd_size);
}

/**
 * @brief 将任务按照其类型进行转发 将nvm的任务发送nvm的queue中
 * 将ssd的任务发送到buffer的queue中
 *        后续需要在buffer中对ssd任务进行判断是否需要转发到ssd的queue中
 *
 * @param task
 */
void do_schedule(io_task_t* task) {
    switch (task->type) {
        case NVM_TASK: {
            // printf("post task num is %d\n",task_num++);
            mix_post_task_to_nvm(task);
            break;
        };
        case SSD_TASK: {
            mix_post_task_to_buffer(
                task);  // ssd的任务需要经过buffer进行处理之后执行
            break;
        };
        default: {
            // printf("post error\n");
            break;
        }
    }
}

static void scheduler(void* arg) {
    int len = 0;
    int i = 0;
    io_task_t* io_task = malloc(TASK_SIZE);
    while (1) {
        // pthread_spin_lock(sched_ctx->schedule_queue_lock);
        len = mix_dequeue(sched_ctx->submit_queue, io_task, 1);
        // pthread_spin_unlock(sched_ctx->schedule_queue_lock);
        if (len == 0) {
            //printf("empty\n");
            continue;
        }
        io_task_t* new_task = handle_task(io_task);

        do_schedule(io_task);

        if (new_task) {
            do_schedule(new_task);
        }
    }
}

/**
 * @brief scheduler初始化函数 包括
 *
 * @param size
 * @param esize
 * @param max_current
 * @return int
 */
int mix_init_scheduler(unsigned int size, unsigned int esize, int max_current) {
    pthread_t scheduler_thread = 0;
    ssd_info_t* ssd_info = NULL;
    nvm_info_t* nvm_info = NULL;
    buffer_info_t* buffer_info = NULL;

    sched_ctx = malloc(sizeof(scheduler_ctx_t));
    if (sched_ctx == NULL) {
        mix_log("mix_init_scheduler", "malloc for sched_ctx failed");
        return -1;
    }

    sched_ctx->max_current = max_current;
    sched_ctx->submit_queue = mix_queue_init(size, esize);
    if (sched_ctx->submit_queue == NULL) {
        mix_log("mix_init_scheduler", "mix queue init failed");
        return -1;
    }

    // sched_ctx->completation_conds =
    //     malloc(sizeof(pthread_cond_t*) * max_current);
    // if (sched_ctx->completation_conds == NULL) {
    //     return -1;
    // }
    // for (int i = 0; i < max_current; i++) {
    //     sched_ctx->completation_conds[i] = malloc(sizeof(pthread_cond_t));
    //     if (sched_ctx->completation_conds[i] == NULL) {
    //         return -1;
    //     }
    //     pthread_cond_init(sched_ctx->completation_conds[i], NULL);
    // }

    // sched_ctx->ctx_mutex = malloc(sizeof(pthread_mutex_t*) * max_current);
    // if (sched_ctx->ctx_mutex == NULL) {
    //     return -1;
    // }
    // for (int i = 0; i < max_current; i++) {
    //     sched_ctx->ctx_mutex[i] = malloc(sizeof(pthread_mutex_t));
    //     if (sched_ctx->ctx_mutex[i] == NULL) {
    //         return -1;
    //     }
    //     pthread_mutex_init(sched_ctx->ctx_mutex[i], NULL);
    // }

    // sched_ctx->schedule_queue_lock = malloc(sizeof(pthread_mutex_t));
    // if (sched_ctx->schedule_queue_lock == NULL) {
    //     return -1;
    // }
    // pthread_spin_init(sched_ctx->schedule_queue_lock, 1);
    ssd_info = mix_ssd_worker_init(size, esize);
    if (ssd_info == NULL) {
        return -1;
    }
    buffer_info = mix_buffer_worker_init(size, esize);
    if (buffer_info == NULL) {
        return -1;
    }
    nvm_info = mix_nvm_worker_init(size, esize);
    if (nvm_info == NULL) {
        return -1;
    }

    mix_nvm_mmap(nvm_info, buffer_info);

    sched_ctx->ssd_info = ssd_info;
    sched_ctx->nvm_info = nvm_info;
    sched_ctx->buffer_info = buffer_info;

    if (pthread_create(&scheduler_thread, NULL, (void*)scheduler, NULL)) {
        printf("create scheduler failed\n");
        return -1;
    }
    return 0;
}