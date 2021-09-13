#ifndef MIX_NVM_QUEUE_H
#define MIx_NVM_QUEUE_H

#include "mix_queue.h"

#include "mix_task.h"

int mix_init_nvm_queue(unsigned int, unsigned int);

int mix_post_task_to_nvm(io_task_t*);

#endif //MIX_NVM_QUEUE_H