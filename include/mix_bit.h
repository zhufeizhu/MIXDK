#ifndef MIX_BIT_H
#define MIX_BIT_H
#include <unistd.h>

typedef struct{
    void* bits;
    __uint32_t bits_num;
    __uint32_t next_bit;
}mix_bitmap;

mix_bitmap* mix_init_bitmap(size_t size);

inline int mix_next_zero_bit(mix_bitmap* bitmap);

int mix_set_bit(int nr, mix_bitmap* bitmap);

int mix_clear_bit(int nr, mix_bitmap* bitmap);

int mix_test_bit(int nr, mix_bitmap* bitmap);

int mix_get_bit(int *addr, mix_bitmap* bitmap);

#endif