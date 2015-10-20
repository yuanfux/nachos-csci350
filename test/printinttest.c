#include "syscall.h"
int h;

int main(){
    Write("\nTesting Printint\n",sizeof("\nTesting Printint\n"), ConsoleOutput);
    h=100;
    Printint(h);
    Write("\n", sizeof("\n"), ConsoleOutput);
    Write("\nTesting Printint finished\n",sizeof("\nTesting Printint finished\n"), ConsoleOutput);
    
}