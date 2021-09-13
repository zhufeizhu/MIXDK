#include "mixdk.h"

int main(){
    mixdk_init();
    char buffer1[100];
    char buffer2[100];
    mixdk_write(buffer1,0,10);
    mixdk_read(buffer2,0,10);
}