// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include <stdlib.h>
#include <climits>
#include <vector>
#include <iostream>
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED
//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");
    
    Thread *t = new Thread("forked thread");
    
    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//	Simple test cases for the threads assignment.
//

// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done

Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
    
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
       t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
        printf ("%s: Releasing Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
       t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
       t1_l1.getName());
    for (int i = 0; i < 10; i++)
       ;
   printf ("%s: Releasing Lock %s\n",currentThread->getName(),
       t1_l1.getName());
   t1_l1.Release();
   t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
       printf("%s: Trying to release Lock %s\n",currentThread->getName(),
        t1_l1.getName());
       t1_l1.Release();
   }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
        t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
        t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
        t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
        t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
        t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
       t3_s1.P();
   t3_l1.Acquire();
   printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
    t3_l1.getName(), t3_c1.getName());
   t3_c1.Signal(&t3_l1);
   printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
   t3_l1.Release();
   t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
        t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
       t4_s1.P();
   t4_l1.Acquire();
   printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
    t4_l1.getName(), t4_c1.getName());
   t4_c1.Broadcast(&t4_l1);
   printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
   t4_l1.Release();
   t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
        t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
        t5_l1.getName());
    t5_l1.Release();
    printf("end of t5_t1");
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
        t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
        t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
        t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
       t1_done.P();

    // Test 2

   printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
   printf("completes\n");

   t = new Thread("t2_t1");
   t->Fork((VoidFunctionPtr)t2_t1,0);

   t = new Thread("t2_t2");
   t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
   t2_done.P();

    // Test 3

   printf("Starting Test 3\n");

   for (  i = 0 ; i < 5 ; i++ ) {
       name = new char [20];
       sprintf(name,"t3_waiter%d",i);
       t = new Thread(name);
       t->Fork((VoidFunctionPtr)t3_waiter,0);
   }
   t = new Thread("t3_signaller");
   t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
   for (  i = 0; i < 2; i++ )
       t3_done.P();

    // Test 4

   printf("Starting Test 4\n");

   for (  i = 0 ; i < 5 ; i++ ) {
       name = new char [20];
       sprintf(name,"t4_waiter%d",i);
       t = new Thread(name);
       t->Fork((VoidFunctionPtr)t4_waiter,0);
   }
   t = new Thread("t4_signaller");
   t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
   for (  i = 0; i < 6; i++ )
       t4_done.P();

    // Test 5

   printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
   printf("completes\n");

   t = new Thread("t5_t1");
   t->Fork((VoidFunctionPtr)t5_t1,0);

   t = new Thread("t5_t2");
   t->Fork((VoidFunctionPtr)t5_t2,0);
    
}
using namespace std;

enum clerkState{AVAILABLE, BUSY, ONBREAK};
//variables for application clerks. Need to be initialzed
Lock ClerkLineLock("ClerkLineLock");
Lock incrementCount("incrementCount");
vector<int> customerApplicationStatus;

vector<Lock*> ApplicationClerkLineLock;
vector<Condition*> ApplicationClerkLineCV;
vector<Condition*> ApplicationClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> ApplicationClerkBribeLineCV;
vector<Condition*> ApplicationClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> ApplicationClerkLineCount;
vector<int> ApplicationClerkBribeLineCount;
vector<clerkState> ApplicationClerkState;
vector<int> ApplicationClerkData;

//variables for picture clerks. Need to be initialzed
vector<Lock*> pictureClerkLineLock;
vector<Condition*> pictureClerkLineCV;
vector<Condition*> pictureClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> pictureClerkBribeLineCV;
vector<Condition*> pictureClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> pictureClerkLineCount;
vector<int> pictureClerkBribeLineCount;
vector<clerkState> pictureClerkState;
vector<int> pictureClerkData;

vector<int> pictureAcceptance;

//vector<bool> pictureClerkCustomerWaiting;
vector<int> passportClerkCustomerId;
vector<clerkState> passportClerkState;
vector<Lock*> passportClerkLineLock;
vector<Condition*> passportClerkLineCV;
vector<Condition*> passportClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> passportClerkBribeLineCV;
vector<Condition*> passportClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> passportClerkLineCount;
vector<int> passportClerkBribeLineCount;

//vector<bool> pictureClerkCustomerWaiting;
vector<int> CashierCustomerId;
vector<clerkState> CashierState;
vector<Lock*> CashierLineLock;
vector<Condition*> CashierLineCV;
vector<Condition*> CashierLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> CashierBribeLineCV;
vector<Condition*> CashierBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> CashierLineCount;
vector<int> CashierBribeLineCount;

Lock* senatorWaitLock;
Condition* senatorWaitCV;

bool hasSenator=false;
bool isFirst=true;
int customerNum=-1;

