#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "mixdk.h"

int thread_num = 0;
int task_num = 0;
char c = 0;
size_t nvm_block_num = (size_t)(16) * 1024 * 1024;

size_t offset = 0;  //

#define BUF_LEN 4

#define TASK

#define BLOCK_SIZE 4096

#define BUF_SIZE BLOCK_SIZE* BUF_LEN

char* buf1;
char* buf2;

void* write_func(void* arg) {
    int idx = *(int*)arg;
    size_t flags = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    printf("idx is %d\n",idx);
    for (size_t i = 0; i < task_num; i += thread_num) {
        // mixdk_read(buf2,BUF_LEN,i,0,i);
        mixdk_write(buf1,BUF_LEN, nvm_block_num + (idx + i)*BUF_LEN,0,idx);
        
        // mixdk_write(buf1,1,nvm_block_num + i*BUF_LEN+1,0,i);
        // mixdk_write(buf1,1,nvm_block_num + i*BUF_LEN+2,0,i);
        // mixdk_write(buf1,1,nvm_block_num + i*BUF_LEN+3,0,i);
        // mixdk_write(buf1,BUF_LEN, nvm_block_num + i*BUF_LEN+2,0,i);
        // mixdk_read(buf2,BLOCK_NUM, nvm_block_num + i*BLOCK_NUM,0,i);
        
    }
    // mixdk_read(buf2, 10 * BUF_LEN, nvm_block_num, 0, 0);
    // for (size_t i = 0; i < task_num; i += thread_num) {
    //     // mixdk_write(buf1,1,nvm_block_num + i*BLOCK_NUM,0,i);
    //     // mixdk_write(buf1,1,nvm_block_num + i*BLOCK_NUM+1,0,i);
    //     mixdk_read(buf2,BLOCK_NUM, i*BLOCK_NUM,0,i);
    //     //printf("read is %s\n",buf2);
    // }

    // clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    // printf("time %d is %lu us\n", idx, (end.tv_sec - start.tv_sec) * 1000000 +
    //(end.tv_nsec - start.tv_nsec) / 1000);
    printf("over\n");
    return NULL;
}

int main(int argc, char** argv) {
    thread_num = atoi(argv[1]);
    task_num = atoi(argv[2]);
    c = argv[3][0];
    printf("mix init begin\n");
    mixdk_init(thread_num);

    printf("mix init succeed\n");
    printf("task num is %d\n", task_num);
    printf("thread num is %d\n", thread_num);
    printf("content is %c\n\n\n", c);

    buf1 = valloc(BUF_SIZE);
    memset(buf1, c, BUF_SIZE);
    buf2 = malloc(BUF_SIZE*20);
    memset(buf2, 0, BUF_SIZE*20);
    // int n = 0;
    pthread_t* pids = malloc(sizeof(pthread_t*) * thread_num);
    struct timespec start, end;

    int* idx = (int*)malloc(sizeof(int) * thread_num);
    for (int i = 0; i < thread_num; i++) {
        idx[i] = i;
        if (pthread_create(pids + i, NULL, write_func, (void*)(idx + i))) {
            perror("create thread");
            return 0;
        }
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    int pre_task_num = 0;
    int current_task_num = 0;
    int retry_time = 0;
    while (1) {
        current_task_num = mix_completed_task_num();
        if (current_task_num == task_num*BUF_LEN)
            break;
        else {
            if (pre_task_num == current_task_num) {
                retry_time++;
                // printf("main retry time is %d and task num is %d\n", retry_time,
                // current_task_num);
                // if(retry_time > 3) break;
            }
            pre_task_num = current_task_num;
            // sleep(1);
        }
    }
    printf("current task num is %d\n", current_task_num);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("time is %lu us\n", (end.tv_sec - start.tv_sec) * 1000000 +
                                   (end.tv_nsec - start.tv_nsec) / 1000);
    pthread_join(pids[0], NULL);

    // memset(buf1, c+1, BUF_SIZE);
    // for (int i = 0; i < thread_num; i++) {
    //     idx[i] = i;
    //     if (pthread_create(pids + i, NULL, write_func, (void*)(idx + i))) {
    //         perror("create thread");
    //         return 0;
    //     }
    // }
    // clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    // while (1) {
    //     current_task_num = mix_completed_task_num();
    //     if (current_task_num == 2*task_num)
    //         break;
    //     else {
    //         if (pre_task_num == current_task_num) {
    //             retry_time++;
    //             //printf("main retry time is %d and task num is %d\n", retry_time,
    //                    //current_task_num);
    //             // if(retry_time > 3) break;
    //         }
    //         pre_task_num = current_task_num;
    //         //sleep(1);
    //     }
    // }
    // printf("current task num is %d\n",current_task_num);
    // clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    // printf("time is %lu us\n", (end.tv_sec - start.tv_sec) * 1000000 +
    //                                (end.tv_nsec - start.tv_nsec) / 1000);
    // pthread_join(pids[0],NULL);
    return 0;
} 
