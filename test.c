#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "mix_queue.h"


#define FIFO_LEN	(1024*1024)	//1MB, must be power of 2

mix_queue_t* queue;

void* read_poller(void* arg){
    while(1){
        io_task_t* task = malloc(TASK_SIZE);
        mix_dequeue(queue,task,1);
        printf("task content is %s\n",task->buf);
        printf("task block is %ld\n\n",task->block);
    }
}


/**
 * 向ssd中写数据
**/
int write_to_ssd(void* src,unsigned int len, unsigned int offset){

}


/**
 * 向nvm中写数据
 * 
**/
int write_to_nvm(void* src,unsigned int len, unsigned int offset){

}

/**
 * 从ssd中读数据
**/
int read_from_ssd(void* dst,unsigned int len, unsigned int offset){

}

/**
 * 从nvm中读数据
**/
int read_from_nvm(void* dst,unsigned int len, unsigned int offset){

}


int main(){   
    queue = mix_queue_init(FIFO_LEN,TASK_SIZE,read_from_ssd,write_to_ssd);

    pthread_t read_thread; 
    if(pthread_create(&read_thread,NULL,read_poller,NULL)){
        printf("create child thread failed\n");
        return 0;
    }

    int max = 10000;
    while(max){
        io_task_t* task = malloc(TASK_SIZE);
        if(task == NULL) continue;
        task->buf = malloc(10);
        memcpy(task->buf,"hello",5);
        task->block = max;
        mix_enqueue(queue,task,1);
        max--;
    }

    pthread_join(read_thread,NULL);
    return 0;
}