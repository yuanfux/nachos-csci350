// exception.cc
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
#include <stdlib.h>
#include <time.h>
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include "synch.h"
#include "network.h"
#include "../network/post.h"

extern "C" { int bzero(char *, int); };
using namespace std;

#define MAX_NUM_LOCK 1000
#define MAX_NUM_CONDITION 1000
#define MAX_NUM_MV 10000
Table lockTable(MAX_NUM_LOCK);
Table cvTable(MAX_NUM_CONDITION);
Table mvTable(MAX_NUM_MV);
Lock* forkLock = new Lock("forkLock");
Lock* executeLock = new Lock("execLock");
Lock* exitLock = new Lock("exitLock");

int currentTLB = 0;

struct MonitorVariable {
    int variable;
    int index;

    MonitorVariable(int data) {

        variable = data;

    }

};

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n = 0;          // The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
        result = machine->ReadMem( vaddr, 1, paddr );
        while (!result) // FALL 09 CHANGES
        {
            result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
        }

        buf[n++] = *paddr;

        if ( !result ) {
            //translation failed
            return -1;
        }

        vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n = 0;          // The number of bytes copied in

    while ( n >= 0 && n < len) {
        // Note that we check every byte's address
        result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

        if ( !result ) {
            //translation failed
            return -1;
        }

        vaddr++;
    }

    return n;
}

typedef int SpaceId;

void Exit_Syscall(int status) {
    executeLock->Acquire();

    AddrSpace* addressSpace = currentThread->space;

    //has more than 1 thread in current process
    if (addressSpace->GetNumThread() > 1) {

        //addressSpace->DeallocateSpaceForThread();
        addressSpace->UpdateThreadNum();
        executeLock->Release();
        currentThread->Finish();

    }
    //the main thread case
    else if (addressSpace->GetNumThread() == 1) {
        //if this is the last process
        if (processTable.GetNumElements() == 1) {
            executeLock->Release();
            interrupt->Halt();
        }
        else if (processTable.GetNumElements() > 1 ) {
            //addressSpace->UpdateThreadNum();
            processTable.Remove(addressSpace->GetSpaceID());
            // addressSpace->DeallocateSpaceForThread();
            executeLock->Release();
            currentThread->Finish();
        }
    }
    //(addressSpace->GetNumThread() <= 0)
    else {
        // printf("Error: number of threads is %d\n", addressSpace->GetNumThread());
        executeLock->Release();
        interrupt->Halt();
    }


}


void exec_thread(int virtualAddress) {
    executeLock->Acquire();
    //initialize register
    currentThread->space->InitRegisters();
    //restore state
    currentThread->space->RestoreState();
    executeLock->Release();
    machine->Run();
    ASSERT(FALSE);
}


SpaceId Exec_Syscall(int vaddr, int len) {
    executeLock->Acquire();
    int virtualAddress = vaddr;

    char* file = new char[len + 1];

    copyin(virtualAddress, len + 1, file);

    OpenFile *newFile = fileSystem->Open(file);

    if (newFile) {
        AddrSpace* addressSpace = new AddrSpace(newFile);
        Thread *thread = new Thread("thread");
        int index = addressSpace->AllocateSpaceForNewThread();
        thread->SetIndex(index);
        thread->space = addressSpace;

        int spaceId = processTable.Put(addressSpace);
        //set space ID for process
        thread->space->SetSpaceID(spaceId);
        // machine->WriteRegister(2, spaceId);
        thread->Fork(exec_thread, 0);
        executeLock->Release();
        return spaceId;
    }
    else {
        printf("%s", "Cannot open file");
        executeLock->Release();
        return -1;
    }

}

