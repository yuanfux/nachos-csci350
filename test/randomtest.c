#include "syscall.h"

int main(){
    Write("\nRANDOM_SYSCALL TEST: ",sizeof("\nRANDOM_SYSCALL TEST:"),ConsoleOutput);
    Printint(Random(4));
    Write("\n",sizeof("\n"),ConsoleOutput);
    
}