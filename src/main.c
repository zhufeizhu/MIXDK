#include "mixdk.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

int thread_num = 0;
int task_num = 0;

int offset = 4*1024*1024 + 10;

void* write_func(void* arg){
    int idx = *(int*)arg;
    char* buf1 = "zellotorld";
    for(int i = idx; i < task_num; i += thread_num){
        mixdk_write(buf1,10,offset + 10 * i,idx);
    }
    return NULL;
}


int main(int argc, char** argv){
    thread_num = atoi(argv[1]);
    task_num = atoi(argv[2]);
    mixdk_init();
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
    for(int i = 0; i < thread_num; i++){
        pthread_join(pids[i],NULL);
    }
    time_t end = time(NULL);
    printf("time is %ld\n",end-start);
}