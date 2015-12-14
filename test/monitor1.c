#include "syscall.h"

int monitorArrayIndex;
int index;
int rv;

int main(){
    
    monitorArrayIndex = CreateMVArrayServer("monitorA", sizeof("monitorA"), 5);
    
    Write("\nMachine 1: Monitor Array created with index: ", sizeof("\nMachine 1: Monitor Array created with index: "), ConsoleOutput);
    
    Printint(monitorArrayIndex);
    
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    rv = GetMVArrayServer(monitorArrayIndex, 2);
    
    Write("\nMachine 1: The value of monitor array from server is : ", sizeof("\nMachine 1: The value of monitor array from server is : "), ConsoleOutput);
    Printint(rv);
    Write("\n", sizeof("\n"), ConsoleOutput);
    
    
    SetMVArrayServer(monitorArrayIndex, 3, 38);
    
    rv = GetMVArrayServer(monitorArrayIndex, 3);
    
    Write("\nMachine 1: The value of monitor array from server is : ", sizeof("\nMachine 1: The value of monitor array from server is : "), ConsoleOutput);
    Printint(rv);
    Write("\n", sizeof("\n"), ConsoleOutput);

    
    
}