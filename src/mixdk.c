#include "mixdk.h"

#ifndef _WIN32
#include <unistd.h>
#endif
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "nvm_worker.h"
#include "mix_task.h"
#include "scheduler.h"

const static uint8_t max_submit_queue_num = 16;

static io_task_t* tasks;

int mixdk_init(uint8_t submit_queue_num) {
    if(submit_queue_num <= 0 || submit_queue_num > 16) return -1;
    tasks = malloc(sizeof(io_task_t)*submit_queue_num);
    mix_init_scheduler(4096 * TASK_SIZE, TASK_SIZE, submit_queue_num);
    return 0;
}



size_t mixdk_write(void* src,
                   size_t len,
                   size_t offset,
                   size_t flags,
                   int idx) {
    tasks[idx].buf = src;
    //memcpy(task->buf,src,len*BLOCK_SIZE);
    tasks[idx].len = len;
    tasks[idx].offset = offset;
    tasks[idx].opcode = MIX_WRITE | flags;
    tasks[idx].ret = 0;
    tasks[idx].queue_idx = idx;

    // printf("offset is %llu\n",offset);

    int ind = mix_post_task_to_io(&tasks[idx]);
    if (ind < 0) {
        return 1;
    }
    // mix_wait_for_task_completed(ind);
    // int ret =  task->ret;
    //free(task);
    return 0;
    // return 0;
}

size_t mixdk_read(void* dst, size_t len, size_t offset, size_t flags, int idx) {
    io_task_t* task = malloc(sizeof(io_task_t));
    task->buf = dst;
    task->len = len;
    task->offset = offset;
    task->opcode = MIX_READ | flags;
    task->ret = malloc(sizeof(atomic_int_fast32_t));
    task->queue_idx = idx;
    *(task->ret) = 0;

    int ind = mix_post_task_to_io(task);
    if (ind < 0) {
        return 1;
    }

    mix_wait_for_task_completed(len,task->ret);
    free(task->ret);
    free(task);
    return 0;
}

size_t mix_completed_task_num() {
    return get_completed_task_num();
}

void mix_segments_clear(){
    mix_rebuild();
}