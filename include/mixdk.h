#ifndef MIX_MIXDK_H
#define MIX_MIXDK_H
#include <unistd.h>

int mixdk_init();

size_t mixdk_write(void* buf, size_t len, size_t offset, int idx);

size_t mixdk_read(void* buf,  size_t len, size_t offset, int idx);

int mixdk_destroy();

#endif //MIX_MIXDK_H