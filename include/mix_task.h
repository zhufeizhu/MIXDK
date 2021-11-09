#ifndef MIX_TASK_H
#define MIX_TASK_H

#include <stdlib.h>
#include "mixdk.h"
#include <stdatomic.h>

#define TASK_SUCCEED 0
#define TASK_FAILED 1

typedef struct io_task{
    char* buf;//8
    size_t len;//4
    size_t offset;//4
    size_t ret;//4
    size_t opcode;//1
    size_t task_index;//1
    atomic_bool* flag;
} __attribute__((packed)) io_task_t;

#endif