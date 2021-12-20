#include "nvm_worker.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "mix_meta.h"
#include "nvm.h"
#include "ssd_worker.h"

#define NVM_QUEUE_NUM 4
#define BLOCK_SZIE 4096

static const int threshold = 4096;
static mix_queue_t** nvm_queue = NULL;
static mix_queue_t* buffer_queue = NULL;
static io_task_t migrate_task;
static io_task_t nvm_task;
static io_task_t buffer_task;

_Atomic int completed_nvm_task_num = 0;

/**
 * @param dst:读取的目标地址
 * @param len:读取的长度
 * @param offset:读取的偏移
 **/
static inline size_t mix_read_from_nvm(void* dst,
                                       size_t len,
                                       size_t offset,
                                       size_t flags) {
    return mix_nvm_read(dst, len, offset, flags);
}

static inline size_t mix_read_from_buffer(void* dst,
                                          size_t src_block,
                                          size_t flags) {
    int dst_block = mix_buffer_block_test(src_block);
    if (dst_block == -1) {
        return 0;
    }
    return mix_buffer_read(dst, src_block, dst_block, flags);
}

/**
 * @param src:写入的内容
 * @param len:写入的长度
 * @param offset:写入的偏移
 **/
static inline size_t mix_write_to_nvm(void* src,
                                      size_t len,
                                      size_t offset,
                                      size_t flags) {
    return mix_nvm_write(src, len, offset, flags);
}

static inline size_t mix_write_to_buffer(void* dst,
                                         size_t src_block,
                                         size_t dst_block,
                                         size_t flags) {
    return mix_buffer_write(dst, src_block, dst_block, flags);
}

static inline void mix_nvm_task_completed(io_task_t* task) {
    completed_nvm_task_num++;
    // if(completed_nvm_task_num > 500000) printf("completed nvm task num is
    // %d\n",completed_nvm_task_num);
    if (task->flag == NULL)
        return;
    atomic_store(task->flag, true);
}

atomic_int mix_get_completed_nvm_task_num() {
    return completed_nvm_task_num;
}

inline io_task_t* get_task_from_nvm_queue(int idx) {
    int len = mix_dequeue(nvm_queue[idx], &nvm_task, 1);
    if (len == 0) {
        return NULL;
    } else {
        return &nvm_task;
    }
}

inline io_task_t* get_task_from_buffer_queue(int idx) {
    while ()
        int len = mix_dequeue(buffer_queue, &buffer_task, 1);
    if ()
}

void mix_migrate_segment(int idx) {
    //将buffer_queue中的所有任务执行完毕
    mix_segment_migration_begin(idx);
    while (mix_dequeue(buffer_queue, &migrate_task, 1)) {
        if (migrate_task.type == CLEAR_TASK) {
            mix_clear_blocks(&migrate_task);
        } else {
            size_t op_code = migrate_task.opcode & (MIX_READ | MIX_WRITE);

            if (op_code == MIX_READ) {
                mix_read_from_buffer(migrate_task.buf, migrate_task.offset,
                                     migrate_task.flag);
            } else if (op_code == MIX_WRITE) {
                mix_write_to_buffer(migrate_task.buf, migrate_task.offset,
                                    migrate_task.flag);
            }
        }
        mix_buffer_task_completed(&migrate_task);
    }
    mix_segment_migration_end(idx);
}

static atomic_int task_num = 0;

