#include "syscall.h"


int main(){
    int i;
    for(i = 0 ; i< 10;i++){
        Exec("../test/Senator", sizeof("../test/Senator"));
    }
    
    
    
}