#include "syscall.h"

int main(){
    Write("\nTestSuit Starts\n");
    Exec("../test/yieldtest", sizeof("../test/yieldtest"));
    Exec("../test/exittest", sizeof("../test/exittest"))
    Exec("../test/forktest", sizeof("../test/forktest"));
    Exec("../test/exectest", sizeof("../test/exectest"));
    Exec("../test/releaseacquiretest", sizeof("../test/releaseacquiretest"));
    Exec("../test/signalwaittest", sizeof("../test/signalwaitforktest"));
    Exec("../test/randomtest", sizeof("../test/randomtest"));
    
}