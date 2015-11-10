#include "syscall.h"

int conditionIndex;
int lockIndex;

int anotherThread(){
    Write("\nThread 1 Trying to Acquire lock\n", sizeof("\nThread 1 Trying to Acquire lock\n"), ConsoleOutput);
    
    AcquireServer(lockIndex);
    
    
    Write("\nThread 1 Acquire lock successfully\n", sizeof("\nThread 1 Acquire lock successfully\n"), ConsoleOutput);
    
    Write("\nThread 1 Trying to Signal to a non-existing condition\n", sizeof("\nThread 1 Trying to Signal to a non-existing condition\n"), ConsoleOutput);
    
    SignalServer(100, lockIndex);
    
    
    Write("\nThread 1 Signals Thread 0\n", sizeof("\nThread 1 Signals Thread 0\n"), ConsoleOutput);
    
    SignalServer(conditionIndex, lockIndex);
    
    Write("\nThread 1 Trying to Release lock\n", sizeof("\nThread 1 Trying to Release lock\n"), ConsoleOutput);
    
    
    ReleaseServer(lockIndex);
    
    Write("\nThread 1 Release lock successfully\n", sizeof("\nThread 1 Release lock successfully\n"), ConsoleOutput);
    
    
    Exit(0);
    
}


int main(){
    Write("\nThread 0 Trying to Create a condition with name: condition\n", sizeof("\nThread 0 Trying to Create a condition with name: condition\n"), ConsoleOutput);
    
    conditionIndex = CreateConditionServer("condition", sizeof("condition"));
    
    Write("\nThread 0 Create condition successfully\n", sizeof("\nThread 0 Create condition successfully\n"), ConsoleOutput);
    
    Write("\nThread 0 Trying to Create a lock with name: lock\n", sizeof("\nThread 0 Trying to Create a lock with name: lock\n"), ConsoleOutput);
    
    lockIndex = CreateLockServer("lock", sizeof("lock"));
    
    Write("\nThread 0 Create lock successfully\n", sizeof("\nThread 0 Create lock successfully\n"), ConsoleOutput);
    
     Write("\nThread 0 Trying to Acquire lock\n", sizeof("\nThread 0 Trying to Acquire lock\n"), ConsoleOutput);
    
    AcquireServer(lockIndex);
    
    
    Write("\nThread 0 Acquire lock successfully\n", sizeof("\nThread 0 Acquire lock successfully\n"), ConsoleOutput);
    
    /*condiiton out of boundary*/
    Write("\nThread 0 Trying to wait on a non-existing condition\n", sizeof("\nThread 0 Trying to wait on a non-existing condition\n"), ConsoleOutput);
    
    WaitServer(100, lockIndex);
    
    
    Fork(anotherThread);
    
    Write("\nThread 0 Trying to Wait now \n", sizeof("\nThread 0 Trying to Wait now\n"), ConsoleOutput);
    
    WaitServer(conditionIndex, lockIndex);
    
    
    Write("\nThread 0 gets Signaled by Thread 1\n", sizeof("\nThread 0 gets Signaled by Thread 1\n"), ConsoleOutput);
    
    
    
    Write("\nThread 0 Trying to Release lock\n", sizeof("\nThread 0 Trying to Release lock\n"), ConsoleOutput);
    
    
    ReleaseServer(lockIndex);
    
    Write("\nThread 0 Release lock successfully\n", sizeof("\nThread 0 Release lock successfully\n"), ConsoleOutput);
    
    Write("\nThread 0 Trying to Destroy lock\n", sizeof("\nThread 0 Trying to Destroy lock\n"), ConsoleOutput);
    
    
    DestroyLockServer(lockIndex);
    
    Write("\nThread 0 Destroy lock successfully\n", sizeof("\nThread 0 Destroy lock successfully\n"), ConsoleOutput);
    
    Write("\nThread 0 Trying to Destroy condition\n", sizeof("\nThread 0 Trying to Destroy condition\n"), ConsoleOutput);
    
    
    DestroyConditionServer(conditionIndex);
    
    Write("\nThread 0 Destroy condition successfully\n", sizeof("\nThread 0 Destroy condition successfully\n"), ConsoleOutput);
    
    
}