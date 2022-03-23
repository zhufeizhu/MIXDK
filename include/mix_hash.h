#ifndef MIX_HASH_H
#define MIX_HASH_H
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct mix_kv {
    uint32_t key;
    uint32_t value;
} mix_kv_t;

typedef struct hash_list_node {
    uint32_t key;
    uint32_t value;
    struct hash_list_node* next;
} hash_list_node_t;

typedef struct hash_node {
    int len;
    int threshold;
    pthread_rwlock_t* rw_lock;
    struct hash_list_node* list;
    struct black_red_tree* brtree;
} hash_node_t;

typedef struct mix_hash {
    int hash_size;
    hash_node_t* hash_nodes;
    int hash_node_entry_idx;  //进行遍历时正在访问的hahs_node的序号
} mix_hash_t;

#ifdef __cplusplus
extern "C" {
#endif

mix_hash_t* mix_hash_init(int size);

void mix_hash_put(mix_hash_t* hash, uint32_t key, uint32_t value);

int mix_hash_get(mix_hash_t* hash, uint32_t key);

int mix_hash_has_key(mix_hash_t* hash, uint32_t key);

void mix_hash_delete(mix_hash_t* hash, uint32_t key);

void mix_hash_free(mix_hash_t* hash);

mix_kv_t mix_hash_get_entry(mix_hash_t* hash);

mix_kv_t mix_hash_get_entry_by_idx(mix_hash_t* hash,int idx);
#ifdef __cplusplus
}
#endif

#endif  // MIX_HASH_H