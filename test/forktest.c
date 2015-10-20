#include "syscall.h"
int count = 0;

void forkTest(){
        Write("\nforkTest: ", sizeof("\nforkTest: "), ConsoleOutput);
        Printint(count);
        Write("\n", sizeof("\n"), ConsoleOutput);
        count= count+1;
        Exit(0);    
}

int main(){
        Write("\nTesting Fork\n", sizeof("\nTesting Fork\n"), ConsoleOutput);
        Write("\nTrying to Fork a Thread with address 0x00000\n", sizeof("\nTrying to Fork a Thread with address 0x00000\n"), ConsoleOutput);
        Fork(0x00000);
        Write("\nTrying to Fork a Thread with address 0xFFFFF\n", sizeof("\nTrying to Fork a Thread with address 0xFFFFF\n"), ConsoleOutput);
        Fork(0xFFFFF);
        Fork(forkTest);
        Fork(forkTest);
        Fork(forkTest);
}