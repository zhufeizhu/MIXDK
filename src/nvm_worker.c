#include "nvm_worker.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mix_meta.h"
#include "nvm.h"
#include "ssd_worker.h"

#define NVM_QUEUE_NUM 4
#define BUFFER_QUEUE_NUM 4

static const int block_size = 4096;
static const int threshold = 1;
static const int stripe_size = 4;
static mix_queue_t** nvm_queue = NULL;
static mix_queue_t** buffer_queue = NULL;
static mix_metadata_t* meta_data = NULL;

static pthread_cond_t rebuild_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex[BUFFER_QUEUE_NUM];

static io_task_t migrate_task;
static io_task_t nvm_task[NVM_QUEUE_NUM];
static io_task_t buffer_task[NVM_QUEUE_NUM];
static atomic_bool rebuild = false;


atomic_int_fast32_t completed_nvm_write_block_num = 0;

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

static inline void mix_nvm_write_block_completed(int nblock) {
    atomic_fetch_add_explicit(&completed_nvm_write_block_num,nblock,memory_order_relaxed);;
}

atomic_int mix_get_completed_nvm_write_block_num() {
    return completed_nvm_write_block_num;
}

io_task_t* get_task_from_nvm_queue(int idx) {
    int len = mix_dequeue(nvm_queue[idx], &nvm_task[idx], 1);
    if (len == 0) {
        return NULL;
    } else {
        return &nvm_task[idx];
    }
}

static atomic_int dequeue_task_num = 0;

io_task_t* get_task_from_buffer_queue(int idx) {
    //if (pthread_mutex_trylock(&mutex[idx])) {
        //return NULL;
    //}
    int len = mix_dequeue(buffer_queue[idx], &buffer_task[idx], 1);
    if (len == 0) {
        //pthread_mutex_unlock(&mutex[idx]);
        return NULL;
    } else {
        return &buffer_task[idx];
    }
}

static atomic_int task_num = 0;

