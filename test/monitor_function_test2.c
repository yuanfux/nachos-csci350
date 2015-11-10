#include "syscall.h"

int monitorIndex;
int rv;

int main(){
    Write("\nMachine 2: Trying to Create a monitor with same name: monitor\n", sizeof("\nMachine 2: Trying to Create a monitor with same name: monitor\n"), ConsoleOutput);
    
    monitorIndex = CreateMVServer("monitor", sizeof("monitor"), 55);

    Write("\nMachine 2: Monitor index returned : ", sizeof("\nMachine 2: Monitor index returned: "), ConsoleOutput);
    
    Printint(monitorIndex);
    
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nMachine 2: Trying to set the monitor with value: 1000\n", sizeof("\nMachine 2: Trying to set the monitor with value: 1000\n"), ConsoleOutput);
    
    SetMVServer(monitorIndex, 1000);
    
     Write("\nMachine 2: Trying to get the monitor\n", sizeof("\nMachine 2: Trying to get the monitor\n"), ConsoleOutput);
    
    rv = GetMVServer(monitorIndex);
    
    Write("\nMachine 2: The value of monitor from server is : ", sizeof("\nMachine 2: The value of monitor from server is : "), ConsoleOutput);
    Printint(rv);
    
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    
    
    
}