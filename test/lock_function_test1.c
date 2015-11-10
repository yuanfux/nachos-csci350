#include "syscall.h"

int lockindex;
int sychLock1; /*for sychronizing all the print message!*/

void anotherThread(){
    
    Write("\nThread 1 Trying to Acquire lock\n", sizeof("\nThread 1 Trying to Acquire lock\n"), ConsoleOutput);
    
    AcquireServer(lockindex);
     Acquire(sychLock1);/*for sychronizing all the print message!*/
    Write("\nThread 1 Acquire lock successfully\n", sizeof("\nThread 1 Acquire lock successfully\n"), ConsoleOutput);
     Release(sychLock1);/*for sychronizing all the print message!*/
    
    
    Acquire(sychLock1);/*for sychronizing all the print message!*/
    Write("\nThread 1 Trying to Release lock\n", sizeof("\nThread 1 Trying to Release lock\n"), ConsoleOutput);
    ReleaseServer(lockindex);
    Write("\nThread 1 Release lock successfully\n", sizeof("\nThread 1 Release lock successfully\n"), ConsoleOutput);
    Release(sychLock1);/*for sychronizing all the print message!*/
    
    Exit(0);
}



int main(){
    sychLock1= CreateLock("lock");
      Write("\nThread 0 Trying to Create a lock with name: lock\n", sizeof("\nThread 0 Trying to Create a lock with name: lock\n"), ConsoleOutput);
    
    lockindex = CreateLockServer("lock", sizeof("lock"));
    
     Write("\nThread 0 Create lock successfully\n", sizeof("\nThread 0 Create lock successfully\n"), ConsoleOutput);
    
     Write("\nThread 0 Trying to Acquire lock\n", sizeof("\nThread 0 Trying to Acquire lock\n"), ConsoleOutput);
    
    AcquireServer(lockindex);
    
    Write("\nThread 0 Acquire lock successfully\n", sizeof("\nThread 0  Acquire lock successfully\n"), ConsoleOutput);
    
    Fork(anotherThread);
    
    Yield();
    Yield();
    Yield();

    Acquire(sychLock1);/*for sychronizing all the print message!*/
    Write("\nThread 0 Trying to Release lock\n", sizeof("\nThread 0 Trying to Release lock\n"), ConsoleOutput);
    ReleaseServer(lockindex);
    Write("\nThread 0 Release lock successfully\n", sizeof("\nThread 0 Release lock  successfully\n"), ConsoleOutput);
    Release(sychLock1);/*for sychronizing all the print message!*/
    
}

