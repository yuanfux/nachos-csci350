#include "syscall.h"

int main(){
    int i;
    Write("\nTestSuit Starts(note: concurrency may casuse messy outputs)\n", sizeof("\nTestSuit Starts(note: concurrency may casuse messy outputs)\n"), ConsoleOutput);
    Exec("../test/yieldtest", sizeof("../test/yieldtest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/forktest", sizeof("../test/forktest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/exec_exittest", sizeof("../test/exec_exittest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/release_acquiretest", sizeof("../test/release_acquiretest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/destroy_condition_locktest", sizeof("../test/destroy_condition_locktest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/signal_waittest", sizeof("../test/signal_waitforktest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/broadcasttest", sizeof("../test/broadcasttest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/randomtest", sizeof("../test/randomtest"));
    for(i = 0; i<20;i++){
        Yield();
    }
    Exec("../test/printinttest", sizeof("../test/printinttest"));
}