int Join_Syscall(SpaceId id) {
    int i;
    return i;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len + 1];  // Kernel buffer to put the name in

    if (!buf) return;

    if ( copyin(vaddr, len, buf) == -1 ) {
        printf("%s", "Bad pointer passed to Create\n");
        delete buf;
        return;
    }

    buf[len] = '\0';

    fileSystem->Create(buf, 0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len + 1];  // Kernel buffer to put the name in
    OpenFile *f;            // The new open file
    int id;             // The openfile id

    if (!buf) {
        // printf("%s", "Can't allocate kernel buffer in Open\n");
        return -1;
    }

    if ( copyin(vaddr, len, buf) == -1 ) {
        printf("%s", "Bad pointer passed to Open\n");
        delete[] buf;
        return -1;
    }

    buf[len] = '\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
        if ((id = currentThread->space->fileTable.Put(f)) == -1 )
            delete f;
        return id;
    }
    else
        return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.

    char *buf;      // Kernel buffer for output
    OpenFile *f;    // Open file for output

    if ( id == ConsoleInput) return;

    if ( !(buf = new char[len]) ) {
        printf("%s", "Error allocating kernel buffer for write!\n");
        return;
    } else {
        if ( copyin(vaddr, len, buf) == -1 ) {
            printf("%s", "Bad pointer passed to to write: data not written\n");
            delete[] buf;
            return;
        }
    }

    if ( id == ConsoleOutput) {
        for (int ii = 0; ii < len; ii++) {
            printf("%c", buf[ii]);
        }

    } else {
        if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
            f->Write(buf, len);
        } else {
            printf("%s", "Bad OpenFileId passed to Write\n");
            len = -1;
        }
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;      // Kernel buffer for input
    OpenFile *f;    // Open file for output

    if ( id == ConsoleOutput) return -1;

    if ( !(buf = new char[len]) ) {
        printf("%s", "Error allocating kernel buffer in Read\n");
        return -1;
    }

    if ( id == ConsoleInput) {
        //Reading from the keyboard
        scanf("%s", buf);

        if ( copyout(vaddr, len, buf) == -1 ) {
            printf("%s", "Bad pointer passed to Read: data not copied\n");
        }
    } else {
        if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
            len = f->Read(buf, len);
            if ( len > 0 ) {
                //Read something from the file. Put into user's address space
                if ( copyout(vaddr, len, buf) == -1 ) {
                    printf("%s", "Bad pointer passed to Read: data not copied\n");
                }
            }
        } else {
            printf("%s", "Bad OpenFileId passed to Read\n");
            len = -1;
        }
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
        delete f;
    } else {
        printf("%s", "Tried to close an unopen file\n");
    }
}

void kernel_thread(int virtualAddress) {
    // printf("in kernel_thread\n");
    //increment the program counter
    machine->WriteRegister(PCReg, virtualAddress);
    machine->WriteRegister(NextPCReg, virtualAddress + 4);

    //restore state
    currentThread->space->RestoreState();
    int stackIndex = currentThread->GetStackIndex();
    machine->WriteRegister(StackReg, stackIndex);

    machine->Run();

}


void Fork_Syscall(int vaddr) {

    executeLock->Acquire();
    if (currentThread->space->GetMemorySize() < vaddr) {
        printf("Error: Virtual Address larger than physical address size\n");
        return;
    }
    else if (vaddr == 0) {
        printf("Error: Virtual Address is zero\n");
        return;
    }
    int virtualAddr = vaddr;
    int index;

    Thread *thread = new Thread("kernel_thread");

    thread->space = currentThread->space;
    index = thread->space->AllocateSpaceForNewThread();
    thread->SetStackIndex(currentThread->space->GetMemorySize() - 16);
    thread->SetIndex(index);
    thread->Fork(kernel_thread, virtualAddr);
    executeLock->Release();
}

void Yield_Syscall() {
    currentThread->Yield();
}

int CreateLock_Syscall() {
    Lock *newLock = new Lock("name");
    lockTable.Put(newLock);
    int lockNumber = lockTable.GetNumElements() - 1;
    return lockNumber;
}

int DestroyLock_Syscall(int lockIndex) {
    if (lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in DestroyLock: lock index out of boundary\n");
        return -1;
    }
    else {

        if (lockTable.Remove(lockIndex) == 0 ) {
            printf("Error in DestroyLock: lock does not exist\n");
            return -1;
        }
        return 0;
    }
}

int CreateCondition_Syscall() {
    Condition *newCV = new Condition("name");
    cvTable.Put(newCV);
    int cvNumber = cvTable.GetNumElements() - 1;
    return cvNumber;
}

int DestroyCondition_Syscall(int conditionIndex) {
    if (conditionIndex >= MAX_NUM_CONDITION || conditionIndex < 0) {
        printf("Error in DestroyCondition: condition index out of boundary\n");
        return -1;
    }
    else {
        if (cvTable.Remove(conditionIndex) == 0) {
            printf("Error in DestroyCondition: condition does not exist\n");
            return -1;

        }
        return 0;
    }
}

