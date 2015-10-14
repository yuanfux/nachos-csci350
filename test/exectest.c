#include "syscall.h"

int
main(){
        int a[0];
        Exec("FUDGE");
        Exec("../test/testFork");
        Print("\nEXEC_SYSCALL TEST\n",sizeof("\nEXEC_SYSCALL TEST\n"),a,0);
        Exec("../test/testFork");
}