#ifndef MIX_NVM_QUEUE_H
#define MIx_NVM_QUEUE_H

#include "mix_queue.h" 
#include "nvm.h"
#include "mix_task.h"

#include <stdatomic.h>

nvm_info_t* mix_init_nvm_queue(unsigned int, unsigned int);

int mix_post_task_to_nvm(io_task_t*);

atomic_int mix_get_completed_nvm_task_num();

#endif //MIX_NVM_QUEUE_H