int Acquire_Syscall(int lockIndex) {
    if (lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in Acquire: lock index out of boundary\n");
        return -1;
    }
    else {
        Lock *lock = (Lock*)lockTable.Get(lockIndex);
        if (lock == NULL) {
            printf("Error: lock doesn't exist\n");
            return -1;
        }
        lock->Acquire();
        return 0;
    }
}

int Release_Syscall(int lockIndex) {
    if (lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in Release: lock index out of boundary\n");
        return -1;
    }
    else {
        Lock *lock = (Lock*)lockTable.Get(lockIndex);
        if (lock == NULL) {
            printf("Error: lock doesn't exist\n");
            return -1;
        }
        lock->Release();
        return 0;
    }
}

int Wait_Syscall(int conditionIndex, int lockIndex) {
    if (conditionIndex >= MAX_NUM_CONDITION || conditionIndex < 0 ||
            lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in Wait: condition or lock index out of boundary\n");
        return -1;
    }
    else {
        Condition *condition = (Condition*)cvTable.Get(conditionIndex);
        if (condition == NULL) {
            printf("Error: condition doesn't exist\n");
            return -1;
        }
        Lock *lock = (Lock*)lockTable.Get(lockIndex);
        if (lock == NULL) {
            printf("Error: lock doesn't exist\n");
            return -1;
        }
        condition->Wait(lock);
        return 0;
    }

}

int Signal_Syscall(int conditionIndex, int lockIndex) {
    if (conditionIndex >= MAX_NUM_CONDITION || conditionIndex < 0 ||
            lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in Signal: condition or lock index out of boundary\n");
        return -1;
    }
    else {
        Condition *condition = (Condition*)cvTable.Get(conditionIndex);
        if (condition == NULL) {
            printf("Error: condition doesn't exist\n");
            return -1;
        }
        Lock *lock = (Lock*)lockTable.Get(lockIndex);
        if (lock == NULL) {
            printf("Error: lock doesn't exist\n");
            return -1;
        }
        condition->Signal(lock);
        return 0;
    }
}

int Broadcast_Syscall(int conditionIndex, int lockIndex) {
    if (conditionIndex >= MAX_NUM_CONDITION || conditionIndex < 0 ||
            lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in Broadcast: condition or lock index out of boundary\n");
        return -1;
    }
    else {
        Condition *condition = (Condition*)cvTable.Get(conditionIndex);
        if (condition == NULL) {
            printf("Error: condition doesn't exist\n");
            return -1;
        }
        Lock *lock = (Lock*)lockTable.Get(lockIndex);
        if (lock == NULL) {
            printf("Error: lock doesn't exist\n");
            return -1;
        }
        condition->Broadcast(lock);
        return 0;
    }
}

void Printint_Syscall(int num) {
    printf("%d", num);
}

int Random_Syscall(int limit) {

    return rand() % limit;

}

#ifdef NETWORK
int CreateMV_Syscall(int data) {
    MonitorVariable* mv = new MonitorVariable(data);
    mvTable.Put(mv);
    mv->index = mvTable.GetNumElements() - 1;
    return mv->index;

}

int GetMV_Syscall(int monitorIndex) {
    if (monitorIndex >= mvTable.GetNumElements() || monitorIndex < 0 ) {
        printf("Error in GetMV_Syscall: monitor index out of boundary\n");
        return -1;
    }

    MonitorVariable* mv = (MonitorVariable*) mvTable.Get(monitorIndex);
    return mv->variable;

}

void SetMV_Syscall(int monitorIndex, int data) {
    if (monitorIndex >= mvTable.GetNumElements() || monitorIndex < 0 ) {
        printf("Error in GetMV_Syscall: monitor index out of boundary\n");
        return;
    }

    MonitorVariable* mv = (MonitorVariable*) mvTable.Get(monitorIndex);
    printf("current value: %d", mv->variable);
    printf("\n");
    mv->variable = data;
    printf("after change current value: %d", mv->variable);
    printf("\n");

}

//network
int AcquireServer_Syscall(int lockIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "25 %d", lockIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 25");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    // printf("before receive thread: %d\n", currentThread->GetIndex());
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    // printf("after receive thread: %d\n", currentThread->GetIndex());
    fflush(stdout);
    return atoi(receive);

}

