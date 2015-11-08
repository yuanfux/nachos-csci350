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

int currentTLB = -1;
int currentIPT = 0;
int nextIPT = 0;
List *IPTQueue;

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
        printf("Error: number of threads is %d\n", addressSpace->GetNumThread());
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
        addressSpace->AllocateSpaceForNewThread();

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
        printf("%s", "Can't allocate kernel buffer in Open\n");
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

    Thread *thread = new Thread("kernel_thread");

    thread->space = currentThread->space;
    thread->space->AllocateSpaceForNewThread();
    thread->SetStackIndex(currentThread->space->GetMemorySize() - 16);
    thread->SetIndex(thread->space->GetNumThread());
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
    
    char* send = new char[100];
    sprintf(send, "25 %d", lockIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 25");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);
    
}

int ReleaseServer_Syscall(int lockIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "26 %d", lockIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 26");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);
}

int WaitServer_Syscall(int conditionIndex, int lockIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "27 %d %d", conditionIndex, lockIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 27");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    
    fflush(stdout);
    return atoi(receive);
}

int SignalServer_Syscall(int conditionIndex, int lockIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "28 %d %d", conditionIndex, lockIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 28");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    
    fflush(stdout);
    return atoi(receive);
}

int BroadcastServer_Syscall(int conditionIndex, int lockIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "29 %d %d", conditionIndex, lockIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 29");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    
    fflush(stdout);
    return atoi(receive);
}

int CreateLockServer_Syscall(int vaddr, int len){
    char* lockName = new char[len];
    copyin(vaddr, len, lockName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "30 %s", lockName);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 30");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    
    return atoi(receive);
    
    
}

int DestroyLockServer_Syscall(int lockIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "31 %d", lockIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 31");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);

}

int CreateConditionServer_Syscall(int vaddr, int len){
    char* cvName = new char[len];
    copyin(vaddr, len, cvName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "32 %s", cvName);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 32");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    
    return atoi(receive);
    
}

int DestroyConditionServer_Syscall(int conditionIndex){
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "33 %d", conditionIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 33");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);
}

int CreateMVServer_Syscall(int vaddr, int len){
    char* mvName = new char[len];
    copyin(vaddr, len, mvName);
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "34 %s", mvName);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 34");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    
    return atoi(receive);

    
}

int GetMVServer_Syscall(int monitorIndex){
    
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "35 %d", monitorIndex);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 31");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);

    
}

int SetMVServer_Syscall(int monitorIndex, int data){
    
    PacketHeader outPktHdr;
    PacketHeader inPktHdr;
    MailHeader outMailHdr;
    MailHeader inMailHdr;
    
    char* send = new char[100];
    sprintf(send, "36 %d %d", monitorIndex, data);
    
    outPktHdr.to = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(send) + 1;
    
    if(!postOffice->Send(outPktHdr, outMailHdr, send)){
        printf("Send failed from syscall 31");
        return -1;
    }
    char* receive = new char[100];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    fflush(stdout);
    return atoi(receive);
    
}


int IPTMissHandler(int vpn) {
    printf("In IPTMissHandler\n");

    int physicalPage;
    physicalPage = currentThread->space->AllocatePhysicalPage();

    if (physicalPage == -1) {
        printf("No empty space found in physical memory\n");
        int count = 0;
        nextIPT = currentIPT;
        while (ipt[nextIPT].valid == FALSE && count <= NumPhysPages) {
            nextIPT = (++nextIPT) % NumPhysPages;
            count++;
        }
        physicalPage = nextIPT;
    }

    // PageTable *pt = currentThread->space->GetPageTable();
    // if (pt[vpn].location == EXECUTABLE) {
    //     printf("before readat\n");
    //     printf("byteOffset: %d, vpn: %d\n", pt[vpn].byteOffset, vpn);
    //     currentThread->space->GetExecutable()->ReadAt(&(machine->mainMemory[physicalPage * PageSize]),
    //             PageSize, 40 + vpn * PageSize);
    //     printf("after readat\n");
    // }

    // pt[vpn].location = MEMORY;
    // pt[vpn].valid = TRUE;
    // pt[vpn].physicalPage = physicalPage;

    // ipt[physicalPage].valid = pt[vpn].valid;
    // ipt[physicalPage].dirty = pt[vpn].dirty;
    // ipt[physicalPage].virtualPage = vpn;
    // ipt[physicalPage].space = currentThread->space;

    currentThread->space->PopulateIPT(vpn, physicalPage);
    currentIPT++;
    printf("done ipt handler\n");
    return physicalPage;
}