int senatorNum=-1;
void Customer(){
  //  cout<<"c4"<<endl;
//determine amount of money customer has
    incrementCount.Acquire();
    int id = customerNum + 1;
    customerNum++;
    incrementCount.Release();
    
    int money;
    int randomNum = rand() % 4;
    
    if(randomNum == 0){
        money = 100;
    }
    else if(randomNum == 1){
        money = 600;
    }
    else if(randomNum == 2){
        money = 1100;
    }
    else if(randomNum == 3){
        money = 1600;
    }
   // cout<<"c21"<<endl;
//find the shortest line of application clerk line
    while(customerApplicationStatus[id]!=10){
    /*    if(customerApplicationStatus[id]==0){
    if(hasSenator){
        cout<<"Customer["<<id<<"] is leaving the Passport Office."<<endl;
    }
    
    ClerkLineLock.Acquire();
    if(money >= 500){//have enough bribe money
        money-=500;
        int myLine;
        
        int shortestApplicationBribeLine = -1;
        int shortestApplicationBribeLineSize = INT_MAX;
        
        for(unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++){//avaliable application clerk check
            
            if(ApplicationClerkBribeLineCount[i] < shortestApplicationBribeLineSize){
                
                shortestApplicationBribeLine = i;
                shortestApplicationBribeLineSize = ApplicationClerkBribeLineCount[i];
                
            }
        }
        
        int shortestPictureBribeLine = -1;
        int shortestPictureBribeLineSize = INT_MAX;
        
        for(unsigned int i = 0;i < pictureClerkLineLock.size(); i++){//available picture clerk check
            if(pictureClerkBribeLineCount[i] < shortestPictureBribeLineSize){
                
                shortestPictureBribeLine = i;
                shortestPictureBribeLineSize = pictureClerkBribeLineCount[i];
                
            }
        }
        if(shortestApplicationBribeLineSize <= shortestPictureBribeLineSize){//determine my line and my line size
            myLine = shortestApplicationBribeLine;
            
           // if(ApplicationClerkState[myLine] == BUSY){
                //wait in the application clerk line
                ApplicationClerkBribeLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in bribe line for ApplicationClerk[" << myLine << "]" << endl;

                ApplicationClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
                ApplicationClerkBribeLineCount[myLine]--;
           // }
            ClerkLineLock.Release();
            ApplicationClerkLineLock[myLine]->Acquire();
            ApplicationClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
            
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);

            //wait clerk to do their job
            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);

            ApplicationClerkLineLock[myLine]->Release();

        }
        else{
            myLine = shortestPictureBribeLine;
            
           // if(PictureClerkState[myLine] == BUSY){
                //wait in the picture clerk line
                pictureClerkBribeLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in bribe line for PictureClerk[" << myLine << "]" << endl;
                pictureClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
                pictureClerkBribeLineCount[myLine]--;
           // }
            ClerkLineLock.Release();
            pictureClerkLineLock[myLine]->Acquire();
            pictureClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to PictureClerk["<<myLine<<"]"<<endl;
            
            pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

            pictureAcceptance[myLine] = rand() % 10; // customer decide whether receive the picture
            pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);

            //wait clerk to do their job
            pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

            pictureClerkLineLock[myLine]->Release();
        }
    }
    
    else{//no bribe money
        
        int myLine;
        int lineSize;
        
        int shortestApplicationLine = -1;
        int shortestApplicationLineSize = INT_MAX;
        
        for(unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++){
            if(ApplicationClerkLineCount[i] < shortestApplicationLineSize){
                
                shortestApplicationLine = i;
                shortestApplicationLineSize = ApplicationClerkLineCount[i];
                
            }
        }
        
        int shortestPictureLine = -1;
        int shortestPictureLineSize = INT_MAX;
        
        for(unsigned int i = 0; i < pictureClerkLineLock.size(); i++){
            if(pictureClerkLineCount[i] < shortestPictureLineSize){
                
                shortestPictureLine = i;
                shortestPictureLineSize = pictureClerkLineCount[i];
                
            }
            
        }
        if(shortestApplicationLineSize <= shortestPictureLineSize){
            myLine = shortestApplicationLine;
            
           // if(ApplicationClerkState[myLine] == BUSY){
                ApplicationClerkLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in regular line for ApplicationClerk[" << myLine << "]" << endl;
                ApplicationClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
                ApplicationClerkLineCount[myLine]--;
            //}
            ClerkLineLock.Release();
            ApplicationClerkLineLock[myLine]->Acquire();
            ApplicationClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            //wait clerk to do their job
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
        }
        else{
            myLine = shortestPictureLine;
           // if(PictureClerkState[myLine] == BUSY){
                pictureClerkLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in regular line for PictureClerk[" << myLine << "]" << endl;
                pictureClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
                pictureClerkLineCount[myLine]--;
           // }
            ClerkLineLock.Release();
            pictureClerkLineLock[myLine]->Acquire();
            pictureClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to PictureClerk["<<myLine<<"]"<<endl;
            pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            //wait clerk to do their job
            pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            
            pictureAcceptance[myLine] = rand() % 10; // customer decide whether receive the picture

            pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            pictureClerkLineLock[myLine]->Release();
        }
    }
        }//if customer application status =0*/
    //second round
    int choseClerk=rand()%2;
       // cout<<"random choice: "<<choseClerk<<endl;
    if((customerApplicationStatus[id]==1)|| (customerApplicationStatus[id]==0 && choseClerk==0)){//has finished applicaiton clerk
        //cout<<"start second round"<<endl;
        ClerkLineLock.Acquire();
        if(money>500){//can bribe
            int myLine;
            int shortestPictureBribeLine = -1;
            int shortestPictureBribeLineSize = INT_MAX;
            
            for(unsigned int i = 0;i < pictureClerkLineLock.size(); i++){
                if(pictureClerkBribeLineCount[i] < shortestPictureBribeLineSize){
                    
                    shortestPictureBribeLine = i;
                    shortestPictureBribeLineSize = pictureClerkBribeLineCount[i];
                    
                }
            }
            myLine = shortestPictureBribeLine;
            
            // if(PictureClerkState[myLine] == BUSY){
            //wait in the picture clerk line
            pictureClerkBribeLineCount[myLine]++;
           cout << "Customer[" << id << "] has gotten in bribe line for PictureClerk[" << myLine << "]" << endl;
            pictureClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
            pictureClerkBribeLineCount[myLine]--;
            // }
            ClerkLineLock.Release();
            pictureClerkLineLock[myLine]->Acquire();
            pictureClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to PictureClerk["<<myLine<<"]"<<endl;
            
            pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            
            pictureAcceptance[myLine] = rand() % 10; // customer decide whether receive the picture
            pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            
            //wait clerk to do their job
            pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            
            pictureClerkLineLock[myLine]->Release();
        
        }
        else{
            int myLine;
            int shortestPictureLine = -1;
            int shortestPictureLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < pictureClerkLineLock.size(); i++){
                if(pictureClerkLineCount[i] < shortestPictureLineSize){
                    
                    shortestPictureLine = i;
                    shortestPictureLineSize = pictureClerkLineCount[i];
                    
                }
                
            }
            myLine = shortestPictureLine;
            // if(PictureClerkState[myLine] == BUSY){
            pictureClerkLineCount[myLine]++;
//            cout << "Customer[" << id << "] pictureClerkLineCount[" << myLine << "] " << pictureClerkLineCount[myLine] << endl;
            cout << "Customer[" << id << "] has gotten in regular line for PictureClerk[" << myLine << "]" << endl;
            pictureClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
            pictureClerkLineCount[myLine]--;           // }
            ClerkLineLock.Release();
            pictureClerkLineLock[myLine]->Acquire();
            pictureClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to PictureClerk["<<myLine<<"]"<<endl;
            pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            //wait clerk to do their job
            pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            pictureAcceptance[myLine] = rand() % 10; // customer decide whether receive the picture
            pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            //wait clerk to do their job
            pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            
            pictureClerkLineLock[myLine]->Release();
            
            
        }
        
        
        
        
    }
    
    else if((customerApplicationStatus[id]==2) || (customerApplicationStatus[id]==0 && choseClerk==1)){//has finished picture clerk
        ClerkLineLock.Acquire();
        if(money>500){//has bribe money
            int myLine;
            int shortestApplicationBribeLine = -1;
            int shortestApplicationBribeLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++){//avaliable application clerk check
                
                if(ApplicationClerkBribeLineCount[i] < shortestApplicationBribeLineSize){
                    
                    shortestApplicationBribeLine = i;
                    shortestApplicationBribeLineSize = ApplicationClerkBribeLineCount[i];
                    
                }
            }
            myLine = shortestApplicationBribeLine;
            
            // if(ApplicationClerkState[myLine] == BUSY){
            //wait in the application clerk line
            ApplicationClerkBribeLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in bribe line for ApplicationClerk[" << myLine << "]" << endl;
            
            ApplicationClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
            ApplicationClerkBribeLineCount[myLine]--;
            // }
            ClerkLineLock.Release();
            ApplicationClerkLineLock[myLine]->Acquire();
            ApplicationClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
            
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            
            //wait clerk to do their job
            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            
            ApplicationClerkLineLock[myLine]->Release();
            
        }
        else{//does not have bribe money
            int myLine;
            int shortestApplicationLine = -1;
            int shortestApplicationLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++){
                if(ApplicationClerkLineCount[i] < shortestApplicationLineSize){
                    
                    shortestApplicationLine = i;
                    shortestApplicationLineSize = ApplicationClerkLineCount[i];
                    
                }
            }
            
            myLine = shortestApplicationLine;
            
            // if(ApplicationClerkState[myLine] == BUSY){
            ApplicationClerkLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in regular line for ApplicationClerk[" << myLine << "]" << endl;
            ApplicationClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
            ApplicationClerkLineCount[myLine]--;
            //}
            ClerkLineLock.Release();
            ApplicationClerkLineLock[myLine]->Acquire();
            ApplicationClerkData[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to ApplicationClerk["<<myLine<<"]"<<endl;
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            //wait clerk to do their job
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
            
        }
        
    }
        
    else if(customerApplicationStatus[id]==3){//passport clerk case
        ClerkLineLock.Acquire();
        
        if(money>500){//has bribe money
            int myLine;
            int shortestPassportBribeLine = -1;
            int shortestPassportBribeLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < passportClerkLineLock.size(); i++){//avaliable application clerk check
                
                if(passportClerkBribeLineCount[i] < shortestPassportBribeLineSize){
                    
                    shortestPassportBribeLine = i;
                    shortestPassportBribeLineSize = passportClerkBribeLineCount[i];
                    
                }
            }
            myLine = shortestPassportBribeLine;
            
            // if(ApplicationClerkState[myLine] == BUSY){
            //wait in the application clerk line
            passportClerkBribeLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in bribe line for PassportClerk[" << myLine << "]" << endl;
            
            passportClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
            passportClerkBribeLineCount[myLine]--;
            // }
            ClerkLineLock.Release();
            passportClerkLineLock[myLine]->Acquire();
            passportClerkCustomerId[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to PassportClerk["<<myLine<<"]"<<endl;
            
            passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
            
            //wait clerk to do their job
            passportClerkBribeLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
            
            
            passportClerkLineLock[myLine]->Release();
            
        }
        else{//does not have bribe money
            int myLine;
            int shortestPassportLine = -1;
            int shortestPassportLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < passportClerkLineLock.size(); i++){
                if(passportClerkLineCount[i] < shortestPassportLineSize){
                    
                    shortestPassportLine = i;
                    shortestPassportLineSize = passportClerkLineCount[i];
                    
                }
            }
            
            myLine = shortestPassportLine;
            
            // if(ApplicationClerkState[myLine] == BUSY){
            passportClerkLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in regular line for PassportClerk[" << myLine << "]" << endl;
            passportClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
            passportClerkLineCount[myLine]--;
            //}
            ClerkLineLock.Release();
            passportClerkLineLock[myLine]->Acquire();
            passportClerkCustomerId[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to PassportClerk["<<myLine<<"]"<<endl;
            passportClerkLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
            //wait clerk to do their job
            passportClerkLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
            passportClerkLineLock[myLine]->Release();
            
        }

        
        
    }

    else if(customerApplicationStatus[id]==6){//cashier case
        ClerkLineLock.Acquire();
        
        if(money>500){//has bribe money
            int myLine;
            int shortestCashierBribeLine = -1;
            int shortestCashierBribeLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < CashierLineLock.size(); i++){//avaliable application clerk check
                
                if(CashierBribeLineCount[i] < shortestCashierBribeLineSize){
                    
                    shortestCashierBribeLine = i;
                    shortestCashierBribeLineSize = CashierBribeLineCount[i];
                    
                }
            }
            myLine = shortestCashierBribeLine;
            
            // if(ApplicationClerkState[myLine] == BUSY){
            //wait in the application clerk line
            CashierBribeLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in bribe line for Cashier[" << myLine << "]" << endl;
            
            CashierBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
            CashierBribeLineCount[myLine]--;
            // }
            ClerkLineLock.Release();
            CashierLineLock[myLine]->Acquire();
            CashierCustomerId[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to Cashier["<<myLine<<"]"<<endl;
            
            CashierBribeLineCV[myLine]->Signal(CashierLineLock[myLine]);
            
            //wait clerk to do their job
            CashierBribeLineCV[myLine]->Wait(CashierLineLock[myLine]);
            
            CashierLineLock[myLine]->Release();
            
        }
        else{//does not have bribe money
            int myLine;
            int shortestCashierLine = -1;
            int shortestCashierLineSize = INT_MAX;
            
            for(unsigned int i = 0; i < CashierLineLock.size(); i++){
                if(CashierLineCount[i] < shortestCashierLineSize){
                    
                    shortestCashierLine = i;
                    shortestCashierLineSize = passportClerkLineCount[i];
                    
                }
            }
            
            myLine = shortestCashierLine;
            
            // if(ApplicationClerkState[myLine] == BUSY){
            CashierLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in regular line for Cashier[" << myLine << "]" << endl;
            CashierLineWaitCV[myLine]->Wait(&ClerkLineLock);
            CashierLineCount[myLine]--;
            //}
            ClerkLineLock.Release();
            CashierLineLock[myLine]->Acquire();
            CashierCustomerId[myLine]=id;
            cout<<"Customer["<<id<<"] has given SSN ["<<id<<"] to Cashier["<<myLine<<"]"<<endl;
            CashierLineCV[myLine]->Signal(CashierLineLock[myLine]);
            //wait clerk to do their job
            CashierLineCV[myLine]->Wait(CashierLineLock[myLine]);
            CashierLineLock[myLine]->Release();
            
        }

        
        
    }
    }//while loop
    
    
}