int ReleaseServer_Syscall(int lockIndex) {
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "26 %d", lockIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 26");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);
}

int WaitServer_Syscall(int conditionIndex, int lockIndex) {
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "27 %d %d", conditionIndex, lockIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 27");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);

    AcquireServer_Syscall(lockIndex);

//    char* receive2 = new char[100];
//
//    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive2);
//
//    fflush(stdout);
    return atoi(receive);
}

int SignalServer_Syscall(int conditionIndex, int lockIndex) {
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "28 %d %d", conditionIndex, lockIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 28");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);

    fflush(stdout);

    return atoi(receive);
}

int BroadcastServer_Syscall(int conditionIndex, int lockIndex) {
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "29 %d %d", conditionIndex, lockIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 29");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);

    fflush(stdout);
    return atoi(receive);
}

int CreateLockServer_Syscall(int vaddr, int len) {
    char* lockName = new char[len];
    copyin(vaddr, len, lockName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "30 %s", lockName);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 30");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    //printf("after createLock receive thread: %d\n, msg: %s", currentThread->GetIndex(), receive);

    fflush(stdout);

    return atoi(receive);


}

int DestroyLockServer_Syscall(int lockIndex) {
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "31 %d", lockIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 31");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);

}

int CreateConditionServer_Syscall(int vaddr, int len) {
    char* cvName = new char[len];
    copyin(vaddr, len, cvName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "32 %s", cvName);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 32");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);

    return atoi(receive);

}

int DestroyConditionServer_Syscall(int conditionIndex) {
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "33 %d", conditionIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 33");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);
}

int CreateMVServer_Syscall(int vaddr, int len, int data) {
    char* mvName = new char[len];
    copyin(vaddr, len, mvName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "34 %s %d", mvName, data);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 34");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);

    return atoi(receive);


}

int GetMVServer_Syscall(int monitorIndex) {

    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "35 %d", monitorIndex);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 35");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);


}

int SetMVServer_Syscall(int monitorIndex, int data) {

    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "36 %d %d", monitorIndex, data);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 36");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);

}

int CreateMVArrayServer_Syscall(int vaddr, int len, int length) {
    char* mvName = new char[len];
    copyin(vaddr, len, mvName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "37 %s %d", mvName, length);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 37");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);

    return atoi(receive);


}

int GetMVArrayServer_Syscall(int monitorArrayIndex, int index) {

    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "38 %d %d", monitorArrayIndex, index);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 38");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);


}

int SetMVArrayServer_Syscall(int monitorArrayIndex, int index, int data) {

    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;

    char* send = new char[MaxMailSize];
    sprintf(send, "39 %d %d %d", monitorArrayIndex, index, data);

    outPktHdr.to = rand() % NumServers;
    outMailHdr.to = 0;
    outMailHdr.from = currentThread->GetIndex();
    outMailHdr.length = strlen(send) + 1;

    if (!postOffice->Send(outPktHdr, outMailHdr, send)) {
        printf("Send failed from syscall 39");
        return -1;
    }
    char* receive = new char[MaxMailSize];
    postOffice->Receive(currentThread->GetIndex(), &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);

}

#endif

void UpdateEvictedPageTable(int swapFileLocation, int virtualPage, int physicalPage) {
    // Update the location of the evicted pagetable entry
    // Either in SwapFile, Executable or Disk.
    // Shouldn't be in Memory
    PageTable *pageTable = ipt[physicalPage].space->GetPageTable();

    if (swapFileLocation != -1) {

        pageTable[virtualPage].swapFileLocation = swapFileLocation;
        pageTable[virtualPage].location = SWAPFILE;
        pageTable[virtualPage].valid = FALSE;

    }
    else {
        if (pageTable[virtualPage].byteOffset != -1) {
            pageTable[virtualPage].location = EXECUTABLE;
        }
        else {
            pageTable[virtualPage].location = DISK;
        }
    }
}


