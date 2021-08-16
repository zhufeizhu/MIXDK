#include "dplist.h"
#include <stdlib.h>
#include <assert.h>

dplist_t *dpl_create(
        void* (*element_copy)(void *element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y))
{
    dplist_t* list = calloc(1,sizeof(dplist_t));
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;


    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element)
{
    assert(list != NULL);
    dplist_node_t* node = (*list)->head;
    dplist_node_t* next;
    while(node){
        if(free_element){
            (*list)->element_free(node->element);
        }else{
            free(node->element);
        }
        next = node->next;
        free(node);
        node = next;
    }
    free(*list);
}

int dpl_size(dplist_t *list){
    if(list == NULL)    return -1;
    else{
        return list->size;
    }
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy){
    if(list == NULL) return NULL;
    dplist_node_t* new_node = calloc(1,sizeof(dplist_node_t));
    if(insert_copy){
        new_node->element = list->element_copy(element);
    }else{
        new_node->element = element;
    }

    if(index <= 0){
        new_node->next = list->head;
        list->head->prev = new_node;
        list->head = new_node;
    }else if(index > list->size){
        list->tail->next = new_node;
        new_node->prev = list->tail;
        list->tail = new_node;
    }else{
        dplist_node_t* node = list->head;
        index--;
        while(node && index){
            node = node->next;
            index--;
        }
        new_node->next = node->next;
        node->next->prev = new_node;
        node->next = new_node;
        new_node->prev = node;
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element){
    if(list == NULL) return NULL;
    if(index <= 0){
        dplist_node_t* node = list->head;
        list->head = node->next;
        list->head->next = NULL;
        if(free_element){
            list->element_free(node);
        }else{
            free(node);
        }
    }else if(inde >= list->size){
        dplist_node_t* node = list->tail;
        list->tail = node->prev;
        list->tail->next = NULL;
        if(free_element){
            list->element_free(node);
        }else{
            free(node);
        }
    }else{
        dplist_node_t* node = list->head;
        index--;
        while(node && index){
            node = node->next;
            index--;
        }
        dplist_node_t* delete_node = node->next;
        node->next = delete_node->next;
        delete_node->next->prev = node;
        if(free_element){
            list->element_free(delete_node);
        }else{
            free(delete_node);
        }
    }
    list->size--;
    return list;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index){
    if(list == NULL || list->head == NULL)  return NULL;
    if(index <= 0)  return list->head;
    if(index >= list->size)   return list->tail;
    dplist_node_t* node = list->head;
    while(node && index){
        node = node->next;
        index--;
    }

    return node;
}

void *dpl_get_element_at_index(dplist_t *list, int index){
    dplist_node_t* node = dpl_get_reference_at_index(list,index);
    if(node == NULL) return NULL;
    else return node->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element){
    if(list == NULL) return NULL;
    dplist_node_t* node = list->head;
    int index = 0;
    while(node){
        if(!list->element_compare(node->element,element)){
            return index;
        }else{
            node = node->next;
        }
    }
    return -1;
}


dplist_node_t *dpl_get_first_reference(dplist_t *list){
    if(list == NULL)    return NULL;
    else return list->head;
}

dplist_node_t *dpl_get_last_reference(dplist_t *list){
    if(list == NULL)    return NULL;
    else return list->tail;
}

dplist_node_t *dpl_get_next_reference(dplist_t *list, dplist_node_t *reference){
    if(list == NULL || reference == NULL)   return NULL;
    return reference->next;
}

dplist_node_t *dpl_get_previous_reference(dplist_t *list, dplist_node_t *reference){
    if(list == NULL || reference == NULL)   return NULL;
    return reference->prev;
}

dplist_node_t *dpl_get_reference_of_element(dplist_t *list, void *element){
    if(list == NULL) return NULL;
    dplist_node_t* node = list->head;
    while(node){
        if(!list->element_compare(node->element,element)){
            return node;
        }else{
            node = node->next;
        }
    }
    return NULL;
}

int dpl_get_index_of_reference(dplist_t *list, dplist_node_t *reference){
    return dpl_get_index_of_element(list,reference->element);
}


dplist_t *dpl_insert_at_reference(dplist_t *list, void *element, dplist_node_t *reference, bool insert_copy){
    
}