// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include "synch.h"

using namespace std;

vector<Lock *> locks;
vector<Condition *> cvs;
Lock* forkLock =new Lock("forkLock");
Lock* executeLock =new Lock("execLock");

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
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
    int n=0;			// The number of bytes copied in

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

void Exit_Syscall(int status){
    AddrSpace* addressSpace=currentThread->space;
    
    //has more than 1 thread in current process
    if(addressSpace->GetNumThread() > 1){
        addressSpace->DeallocateSpaceForThread();
        currentThread->Finish();
        
    }
    //the main thread case
    if(addressSpace->GetNumThread() == 1){
        processTable.Remove(addressSpace->GetSpaceID());
        addressSpace->DeallocateSpaceForThread();
        currentThread->Finish();
        
        //if this is the last process
        if(processTable.GetNumElements() == 0){
            interrupt->Halt();
        }
        
    }


}


void exec_thread(int virtualAddress){
    executeLock->Acquire();
    //initialize register
    currentThread->space->InitRegisters();
    //restore state
    currentThread->space->RestoreState();
    
    executeLock->Release();
    
    machine->Run();
}


SpaceId Exec_Syscall(char *name){
    executeLock->Acquire();
    int virtualAddress = machine->ReadRegister(4);
    
    char* file=new char[100];
    
    copyin(virtualAddress, 100, file);
    
    OpenFile *newFile = fileSystem->Open(file);
    
    if(newFile){
        AddrSpace* addressSpace = new AddrSpace(newFile);
        Thread *thread = new Thread("thread");
        addressSpace->AllocateSpaceForNewThread();
        thread->space = addressSpace;
        
        int spaceId = processTable.Put(addressSpace);
        //set space ID for process
        thread->space->SetSpaceID(spaceId);
        
        machine->WriteRegister(2, spaceId);
        
        thread->Fork(exec_thread, virtualAddress);
        executeLock->Release();
        return 0;
    }
    else{
        printf("%s", "Cannot open file");
        executeLock->Release();
        return -1;
    }
    
}

int Join_Syscall(SpaceId id){
    int i;
    return i;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

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
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
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
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
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
      printf("%s","Tried to close an unopen file\n");
    }
}

void kernel_thread(int virtualAddress){
    //increment the program counter
    machine->WriteRegister(PCReg, virtualAddress);
    machine->WriteRegister(NextPCReg, virtualAddress+4);
    //restore state
    currentThread->space->RestoreState();
    machine->WriteRegister(StackReg, machine->ReadRegister(StackReg)-16);
    machine->Run();
    
}


void Fork_Syscall(void (*func)()){
    
    forkLock->Acquire();
    
    int virtualAddr = machine->ReadRegister(4);
    
    Thread *thread=new Thread("newThread");
    
    AddrSpace* addressSpace=currentThread->space;
    
    addressSpace->AllocateSpaceForNewThread();
    
    thread->Fork(kernel_thread, virtualAddr);
    
    forkLock->Release();
}

void Yield_Syscall(){
    currentThread->Yield();
}   

int CreateLock_Syscall(){

    Lock *newLock = new Lock("name");
    locks.push_back(newLock);
    int lockNumber = locks.size() - 1;
    return lockNumber;
}

int DestroyLock_Syscall(int lockIndex){
    locks.erase(locks.begin() + lockIndex);
    return 0;
  }

int CreateCondition_Syscall(){
    Condition *newCV = new Condition("name");
    cvs.push_back(newCV);
    int cvNumber = cvs.size() - 1;
    return cvNumber;
  }

int DestroyCondition_Syscall(int conditionIndex){
    cvs.erase(cvs.begin() + conditionIndex);
    return 0;
  }

int Acquire_Syscall(int lockIndex){
    Lock *lock = locks[lockIndex];
    lock->Acquire();
    return 0;
}

int Release_Syscall(int lockIndex){
    Lock *lock = locks[lockIndex];
    lock->Release();
    return 0;
}

int Wait_Syscall(int conditionIndex, int lockIndex){
    Condition *condition = cvs[conditionIndex];
    Lock *lock = locks[lockIndex];
    condition->Wait(lock);
    return 0;

}

int Signal_Syscall(int conditionIndex, int lockIndex){
    Condition *condition = cvs[conditionIndex];
    Lock *lock = locks[lockIndex];
    condition->Signal(lock);
    return 0;
  }

int Broadcast_Syscall(int conditionIndex, int lockIndex){
    Condition *condition = cvs[conditionIndex];
    Lock *lock = locks[lockIndex];
    condition->Broadcast(lock);
    return 0;
  }

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

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
    rv = Exec_Syscall((char *)machine->ReadRegister(4));
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
      case SC_Fork:
    DEBUG('a', "Fork syscall.\n");
    Fork_Syscall((void (*)())machine->ReadRegister(4));
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
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