void ApplicationClerk(int myLine){
 // cout << "ApplicationClerk debug" << endl;
    int incoming=0;
    while(true){
        //cout<<"m3"<<endl;
    bool InBribeLine=false;
        senatorWaitLock->Acquire();
        if(hasSenator){
            if(isFirst){
            isFirst=FALSE;
           // cout<<"m2"<<endl;
            cout<<"senator data received by applicaiton clerk["<<myLine<<"]"<<endl;
            senatorWaitCV->Signal(senatorWaitLock);
            }
        }
        senatorWaitLock->Release();
    ClerkLineLock.Acquire();
    if(ApplicationClerkBribeLineCount[myLine]>0){
        ApplicationClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
        cout<<"ApplicationClerk["<<myLine<<"] has signalled a Customer to come to their counter."<<endl;
        ApplicationClerkState[myLine]=BUSY;
        InBribeLine=true;
    }
    else if(ApplicationClerkLineCount[myLine]>0){
        ApplicationClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
        cout<<"ApplicationClerk["<<myLine<<"] has signalled a Customer to come to their counter."<<endl;
        ApplicationClerkState[myLine]=BUSY;

    }
    else{
        ApplicationClerkState[myLine]=AVAILABLE;
        ClerkLineLock.Release();
        currentThread->Yield();//context switch
        continue;
    }
        ApplicationClerkLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        //wait for customer data
        if(InBribeLine){//in bribe line
            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            //do my job customer now waiting
            cout<<"ApplicationClerk["<<myLine<<"] has received SSN ["<<ApplicationClerkData[myLine]<<"] from Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            cout<<"ApplicationClerk["<<myLine<<"] has recorded a completed application for Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            cout<<"ApplicationClerk["<<myLine<<"] has received $500 from Customer["<<ApplicationClerkData[myLine]<<"]"<<endl;
            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
        }
        else{//not in bribe line
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            //do my job customer now waiting
            cout<<"ApplicationClerk["<<myLine<<"] has received SSN ["<<ApplicationClerkData[myLine]<<"] from Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            cout<<"ApplicationClerk["<<myLine<<"] has recorded a completed application for Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
            
        }
        currentThread->Yield();//context switch
    }//while
}

