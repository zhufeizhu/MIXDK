#include "mixdk.h"
#include "io_scheduler.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#define PM_TASK 0
#define SSD_TASK 1

/* size of each element in the pmem pool */
#define ELEMENT_SIZE 4096

/* size of the pmemblk pool -- 1 GB */
#define POOL_SIZE ((size_t)(1 << 30))

struct MIXDK mixdk;
struct IO_QUEUE queue;

pthread_t io_thread;

int mixdk_init(char* path){
    if(!pmdk_init(path)||!spdk_init()){
        return -1;
    }

	if(!pthread_craete(&io_thread,NULL,mix_scheduler,&mixdk)){
		perror("create thread failed");
		return -1;
	}
	
    return 0;
}

static int pmdk_init(char* path){
	/* create the pmemblk pool or open it if it already exists */
	mixdk.pbp = pmemblk_create(path, ELEMENT_SIZE, POOL_SIZE, 0666);
    if (mixdk.pbp == NULL)
	    mixdk.pbp = pmemblk_open(path, ELEMENT_SIZE);

	if (mixdk.pbp == NULL) {
		perror(path);
		exit(1);
	}

    /* how many elements fit into the file? */
	mixdk.nelements = pmemblk_nblock(mixdk.pbp);
}

static int spdk_init(char* bdev_name){
	struct  mixdk_context context = {};
	struct spdk_app_opts opts = {};
	int rc = 0;
	spdk_app_opts_init(&opts, sizeof(opts));
	opts.name = "mixdk_bdev";

	context.bdev_name = bdev_name;

	

}

static int getTaskType(int block){
	return PM_TASK;
}

static struct IO_TASk* genIOTask(char* buf,int nbyte,int offset,int block){
	struct IO_TASK* task = malloc(sizeof(struct IO_TASK));
	task->flag = (u_int8_t)getTaskType(block);
	task->buf = malloc(len);
	memcpy(task->buf,buf,len);
	task->len = nbyte;
	task->offset = offset;
	return task;
}

static void submit_io_task(struct IO_TASk* task, void(void* args)* func){
	pthread_mutex_lock(queue.queue_lock);

	

	pthread_mutex_unlock(queue.queue_lock);
}

int mixdk_read(char* buf, int nbytes, int offset, int block){
	struct IO_TASK* task = genIOTask(buf,nbyte,offset,block);
	submit_io_task(task,NULL);
}


int mixdk_write(char* buf, int nbytes, int offset, int block){
	struct IO_TASK* task = genIOTask(buf,nbyte,offset,block);
	submit_io_task(task,NULL);
}

int mixdk_destroy(){

}