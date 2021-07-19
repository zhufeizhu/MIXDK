#ifndef MIXDK_H_
#define MIXDK_H_
#include <libpmemblk.h>
#include <pthread.h>

struct MIX_DK{
    PMEMblkpool *pbp;
    size_t nelements;
};

struct IO_TASK{
    char* buf;
    size_t len;
    size_t block;
    size_t offset;
    /*flag[0]:请求类型 0为读 1为写*/
    /*flag[1]:请求介质:0为pm 1为ssd*/
    u_int8_t flag; 
};

struct IO_QUEUE{
    size_t head;
    size_t tail;
    size_t len;
    struct IO_TASK* tasks;
    pthread_mutex_t* queue_lock;
}

int mixdk_init();

int mixdk_write(char* buf, int nbytes, int offset, int block);

int mixdk_read(char* buf, int nbytes, int offset, int block);

int mixdk_destroy();

#endif