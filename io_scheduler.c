#include "io_scheduler.h"
#include "mix_queue.h"
#include "mixdk.h"
#include <assert.h>
#include <stdlib.h>


void* mix_scheduler(void* arg){
    mix_queue_t* queue = ((mixdk_t*)arg)->queue;
    for(;;){
        io_task_t* task = calloc(1,sizeof(io_task_t));
        if(task == NULL) continue;
        mix_dequeue(queue,task,sizeof());
        
    }
}