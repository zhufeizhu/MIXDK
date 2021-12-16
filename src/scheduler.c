#include "scheduler.h"

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "mixdk.h"
#include "mix_task.h"
#include "ssd_queue.h"
#include "nvm_queue.h"
#include "mix_meta.h"

//数据定义区
static scheduler_ctx_t *sched_ctx = NULL;
static const size_t threshold = 4096;

static io_task_t* p_task;

int mix_wait_for_task_completed(atomic_bool* flag){
    while(atomic_load(flag) == false){};
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

size_t get_completed_task_num(){
    return mix_get_completed_nvm_task_num() + mix_get_completed_ssd_task_num();
}

static atomic_int task_num = 0;

int mix_post_task_to_io(io_task_t *task)
{
    int l = 0;
    while(l == 0){
        l = mix_enqueue(sched_ctx->submit_queue, task, 1);
    }
    
    return l;
}

/**
 * @brief 根据传入的task 将其拆分成多个task(io_task_v) 需要将其中的每个task进行处理
 * 
 * @param task 用户传入的io操作
 * @return io_task_t** 对用户传入的io操作的拆分
 */
static inline io_task_t* schedule(io_task_t *task)
{
    //当前task的offset在nvm的范围内
    if ((size_t)task->offset < sched_ctx->metadata->nvm_size && task->offset >= 0) {
        // printf("nvm task\n");
        task->type = NVM_TASK;
        return NULL;
    }
    
    if ((size_t)task->offset >= sched_ctx->metadata->nvm_size && (size_t)task->offset < (sched_ctx->metadata->nvm_size + sched_ctx->metadata->ssd_size)) {
        if(task->opcode & MIX_WRITE){
            // 写到ssd的请求分成大写和小写
            // 针对小写 需要将该请求写到 buffer区 同时添加对应的元数据
            // 针对大写 需要将该请求拆分成一个block大小的请求 用于查询当前block是否在buffer中 如果在的话 清空其对应的元数据 同时并行的将内容写到ssd中
            if(task->len <= threshold){//将写向ssd的小块 重定向的nvm中
                return NULL;
            }else{
                // ssd的大写 需要清空metadata中记录的相关ssd的数据信息
                // 这里生成的是删除旧block的元数据任务
                p_task->len = task->len;
                p_task->offset = task->offset;
                p_task->type = CLEAR_TASK;
                return p_task;
            }
        }else if(task->opcode & MIX_READ){
            // 读操作直接重定向到nvm中 让nvm队列执行相关的查询操作
            task->redirect = 1;
            return NULL;
        }
    }
    //printf("[task offset]:%llu\n [nvm_size]:%llu\n [ssd_size]:%llu\n",(size_t)task->offset,nvm_size,ssd_size);
}

static void scheduler(void *arg)
{
    int len = 0;
    int i = 0;
    io_task_t* io_task = malloc(TASK_SIZE);
    while (1) {
        //pthread_spin_lock(sched_ctx->schedule_queue_lock);
        len = mix_dequeue(sched_ctx->submit_queue, io_task, 1);
        //pthread_spin_unlock(sched_ctx->schedule_queue_lock);
        if (len == 0) {
            //printf("empty\n");
            continue;
        }
        io_task_t* new_task = schedule(io_task);
        // 并发执行大写时删除buffer中元数据的动作
        if(new_task != NULL && new_task->type == CLEAR_TASK){
            mix_post_task_to_nvm(new_task);
        }
        
        switch(io_task->type){
            case NVM_TASK:
            {
                //printf("post task num is %d\n",task_num++);
                mix_post_task_to_nvm(io_task);
                break;
            };
            case SSD_TASK:
            {
                mix_post_task_to_ssd(io_task);
                break;
            };
            default:{
                //printf("post error\n");
                break;
            }
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
int mix_init_scheduler(unsigned int size, unsigned int esize, int max_current)
{
    pthread_t scheduler_thread = 0;
    ssd_info_t* ssd_info = NULL;
    nvm_info_t* nvm_info = NULL;

    sched_ctx = malloc(sizeof(scheduler_ctx_t));
    if (sched_ctx == NULL) {
        mix_log("mix_init_scheduler","malloc for sched_ctx failed");
        return -1;
    }

    sched_ctx->max_current = max_current;
    sched_ctx->submit_queue = mix_queue_init(size, esize);
    if (sched_ctx->submit_queue == NULL) {
        mix_log("mix_init_scheduler","mix queue init failed");
        return -1;
    }

    sched_ctx->completation_conds = malloc(sizeof(pthread_cond_t *) * max_current);
    if (sched_ctx->completation_conds == NULL) {
        return -1;
    }
    for (int i = 0; i < max_current; i++) {
        sched_ctx->completation_conds[i] = malloc(sizeof(pthread_cond_t));
        if (sched_ctx->completation_conds[i] == NULL) {
            return -1;
        }
        pthread_cond_init(sched_ctx->completation_conds[i],NULL);
    }

    sched_ctx->ctx_mutex = malloc(sizeof(pthread_mutex_t*) * max_current);
    if (sched_ctx->ctx_mutex == NULL) {
        return -1;
    }
    for (int i = 0; i < max_current; i++) {
        sched_ctx->ctx_mutex[i] = malloc(sizeof(pthread_mutex_t));
        if (sched_ctx->ctx_mutex[i] == NULL) {
            return -1;
        }
        pthread_mutex_init(sched_ctx->ctx_mutex[i],NULL);
    }

    sched_ctx->bitmap_lock = malloc(sizeof(pthread_mutex_t));
    if (sched_ctx->bitmap_lock == NULL) {
        return -1;
    }
    pthread_mutex_init(sched_ctx->bitmap_lock,NULL);

    sched_ctx->schedule_queue_lock = malloc(sizeof(pthread_mutex_t));
    if (sched_ctx->schedule_queue_lock == NULL) {
        return -1;
    }
    pthread_spin_init(sched_ctx->schedule_queue_lock, 1);

    sched_ctx->bitmap_lock = malloc(max_current / 8);
    if (sched_ctx->bitmap_lock == NULL) {
        return -1;
    }
    memset(sched_ctx->bitmap_lock, 0, max_current / 8);

    ssd_info = mix_init_ssd_queue(size,esize);
    if (ssd_info == NULL) {
        return -1;
    }

    nvm_info = mix_init_nvm_queue(size,esize);
    if (nvm_info == NULL) {
        return -1;
    }

    sched_ctx->ssd_info = ssd_info;
    sched_ctx->nvm_info = nvm_info;

    sched_ctx->metadata = malloc(sizeof(scheduler_metadata_t));
    if(sched_ctx->metadata == NULL) {
        return -1;
    }

    sched_ctx->metadata->nvm_size = nvm_info->nvm_capacity; //nvm的大小
    sched_ctx->metadata->ssd_size = ssd_info->ssd_capacity; //ssd的大小

    sched_ctx->metadata->size = (sched_ctx->metadata->nvm_size + sched_ctx->metadata->ssd_size) / 2048;//8byte->4kb
    sched_ctx->metadata->ssd_saddr = 0;
    sched_ctx->metadata->nvm_saddr = sched_ctx->metadata->size;

    if (pthread_create(&scheduler_thread, NULL, (void*)scheduler, NULL)) {
        printf("create scheduler failed\n");
        return -1;
    }
    return 0;
}