void PictureClerk(int myLine){
 // cout << "pictureClerk debug" << endl;
  while(true){
    //cout << "before lock" << endl;
    
    ClerkLineLock.Acquire();
     //cout << "after lock" << endl;
    bool inBribeLine = false;
    int id = 0; 
//    cout << "Clerk pictureClerkLineCount[" << myLine << "] " << pictureClerkLineCount[myLine] << endl;
    if (pictureClerkBribeLineCount[myLine] > 0){
      pictureClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
      cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
      pictureClerkState[myLine] = BUSY;
      inBribeLine = true;
    } else if(pictureClerkLineCount[myLine] > 0){      pictureClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
      cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
      pictureClerkState[myLine] = BUSY;
    } else{
      pictureClerkState[myLine] = AVAILABLE;
      ClerkLineLock.Release();
      currentThread->Yield();//context switch
      continue;
    }

    pictureClerkLineLock[myLine]->Acquire();
    ClerkLineLock.Release();
    if (inBribeLine){

      pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
      id = pictureClerkData[myLine];
      cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
      cout << "PictureClerk [" << myLine << "] has taken a picture of Customer [" << id << "]" << endl;

      pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
      pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

      if (pictureAcceptance[myLine] > 2){
        cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

        int numCalls = rand() % 80 + 20;
        for (int i = 0; i < numCalls; i++){
          currentThread->Yield();
        }

        customerApplicationStatus[id] += 2;
        // if (pictureClerkCustomerWaiting[myLine] == true){
          pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
        // }
      } else{
          
        cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
          pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
      }

    } else{

      pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
      id = pictureClerkData[myLine];

      cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
      cout << "PictureClerk [" << myLine << "] has taken a picture of Customer [" << id << "]" << endl;

      pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
      pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

      if (pictureAcceptance[myLine] > 2){
        cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

        int numCalls = rand() % 80 + 20;
        for (int i = 0; i < numCalls; i++){
          currentThread->Yield();
        }

        customerApplicationStatus[id] += 2;
        // if (pictureClerkCustomerWaiting[myLine] == true){
          pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
        // }
      } else{
        cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
          pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
      }

    }
    
    pictureClerkLineLock[myLine]->Release();

  }
}

