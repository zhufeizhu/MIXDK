#include "mix_hash.h"

#include <stdlib.h>
#include <assert.h>

static inline int hash_func(size_t key, int size){
    return key % size;
}

mix_hash_t* mix_hash_init(int size){
    mix_hash_t* hash = malloc(sizeof(mix_hash_t));
    if(hash == NULL){
        perror("alloc memory for hash failed");
        return NULL;
    }
    hash->hash_nodes = malloc(sizeof(hash_node) * size);
    if(hash->hash_nodes == NULL){
        perror("alloc memory for hash failed");
        return NULL;
    }
    hash->has_size = size;
    return hash;
}

void mix_hash_put(mix_hash_t* hash, size_t key, size_t value){
    int hash_key = hash_func(key,hash->has_size);

    hash_node node = hash->hash_nodes[hash_key];

    hash_list_node_t* new_node = malloc(sizeof(hash_list_node_t));
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    if(node.list == NULL){
        node.list = new_node;
        node.len = 1;
    }else{
        hash_list_node_t* list_node = node.list;
        while(list_node->next){
            list_node = list_node->next;
        }
        list_node->next = new_node;
        node.len++;
    }
}

size_t mix_hash_get(mix_hash_t* hash, size_t key, size_t value){
    int hash_key = hash_func(key,hash->has_size);

    hash_list_node_t* node = hash->hash_nodes[hash_key].list;

    while(node){
        if(node->key == key){
            return node->value;
        }else{
            node = node->next;
        }
    }
    return -1;
}

int bloom_filter(size_t key){
    
}

/**
 * @brief 判断hash中是否有对应的key-value
 * 
 * @param hash 
 * @param key 
 * @return int 
 */
int mix_hash_has_key(mix_hash_t* hash, size_t key){
    int hash_key = hash_func(key,hash->has_size);

    hash_list_node_t* node = hash->hash_nodes[hash_key].list;

    while(node && node->key != key){
        node = node->next;
    }

    if(node == NULL) return 0;
    else return 1;
}
