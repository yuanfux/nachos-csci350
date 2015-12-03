#include "syscall.h"


int main(){
    int i;
    for(i = 0 ; i< 2;i++){
        Exec("../test/Customer", sizeof("../test/Customer"));
    }
    
    
    
}