void PassportClerk(int myLine){
    int id = 0;
//    passportClerkState[myLine] = ONBREAK;
    
    while(true){
        
        ClerkLineLock.Acquire();
        bool inBribeLine = false;
        
        if (passportClerkState[myLine] != ONBREAK){
            if (passportClerkBribeLineCount[myLine] > 0){
                passportClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "PassportClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                passportClerkState[myLine] = BUSY;
                inBribeLine = true;
            } else if(passportClerkLineCount[myLine] > 0){
                passportClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "PassportClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                passportClerkState[myLine] = BUSY;
            } else{
                passportClerkState[myLine] = AVAILABLE;
                ClerkLineLock.Release();
                currentThread->Yield();//context switch
                continue;
            }
        }
        else{
            ClerkLineLock.Release();
            currentThread->Yield();//context switch
            continue;
        }
        
        passportClerkLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        
        if (inBribeLine){
            
            passportClerkBribeLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];
            
            //Collect Bribe Money From Customer
//            passportMoenyLock->Acquire();
//            MoneyFromPassportClerk += 500;
//            cout<<"PassportClerk["<<myLine<<"] has received $500 from Customer["<<id<<"]"<<endl;
//            passportMoenyLock->Release();
            
            cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            int passportClerkPunishment = rand() % 100;
            if (passportClerkPunishment > 5){
                
                cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;
                
                int numCalls = rand() % 80 + 20;
                for (int i = 0; i < numCalls; i++){
                    currentThread->Yield();
                }
                
                customerApplicationStatus[id] += 3;
                cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
                passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
            } else{
                
                cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
                passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
            }
            
        } else{
            
            passportClerkLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];

            cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            int passportClerkPunishment = rand() % 100;
            if (passportClerkPunishment > 5){
                cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;
                
                int numCalls = rand() % 80 + 20;
                for (int i = 0; i < numCalls; i++){
                    currentThread->Yield();
                }
                
                customerApplicationStatus[id] += 3;
                cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
                passportClerkLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
            } else{
                
                cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
                passportClerkLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
            }
            
        }
        
        passportClerkLineLock[myLine]->Release();
        
    }
}

