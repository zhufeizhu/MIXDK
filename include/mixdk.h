#ifndef MIX_MIXDK_H
#define MIX_MIXDK_H
#include <unistd.h>
#define MIX_READ 1
#define MIX_WRITE 1 << 2
#define MIX_LATENCY 1 << 3
#define MIX_THROUGHOUT 1 << 4
#define MIX_SYNC 1 << 5

#define size_t unsigned long long

int mixdk_init();

size_t mixdk_write(void* buf, size_t len, size_t offset, size_t flags, int idx);

size_t mixdk_read(void* buf, size_t len, size_t offset, size_t flags, int idx);

int mixdk_destroy();

void mixdk_flush();

size_t mix_completed_task_num();

#endif  // MIX_MIXDK_H