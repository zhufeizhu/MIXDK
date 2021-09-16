#include "mixdk.h"
#include <stdio.h>
#include <string.h>


int main(){
    mixdk_init();
    char* buf1 = "hello world";

    int n = mixdk_write(buf1,strlen(buf1),0);

    printf("mix_dk write %d\n",n);

    char buf2[100];
    memset(buf2,0,100);

    n = mixdk_read(buf2,11,0);

    printf("mix_dk read %d %s\n",n,buf2);
    
}