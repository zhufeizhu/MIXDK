#include "mixdk.h"
#include "scheduler.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#define PM_TASK 0
#define SSD_TASK 1 m   
#define SSD_TASK 1 m  

/* size of each element in the pmem pool */
#define ELEMENT_SIZE 4096




/* size of the pmemblk pool -- 1 GB */
#define POOL_SIZE ((size_t)(1 << 30))

int mix_init(){

}

int mix_write(void* src,unsigned int len, unsigned int offset){
    assert(src != NULL);
    assert(src > 0 && offset >=0);

    io_task_t* task = malloc(sizeof(io_task_t));

    task->buf = src;
    task->len = len;
    task->offset = offset;
    task->opcode = MIX_WRITE;
    
    int ind = mix_post_task_to_io(task);

    if(!ind){
        return 0;
    }
    
    pthread_cond_t* write_cond = mix_get_cond(ind);

    pthread_cond_wait(write_cond,)
}


int mix_read(void* dst,unsigned int len, unsigned int offset){

}