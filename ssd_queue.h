#ifndef MIX_SSD_QUEUE_H
#define MIX_SSD_QUEUE_H

#include "mix_queue.h"

int mix_init_ssd_queue(unsigned int, unsigned int);

int mix_post_task_to_ssd(io_task_t*);

#endif //MIX_SSD_QUEUE_H