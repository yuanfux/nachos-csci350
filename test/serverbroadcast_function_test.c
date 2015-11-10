#include "syscall.h"
int condition0;
int lock0;

void Thread3(){
    AcquireServer(lock0);
    
    Write("\nThread3 Broadcasts Thread 1 & 2\n", sizeof("\nThread3 Broadcasts Thread 1 & 2\n"), ConsoleOutput);
    
    BroadcastServer(condition0, lock0);
    
    ReleaseServer(lock0);
    Exit(0);
}

void Thread2(){
    AcquireServer(lock0);
    Fork(Thread3);
    Write("\nThread2 Waits\n", sizeof("\nThread2 Waits\n"), ConsoleOutput);
    WaitServer(condition0, lock0);
    Write("\nThread2 gets Broadcasted by Thread3\n", sizeof("\nThread2 gets Broadcasted by Thread3\n"), ConsoleOutput);
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

    AcquireServer(lock0);
    Fork(Thread2);
    Write("\nThread1 Waits\n", sizeof("\nThread1 Waits\n"), ConsoleOutput);
     WaitServer(condition0, lock0);
     Write("\nThread1 gets Broadcasted by Thread3\n", sizeof("\nThread1 gets Broadcasted by Thread3\n"), ConsoleOutput);
    ReleaseServer(lock0);
    
}