int EvictIPT() {
    int physicalPage, virtualPage;

    // Choose a page to evcit depending on policy
    if (evictPolicy == FIFO) {
        physicalPage = (int)evictQueue->Remove();
    }
    else if (evictPolicy == RAND) {
        physicalPage = rand() % NumPhysPages;
        if (ipt[physicalPage].use == FALSE) {
            ipt[physicalPage].use = TRUE;
        }
    }
    else {
        //default FIFO
        physicalPage = (int)evictQueue->Remove();
    }

    // Invalidate the evicted page
    ipt[physicalPage].valid = FALSE;
    virtualPage = ipt[physicalPage].virtualPage;

    // If the evicted IPT page is in TLB
    // Copy the dirty bit of the TLB entry to IPT
    if (ipt[physicalPage].space == currentThread->space) {
        for (int i = 0; i < TLBSize; i++)
        {
            if (machine->tlb[i].virtualPage == virtualPage && machine->tlb[i].valid == TRUE && machine->tlb[i].physicalPage == physicalPage) {
                ipt[physicalPage].dirty = machine->tlb[i].dirty;
                machine->tlb[i].valid = FALSE;
                break;
            }
        }
    }


    int swapFileLocation = -1;

    if (ipt[physicalPage].dirty == TRUE) {

        // If the page is dirty, then it should be written to SwapFile
        swapFileLocation = swapFileBitMap.Find() * PageSize;
        ASSERT(swapFileLocation != -1);
        swapFile->WriteAt(&(machine->mainMemory[physicalPage * PageSize]), PageSize, swapFileLocation);

        UpdateEvictedPageTable(swapFileLocation, virtualPage, physicalPage);
    }
    else {
        UpdateEvictedPageTable(swapFileLocation, virtualPage, physicalPage);

    }

    ipt[physicalPage].valid = FALSE;

    return physicalPage;

}


int AllocatePhysicalPage() {
    int physicalPage;

    // Try to find an empty spce in Memory
    physicalPage = memoryMap.Find();

    return physicalPage;
}

void PopulateMemory(int virtualPage, int physicalPage) {
    // Load data to the memory location
    // Either from Executable or SwapFile.
    PageTable *pageTable = currentThread->space->GetPageTable();

    if (pageTable[virtualPage].location == EXECUTABLE) {
        ASSERT(pageTable[virtualPage].byteOffset != -1);
        currentThread->space->GetExecutable()->ReadAt(&(machine->mainMemory[physicalPage * PageSize]),
                PageSize, pageTable[virtualPage].byteOffset);
    }
    else if (pageTable[virtualPage].location == SWAPFILE) {
        ASSERT(pageTable[virtualPage].swapFileLocation != -1);
        swapFile->ReadAt(&(machine->mainMemory[physicalPage * PageSize]),
                         PageSize, pageTable[virtualPage].swapFileLocation);
        ipt[physicalPage].dirty = TRUE;

    }
    // If the page is already in Memory, there must be an error
    else if (pageTable[virtualPage].location == MEMORY) {
        interrupt->Halt();
    }
    else {
        // Read from DISK
    }


}

void UpdateLoadedPageTable(int virtualPage, int physicalPage) {
    // After loaded to memory, update location
    // and the physical page number of the pagetable entry
    PageTable *pageTable = currentThread->space->GetPageTable();

    pageTable[virtualPage].location = MEMORY;
    pageTable[virtualPage].valid = TRUE;
    pageTable[virtualPage].dirty = FALSE;
    pageTable[virtualPage].use = FALSE;
    pageTable[virtualPage].physicalPage = physicalPage;

}

void UpdateLoadedIPT(int virtualPage, int physicalPage) {
    //update the loaded ipt page
    ipt[physicalPage].valid = TRUE;
    ipt[physicalPage].use = FALSE;
    ipt[physicalPage].virtualPage = virtualPage;
    ipt[physicalPage].physicalPage = physicalPage;
    ipt[physicalPage].space = currentThread->space;
}

int IPTMissHandler(int virtualPage) {

    int physicalPage;
    physicalPage = AllocatePhysicalPage();

    // Didn't find empty space in memory, evict IPT page
    if (physicalPage == -1) {
        physicalPage = EvictIPT();
    }

    PopulateMemory(virtualPage, physicalPage);
    UpdateLoadedPageTable(virtualPage, physicalPage);
    UpdateLoadedIPT(virtualPage, physicalPage);

    evictQueue->Append((void *)physicalPage);

    return physicalPage;
}

