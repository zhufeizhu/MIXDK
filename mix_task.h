#ifndef MIX_TASK_H
#define MIX_TASK_H

#include <stdlib.h>

#define TASK_SUCCEED 0
#define TASK_FAILED 1

typedef struct io_task{
    char* buf;
    size_t len;
    size_t offset;
    u_int8_t opcode;
    u_int8_t ret;
    u_int16_t task_index;
    void (*on_task_succeed)(struct io_task*);
    void (*on_task_failed)(struct io_task*);
    char padding[28];//保证结果是2的整次幂 目前是32
} __attribute__((packed)) io_task_t;

#endif