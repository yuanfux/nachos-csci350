#include "syscall.h"

int main(){
        Write("\nTesting Multiple matmult\n",sizeof("\nTesting Multiple matmult\n"), ConsoleOutput);
        Exec("../test/matmult", sizeof("../test/matmult"));
        Exec("../test/matmult", sizeof("../test/matmult"));
    
}