void Cashier(int myLine){
    // CashierState[myLine] = ONBREAK;
    int id = 0;
    
    while (true){
        
        ClerkLineLock.Acquire();
        bool inBribeLine = false;
        //int id = CashierCustomerId[myLine];
        
       
        if (CashierState[myLine] != ONBREAK){
            //When CashierState != ONBREAK
            if (CashierBribeLineCount[myLine] > 0){
                CashierBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
                inBribeLine = true;
            }   else if (CashierLineCount[myLine] > 0){
                CashierLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
            }
            else {
                ClerkLineLock.Release();
                CashierState[myLine] = AVAILABLE;     //!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                // CashierCV[myLine]->Wait(CashierLock[myLine]);
                currentThread->Yield();
                continue;
            }
        }
        else {  //When CashierState == ONBREAK, Do Nothing
            ClerkLineLock.Release();
            currentThread->Yield();
            continue;
        }
        
        CashierLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        
        if (inBribeLine){
            //In BribeLine
            
            
            CashierBribeLineCV[myLine]->Wait(CashierLineLock[myLine]);
            id = CashierCustomerId[myLine];
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
      
            
            
            int cashierPunishment = rand() % 100;
            if (cashierPunishment > 5) {  // Passed All the tests (Certified)
                cout << "Cashier [" << myLine << "] has verified that Customer [" << id << "] has been certified by a PassportClerk" << endl;
                
                //Collect Fee From Customer
                // cashierMoneyLock.Acquire();
                // MoneyFromCashier += 100;
                // cashierMoneyLock.Release();
                
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                cout << "Cashier [" << id << "] has provided Customer[identifier] their completed passport" << endl;
                customerApplicationStatus[id] += 4;
                cout << "Cashier [" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
                CashierBribeLineCV[myLine]->Signal(CashierLineLock[myLine]);
            }
            else {  //Not yet certified
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
                
                CashierBribeLineCV[myLine]->Signal(CashierLineLock[myLine]);
                
            }
        }
        
        else {
            // NOT inBribeLine
         
            
            CashierLineCV[myLine]->Wait(CashierLineLock[myLine]);
            id = CashierCustomerId[myLine];
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            
            int cashierPunishment = rand() % 100;
            if (cashierPunishment > 5) {   // Passed All the tests (Certified)
                cout << "Cashier [" << myLine << "] has verified that Customer [" << id << "] has been certified by a PassportClerk" << endl;
                
                
                //Collect Fee From Customer
                // cashierMoneyLock.Acquire();
                // MoneyFromCashier += 100;
                // cashierMoneyLock.Release();
                
                
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
                
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                cout << "Cashier [" << id << "] has provided Customer[identifier] their completed passport" << endl;
                customerApplicationStatus[id] += 4;
                cout << "Cashier [" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
                CashierLineCV[myLine]->Signal(CashierLineLock[myLine]);
            
            }
            else {  //Not yet Certified

                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
                
                CashierLineCV[myLine]->Signal(CashierLineLock[myLine]);
                
            }
        }
        
        CashierLineLock[myLine]->Release();
    }   //while loop
}

void Senator(){
     hasSenator=TRUE;
    ClerkLineLock.Acquire();
    cout<<"a senator shows up"<<endl;
    int id=senatorNum+1;
    senatorNum++;
    senatorWaitLock->Acquire();
    //cout<<"mighty"<<endl;
    senatorWaitCV->Wait(senatorWaitLock);//wait for a clerk
    cout<<"senator has filed application now"<<endl;
    
    senatorWaitLock->Release();
    
    hasSenator=FALSE;
    isFirst=TRUE;
    ClerkLineLock.Release();
    
}


