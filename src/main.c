#include "mixdk.h"

int main(){
    mixdk_init();
    char buffer1[100];
    char buffer2[100];
    int i = 100000;
    while(i > 0){
        if(!mixdk_write(buffer1,10,0)){
            printf("write succeed!");
        }
        mixdk_read(buffer2,10,0);
    }
    
}