#include "syscall.h"

int main(){
        Write("\nTesting Multiple sort\n",sizeof("\nTesting Multiple sort\n"), ConsoleOutput);
    
        Exec("../test/sort", sizeof("../test/sort"));
        Exec("../test/sort", sizeof("../test/sort"));
    
}