#ifndef MIX_NVM_WORKER_H
#define MIx_NVM_WORKER_H

#include "mix_queue.h"
#include "mix_task.h"
#include "nvm.h"

#include <stdatomic.h>

nvm_info_t* mix_nvm_worker_init(unsigned int, unsigned int);

int mix_post_task_to_nvm(io_task_t*);

int mix_post_task_to_buffer(io_task_t*);

atomic_int mix_get_completed_nvm_task_num();

#endif  // MIX_NVM_WORKER_H