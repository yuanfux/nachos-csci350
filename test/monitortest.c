#include "syscall.h"

int main(){
    int buffer;
    int buffer2;
    int i;
    Write("\nTesting Monitor Variable Syscall\n",sizeof("\nTesting Monitor Variable Syscall\n"),ConsoleOutput);
    buffer = 500;
    buffer2 = 38;
    CreateMV(buffer);
    
    CreateMV(buffer2);
    Printint(GetMV(0));
    Write("\n",sizeof("\n"),ConsoleOutput);
    Printint(GetMV(1));
    Write("\n",sizeof("\n"),ConsoleOutput);
    
    SetMV(0,1000);
    Printint(GetMV(0));
    Write("\n",sizeof("\n"),ConsoleOutput);
    Printint(GetMV(1));
    Write("\n",sizeof("\n"),ConsoleOutput);
    
    for(i = 0; i < 10 ; i++){
        CreateMV(i);
    }
    
    for(i = 0; i < 12 ; i++){
        Printint(GetMV(i));
    }
    
    
    Write("\nTesting Monitor Variable Syscall Finish\n",sizeof("\nTesting Monitor Variable Syscall Finish\n"),ConsoleOutput);

}