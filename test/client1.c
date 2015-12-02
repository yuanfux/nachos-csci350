#include "syscall.h"

int lockindex;
int conditionindex;
int i;
void main() {

    lockindex = CreateLockServer("lock", sizeof("lock"));

    Write("\nC1: lock index: ", sizeof("\nC1: lock index: "), ConsoleOutput);

    Printint(lockindex);

    Write("\n", sizeof("\n"), ConsoleOutput);


    conditionindex = CreateConditionServer("condition", sizeof("condition"));

    Write("\nC1: condition index: ", sizeof("\nC1: condition index: "), ConsoleOutput);

    Printint(conditionindex);

    Write("\n", sizeof("\n"), ConsoleOutput);



    Write("\nC1 Trying to Acquire lock\n", sizeof("\nC1 Trying to Acquire lock\n"), ConsoleOutput);


    AcquireServer(lockindex);

    Write("\nC1 Acquired lock\n", sizeof("\nC1 Acquired lock\n"), ConsoleOutput);

    Write("\nC1 Trying to Wait\n", sizeof("\nC1 Trying to Wait\n"), ConsoleOutput);

    
    WaitServer(conditionindex, lockindex);
    
    Write("\nC1 Gets signaled\n", sizeof("\nC1 Gets signaled\n"), ConsoleOutput);

    Write("\nC1 Trying to Release lock\n", sizeof("\nC1 Trying to Release lock\n"), ConsoleOutput);

    ReleaseServer(lockindex);

    Write("\nC1 Released lock\n", sizeof("\nC1 Released lock\n"), ConsoleOutput);



}
