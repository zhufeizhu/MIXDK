#ifndef MIX_HASH_H
#define MIX_HASH_H
#include <stdlib.h>

typedef struct black_red_tree{
    
}black_red_tree;


typedef struct hash_list_node{
    size_t key;
    size_t value;
    struct hash_list_node* next;
}hash_list_node_t;


typedef struct hash_node{
    int len;
    int threshold;
    struct hash_list_node* list;
    struct black_red_tree* brtree;
}hash_node;


typedef struct mix_hash{
    int has_size;
    struct hash_node* hash_nodes;
}mix_hash_t;


mix_hash_t* mix_hash_init(int size);

void mix_hash_put(mix_hash_t* hash, size_t key, size_t value);

size_t mix_hash_get(mix_hash_t* hash, size_t key, size_t value);

int mix_hash_has_key(mix_hash_t* hash, size_t key);

#endif //MIX_HASH_H