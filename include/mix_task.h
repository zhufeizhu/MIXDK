#ifndef MIX_TASK_H
#define MIX_TASK_H

#include <stdlib.h>

#define TASK_SUCCEED 0
#define TASK_FAILED 1

typedef struct io_task{
    char* buf;//8
    size_t len;//4
    size_t offset;//4
    size_t ret;//4
    void (*on_task_completed)(struct io_task*);//8
    __uint8_t opcode;//1
    __uint8_t task_index;//1
    struct io_task* original_task;
} __attribute__((packed)) io_task_t;

#endif