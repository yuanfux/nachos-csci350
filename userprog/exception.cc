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

using namespace std;

#define MAX_NUM_LOCK 100
#define MAX_NUM_CONDITION 100

Table lockTable(MAX_NUM_LOCK);
Table cvTable(MAX_NUM_CONDITION);
Lock* forkLock = new Lock("forkLock");
Lock* executeLock = new Lock("execLock");
Lock* exitLock = new Lock("exitLock");

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

int copyinint(unsigned int vaddr, int len, int *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n = 0;                  // The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
        result = machine->ReadMem( vaddr, 4, paddr );
        while (!result) // FALL 09 CHANGES
        {
            result = machine->ReadMem( vaddr, 4, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
        }

        buf[n++] = *paddr;

        if ( !result ) {
            //translation failed
            return -1;
        }

        vaddr += 4;
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
    printf ("In Exit_Syscall\n");

    AddrSpace* addressSpace = currentThread->space;
    
    //has more than 1 thread in current process
    if (addressSpace->GetNumThread() > 1) {
        
        printf("\nThread:%s exits current thread number: %d, current process number: %d \n", currentThread->getName(), currentThread->space->GetNumThread(),processTable.GetNumElements() );
        //addressSpace->DeallocateSpaceForThread();
        addressSpace->UpdateThreadNum();
        executeLock->Release();
        currentThread->Finish();

    }
    //the main thread case
    else if (addressSpace->GetNumThread() == 1) {
        //if this is the last process
        if (processTable.GetNumElements() == 1) {
            printf("\nlast Thread in last process:%s exits current thread number: %d, current process number: %d  \n", currentThread->getName(), currentThread->space->GetNumThread(),processTable.GetNumElements());
            executeLock->Release();
            interrupt->Halt();
        }
        else if(processTable.GetNumElements() > 1 ){
             printf("\nlast Thread:%s exits current thread number: %d, current process number: %d  \n", currentThread->getName(), currentThread->space->GetNumThread(),processTable.GetNumElements());
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
    printf ("In Exec_Syscall\n");
    executeLock->Acquire();
    int virtualAddress = vaddr;

    char* file = new char[len+1];

    copyin(virtualAddress, len+1, file);

    OpenFile *newFile = fileSystem->Open(file);

    if (newFile) {
        AddrSpace* addressSpace = new AddrSpace(newFile);
        Thread *thread = new Thread("thread");
        addressSpace->AllocateSpaceForNewThread();
        
        thread->space = addressSpace;
        printf("before put, the process number: %d \n", processTable.GetNumElements());
        
        int spaceId = processTable.Put(addressSpace);
        printf("after put, the process number: %d \n", processTable.GetNumElements());
        //set space ID for process
        thread->space->SetSpaceID(spaceId);
      //  printf("1\n");
       // machine->WriteRegister(2, spaceId);
       // printf("3\n");
        thread->Fork(exec_thread, 0);
        executeLock->Release();
       //  printf("4\n");
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
    executeLock->Acquire();
    machine->WriteRegister(PCReg, virtualAddress);
    machine->WriteRegister(NextPCReg, virtualAddress + 4);
    //restore state
    currentThread->space->RestoreState();
    machine->WriteRegister(StackReg, currentThread->space->GetMemorySize() - 16);// - divRoundUp(UserStackSize,PageSize));
   // printf("starkreg: %d\n", StackReg);
    executeLock->Release();
    machine->Run();

}


void Fork_Syscall(int vaddr) {

    printf ("In Fork_Syscall\n");
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

    Thread *thread = new Thread("newThread");

    thread->space = currentThread->space;
    thread->space->AllocateSpaceForNewThread();
    thread->space->RestoreState();
    thread->Fork(kernel_thread, virtualAddr);
    executeLock->Release();
}

void Print_Syscall(unsigned int vaddr, int len, unsigned int args, int argsSize) {
    // Print string of length len to synchronized console including
    // the integer arguments supplied in the args array.
    // printLock->Acquire();
    char* c_buf;
    c_buf = new char[len];
    copyin(vaddr, len, c_buf);

    int* i_buf;
    i_buf = new int[argsSize];
    copyinint(args, argsSize, i_buf);

    switch (argsSize) {
    case 0:
        printf(c_buf);
        break;
    case 1:
        printf(c_buf, *i_buf);
        break;
    case 2:
        printf(c_buf, *i_buf, *(i_buf + 1));
        break;
    case 3:
        printf(c_buf, *i_buf, *(i_buf + 1), *(i_buf + 2));
        break;
    case 4:
        printf(c_buf, *i_buf, *(i_buf + 1), *(i_buf + 2), *(i_buf + 3));
        break;
    }
    delete c_buf;
    delete i_buf;
    // printLock->Release();

}

void Yield_Syscall() {
    printf ("Thread yield successfully!\n");
    currentThread->Yield();
}

int CreateLock_Syscall() {
    printf ("In CreateLock_Syscall\n");
    Lock *newLock = new Lock("name");
    lockTable.Put(newLock);
    int lockNumber = lockTable.GetNumElements() - 1;
    return lockNumber;
}

int DestroyLock_Syscall(int lockIndex) {
    printf ("In DestroyLock_Syscall\n");
    if (lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in DestroyLock: lock index out of boundary\n");
        return -1;
    }
    else {
        lockTable.Remove(lockIndex);
        return 0;
    }
}

int CreateCondition_Syscall() {
    printf ("In CreateCondition_Syscall\n");
    Condition *newCV = new Condition("name");
    cvTable.Put(newCV);
    int cvNumber = cvTable.GetNumElements() - 1;
    return cvNumber;
}

int DestroyCondition_Syscall(int conditionIndex) {
    printf ("In DestroyCondition_Syscall\n");
    if (conditionIndex >= MAX_NUM_CONDITION || conditionIndex < 0) {
        printf("Error in DestroyCondition: condition index out of boundary\n");
        return -1;
    }
    else {
        cvTable.Remove(conditionIndex);
        return 0;
    }
}

int Acquire_Syscall(int lockIndex) {
    printf ("In Acquire_Syscall\n");
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
        printf("Acquire Lock successfully!\n");
        return 0;
    }
}

int Release_Syscall(int lockIndex) {
    printf ("In Release_Syscall\n");
    if (lockIndex >= MAX_NUM_LOCK || lockIndex < 0) {
        printf("Error in Release: lock index out of boundary\n");
        return -1;
    }
    else {
        Lock *lock = (Lock*)lockTable.Get(lockIndex);
        printf("current thread name: %s",currentThread->getName());
        if (lock == NULL) {
            printf("Error: lock doesn't exist\n");
            return -1;
        }
        lock->Release();
        return 0;
    }
}

int Wait_Syscall(int conditionIndex, int lockIndex) {
    printf ("In Wait_Syscall\n");
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
        printf("Wait successfully!\n");
        return 0;
    }

}

int Signal_Syscall(int conditionIndex, int lockIndex) {
    printf ("In Signal_Syscall\n");
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
        printf("Signal successfully!\n");
        return 0;
    }
}

int Broadcast_Syscall(int conditionIndex, int lockIndex) {
    printf ("In Broadcast_Syscall\n");
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
        printf("Broadcast successfully");
        return 0;
    }
}

void Printint_Syscall(int num){
    printf("%d", num);
}

int Random_Syscall(int limit){
    
    return rand() % limit;
    
    
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
            rv = Exec_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
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
            Exit_Syscall(machine->ReadRegister(4));
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
        case SC_Print:
            DEBUG('a', "Print syscall.\n");
            Print_Syscall(machine->ReadRegister(4),
                          machine->ReadRegister(5),
                          machine->ReadRegister(6),
                          machine->ReadRegister(7));
            break;
        case SC_Printint:
            DEBUG('a', "Printint syscall.\n");
            Printint_Syscall(machine->ReadRegister(4));
            break;
        case SC_Random:
            DEBUG('a', "Random syscall.\n");
            rv = Random_Syscall(machine->ReadRegister(4));
            break;
        }


        // Put in the return value and increment the PC
        machine->WriteRegister(2, rv);
        machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
        return;
    } else {
        cout << "Unexpected user mode exception - which:" << which << "  type:" << type << endl;
        interrupt->Halt();
    }
}
