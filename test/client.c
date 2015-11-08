#include "syscall.h"

int main(){
    int lockindex = CreateLockServer("lock1", sizeof("lock1"));
    
    Write("\nCreate a lock with index: ", sizeof("\nCreate a lock with index: "), ConsoleOutput);
    Printint(lockindex);
     Write("\n", sizeof("\n"), ConsoleOutput);
    
}