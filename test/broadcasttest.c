#include "syscall.h"
int condition0;
int lock0;

void Thread2(){
    Acquire(lock0);
    
    Write("\nThread2 Waits\n", sizeof("\nThread2 Waits\n"), ConsoleOutput);

    Wait(condition0, lock0);
    Write("\nThread2 gets Broadcasted by Thread 1\n", sizeof("\nThread2 gets Broadcasted by Thread1\n"), ConsoleOutput);
    Release(lock0);
    Exit(0);
}

void Thread3(){
    Acquire(lock0);
    
    Write("\nThread3 Waits\n", sizeof("\nThread3 Waits\n"), ConsoleOutput);

    Wait(condition0, lock0);
    Write("\nThread3 gets Broadcasted by Thread 1\n", sizeof("\nThread3 gets Broadcasted by Thread1\n"), ConsoleOutput);
    Release(lock0);
    Exit(0);
}


int main(){
    Write("\nTesting Broadcast\n",sizeof("\nTesting Broadcast\n"),ConsoleOutput);
    Write("\nCreate Condition0\n", sizeof("\nCreate Condition0\n"), ConsoleOutput);
    condition0 = CreateCondition("Condition0");
    Write("\nCreate Condition0 successfully\n", sizeof("\nCreate Condition0 successfully\n"), ConsoleOutput);
    Write("\nCreate Lock0\n", sizeof("\nCreate Lock0\n"), ConsoleOutput);
    lock0 = CreateLock("Lock0");
    Write("\nCreate Lock0 successfully\n", sizeof("\nCreate Lock0 successfully\n"), ConsoleOutput);
    Fork(Thread2);
    Fork(Thread3);
    Yield();
    Acquire(lock0);
    Write("\nThread1 Broadcasts Thread2 & Thread 3\n", sizeof("\nThread1 Broadcasts Thread2 & Thread 3\n"), ConsoleOutput);
    Broadcast(condition0, lock0);
    Release(lock0);

}