void PassportOffice(){
    
    int numApplicationClerk;
    int numPictureClerk;
    int numPassportClerk;
    int numCashier;
    int numCustomer;
    int numSenator;
    Thread* t1;
    char integer[32];

    cout<<"Passport Office Simulation started."<<endl;

    cout<<"Enter the numebr of application clerks(1-5): "<<endl;
    cin>>numApplicationClerk;
    cout<<"Enter the numebr of picture clerks(1-5): "<<endl;
    cin>>numPictureClerk;
    cout<<"Enter the numebr of passport clerks(1-5): "<<endl;
    cin>>numPassportClerk;
    cout<<"Enter the numebr of cashiers(1-5): "<<endl;
    cin>>numCashier;
    cout<<"Enter the numebr of customers: "<<endl;
    cin>>numCustomer;
    cout<<"Enter the number of senators: "<<endl;
    cin>>numSenator;
    
    
    
    
    
    for(int i=0;i<numApplicationClerk;i++){
      char lockName[100]="ApplicationLock";
      strcat(lockName,integer);
      
      sprintf(integer,"%d",i );
      //application lock initialize
      Lock* applicationLock=new Lock(lockName);
      ApplicationClerkLineLock.push_back(applicationLock);
      
      //aplication CV initialize
      char applicationCVName[100]="applicationCV";
      strcat(applicationCVName, integer);
      Condition* applicationCV=new Condition(applicationCVName);
      ApplicationClerkLineCV.push_back(applicationCV);
      //application bribe CV initialize
      char applicaitonBribeCVName[100]="applicaitonBribeCV";
      strcat(applicaitonBribeCVName, integer);
      Condition* applicationBribeCV=new Condition(applicaitonBribeCVName);
      ApplicationClerkBribeLineCV.push_back(applicationBribeCV);
      //application Wait CV initialize
      char applicationWaitCVName[100]="applicationWaitCV";
      strcat(applicationWaitCVName, integer);
      Condition* applicationWaitCV=new Condition(applicationWaitCVName);
      ApplicationClerkLineWaitCV.push_back(applicationWaitCV);
      //application Bribe Wait CV initialize
      char applicationBribeWaitCVName[100]="applicationWaitCV";
      strcat(applicationBribeWaitCVName, integer);
      Condition* applicationBribeWaitCV=new Condition(applicationBribeWaitCVName);
      ApplicationClerkBribeLineWaitCV.push_back(applicationBribeWaitCV);
      //application line size intialize
      ApplicationClerkLineCount.push_back(0);
      //application bribe line size initialize
      ApplicationClerkBribeLineCount.push_back(0);
      //application clerk state initialize
      clerkState ct=AVAILABLE;
      ApplicationClerkState.push_back(ct);
      //application data initialize
      ApplicationClerkData.push_back(0);
      
    }
   
    for(int i=0;i<numPictureClerk;i++){
      char lockName[100]="PictureLock";
      strcat(lockName,integer);
      
      sprintf(integer,"%d",i );
      //application lock initialize
      Lock* pictureLock=new Lock(lockName);
      pictureClerkLineLock.push_back(pictureLock);
      
      //aplication CV initialize
      char pictureCVName[100]="pictureCV";
      strcat(pictureCVName, integer);
      Condition* pictureCV=new Condition(pictureCVName);
      pictureClerkLineCV.push_back(pictureCV);
      //application bribe CV initialize
      char pictureBribeCVName[100]="pictureBribeCV";
      strcat(pictureBribeCVName, integer);
      Condition* pictureBribeCV=new Condition(pictureBribeCVName);
      pictureClerkBribeLineCV.push_back(pictureBribeCV);
      //application Wait CV initialize
      char pictureWaitCVName[100]="pictureWaitCV";
      strcat(pictureWaitCVName, integer);
      Condition* pictureWaitCV=new Condition(pictureWaitCVName);
      pictureClerkLineWaitCV.push_back(pictureWaitCV);
      //application Bribe Wait CV initialize
      char pictureBribeWaitCVName[100]="pictureWaitCV";
      strcat(pictureBribeWaitCVName, integer);
      Condition* pictureBribeWaitCV=new Condition(pictureBribeWaitCVName);
      pictureClerkBribeLineWaitCV.push_back(pictureBribeWaitCV);
      //application line size intialize
      pictureClerkLineCount.push_back(0);
      //application bribe line size initialize
      pictureClerkBribeLineCount.push_back(0);
      //application clerk state initialize
      clerkState ct=AVAILABLE;
      pictureClerkState.push_back(ct);
      //application data initialize
      pictureClerkData.push_back(0);
      pictureAcceptance.push_back(0);
      
    }
   
    for(int i=0;i<numPassportClerk;i++){
      char lockName[100]="PassportLock";
      strcat(lockName,integer);
      
      sprintf(integer,"%d",i );
      //application lock initialize
      Lock* passportLock=new Lock(lockName);
      passportClerkLineLock.push_back(passportLock);
      
      //aplication CV initialize
      char passportCVName[100]="passportCV";
      strcat(passportCVName, integer);
      Condition* passportCV=new Condition(passportCVName);
      passportClerkLineCV.push_back(passportCV);
      //application bribe CV initialize
      char passportBribeCVName[100]="passportBribeCV";
      strcat(passportBribeCVName, integer);
      Condition* passportBribeCV=new Condition(passportBribeCVName);
      passportClerkBribeLineCV.push_back(passportBribeCV);
      //application Wait CV initialize
      char passportWaitCVName[100]="passportWaitCV";
      strcat(passportWaitCVName, integer);
      Condition* passportWaitCV=new Condition(passportWaitCVName);
      passportClerkLineWaitCV.push_back(passportWaitCV);
      //application Bribe Wait CV initialize
      char passportBribeWaitCVName[100]="passportWaitCV";
      strcat(passportBribeWaitCVName, integer);
      Condition* passportBribeWaitCV=new Condition(passportBribeWaitCVName);
      passportClerkBribeLineWaitCV.push_back(passportBribeWaitCV);
      //application line size intialize
      passportClerkLineCount.push_back(0);
      //application bribe line size initialize
      passportClerkBribeLineCount.push_back(0);
      //application clerk state initialize
      clerkState ct=AVAILABLE;
      passportClerkState.push_back(ct);
      //application data initialize
      passportClerkCustomerId.push_back(0);
      
    }
   
    for(int i=0;i<numCashier;i++){
      char lockName[100]="CashierLock";
      strcat(lockName,integer);
      
      sprintf(integer,"%d",i );
      //application lock initialize
      Lock* CashierLock=new Lock(lockName);
      CashierLineLock.push_back(CashierLock);
      
      //aplication CV initialize
      char CashierCVName[100]="cashierCV";
      strcat(CashierCVName, integer);
      Condition* CashierCV=new Condition(CashierCVName);
      CashierLineCV.push_back(CashierCV);
      //application bribe CV initialize
      char CashierBribeCVName[100]="cashierBribeCV";
      strcat(CashierBribeCVName, integer);
      Condition* CashierBribeCV=new Condition(CashierBribeCVName);
      CashierBribeLineCV.push_back(CashierBribeCV);
      //application Wait CV initialize
      char CashierWaitCVName[100]="cashierWaitCV";
      strcat(CashierWaitCVName, integer);
      Condition* CashierWaitCV=new Condition(CashierWaitCVName);
      CashierLineWaitCV.push_back(CashierWaitCV);
      //application Bribe Wait CV initialize
      char CashierBribeWaitCVName[100]="cashierWaitCV";
      strcat(CashierBribeWaitCVName, integer);
      Condition* CashierBribeWaitCV=new Condition(CashierBribeWaitCVName);
      CashierBribeLineWaitCV.push_back(CashierBribeWaitCV);
      //application line size intialize
      CashierLineCount.push_back(0);
      //application bribe line size initialize
      CashierBribeLineCount.push_back(0);
      //application clerk state initialize
      clerkState ct=AVAILABLE;
      CashierState.push_back(ct);
      //application data initialize
      CashierCustomerId.push_back(0);
      
    }

    for(int i =0 ;i<5 ;i++){
      customerApplicationStatus.push_back(0);
    }
        cout<<"Test case 1: 2 customers and 1 application clerk: "<<endl;
    
//    for(int i=0;i<numApplicationClerk;i++){
//        char applicationClerk[100]="applicationClerk";
//        
//        strcat(applicationClerk,integer);
//        
//        t1=new Thread(applicationClerk);
//        t1->Fork((VoidFunctionPtr)ApplicationClerk,i);
//        
//    }
//    
//    for(int i=0;i<numCustomer;i++){
//        char customer[100]="customer";
//        strcat(customer, integer);
//        t1=new Thread(customer);
//        t1->Fork((VoidFunctionPtr)Customer,0);
//    }
//*
//
//        Lock* l1=new Lock("lock1");
//        ApplicationClerkLineLock.push_back(l1);
//    
//        Condition* c1=new Condition("Condition1");
//        ApplicationClerkLineCV.push_back(c1);
//
//
//        Condition* c2=new Condition("Condition2");
//        ApplicationClerkBribeLineCV.push_back(c2);
//    
//        Condition* c3=new Condition("Condition3");
//        ApplicationClerkLineWaitCV.push_back(c3);
//    
//        Condition* c4=new Condition("Condition4");
//        ApplicationClerkBribeLineWaitCV.push_back(c4);
//
//        ApplicationClerkLineCount.push_back(0);
//
//        ApplicationClerkBribeLineCount.push_back(0);
//
//        clerkState ct=AVAILABLE;
//        ApplicationClerkState.push_back(ct);
    senatorWaitLock=new Lock("senator");
    senatorWaitCV=new Condition("senatorCV");

    cout << "applicationClerk Threads Start" << endl;
    t1=new Thread("ApplicationClerk1");
    t1->Fork((VoidFunctionPtr)ApplicationClerk,0);

    t1=new Thread("ApplicationClerk2");
    t1->Fork((VoidFunctionPtr)ApplicationClerk,1);
    t1=new Thread("ApplicationClerk3");
    t1->Fork((VoidFunctionPtr)ApplicationClerk,2);
    t1=new Thread("ApplicationClerk4");
    t1->Fork((VoidFunctionPtr)ApplicationClerk,3);
    t1=new Thread("ApplicationClerk5");
    t1->Fork((VoidFunctionPtr)ApplicationClerk,4);


    cout << "pictureClerk Threads Start" << endl;
    t1=new Thread("PictureClerk1");
    t1->Fork((VoidFunctionPtr)PictureClerk,0);
    t1=new Thread("PictureClerk2");
    t1->Fork((VoidFunctionPtr)PictureClerk,1);
    t1=new Thread("PictureClerk3");
    t1->Fork((VoidFunctionPtr)PictureClerk,2);
    t1=new Thread("PictureClerk4");
    t1->Fork((VoidFunctionPtr)PictureClerk,3);
    t1=new Thread("PictureClerk5");
    t1->Fork((VoidFunctionPtr)PictureClerk,4);
    
    cout << "PassportClerk Threads Start" << endl;
    t1=new Thread("PassportClerk1");
    t1->Fork((VoidFunctionPtr)PassportClerk,0);
    t1=new Thread("PassportClerk2");
    t1->Fork((VoidFunctionPtr)PassportClerk,1);
    t1=new Thread("PassportClerk3");
    t1->Fork((VoidFunctionPtr)PassportClerk,2);
    t1=new Thread("PassportClerk4");
    t1->Fork((VoidFunctionPtr)PassportClerk,3);
    t1=new Thread("PassportClerk5");
    t1->Fork((VoidFunctionPtr)PassportClerk,4);
    
    cout << "Cashier Threads Start" << endl;
    t1=new Thread("Cashier1");
    t1->Fork((VoidFunctionPtr)Cashier,0);
    t1=new Thread("Cashier2");
    t1->Fork((VoidFunctionPtr)Cashier,1);
    t1=new Thread("Cashier3");
    t1->Fork((VoidFunctionPtr)Cashier,2);
    t1=new Thread("Cashier4");
    t1->Fork((VoidFunctionPtr)Cashier,3);
    t1=new Thread("Cashier5");
    t1->Fork((VoidFunctionPtr)Cashier,4);

    cout << "Customer threads start" << endl;
    t1=new Thread("Customer1");
    t1->Fork((VoidFunctionPtr)Customer,0);
    t1=new Thread("Customer2");
    t1->Fork((VoidFunctionPtr)Customer,0);

//        t1=new Thread("Senator1");
//        t1->Fork((VoidFunctionPtr)Senator, 0);

    t1=new Thread("Customer3");
    t1->Fork((VoidFunctionPtr)Customer,0);
    t1=new Thread("Customer4");
    t1->Fork((VoidFunctionPtr)Customer,0);
    t1=new Thread("Customer5");
    t1->Fork((VoidFunctionPtr)Customer,0);
}




#endif
