#include "syscall.h"


int main(){

    int i;
    for(i = 0 ; i< 5;i++){
        Exec("../test/Cashier", sizeof("../test/Cashier"));
    }
    
    
    
}