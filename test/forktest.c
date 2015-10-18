#include "syscall.h"
int i = 1, j = 1;

void forkTest(){
        int a[] = {i};
        Print("\nForking function %d.\n\n",22,a,1);  
        i= i+1; 
        Exit(0);    
}

int
main(){
        int a[0];
        Print("\nFORK_SYSCALL TEST\n",sizeof("\nFORK_SYSCALL TEST\n"),a,0);
        Print("\n-----------------begin fork test1\n",sizeof("\n-----------------begin fork test1\n"),a,0);
        Fork(forkTest);
        Print("\n-----------------after fork test1\n",sizeof("\n-----------------begin fork test1\n"),a,0);
        Print("\n-----------------begin fork test2\n",sizeof("\n-----------------begin fork test1\n"),a,0);
        Fork(forkTest);
        Print("\n-----------------after fork test2\n",sizeof("\n-----------------begin fork test1\n"),a,0);
        Print("\n-----------------begin fork test3\n",sizeof("\n-----------------begin fork test1\n"),a,0);
        Fork(forkTest);
        Print("\n-----------------after fork test3\n",sizeof("\n-----------------begin fork test1\n"),a,0);
        Fork(0xffffff);
        Fork(0x000000);
        Print("\nFORK_SYSCALL TEST DONE\n",sizeof("\nFORK_SYSCALL TEST DONE\n"),a,0);
        Print("\n------------------------\n",sizeof("\n------------------------\n"),a,0);

        return 0;
}