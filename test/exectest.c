#include "syscall.h"

int
main(){
        int a[0];
        Print("\nEXEC_SYSCALL TEST\n",sizeof("\nEXEC_SYSCALL TEST\n"),a,0);
        Exec("../test/releasetest");
        Exec("../test/releasetest");
        Exit(0);
}