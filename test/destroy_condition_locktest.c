#include "syscall.h"

int lock0;
int condition0;

int main(){
    Write("\nTesting Destroy Condition & Lock\n",sizeof("\nTesting Destroy Condition & Lock\n"),ConsoleOutput);
    
    Write("\nTrying to destroy an non existing lock\n", sizeof("\nTrying to destroy an non existing lock\n"), ConsoleOutput);
    DestroyLock(50);
    
    Write("\nTrying to destroy an non existing condition\n", sizeof("\nTrying to destroy an non existing condition\n"), ConsoleOutput);
    DestroyCondition(50);
    
    Write("\nCreate Lock0\n", sizeof("\nCreate Lock0\n"), ConsoleOutput);
    lock0=CreateLock("Lock0");
    Write("\nCreate Lock0 successfully\n", sizeof("\nCreate Lock0 successfully\n"), ConsoleOutput);
    
    
    Write("\nDestroy Lock0\n", sizeof("\nDestroy Lock0\n"), ConsoleOutput);
    DestroyLock(lock0);
    Write("\nDestroy Lock0 successfully\n", sizeof("\nDestroy Lock0 successfully\n"), ConsoleOutput);
    
    Write("\nCreate Condition0\n", sizeof("\nCreate Condition0\n"), ConsoleOutput);
    condition0=CreateCondition("Condition0");
    Write("\nCreate Condition0 successfully\n", sizeof("\nCreate Condition0 successfully\n"), ConsoleOutput);
    
    
    Write("\nDestroy Condition0\n", sizeof("\nDestroy Condition0\n"), ConsoleOutput);
    DestroyCondition(condition0);
    Write("\nDestroy Condition0 successfully\n", sizeof("\nDestroy Condition0 successfully\n"), ConsoleOutput);

    
    
    
    Write("\nTesting Destroy Condition & Lock finished\n",sizeof("\nTesting Destroy Condition & Lock finished\n"),ConsoleOutput);

}