int PopulateTLB(int ppn, int vpn) {
    // printf("In PopulateTLB\n");
    currentTLB = (++currentTLB) % TLBSize;

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    machine->tlb[currentTLB].dirty = FALSE;
    machine->tlb[currentTLB].valid = TRUE;
    machine->tlb[currentTLB].virtualPage = vpn;
    machine->tlb[currentTLB].physicalPage = ppn;

    (void) interrupt->SetLevel(oldLevel);

    return 0;
}

int PageFaultHandler(int vaddr) {
    // printf("In PageFaultHandler\n");
    int vpn = vaddr / PageSize;
    TranslationEntry *pageTable = currentThread->space->GetPageTable();
    int ppn = -1;
    for (int i = 0; i < NumPhysPages; i++)
    {
        if (ipt[i].virtualPage == vpn && ipt[i].valid == TRUE && ipt[i].space == currentThread->space) {
            ppn = i;
            break;
        }
    }
    if (ppn == -1) {
        ppn = IPTMissHandler(vpn);

    }

    PopulateTLB(ppn, vpn);
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
                DEBUG('a', "Acquire syscall.\n");
                rv = AcquireServer_Syscall(machine->ReadRegister(4));
                break;
            case SC_ReleaseServer:
                DEBUG('a', "Release syscall.\n");
                rv = ReleaseServer_Syscall(machine->ReadRegister(4));
                break;
            case SC_WaitServer:
                DEBUG('a', "Wait syscall.\n");
                rv = WaitServer_Syscall(machine->ReadRegister(4),
                                  machine->ReadRegister(5));
                break;
            case SC_SignalServer:
                DEBUG('a', "Signal syscall.\n");
                rv = SignalServer_Syscall(machine->ReadRegister(4),
                                    machine->ReadRegister(5));
                break;
            case SC_BroadcastServer:
                DEBUG('a', "Broadcast syscall.\n");
                rv = BroadcastServer_Syscall(machine->ReadRegister(4),
                                       machine->ReadRegister(5));
                break;
            case SC_CreateLockServer:
                DEBUG('a', "CreateLock syscall.\n");
                rv = CreateLockServer_Syscall(machine->ReadRegister(4),
                                              machine->ReadRegister(5));
                break;
            case SC_DestroyLockServer:
                DEBUG('a', "DestroyLock syscall.\n");
                rv = DestroyLockServer_Syscall(machine->ReadRegister(4));
                break;
            case SC_CreateConditionServer:
                DEBUG('a', "CreateCondition syscall.\n");
                rv = CreateConditionServer_Syscall(machine->ReadRegister(4),
                                                   machine->ReadRegister(5));
                break;
            case SC_DestroyConditionServer:
                DEBUG('a', "DestroyCondition syscall.\n");
                rv = DestroyConditionServer_Syscall(machine->ReadRegister(4));
                break;
            case SC_CreateMVServer:
                DEBUG('a', "CreateMVServer syscall.\n");
                rv = CreateMVServer_Syscall(machine->ReadRegister(4),
                                            machine->ReadRegister(5));
                break;
            case SC_GetMVServer:
                DEBUG('a', "GetMVServer syscall.\n");
                rv = GetMVServer_Syscall(machine->ReadRegister(4));
                break;
            case SC_SetMVServer:
                DEBUG('a', "SetMVServer syscall.\n");
                SetMVServer_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
                break;
    

        }


        // Put in the return value and increment the PC
        machine->WriteRegister(2, rv);
        machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
        return;
    } else if (which == PageFaultException) {
        DEBUG('a', "PageFaultException triggered\n");
        // interrupt->Halt();
        PageFaultHandler(machine->ReadRegister(39));
    }
    else {
        cout << "Unexpected user mode exception - which:" << which << "  type:" << type << endl;
        interrupt->Halt();
    }
}
