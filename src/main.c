#include "mixdk.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

int thread_num = 0;
int task_num = 0;

size_t offset = (size_t)0;//128 * 1024 * 1024 * 1024;

#define BUF_SIZE 4096

char buf1[BUF_SIZE];

void* write_func(void* arg){
    int idx = *(int*)arg;
    size_t flags = MIX_SYNC;
    for(int i = idx; i < task_num; i += thread_num){
        //printf("[%d]:task %d\n",idx,i);
        mixdk_write(buf1,BUF_SIZE,offset + BUF_SIZE * i,flags,i);
        //printf("finish %d\n",i);
    }
    //printf("over\n");
    return NULL;
}

int main(int argc, char** argv){
    thread_num = atoi(argv[1]);
    task_num = atoi(argv[2]);
    char c = argv[3][0];
    mixdk_init();
    printf("%c\n",c);

    memset(buf1,c,BUF_SIZE);
    // int n = 0;
    pthread_t* pids = malloc(sizeof(pthread_t*)*thread_num);
    time_t start  = time(NULL);
    int* idx = (int*)malloc(sizeof(int)*thread_num);
    for(int i = 0; i < thread_num; i++){
        idx[i] = i;
        if(pthread_create(pids + i,NULL,write_func,(void*)(idx+i))){
            perror("create thread");
            return 0;
        }
    }
    int pre_task_num = 0;
    int current_task_num = 0;
    int retry_time = 0;
    while(1){
        current_task_num = mix_completed_task_num();
        if(current_task_num == task_num) break;
        else{
            if(pre_task_num == current_task_num){
                retry_time++;
                printf("main retry time is %d and task num is %d\n",retry_time,current_task_num);
                //if(retry_time > 3) break;
            }
            pre_task_num = current_task_num;
            sleep(1);
        }
    }
    time_t end = time(NULL);
    printf("time is %ld\n",end-start);
}