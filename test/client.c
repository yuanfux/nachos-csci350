#include "syscall.h"

int main(){
    int lockindex = CreateLockServer("lock1", sizeof("lock1"));
    
    Write("\nCreate a lock with index: ", sizeof("\nCreate a lock with index: "), ConsoleOutput);
    Printint(lockindex);
     Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nTrying to Acquire lock0\n", sizeof("\nTrying to Acquire lock0\n"), ConsoleOutput);
    
    AcquireServer(0);
    
     Write("\nTrying to Release lock0\n", sizeof("\nTrying to Release lock0\n"), ConsoleOutput);
    
    ReleaseServer(0);
    
}