//根据逻辑地址从ssd中读取数据 这是一个阻塞的动作
static size_t redirect_read(io_task_t* task, int idx) {
    // 检查每一个块 判断是否在buffer中
    size_t ret = 0;
    io_task_t read_task;
    read_task.offset = -1;  //一个很大的整数
    read_task.len = 0;
    read_task.opcode = MIX_READ;
    int offset = 0;
    for (int i = 0; i < task->len; i++) {
        if ((offset = mix_buffer_block_test(meta_data, task->offset + i, idx)) == -1) {
            if (read_task.offset == -1) {
                read_task.offset = task->offset + i;
                read_task.len = 1;
                read_task.buf = task->buf + BLOCK_SIZE * (read_task.offset - task->offset);
                read_task.ret = task->ret;
            } else {
                read_task.len++;
            }
        } else {
            if (read_task.len > 0)
                mix_post_task_to_ssd(&read_task, idx);
            ret += mix_read_from_buffer(task->buf + i * BLOCK_SIZE, offset + idx * meta_data->per_block_num,
                                        task->opcode);
            read_task.offset = -1;
            read_task.len = 0;
        }
    }
    if (read_task.offset != -1) {
        mix_post_task_to_ssd(&read_task, idx);
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

size_t meta_time = 0;
size_t data_time = 0;
static int redirect_write(io_task_t* task, int idx) {
    // idx = 0;
    //将所有要写的ssd的数据都删除 然后将任务转发到ssd中
    if (task->len <= threshold) {
        int offset = mix_buffer_block_test(meta_data, task->offset, idx);
        if (offset == -1) {
            //如果不在buffer中
            offset = mix_get_next_free_block(meta_data, idx);
            if (offset == -1) {
                printf("id is %lld\n",task->offset);
                struct timespec start,end;
                clock_gettime(CLOCK_MONOTONIC_RAW, &start);
                //printf("[migrate begin %d]\n", idx);
                mix_migrate_segment(idx);
                //printf("[migrate end %d]\n", idx);
                offset = mix_get_next_free_block(meta_data, idx);
                clock_gettime(CLOCK_MONOTONIC_RAW, &end);
                printf("migrate %d segment time3 is %lu us\n", idx,(end.tv_sec - start.tv_sec) * 1000000 +
                    (end.tv_nsec - start.tv_nsec) / 1000);
            }
            mix_write_redirect_blockmeta(meta_data, idx, task->offset, offset);
        }
        mix_write_to_buffer(task->buf, task->offset, idx * meta_data->per_block_num + offset,task->opcode);
        return task->len;
    } else {
        mix_post_task_to_ssd(task, idx);
        mix_clear_block(meta_data, task, idx);
        return 0;
    }
}
static struct timespec start1, start2, end;
/**
 * @brief 将满了的segment中的数据迁移到对应的ssd中
 * 在正式开始迁移前需要保证buffer_queue中的task都执行完成
 *
 * @param idx 需要执行迁移的segment的下标
 */
void mix_migrate_segment(int idx) {
    int segment_idx = idx;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start1);
    while(!mix_ssd_queue_is_empty(idx));
    clock_gettime(CLOCK_MONOTONIC_RAW, &start2);
    pthread_mutex_lock(meta_data->migrate_mutex);
    mix_segment_migration_begin(meta_data, segment_idx);
    //等待ssd_queue中的所有任务执行完毕
    //mix_migrate(meta_data, segment_idx);
    mix_segment_migration_end(meta_data, segment_idx);
    pthread_mutex_unlock(meta_data->migrate_mutex);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("migrate %d segment time1 is %lu us\n", idx,(end.tv_sec - start1.tv_sec) * 1000000 +
                                   (end.tv_nsec - start1.tv_nsec) / 1000);
    printf("migrate %d segment time2 is %lu us\n", idx,(end.tv_sec - start2.tv_sec) * 1000000 +
    (end.tv_nsec - start2.tv_nsec) / 1000);
}
static atomic_int nvm_task_num = 0;
// atomic_int task_num = 0;



char* buf = NULL;

static atomic_int_fast8_t rebuild_num = 0;

static void nvm_worker(void* arg) {
    int idx = *(int*)arg;  //当前线程对应的队列序号

    printf("[nvm worker %d init]\n", idx);
    pthread_mutex_lock(&mutex[idx]);
    while(!rebuild) pthread_cond_wait(&rebuild_cond,&mutex[idx]);
    if(mix_buffer_rebuild(meta_data,idx)){
        rebuild_num++;
    }
    pthread_mutex_unlock(&mutex[idx]);

    int len = 0;
    int ret = 0;
    uint8_t seq = 0;
    uint8_t mask = 0x01;
    io_task_t* task = NULL;
    while (1) {
        if (seq & mask)
            task = get_task_from_nvm_queue(idx);
        else {
            task = get_task_from_buffer_queue(idx);
        }

        seq++;
        if (task == NULL) {
            continue;
        }

        size_t op_code = task->opcode & (MIX_READ | MIX_WRITE | MIX_CLEAR);
        if (task->type == NVM_TASK) {
            switch (op_code) {
                case MIX_READ: {
                    ret = mix_read_from_nvm(task->buf, task->len, task->offset,
                                            task->opcode);
                    atomic_fetch_add_explicit(task->ret,ret,memory_order_relaxed);
                    break;
                };
                case MIX_WRITE: {
                    ret = mix_write_to_nvm(task->buf, task->len, task->offset,
                                           task->opcode);
                    mix_nvm_write_block_completed(ret);
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
                    atomic_fetch_add_explicit(task->ret,ret,memory_order_relaxed);
                    break;
                }
                case MIX_WRITE: {
                    ret = redirect_write(task, idx);
                    mix_nvm_write_block_completed(ret);
                    break;
                }
                default: {
                    ret = 0;
                    break;
                }
            }
        }
    }
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
        return NULL;
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

    buffer_queue = malloc(sizeof(mix_queue_t*) * BUFFER_QUEUE_NUM);
    for (int i = 0; i < BUFFER_QUEUE_NUM; i++) {
        pthread_mutex_init(&mutex[i], NULL);
        buffer_queue[i] = mix_queue_init(size, esize);
        if (buffer_queue == NULL) {
            mix_metadata_free(meta_data);
            return NULL;
        }
    }

    return buffer_info;
}

inline void mix_nvm_mmap(nvm_info_t* nvm_info, buffer_info_t* buffer_info) {
    mix_mmap(nvm_info, buffer_info);
}

int pre_nvm_ind = 0;

static atomic_int retry_time = 0;

/**
 * task入队到nvm_queue中 nvm_queue的个数在编译器确定
 * 目前入队的策略是循环遍历
 **/
int mix_post_task_to_nvm(io_task_t* task) {
    // printf("post task to nvm %lld\n",task->offset);
    while(!mix_enqueue(nvm_queue[task->queue_idx], task, 1));
    return 1;
}

static atomic_int buffer_task_num = 0;

int mix_post_task_to_buffer(io_task_t* task) {
    int offset = task->offset;
    int end = task->offset + task->len;
    int idx = (task->offset / stripe_size) % BUFFER_QUEUE_NUM;
    int len = 0;
    io_task_t post_task;
    //mix_nvm_write_block_completed(task->len);


    while (offset < end) {
        post_task.offset = offset;
        len = stripe_size - (post_task.offset % stripe_size);
        post_task.len = (len > (end - post_task.offset)) ? (end - offset) : len;
        offset += post_task.len;
        post_task.buf = task->buf + (post_task.offset - task->offset) * SSD_BLOCK_SIZE;
        post_task.opcode = task->opcode;
        post_task.type = task->type;
        post_task.ret = task->ret;
        while (!mix_enqueue(buffer_queue[idx], &post_task, 1));
        
        // pthread_mutex_unlock(&ssd_mutex[idx]);
        idx = (idx + 1) % BUFFER_QUEUE_NUM;
    }
}


void mix_rebuild(){
    for(int i = 0; i < SEGMENT_NUM; i++){
        mix_segment_clear(meta_data,i);
    }
    rebuild = true;
    pthread_cond_broadcast(&rebuild_cond);
    while(rebuild_num < SEGMENT_NUM);
}