#include "syscall.h"

int main(){
    
    Write("\nMachine 2 Trying to Acquire lock\n", sizeof("\nMachine 2 Trying to Acquire lock\n"), ConsoleOutput);
    
    AcquireServer(0);
    
    Write("\nMachine 2 Acquire lock successfully\n", sizeof("\nMachine 2  Acquire lock successfully\n"), ConsoleOutput);
    
    Write("\nMachine 2 Trying to Release lock0\n", sizeof("\nMachine 2 Trying to Release lock0\n"), ConsoleOutput);
    ReleaseServer(0);
    
    Write("\nMachine 2 Release lock0 successfully\n", sizeof("\nMachine 2 Release lock0  successfully\n"), ConsoleOutput);
    
}