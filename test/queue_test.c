#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "mix_queue.h"

mix_queue_t* queue;

int empty = 0;
int task_num;

_Atomic int produce_task_num = 0;

void* producer(void* arg){
    int l = 0;
    int value = 1;
    printf("tasj num is %d\n",task_num);
    for(int i = 1; i < task_num; i = i + 2){
        //printf("%d\n",i);
        value = i;
        while(l == 0){
            l = mix_enqueue_lockfree(queue,&value,1);
        }
        produce_task_num++;
        l = 0;
    }
    return NULL;
}

int consume_task_num = 0;

void* consumer(void* arg){
    int l = 0;
    int value = 1;
    while(1){
        l = mix_dequeue_lockfree(queue,&value,1);
        
        if(l > 0) {
            consume_task_num++;
            if(value > 9990)
                printf("value is %d\n",value);
        }
        else{
            if(empty) return NULL;
        }
    }
    return;
}

int main(int argc, char** argv){
    int producer_num = atoi(argv[1]);
    task_num =atoi(argv[2]);
    printf("producer num is %d\n",producer_num);
    printf("task num is %llu\n",task_num);

    queue = mix_queue_init(1024*sizeof(int),sizeof(int));


    pthread_t* producers;
    pthread_t consumers;

    producers = malloc(sizeof(pthread_t) * producer_num);

    struct timespec start,end;

    clock_gettime(CLOCK_MONOTONIC_RAW,&start);
    for(int i = 0 ; i < producer_num; i++){
        pthread_create(&producers[i],NULL,producer,NULL);
    }

    pthread_create(&consumers,NULL,consumer,NULL);

    for(int i = 0; i < producer_num; i++){
        pthread_join(producers[i],NULL);
    }

    empty = 1;

    pthread_join(consumers,NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW,&end);


    printf("all task %d\n",task_num);
    printf("produce task %d\n",produce_task_num);
    printf("consume task %d\n",consume_task_num);
    printf("time is %llu us\n",(end.tv_sec - start.tv_sec)*1000000 + (end.tv_nsec - start.tv_nsec)/1000);
    return 0;
}