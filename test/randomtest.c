#include "syscall.h"

int main(){
    Write("\nTesting Random(0 <= num <= 3): ",sizeof("\nTesting Random(0 <= num <= 3): "),ConsoleOutput);
    Printint(Random(4));
    Write("\n",sizeof("\n"),ConsoleOutput);
    
}