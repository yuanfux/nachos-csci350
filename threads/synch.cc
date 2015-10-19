// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
} 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
       scheduler->ReadyToRun(thread);
   value++;
   (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    name = debugName;
    state = FREE;
    queue = new List;
    lockHolder = NULL;
}

Lock::~Lock() {
    delete queue;
    delete lockHolder;
}

void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupt

    if (isHeldByCurrentThread()){                   //check holder of the lock
        (void) interrupt->SetLevel(oldLevel);       //if the holder of the lock is
        return;                                     //current thread, then nothing happens
    }

    if (state == FREE){                 //if the lock is not acquired by any threads
        state = BUSY;                   //set the lock to be busy
        lockHolder = currentThread;     //set current thread to be the holder of the lock
    }
    else{
        queue->Append((void *)currentThread);   //if the lock is busy, put current thread
        currentThread->Sleep();                 //into waiting queue and put to sleep
    }

    (void) interrupt->SetLevel(oldLevel);   //restore interrupt
}

void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if (!isHeldByCurrentThread()){                                  //if a thread tries to release the lock
        printf("Error. Current thread is not the lock holder\n");   //but is not the holder, send error message
        (void) interrupt->SetLevel(oldLevel);                       //and exit
        return;
    }
    if (!queue->IsEmpty()){
        //if the queue of waiting lock is not empty
        lockHolder = (Thread *)queue->Remove();     //remove the first thread from the queue
        scheduler->ReadyToRun(lockHolder);          //put the thread to ready state

    }
    else {
        state = FREE;           //if the queue is empty, then simply free the lock
        lockHolder = NULL;      //set holder of the lock to be NULL
    }
    
    (void) interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread(){
    return lockHolder == currentThread;
}

Condition::Condition(char* debugName) {
    waitingLock = NULL;
    name = debugName;
    queue = new List;
}

Condition::~Condition() { 
    delete queue;
    delete waitingLock;
}

void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    if ( conditionLock == NULL ){                       //always check if there's argument
        printf("Error. Please pass a lock in the argument.\n");
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    if (waitingLock == NULL){           //if no lock is waiting
        waitingLock = conditionLock;    //make current lock the as waiting lock
    }

    if (waitingLock != conditionLock){                  //if the thread uses a different lock
        printf("Error. Please use the same lock.\n");   //print error message and return
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    queue->Append((void *)currentThread);   //put current thread on waiting queue

    conditionLock->Release();               //give up the lock to other threads
    currentThread->Sleep();                 //put thread to sleep
    conditionLock->Acquire();               //acquire the same lock again after been signalled

    (void) interrupt->SetLevel(oldLevel);

}

void Condition::Signal(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    if ( conditionLock == NULL ){
        printf("Error. Please pass a lock in the argument.\n");
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    if (queue->IsEmpty()){                                  //if the waiting queue is empty
        printf("Warning. There's nothing to signal.\n");    //then it doesn't need to signal any thread
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    if (waitingLock != conditionLock){
        printf("Error. Please use the same lock.\n");
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    scheduler->ReadyToRun((Thread *)queue->Remove());   //put the first thread in waiting queue to ready state

    if (queue->IsEmpty()){      //if the waiting queue is empty
        waitingLock = NULL;     //clean the record of lock for the conditional variable
    }

    (void) interrupt->SetLevel(oldLevel);
}

void Condition::Broadcast(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    if ( conditionLock == NULL ){
        printf("Error. Please pass a lock in the argument.\n");
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    if (waitingLock != conditionLock){
        printf("Error. Please use the same lock.\n");
        (void) interrupt->SetLevel(oldLevel);
        return;
    }

    (void) interrupt->SetLevel(oldLevel);

    while (!queue->IsEmpty()){              //if the queue is not empty
        Condition::Signal(conditionLock);   //signal all threads.
    }
}