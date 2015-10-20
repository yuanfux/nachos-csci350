#include "syscall.h"

int main(){
        Write("\nTesting Yield\n",sizeof("\nTesting Yield\n"),ConsoleOutput);
        Yield();
        Write("\nTesting Yield successfully\n",sizeof("\nTesting Yield successfully\n"),ConsoleOutput);

}