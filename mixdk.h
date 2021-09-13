#ifndef MIXDK_H_
#define MIXDK_H_
#include "mix_queue.h"
#include <libpmemblk.h>
#include <pthread.h>



struct io_queue{
    mix_queue* queue;
    pthread_mutex_t* queue_lock;
}io_queue_t;

struct mixdk_context {
	struct spdk_bdev *bdev;
	struct spdk_bdev_desc *bdev_desc;
	struct spdk_io_channel *bdev_io_channel;
	char *buff;
	char *bdev_name;
}mixdk_context_t;

struct mixdk{
    PMEMblkpool *pbp;
    mixdk_context_t* context;
    mix_ring* queue;
    size_t nelements;
}mixdk_t;

int mixdk_init();

int mixdk_write(char* buf, int nbytes, int offset, int block);

int mixdk_read(char* buf, int nbytes, int offset, int block);

int mixdk_destroy();

#endif