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
static mix_metadata_t* meta_data = NULL;

static io_task_t migrate_task;
static io_task_t nvm_task;
static io_task_t buffer_task;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

_Atomic int completed_nvm_task_num = 0;

void mix_migrate_segment(int idx);

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
                                          size_t dst_block,
                                          size_t flags) {
    return mix_buffer_read(dst, dst_block, flags);
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

io_task_t* get_task_from_nvm_queue(int idx) {
    int len = mix_dequeue(nvm_queue[idx], &nvm_task, 1);
    if (len == 0) {
        return NULL;
    } else {
        return &nvm_task;
    }
}

io_task_t* get_task_from_buffer_queue(int idx) {
    while (mix_segment_migrating(meta_data, idx))
        ;

    pthread_mutex_lock(&mutex);
    int len = mix_dequeue(buffer_queue, &buffer_task, 1);
    pthread_mutex_unlock(&mutex);
    if (len == 0) {
        return NULL;
    } else {
        return &buffer_task;
    }
}

static atomic_int task_num = 0;

static size_t redirect_read(io_task_t* task, int idx) {
    // 检查每一个块 判断是否在buffer中
    size_t ret;
    io_task_t read_task;
    read_task.offset = -1;  //一个很大的整数
    read_task.len = 0;
    read_task.opcode = MIX_READ;
    read_task.flag = task->flag;
    int offset = 0;
    for (int i = 0; i < task->len; i++) {
        if ((offset = mix_buffer_block_test(meta_data, task->offset + i))) {
            if (read_task.offset == -1) {
                read_task.offset = task->offset + i;
                read_task.len = 1;
                read_task.buf = task->buf + BLOCK_SIZE * read_task.offset;
            } else {
                read_task.len++;
            }
        } else {
            mix_post_task_to_ssd(&read_task);
            ret += mix_read_from_buffer(task->buf + i * BLOCK_SIZE, offset,
                                        task->opcode);
            read_task.offset = -1;
            read_task.len = 0;
        }
    }
    if (read_task.offset != -1) {
        mix_post_task_to_ssd(&read_task);
    }
    return ret;
}

/**
 * @brief 执行ssd重定向写
 * 小写: 将内容写到buffer中 并在元数据中添加对应的信息
 * 大写: 将任务转发到ssd_queue中 并清楚元数据中对应的信息
 *
 * @param task 要执行重定向写的task
 * @param idx you
 * @return int
 */
static int redirect_write(io_task_t* task, int idx) {
    //将所有要写的ssd的数据都删除 然后将任务转发到ssd中
    if (task->len <= threshold) {
        int offset = mix_buffer_block_test(meta_data, task->offset);
        if (offset == -1) {
            //如果不在buffer中
            offset = mix_get_next_free_block(meta_data, idx);
            if (offset == -1) {
                //表明当前segment已经满了
                //需要将当前segment进行迁移
                mix_migrate_segment(idx);

                //迁移后重新获取重定向的地址
                offset = mix_get_next_free_block(meta_data, idx);
            }

            mix_write_to_buffer(task->buf, task->offset, offset, task->opcode);
        }
    } else {
        //将当前task中包含的元数据都清空
        //然后将数据异步的转发到ssd的queue中
        mix_post_task_to_ssd(task);
        mix_clear_blocks(meta_data, task);
    }
}

/**
 * @brief 将满了的segment中的数据迁移到对应的ssd中
 * 在正式开始迁移前需要保证buffer_queue中的task都执行完成
 *
 * @param idx 需要执行迁移的segment的下标
 */
void mix_migrate_segment(int idx) {
    int segment_idx = idx;

    mix_segment_migration_begin(meta_data, segment_idx);
    //将buffer_queue中的所有任务执行完毕
    while (mix_dequeue(buffer_queue, &migrate_task, 1)) {
        size_t op_code = migrate_task.opcode & (MIX_READ | MIX_WRITE);
        if (op_code == MIX_READ) {
            redirect_read(&migrate_task, idx);
        } else if (op_code == MIX_WRITE) {
            while (!mix_has_free_block(meta_data, (idx++) % SEGMENT_NUM))
                ;
            redirect_write(&migrate_task, idx);
        }
    }

    //等待ssd_queue中的所有任务执行完毕
    while (!mix_ssd_queue_is_empty())
        ;
    mix_migrate(meta_data, segment_idx);
    mix_segment_migration_end(meta_data, segment_idx);
}

static void nvm_worker(void* arg) {
    int idx = *(int*)arg;  //当前线程对应的队列序号
    int len = 0;
    int ret = 0;
    uint8_t seq = 0;
    uint8_t mask = 0x01;
    io_task_t* task = NULL;
    while (1) {
        if (seq & mask)
            task = get_task_from_nvm_queue(idx);
        else
            task = get_task_from_buffer_queue(idx);

        seq++;
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
                    ret = mix_write_to_nvm(task->buf, task->len, task->offset,
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
                    ret = redirect_read(task, idx);
                    break;
                }
                case MIX_WRITE: {
                    ret = redirect_write(task, idx);
                    break;
                }
                default: {
                    ret = 0;
                    break;
                }
            }
        }
        task->ret += ret;
        mix_nvm_task_completed(task);
    }
    // impossible run here
    free(task);
    return;
}

int indexs[NVM_QUEUE_NUM];

nvm_info_t* mix_nvm_worker_init(unsigned int size, unsigned int esize) {
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

buffer_info_t* mix_buffer_worker_init(unsigned int size, unsigned int esize) {
    buffer_info_t* buffer_info = NULL;
    if ((buffer_info = mix_buffer_init()) == NULL) {
        return NULL;
    }

    meta_data = mix_metadata_init(buffer_info->block_num);
    if (meta_data == NULL) {
    }

    buffer_queue = mix_queue_init(size, esize);
    if (buffer_queue == NULL) {
        mix_metadata_free(meta_data);
        return NULL;
    }

    return buffer_info;
}

inline void mix_nvm_mmap(nvm_info_t* nvm_info, buffer_info_t* buffer_info) {
    mix_mmap(nvm_info, buffer_info);
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
        if (task->queue_idx > 0) {
            l = mix_enqueue(nvm_queue[task->queue_idx], task, 1);
        } else {
            l = mix_enqueue(nvm_queue[pre_nvm_ind], task, 1);
            pre_nvm_ind = (pre_nvm_ind + 1) % NVM_QUEUE_NUM;
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
