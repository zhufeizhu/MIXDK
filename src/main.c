#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "mixdk.h"

int thread_num = 0;
int task_num = 0;

size_t nvm_block_num = (size_t)(16) * 1024 * 1024;

size_t offset = 0;  //

#define BUF_SIZE 4096

char buf1[BUF_SIZE];

void* write_func(void* arg) {
    int idx = *(int*)arg;
    size_t flags = MIX_SYNC;
    for (size_t i = idx; i < task_num; i += thread_num) {
        // if(i > 500000)
        // printf("[%d]:task offset is %llu\n",idx,offset + BUF_SIZE * i);
        mixdk_write(buf1, 1, nvm_block_num + i, flags, i);
        // printf("finish %d\n",i);
    }
    // printf("over\n");
    return NULL;
}

int main(int argc, char** argv) {
    thread_num = atoi(argv[1]);
    task_num = atoi(argv[2]);
    char c = argv[3][0];
    printf("mix begin init\n");

    mixdk_init();

    printf("mix end init\n");

    printf("%c\n", c);

    memset(buf1, c, BUF_SIZE);
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
        if (current_task_num == task_num)
            break;
        else {
            if (pre_task_num == current_task_num) {
                retry_time++;
                printf("main retry time is %d and task num is %d\n", retry_time,
                       current_task_num);
                // if(retry_time > 3) break;
            }
            pre_task_num = current_task_num;
            //sleep(1);
        }
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("time is %lu us\n", (end.tv_sec - start.tv_sec) * 1000000 +
                                   (end.tv_nsec - start.tv_nsec) / 1000);
}