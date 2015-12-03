#include "syscall.h"


int main(){
    int i;
    
    for(i = 0 ; i< 1;i++){
        Exec("../test/ApplicationClerk", sizeof("../test/ApplicationClerk"));
    }
    
    
}