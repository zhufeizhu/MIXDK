#include <stdio.h>
#define __USE_GNU 1
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

extern char *optarg;
extern int optind, opterr, optopt;

#define BLOCK_SIZE 4096

int main(int argc, char** argv){
    int opt;
    char* path = NULL;
    char* device = NULL;
    while ((opt = getopt(argc, argv, "t:d:")) != -1) {
        switch (opt) {
            case 't':{
                int len = strlen(optarg);
                path = malloc(len);
                strcpy(path,optarg);
                break;
            }   
            case 'd':{
                int len = strlen(optarg);
                device = malloc(len);
                strcpy(device,optarg);
                break;
            }
            default: /* '?' */
                fprintf(stderr, "Usage: %s -t trace_path -d device_path\n",argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    printf("trace file is %s\n",path);
    printf("device is %s\n",device);

    FILE* file = fopen(path,"r");
    if(file == NULL) {
        printf("no file\n");
        return 0;
    }

    int fd = open(device,O_RDWR|O_DIRECT);
    if(fd < 0){
        printf("device %s不存在\n",device);
        return 0;
    }

    char type;
    int offset;
    int num;
    char* block = NULL;
    block = valloc(4096*1024);
    if(block == NULL){
        printf("malloc alignment memory failed\n");
        return 0;
    }
    memset(block,0,4096*1024);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    int all_block_num = 0;
    while(!feof(file)){
        fscanf(file,"%c,%d,%d",&type,&offset,&num);
        if(type == 'W'){
            pwrite(fd,block,num*BLOCK_SIZE,offset*BLOCK_SIZE);
            all_block_num += num;
        }else if(type == 'R'){
            pread(fd,block,num*BLOCK_SIZE,offset*BLOCK_SIZE);
        }
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    printf("time is %lu us\n", (end.tv_sec - start.tv_sec) * 1000000 +
                                   (end.tv_nsec - start.tv_nsec) / 1000);
    printf("write block num is %d\n",all_block_num);
    return 0;
}