#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU 1
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <stdatomic.h>



int task_num = 0;
int thread_num = 0;

atomic_int_fast8_t flag = 0;

int fd = 0;
char* buf;

void* write_func(void* arg){
    int idx = *(int*)arg;
    for(int i = 0; i < task_num; i+= thread_num){
        pwrite(fd,buf,4096*4,(i*thread_num + idx)*4096*4);
    }
    printf("thread %d finish\n",idx);
    flag++;
    return NULL;
}


int main(int argc, char** argv){
    int block_size = 4096;//4K
    int len = 4096;//1G
    task_num = atoi(argv[1]);
    thread_num = atoi(argv[2]);
    fd = open("/dev/nvme0n1",O_RDWR|O_DIRECT);

    if(fd < 0){
        perror("open");
        return -1;
    }

    // char* buf1 = "hello world";
    // int len = strlen(buf1);
    // printf("%d\n",len);
    // len = pwrite(raw_ssd_fd,buf1,strlen(buf1),0);
    // printf("%d\n",len);

    // if(len <= 0){
    //     perror("pwrite");
    // }


    // char* buf1 = "hello2022";
    // // char buf1[10000];
    // len = pwrite(raw_nvm_fd,buf1,9,0);
    // printf("%d\n",len);
    // if(len <= 0){
    //     perror("pread");
    // }

    //fsync(raw_nvm_fd);
    buf = valloc(4096*4);
    memset(buf,'o',4096*4);

    pthread_t pids[4];
    struct timespec start, end;
    int ids[4];
    for(int i = 0; i < thread_num; i++){
        ids[i] = i;
        pthread_create(&pids[i],NULL,write_func,&ids[i]);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    while(flag < thread_num);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("time is %lu us\n", (end.tv_sec - start.tv_sec) * 1000000 +
                                   (end.tv_nsec - start.tv_nsec) / 1000);

    for(int i = 0; i < thread_num; i++){
        pthread_join(pids[i],NULL);
    }


    return 0;
}