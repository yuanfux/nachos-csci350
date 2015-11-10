#include "syscall.h"

int monitorIndex;
int rv;

int main(){
    Write("\nMachine 1: Trying to Create a monitor with name: monitor value: 55\n", sizeof("\nMachine 1: Trying to Create a monitor with name: monitor value: 55\n"), ConsoleOutput);
    
    monitorIndex = CreateMVServer("monitor", sizeof("monitor"), 55);
    
    Write("\nMachine 1: Monitor created with monitor index: ", sizeof("\nMachine 1: Monitor created with Monitor index: "), ConsoleOutput);
    
    Printint(monitorIndex);
    
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nMachine 1: Trying to get the monitor", sizeof("\nMachine 1: Trying to get the monitor\n"), ConsoleOutput);
    
    rv = GetMVServer(monitorIndex);
    
    Write("\nMachine 1: The value of monitor from server is : ", sizeof("\nMachine 1: The value of monitor from server is : "), ConsoleOutput);
    Printint(rv);
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    Write("\nMachine 1: Trying to get a non-existing monitor with index: 100\n", sizeof("\nMachine 1: Trying to get a non-existing monitor with index: 100\n"), ConsoleOutput);
    
    GetMVServer(100);
    
    Write("\nMachine 1: Trying to set a non-existing monitor with index: 100\n", sizeof("\nMachine 1: Trying to set a non-existing monitor with index: 100\n"), ConsoleOutput);
    
    SetMVServer(100, 38);
    
    
}