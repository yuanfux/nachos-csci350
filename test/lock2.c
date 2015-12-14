#include "syscall.h"

int lockindex;
void main(){
    lockindex = CreateLockServer("lock", sizeof("lock"));
    Write("\nC2: lock index: ", sizeof("\nC2: lock index: "), ConsoleOutput);
    Printint(lockindex);
    
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nC2 Trying to Acquire lock\n", sizeof("\nC2 Trying to Acquire lock\n"), ConsoleOutput);
    
    
    AcquireServer(lockindex);
    
    Write("\nC2 Acquired lock\n", sizeof("\nC2 Acquired lock\n"), ConsoleOutput);
    
    
    Write("\nC2 Trying to Release lock\n", sizeof("\nC2 Trying to Release lock\n"), ConsoleOutput);
    
    ReleaseServer(lockindex);
    
    Write("\nC2 Released lock\n", sizeof("\nC2 Released lock\n"), ConsoleOutput);

    
    
}