#ifndef MIX_MIXDK_H
#define MIX_MIXDK_H

int mixdk_init();

int mixdk_write(void* buf, unsigned int len, unsigned int offset);

int mixdk_read(void* buf,  unsigned int len, unsigned int offset);

int mixdk_destroy();

#endif //MIX_MIXDK_H