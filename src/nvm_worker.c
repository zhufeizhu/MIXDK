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
#define BLOCK_SZIE 4096

static const int threshold = 1;
static mix_queue_t** nvm_queue = NULL;
static mix_queue_t** buffer_queue = NULL;
static mix_metadata_t* meta_data = NULL;

static io_task_t migrate_task;
static io_task_t nvm_task[NVM_QUEUE_NUM];
static io_task_t buffer_task[NVM_QUEUE_NUM];

pthread_mutex_t mutex[BUFFER_QUEUE_NUM];

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
    if(task->len == task->ret){
        completed_nvm_task_num++;
        return;
    }
    
    // if(completed_nvm_task_num > 500000) printf("completed nvm task num is
    // %d\n",completed_nvm_task_num);
    //if (task->flag == NULL)
    //return;
    //atomic_store(task->flag, true);
}

atomic_int mix_get_completed_nvm_task_num() {
    return completed_nvm_task_num;
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
    if(pthread_mutex_trylock(&mutex[idx])){
        return NULL;
    }
    int len = mix_dequeue(buffer_queue[idx], &buffer_task[idx], 1);
    if (len == 0) {
        pthread_mutex_unlock(&mutex[idx]);
        return NULL;
    } else {
        return &buffer_task[idx];
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
        if ((offset = mix_buffer_block_test(meta_data, task->offset + i,idx))) {
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

size_t meta_time = 0;
size_t data_time = 0;
static int redirect_write(io_task_t* task, int idx) {
    //idx = 0;
    //将所有要写的ssd的数据都删除 然后将任务转发到ssd中
    if (task->len <= threshold) {
        //printf("post task to buffer\n");
        //struct timespec start, end;
        //clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        int offset = mix_buffer_block_test(meta_data, task->offset,idx);
        if (offset == -1) {
            //如果不在buffer中
            offset = mix_get_next_free_block(meta_data, idx);
            if (offset == -1) {
                //表明当前segment已经满了
                //需要将当前segment进行迁移
                printf("migrate begin %d\n",idx);
                mix_migrate_segment(idx);
                printf("migrate end %d\n",idx);
                //迁移后重新获取重定向的地址
                offset = mix_get_next_free_block(meta_data, idx);
            }
            mix_write_redirect_blockmeta(meta_data,idx,task->offset,offset);
        }
        //clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        //meta_time += (end.tv_sec - start.tv_sec) * 1000000 +
                                   //(end.tv_nsec - start.tv_nsec) / 1000;
        //printf("meta time is %llu us\n", meta_time);
        //clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        mix_write_to_buffer(task->buf, task->offset, offset, task->opcode);
        //clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        //data_time = (end.tv_sec - start.tv_sec) * 1000000 +
                                   //(end.tv_nsec - start.tv_nsec) / 1000;
        //printf("data time is %llu us\n", meta_time);
        return task->len;
    } else {
        //将当前task中包含的元数据都清空
        //然后将数据异步的转发到ssd的queue中
        //printf("post task to ssd\n");
        mix_post_task_to_ssd(task);
        mix_clear_blocks(meta_data, task);
        return 0;
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
    //等待ssd_queue中的所有任务执行完毕
    while (!mix_ssd_queue_is_empty())
        ;
    mix_migrate(meta_data, segment_idx);
    mix_segment_migration_end(meta_data, segment_idx);
}
atomic_int empty_num = 0;
//atomic_int task_num = 0;

static struct timespec start, end;

char* buf = NULL;

static void nvm_worker(void* arg) {
    int idx = *(int*)arg;  //当前线程对应的队列序号
    printf("nvm worker %d init\n",idx);
    int len = 0;
    int ret = 0;
    uint8_t seq = 0;
    uint8_t mask = 0x01;
    io_task_t* task = NULL;
    while (1) {
        if (seq & mask)
            task = get_task_from_nvm_queue(idx);
        else{
            task = get_task_from_buffer_queue(idx);
        }
        
        seq++;
        if (task == NULL){
            // if(idx == 0 && start == 1){
            //     printf("emtpy task num is %d\n",empty_num++);
            // }
            continue; 
        }else{
            //printf("task num is %d and idx is %d\n",task_num++,idx);
        }
    
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
                    pthread_mutex_unlock(&mutex[idx]);
                    break;
                }
                case MIX_WRITE: {
                    //printf("redirect_write %d\n",empty_num++);
                    ret = redirect_write(task, idx);
                    //printf("redirect ret is %d\n",ret);
                    pthread_mutex_unlock(&mutex[idx]);
                    break;
                }
                default: {
                    ret = 0;
                    pthread_mutex_unlock(&mutex[idx]);
                    break;
                }
            }
            
        }
        //printf("task ret is %ld\n",task->ret);
        //printf("1.ret is %d and task ret is %ld\n",ret,task->ret);
        task->ret = task->ret + ret;
        //printf("task ret is %ld\n",task->ret);
        //printf("2.ret is %d and task ret is %ld\n",ret,task->ret);
        mix_nvm_task_completed(task); 
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

    buffer_queue = malloc(sizeof(mix_queue_t*)*BUFFER_QUEUE_NUM);
    for(int i = 0; i < BUFFER_QUEUE_NUM; i++){
        pthread_mutex_init(&mutex[i],NULL);
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
    int l = 0;
    //printf("post task to nvm %lld\n",task->offset);
    while (l == 0) {        
        l = mix_enqueue(nvm_queue[task->queue_idx], task, 1);
        //l = mix_enqueue(nvm_queue[0], task, 1);
            //if(l == 0){
                //printf("retry time is %d\n",retry_time++);
    }
    return l;
}


static atomic_int buffer_task_num = 0;

int mix_post_task_to_buffer(io_task_t* task) {
    int len = 0;
    int idx = task->offset % SEGMENT_NUM;
    //printf("buffer task num is %d and idx is %d\n",buffer_task_num++,idx);
    while (!len) {
        len = mix_enqueue(buffer_queue[idx], task, 1);
    }
}
