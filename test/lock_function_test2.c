#include "syscall.h"

int main(){
    int lockindex;
    
    Write("\nTrying to Create a lock with name: lock0\n", sizeof("\nTrying to Create a lock with name: lock0\n"), ConsoleOutput);
    
    lockindex = CreateLockServer("lock0", sizeof("lock0"));
    
    Write("\nLock created with lock index: ", sizeof("\nLock created with lock index: "), ConsoleOutput);
    
    Printint(lockindex);
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nTrying to Acquire a non-existing lock100\n", sizeof("\nTrying to Acquire a non-existing lock100\n"), ConsoleOutput);
    
    AcquireServer(100);
    
    
    Write("\nTrying to Acquire lock0\n", sizeof("\nTrying to Acquire lock0\n"), ConsoleOutput);
    
    AcquireServer(lockindex);
    
    Write("\nAcquire lock0 successfully\n", sizeof("\nAcquire lock0 successfully\n"), ConsoleOutput);
    
    Write("\nTrying to Acquire lock0 again\n", sizeof("\nTrying to Acquire lock0 again\n"), ConsoleOutput);
    
    AcquireServer(lockindex);
    
    Write("\nTrying to Release a non-existing lock100\n", sizeof("\nTrying to Release a non-existing lock100\n"), ConsoleOutput);
    
    ReleaseServer(100);
    
    Write("\nTrying to Release lock0\n", sizeof("\nTrying to Release lock0\n"), ConsoleOutput);
    
    ReleaseServer(lockindex);
    
    Write("\nRelease lock0 successfully\n", sizeof("\nRelease lock0 successfully\n"), ConsoleOutput);
    
    Write("\nTrying to Destroy lock0\n", sizeof("\nTrying to Destroy lock0\n"), ConsoleOutput);
    
    DestroyLockServer(lockindex);
    
    Write("\nDestroy lock0 successfully\n", sizeof("\nDestroy lock0 successfully\n"), ConsoleOutput);
    
    
    
}