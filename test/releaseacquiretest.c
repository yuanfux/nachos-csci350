#include "syscall.h"

int lock0;

int main(){
        Write("\nTesting Release & Acquire\n",sizeof("\nTesting Release & Acquire\n"),ConsoleOutput);
        Write("\nTrying to release an non existing lock\n", sizeof("\nTrying to release an non existing lock\n"), ConsoleOutput);
        Release(0);
        Write("\nCreate Lock0\n", sizeof("\nCreate Lock0\n"), ConsoleOutput);
        lock0=CreateLock("Lock0");
        Write("\nCreate Lock0 successfully\n", sizeof("\nCreate Lock0 successfully\n"), ConsoleOutput);
        Write("\nTrying to release an non acquired lock0\n", sizeof("\nTrying to release an non acquired lock0\n"), ConsoleOutput);
        Release(lock0);
        Write("\nAcquire lock0\n", sizeof("\nAcquire lock0\n"), ConsoleOutput);
        Acquire(lock0);
        Write("\nAcquire lock0 successfully\n", sizeof("\nAcquire lock0 successfully\n"), ConsoleOutput);
        Write("\nRelease lock0\n", sizeof("\nRelease lock0\n"), ConsoleOutput);
        Release(lock0);
        Write("\nRelease lock0 successfully\n", sizeof("\nRelease lock0 successfully\n"), ConsoleOutput);
        Write("\nTesting Release & Acquire finished\n",sizeof("\nTesting Release & Acquire finished\n"),ConsoleOutput);
}