int PopulateTLB(int physicalPage, int virtualPage) {

    //if the evicted TLB is valid, update dirty bit of corresponding IPT entry
    if (machine->tlb[currentTLB].valid == TRUE) {
        ipt[machine->tlb[currentTLB].physicalPage].dirty = machine->tlb[currentTLB].dirty;
    }

    machine->tlb[currentTLB].dirty = ipt[physicalPage].dirty;
    machine->tlb[currentTLB].valid = ipt[physicalPage].valid;
    machine->tlb[currentTLB].virtualPage = ipt[physicalPage].virtualPage;
    machine->tlb[currentTLB].physicalPage = ipt[physicalPage].physicalPage;
    machine->tlb[currentTLB].use = ipt[physicalPage].use;
    machine->tlb[currentTLB].readOnly = ipt[physicalPage].readOnly;

    currentTLB = (++currentTLB) % TLBSize;


    return 0;
}

int PageFaultHandler(int vaddr) {
    // Turn off interrupt for the whole pagefault handling process
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    int virtualPage = vaddr / PageSize;
    int physicalPage = -1;

    //Try to find matching IPT entry
    for (int i = 0; i < NumPhysPages; i++)
    {
        if (ipt[i].virtualPage == virtualPage && ipt[i].valid == TRUE && ipt[i].space == currentThread->space) {
            physicalPage = i;
            break;
        }
    }

    //No matching IPT entry
    if (physicalPage == -1) {
        physicalPage = IPTMissHandler(virtualPage);

    }

    //Update TLB entries before exit PageFault Handler
    PopulateTLB(physicalPage, virtualPage);

    (void) interrupt->SetLevel(oldLevel);
    return 0;

}




void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv = 0; // the return value from a syscall

    if ( which == SyscallException ) {
        switch (type) {
        default:
            DEBUG('a', "Unknown syscall - shutting down.\n");
        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;
        case SC_Exit:
            DEBUG('a', "Exit syscall.\n");
            Exit_Syscall(machine->ReadRegister(4));
            break;
        case SC_Exec:
            DEBUG('a', "Exec syscall.\n");
            rv = Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
        case SC_Join:
            DEBUG('a', "Join syscall.\n");
            rv = Join_Syscall(machine->ReadRegister(4));
            break;
        case SC_Create:
            DEBUG('a', "Create syscall.\n");
            Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
        case SC_Open:
            DEBUG('a', "Open syscall.\n");
            rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
        case SC_Write:
            DEBUG('a', "Write syscall.\n");
            Write_Syscall(machine->ReadRegister(4),
                          machine->ReadRegister(5),
                          machine->ReadRegister(6));
            break;
        case SC_Read:
            DEBUG('a', "Read syscall.\n");
            rv = Read_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5),
                              machine->ReadRegister(6));
            break;
        case SC_Close:
            DEBUG('a', "Close syscall.\n");
            Close_Syscall(machine->ReadRegister(4));
            break;
        case SC_Fork:
            DEBUG('a', "Fork syscall.\n");
            Fork_Syscall(machine->ReadRegister(4));
            break;
        case SC_Yield:
            DEBUG('a', "Yield syscall.\n");
            Yield_Syscall();
            break;
        case SC_Acquire:
            DEBUG('a', "Acquire syscall.\n");
            rv = Acquire_Syscall(machine->ReadRegister(4));
            break;
        case SC_Release:
            DEBUG('a', "Release syscall.\n");
            rv = Release_Syscall(machine->ReadRegister(4));
            break;
        case SC_Wait:
            DEBUG('a', "Wait syscall.\n");
            rv = Wait_Syscall(machine->ReadRegister(4),
                              machine->ReadRegister(5));
            break;
        case SC_Signal:
            DEBUG('a', "Signal syscall.\n");
            rv = Signal_Syscall(machine->ReadRegister(4),
                                machine->ReadRegister(5));
            break;
        case SC_Broadcast:
            DEBUG('a', "Broadcast syscall.\n");
            rv = Broadcast_Syscall(machine->ReadRegister(4),
                                   machine->ReadRegister(5));
            break;
        case SC_CreateLock:
            DEBUG('a', "CreateLock syscall.\n");
            rv = CreateLock_Syscall();
            break;
        case SC_DestroyLock:
            DEBUG('a', "DestroyLock syscall.\n");
            rv = DestroyLock_Syscall(machine->ReadRegister(4));
            break;
        case SC_CreateCondition:
            DEBUG('a', "CreateCondition syscall.\n");
            rv = CreateCondition_Syscall();
            break;
        case SC_DestroyCondition:
            DEBUG('a', "DestroyCondition syscall.\n");
            rv = DestroyCondition_Syscall(machine->ReadRegister(4));
            break;
        case SC_Printint:
            DEBUG('a', "Printint syscall.\n");
            Printint_Syscall(machine->ReadRegister(4));
            break;
        case SC_Random:
            DEBUG('a', "Random syscall.\n");
            rv = Random_Syscall(machine->ReadRegister(4));
            break;
