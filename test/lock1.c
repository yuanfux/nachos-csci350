#include "syscall.h"

int lockindex;
void main(){
    char* buf = "lock ";
    buf[4] = '0'+79;
    
     Write(buf, 5, ConsoleOutput);
    lockindex = CreateLockServer("lock", sizeof("lock"));
    Write("\nC1: lock index: ", sizeof("\nC1: lock index: "), ConsoleOutput);
    Printint(lockindex);
    
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nC1 Trying to Acquire lock\n", sizeof("\nC1 Trying to Acquire lock\n"), ConsoleOutput);
    
    
    AcquireServer(lockindex);
    
    Write("\nC1 Acquired lock\n", sizeof("\nC1 Acquired lock\n"), ConsoleOutput);
    Write("\nC1 never Released lock\n", sizeof("\nC1 never Released lock\n"), ConsoleOutput);

  /*  Write("\nC1 Trying to Release lock\n", sizeof("\nC1 Trying to Release lock\n"), ConsoleOutput);
    
    ReleaseServer(lockindex);
    
    Write("\nC1 Released lock\n", sizeof("\nC1 Released lock\n"), ConsoleOutput);*/
    



}