#include "syscall.h"

int monitorIndex;
int rv;


int main(){

    Write("\nMachine 3: Trying to Create a monitor with same name: monitor\n", sizeof("\nMachine 3: Trying to Create a monitor with same name: monitor\n"), ConsoleOutput);

    monitorIndex = CreateMVServer("monitor", sizeof("monitor"), 55);

   Write("\nMachine 3: Monitor index returned : ", sizeof("\nMachine 3: Monitor index returned: "), ConsoleOutput);

    Printint(monitorIndex);
    
    Write("\nMachine 3: Trying to get the monitor", sizeof("\nMachine 3: Trying to get the monitor\n"), ConsoleOutput);
    
    rv = GetMVServer(monitorIndex);
    
    Write("\nMachine 3: The value of monitor from server is : ", sizeof("\nMachine 3: The value of monitor from server is : "), ConsoleOutput);
    Printint(rv);
    
       Write("\n", sizeof("\n"), ConsoleOutput);
    

}