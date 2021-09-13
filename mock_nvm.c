#include "mock_nvm.h"

int mix_mock_nvm_init(){
    
}

unsigned int mix_mock_nvm_read(void* dst, unsigned int len, unsigned int offset){
    printf("type: read nvm, len: %d, offset: %d\n");
}

unsigned int mix_mock_nvm_write(void* src, unsigned int len, unsigned int offset){
    printf("type: write nvm, len: %d, offset: %d\n");
}