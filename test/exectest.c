#include "syscall.h"

int
main(){
        int a[0];
        Print("\nEXEC_SYSCALL TEST\n",sizeof("\nEXEC_SYSCALL TEST\n"),a,0);
        Exec("../test/forktest", sizeof("../test/forktest"));
        Exec("../test/forktest", sizeof("../test/forktest"));
        Print("\nEXEC_SYSCALL TEST DONE\n",sizeof("\nEXEC_SYSCALL TEST DONE\n"),a,0);
        return 0;
}