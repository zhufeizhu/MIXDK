#ifndef MIX_HASH_H
#define MIX_HASH_H
#include <stdlib.h>
#include <pthread.h>

typedef struct hash_list_node{
    uint32_t key;
    uint32_t value;
    struct hash_list_node* next;
}hash_list_node_t;

typedef struct hash_node{
    int len;
    int threshold;
    pthread_rwlock_t* rw_lock;
    struct hash_list_node* list;
    struct black_red_tree* brtree;
}hash_node_t;

typedef struct mix_hash{
    int hash_size;
    hash_node_t* hash_nodes;
}mix_hash_t;


#ifdef __cplusplus
extern "C"{
#endif

mix_hash_t* mix_init_hash(int size);

void mix_hash_put(mix_hash_t* hash, uint32_t key, uint32_t value);

int mix_hash_get(mix_hash_t* hash, uint32_t key);

int mix_hash_has_key(mix_hash_t* hash, uint32_t key);

void mix_hash_delete(mix_hash_t* hash, uint32_t key);

void mix_free_hash(mix_hash_t* hash);
#ifdef __cplusplus
}
#endif

#endif //MIX_HASH_H