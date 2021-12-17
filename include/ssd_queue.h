#ifndef MIX_SSD_QUEUE_H
#define MIX_SSD_QUEUE_H

#include "mix_queue.h"
#include "mix_task.h"
#include "ssd.h"

#include <stdatomic.h>

ssd_info_t* mix_init_ssd_queue(unsigned int, unsigned int);

int mix_post_task_to_ssd(io_task_t*);

atomic_int mix_get_completed_ssd_task_num();

#endif  // MIX_SSD_QUEUE_H