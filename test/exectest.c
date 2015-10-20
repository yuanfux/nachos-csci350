#include "syscall.h"

int main(){
        Write("\nTesting Execute\n",sizeof("\nTesting Execute\n"), ConsoleOutput);
        Exec("../test/forktest", sizeof("../test/forktest"));
        Exec("../test/forktest", sizeof("../test/forktest"));
    
}