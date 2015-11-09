#include "syscall.h"
int condition0;
int lock0;

void Thread2(){
    AcquireServer(lock0);
    
    Write("\nThread2 Waits\n", sizeof("\nThread2 Waits\n"), ConsoleOutput);
    
    WaitServer(condition0, lock0);
    Write("\nThread2 gets Broadcasted by Thread 1\n", sizeof("\nThread2 gets Broadcasted by Thread1\n"), ConsoleOutput);
    ReleaseServer(lock0);
    Exit(0);
}

void Thread3(){
    AcquireServer(lock0);
    
    Write("\nThread3 Waits\n", sizeof("\nThread3 Waits\n"), ConsoleOutput);
    
    WaitServer(condition0, lock0);
    Write("\nThread3 gets Broadcasted by Thread 1\n", sizeof("\nThread3 gets Broadcasted by Thread1\n"), ConsoleOutput);
    ReleaseServer(lock0);
    Exit(0);
}


int main(){
    Write("\nTesting BroadcastServer\n",sizeof("\nTesting BroadcastServer\n"),ConsoleOutput);
    Write("\nCreate Condition0\n", sizeof("\nCreate Condition0\n"), ConsoleOutput);
    condition0 = CreateConditionServer("Condition0", sizeof("Condition0"));
    Write("\nCreate Condition0 successfully\n", sizeof("\nCreate Condition0 successfully\n"), ConsoleOutput);
    Write("\nCreate Lock0\n", sizeof("\nCreate Lock0\n"), ConsoleOutput);
    lock0 = CreateLockServer("Lock0", sizeof("Lock0"));
    Write("\nCreate Lock0 successfully\n", sizeof("\nCreate Lock0 successfully\n"), ConsoleOutput);
    Fork(Thread2);
    Fork(Thread3);
    Yield();
    Yield();
    Yield();
    AcquireServer(lock0);
    Write("\nThread1 Broadcasts Thread2 & Thread 3\n", sizeof("\nThread1 Broadcasts Thread2 & Thread 3\n"), ConsoleOutput);
    BroadcastServer(condition0, lock0);
    ReleaseServer(lock0);
    
}