#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

inline int mix_set_bit(int nr,int * addr)  
{  
    int mask, retval;  

    addr += nr >> 5;                 //nr大于31时，把高27的值做为当前地址的偏移，  
    mask = 1 << (nr & 0x1f);         //获取31范围内的值，并把1向左偏移该位数  
    retval = (mask & *addr) != 0;    //位置置1  
    *addr |= mask;  
    return retval;                   //返回置数值  
}  

inline int mix_clear_bit(int nr, int * addr)  
{  
    int mask, retval;  
  
    addr += nr >> 5;  
    mask = 1 << (nr & 0x1f);  
    retval = (mask & *addr) != 0;  
    *addr &= ~mask;  
    return retval;  
}  

inline int mix_test_bit(int nr, int * addr)  
{  
    int mask;  
  
    addr += nr >> 5;  
    mask = 1 << (nr & 0x1f);  
    return ((mask & *addr) != 0);  
}  