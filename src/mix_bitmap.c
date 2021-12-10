#include "mix_bitmap.h"

#include <stdlib.h>
#include <string.h>

#include "mix_log.h"

#define BITS_PER_BYTE 8

mix_bitmap_t* mix_bitmap_init(int bytes){
    mix_bitmap_t* bitmap = malloc(sizeof(mix_bitmap_t));
    if(bitmap == NULL){
        mix_log("mix_bitmap_init","malloc for bitmap failed");
        return NULL;        
    }

    bitmap->bytes = bytes;
    bitmap->next_bit = 0;
    bitmap->nvm_offset = 0;
    bitmap->array = malloc(bytes * sizeof(char));
    if(bitmap->array == NULL){
        mix_log("mi_bitmap_init","malloc for bitmap->array failed");
        free(bitmap);
        return NULL;
    }
    memset(bitmap->array,0,bytes * sizeof(char));
    return bitmap;
}

int mix_bitmap_free(mix_bitmap_t* bitmap){
    if(bitmap == NULL) return 0;

    free(bitmap->array);
    free(bitmap);
    bitmap = NULL;
    return 0;
}

/**
 * @brief 获取bitmap的下一个zero bit
 * 
 * @param bitmap 
 * @return size_t bitmap的bit位
 */
inline int mix_bitmap_next_zero_bit(mix_bitmap_t* bitmap){
    int next_zero_bit = bitmap->next_bit;
    if(mix_bitmap_test_bit(next_zero_bit,bitmap)){
        if(mix_bitmap_set_bit(next_zero_bit,bitmap)){
            bitmap->next_bit = (next_zero_bit + 1)/(bitmap->bytes * BITS_PER_BYTE);
            return next_zero_bit;
        }else{
            mix_log("mix_bitmap_next_zero_bit","set bit failed");
            return -1;
        }
    }else{
        while(!mix_bitmap_test_bit(next_zero_bit,bitmap)){
            next_zero_bit++;
        }
        if(mix_bitmap_set_bit(next_zero_bit,bitmap)){
            bitmap->next_bit = (next_zero_bit + 1)/(bitmap->bytes * BITS_PER_BYTE);
            return next_zero_bit;
        }else{
            mix_log("mix_bitmap_next_zero_bit","set bit failed");
            return -1;
        }
    }
}

int mix_bitmap_set_bit(int nr, mix_bitmap_t* bitmap){
    char mask,retval;
    char* addr = bitmap->array;
  
    addr += nr >> 3;                   //得char的index
    mask = 1 << (nr & 0x07);           //得char内的offset
    retval = (mask & *addr) != 0;    
    *addr |= mask;  
    return (int)retval;                   //返回置数值
}

int mix_bitmap_clear_bit(int nr, mix_bitmap_t* bitmap){
    char mask, retval;  
    char* addr = bitmap->array;
    addr += nr >> 3;  
    mask = 1 << (nr & 0x07);  
    retval = (mask & *addr) != 0;  
    *addr &= ~mask;  
    return (int)retval;
}

int mix_bitmap_test_bit(int nr, mix_bitmap_t* bitmap){
    char mask;
    char* addr = bitmap->array;  
    addr += nr >> 3;  
    mask = 1 << (nr & 0x07);
    return (int)((mask & *addr) != 0);  
}

/**
 * @brief 支持计数的bitmap 将每个byte分割成了2个"bit"位 每个"bit"占用了3位 即支持最大的计数
 *        大小为8 
 * @param btimap 
 * @param index 
 * @param offset 
 * @return int 
 */
int mix_counting_bitmap_increment(mix_bitmap_t* bitmap, unsigned int index, long offset){
    long access = index / 2 + offset;
    __uint8_t temp;
    __uint8_t n = bitmap->array[access];

    if (index % 2 != 0) {
        temp = (n & 0x0f);
        n = (n & 0xf0) + ((n & 0x0f) + 0x01);
    } else {
        temp = (n & 0xf0) >> 4;
        n = (n & 0x0f) + ((n & 0xf0) + 0x10);
    }
    
    if (temp == 0x0f) {
        mix_log("mix_counting_bitmap_increment","4 bit int overflow");
        return -1;
    }

    bitmap->array[access] = n;
    return 0;
}

int mix_counting_bitmap_decrement(mix_bitmap_t* bitmap, unsigned int index, long offset){
    long access = index / 2 + offset;
    __uint8_t temp;
    __uint8_t n = bitmap->array[access];

    if (index % 2 != 0) {
        temp = (n & 0x0f);
        n = (n & 0xf0) + ((n & 0x0f) - 0x01);
    } else {
        temp = (n & 0xf0) >> 4;
        n = (n & 0x0f) + ((n & 0xf0) - 0x10);
    }
    
    if (temp == 0x00) {
        mix_log("mix_counting_bitmap_decrement","decrementing zero");
        return -1;
    }
    
    bitmap->array[access] = n;
    return 0;
}

int mix_counting_bitmap_check(mix_bitmap_t* bitmap, unsigned int index, long offset){
    long access = index / 2 + offset;

    if(index % 2 != 0){
        return bitmap->array[access] & 0x0f;
    }else{
        return bitmap->array[access] & 0xf0;
    }
}