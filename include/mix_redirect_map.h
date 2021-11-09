#ifndef MIX_REDIRECT_MAP_H
#define MIX_REDIRECT_MAP_H

#include <stdlib.h>

typedef struct mix_kv{
    size_t original_addr;
    size_t redirect_addr;
}mix_kv_t;

size_t mix_store_redirect_kv();

size_t mix_load_redirect_kv();

#endif //MIX_REDIRECT_MAP_H