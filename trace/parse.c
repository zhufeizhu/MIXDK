#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv){
    FILE* file = fopen(argv[1],"r");
    if(file == NULL) {
        printf("no file\n");
        return 0;
    }

    int fd = open("output.txt",O_CREAT|O_WRONLY);
    if(fd <= 0){
        printf("create output.txt failed\n");
        return 0;
    }

    char type;
    char s1[5];
    char s2[20];
    char s3[10];
    long long write_block_num = 0;
    long long read_block_num = 0;
    while(!feof(file)){
        fscanf(file,"%[^','],%[^','],%[^','],%s\n",&type,s1,s2,s3);
        if(type == 'Q' && (s1[0] == 'W' || s1[0] =='R')){
            //fprintf(output,"%d,%d\n",offset/8,num/8);
            long long offset = atoll(s2)/8;
            int n = atoi(s3)/8;
            if(s1[0] == 'W'){
                write_block_num += n;
            }else{
                read_block_num += n;
            }
            printf("%c,%lld,%d\n",s1[0],offset,n);
        }
    }
    printf("write blocks num is %lld, read block num is %lld\n",write_block_num,read_block_num);
    return 0;
}