#ifdef NETWORK
        case SC_CreateMV:
            DEBUG('a', "CreateMV syscall.\n");
            rv = CreateMV_Syscall(machine->ReadRegister(4));
            break;
        case SC_GetMV:
            DEBUG('a', "GetMV syscall.\n");
            rv = GetMV_Syscall(machine->ReadRegister(4));
            break;
        case SC_SetMV:
            DEBUG('a', "SetMV syscall.\n");
            SetMV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
        //network
        case SC_AcquireServer:
            DEBUG('a', "AcquireServer syscall.\n");
            rv = AcquireServer_Syscall(machine->ReadRegister(4));
            break;
        case SC_ReleaseServer:
            DEBUG('a', "ReleaseServer syscall.\n");
            rv = ReleaseServer_Syscall(machine->ReadRegister(4));
            break;
        case SC_WaitServer:
            DEBUG('a', "WaitServer syscall.\n");
            rv = WaitServer_Syscall(machine->ReadRegister(4),
                                    machine->ReadRegister(5));
            break;
        case SC_SignalServer:
            DEBUG('a', "SignalServer syscall.\n");
            rv = SignalServer_Syscall(machine->ReadRegister(4),
                                      machine->ReadRegister(5));
            break;
        case SC_BroadcastServer:
            DEBUG('a', "BroadcastServer syscall.\n");
            rv = BroadcastServer_Syscall(machine->ReadRegister(4),
                                         machine->ReadRegister(5));
            break;
        case SC_CreateLockServer:
            DEBUG('a', "CreateLockServer syscall.\n");
            rv = CreateLockServer_Syscall(machine->ReadRegister(4),
                                          machine->ReadRegister(5));
            break;
        case SC_DestroyLockServer:
            DEBUG('a', "DestroyLockServer syscall.\n");
            rv = DestroyLockServer_Syscall(machine->ReadRegister(4));
            break;
        case SC_CreateConditionServer:
            DEBUG('a', "CreateConditionServer syscall.\n");
            rv = CreateConditionServer_Syscall(machine->ReadRegister(4),
                                               machine->ReadRegister(5));
            break;
        case SC_DestroyConditionServer:
            DEBUG('a', "DestroyConditionServer syscall.\n");
            rv = DestroyConditionServer_Syscall(machine->ReadRegister(4));
            break;
        case SC_CreateMVServer:
            DEBUG('a', "CreateMVServerServer syscall.\n");
            rv = CreateMVServer_Syscall(machine->ReadRegister(4),
                                        machine->ReadRegister(5),
                                        machine->ReadRegister(6));
            break;
        case SC_GetMVServer:
            DEBUG('a', "GetMVServerServer syscall.\n");
            rv = GetMVServer_Syscall(machine->ReadRegister(4));
            break;
        case SC_SetMVServer:
            DEBUG('a', "SetMVServerServer syscall.\n");
            SetMVServer_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;


        case SC_CreateMVArrayServer:
            DEBUG('a', "CreateMVServerServer syscall.\n");
            rv = CreateMVArrayServer_Syscall(machine->ReadRegister(4),
                                             machine->ReadRegister(5),
                                             machine->ReadRegister(6));
            break;
        case SC_GetMVArrayServer:
            DEBUG('a', "GetMVServerServer syscall.\n");
            rv = GetMVArrayServer_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
        case SC_SetMVArrayServer:
            DEBUG('a', "SetMVServerServer syscall.\n");
            SetMVArrayServer_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
            break;

#endif
        }


        // Put in the return value and increment the PC
        machine->WriteRegister(2, rv);
        machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
        return;
    } else if (which == PageFaultException) {
        DEBUG('a', "PageFaultException triggered\n");
        PageFaultHandler(machine->ReadRegister(BadVAddrReg));
    }
    else {
        cout << "Unexpected user mode exception - which:" << which << "  type:" << type << endl;
        interrupt->Halt();
    }
}
