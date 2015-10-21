#include "syscall.h"
int condition0;
int lock0;

void Thread2(){
    Acquire(lock0);
    Signal(condition0, lock0);
    Write("\nThread2 Signals Threads1\n", sizeof("\nThread2 Signals Threads1\n"), ConsoleOutput);
    Release(lock0);
    Exit(0);
}



int main(){
        Write("\nTesting Signal & Wait\n",sizeof("\nTesting Signal & Wait\n"),ConsoleOutput);
        Write("\nCreate Condition0\n", sizeof("\nCreate Condition0\n"), ConsoleOutput);
        condition0 = CreateCondition("Condition0");
        Write("\nCreate Condition0 successfully\n", sizeof("\nCreate Condition0 successfully\n"), ConsoleOutput);
        Write("\nCreate Lock0\n", sizeof("\nCreate Lock0\n"), ConsoleOutput);
        lock0 = CreateLock("Lock0");
        Write("\nCreate Lock0 successfully\n", sizeof("\nCreate Lock0 successfully\n"), ConsoleOutput);
        Fork(Thread2);
        Acquire(lock0);
        Write("\nThread1 Waits\n", sizeof("\nThread1 Waits\n"), ConsoleOutput);
        Wait(condition0, lock0);
        Write("\nThread1 gets Signaled by Threads2\n", sizeof("\nThread1 gets Signaled by Threads2\n"), ConsoleOutput);
        Write("\nTrying to signal an non existing condition\n", sizeof("\nTrying to signal an non existing condition\n"), ConsoleOutput);
        Signal(10,lock0);
        Write("\nTrying to signal an non existing lock\n", sizeof("\nTrying to signal an non existing lock\n"), ConsoleOutput);
        Signal(condition0,10);
        Write("\nTrying to signal an non waited condition\n", sizeof("\nTrying to signal an non waited condition\n"), ConsoleOutput);
        Signal(condition0,lock0);
        Write("\nTrying to wait an non existing condition\n", sizeof("\nTrying to wait an non existing condition\n"), ConsoleOutput);
        Wait(10,lock0);
        Write("\nTrying to wait an non existing lock\n", sizeof("\nTrying to wait an non existing lock\n"), ConsoleOutput);
        Wait(condition0,10);
        Write("\nTesting Signal & Wait finished\n",sizeof("\nTesting Signal & Wait finished\n"),ConsoleOutput);
}