static void nvm_worker(void* arg) {
    int idx = *(int*)arg;  //当前线程对应的队列序号
    int len = 0;
    int ret = 0;
    io_task_t* task = NULL;
    while (1) {
        task = get_task_from_nvm_queue(idx);
        if (task == NULL)
            continue;

        size_t op_code = task->opcode & (MIX_READ | MIX_WRITE);
        if (task->type == NVM_TASK) {
            switch (op_code) {
                case MIX_READ: {
                    ret = mix_read_from_nvm(task->buf, task->len, task->offset,
                                            task->opcode);
                    break;
                };
                case MIX_WRITE: {
                    ret - mix_write_to_nvm(task->buf, task->len, task->offset,
                                           task->opcode);
                    break;
                }
                default: {
                    ret = 0;
                    break;
                }
            }
        } else if (task->type == SSD_TASK) {
            switch (op_code) {
                case MIX_READ: {
                }
                case MIX_WRITE: {
                    //将所有要写的ssd的数据都删除 然后将任务转发到ssd中
                    if (task->len <= threshold) {
                        int offset = mix_buffer_block_test(task->offset);
                        if (offset == -1) {
                            //如果不在buffer中
                            offset = mix_get_next_free_block(idx);
                            if (offset == -1) {
                                //表明当前segment已经满了
                                //需要将当前segment进行迁移
                                mix_segment_migration_begin(idx);
                                mix_migrate_segment(idx);
                                mix_segment_migration_end(idx);
                            }
                        }
                    } else {
                        //将当前task中包含的元数据都清空
                        //然后将数据异步的转发到ssd的queue中
                        mix_post_task_to_ssd(task);
                        mix_clear_blocks(task);
                    }
                }
                default: {
                    ret = 0;
                    break;
                }
            }
        }

        if (task->type == CLEAR_TASK) {
            //先清空元数据 再将请求转发到ssd中
            mix_clear_blocks(&migrate_task);
            mix_post_task_to_ssd(&migrate_task);
        }

        if (task->type)

            size_t op_code = task->opcode & (MIX_READ | MIX_WRITE);
        switch (op_code) {
            case MIX_READ: {
                if (task->redirect == 1) {
                    ret = mix_read_from_buffer(task->buf, task->offset,
                                               task->opcode);
                    if (!ret) {
                        //表明当前block不在buffer 需要转发到ssd
                        mix_post_task_to_ssd(task);
                    }
                } else {
                    ret = mix_read_from_nvm(task->buf, task->len, task->offset,
                                            task->opcode);
                }
                break;
            };
            case MIX_WRITE: {
                if (task->redirect == 1) {
                    // 写buffer
                    uint32_t bit = mix_get_next_free_block(idx);
                    ret = mix_write_to_buffer(task->buf, task->len, bit,
                                              task->opcode);
                    if (ret) {
                        if (mix_write_redirect_block(idx, task->offset, bit)) {
                            //表明当前已经满了 此时需要执行迁移操作
                            mix_migrate_segment(idx);
                        }
                    }
                } else {
                    //写nvm
                    ret = mix_write_to_nvm(task->buf, task->len, task->offset,
                                           task->opcode);
                }
                break;
            };
            default: {
                ret = 0;
                break;
            }
                task->ret = ret;
                mix_nvm_task_completed(task);
        }
    }
    // impossible run here
    free(task);
    return;
}

int indexs[NVM_QUEUE_NUM];

nvm_info_t* mix_init_nvm_worker(unsigned int size, unsigned int esize) {
    nvm_info_t* nvm_info = NULL;
    pthread_t pid[NVM_QUEUE_NUM];

    if ((nvm_info = mix_nvm_init()) == NULL) {
        return NULL;
    }

    nvm_queue = malloc(NVM_QUEUE_NUM * sizeof(mix_queue_t*));
    if (nvm_queue == NULL) {
        perror("alloc memory for mix queue failed\n");
    }

    for (int i = 0; i < NVM_QUEUE_NUM; i++) {
        nvm_queue[i] = mix_queue_init(size, esize);
        if (nvm_queue[i] == NULL) {
            return NULL;
        }
    }

    for (int i = 0; i < NVM_QUEUE_NUM; i++)
        indexs[i] = i;

    for (int i = 0; i < NVM_QUEUE_NUM; i++) {
        if (pthread_create(&pid[i], NULL, (void*)nvm_worker,
                           (void*)(indexs + i))) {
            printf("create ssd queue failed\n");
            free(nvm_info);
            return NULL;
        }
    }

    return nvm_info;
}

buffer_info_t* mix_init_buffer(unsigned int size, unsigned int esize) {
    buffer_info_t* buffer_info = NULL;
    if ((buffer_info = mix_buffer_init()) == NULL) {
        return NULL;
    }

    mix_init_metadata(buffer_info->block_num);

    buffer_queue = mix_queue_init(size, esize);
    if (buffer_queue == NULL) {
        mix_free_metadata();
        return NULL;
    }

    return buffer_info;
}

int pre_nvm_ind = 0;

int retry_time = 0;

/**
 * task入队到nvm_queue中 nvm_queue的个数在编译器确定
 * 目前入队的策略是循环遍历
 **/
int mix_post_task_to_nvm(io_task_t* task) {
    int l = 0;
    while (l == 0) {
        if (task->queue_idx < 0) {
            if (mix_has_free_block(pre_nvm_ind)) {
                l = mix_enqueue(nvm_queue[pre_nvm_ind], task, 1);
            } else {
                l = 0;
            }
            pre_nvm_ind = (pre_nvm_ind + 1) % NVM_QUEUE_NUM;
        } else {
            l = mix_enqueue(nvm_queue[task->queue_idx], task, 1);
        }
    }
    return l;
}

int mix_post_task_to_buffer(io_task_t* task) {
    int len = 0;
    while (!len) {
        len = mix_enqueue(buffer_queue, task, 1);
    }
}
