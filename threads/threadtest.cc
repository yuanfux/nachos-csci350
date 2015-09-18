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
Lock incrementCount("count");
vector<Lock*> ApplicationClerkLineLock;
vector<Condition*> ApplicationClerkLineCV;
vector<Condition*> ApplicationClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> ApplicationClerkBribeLineCV;
vector<Condition*> ApplicationClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> ApplicationClerkLineCount;
vector<int> ApplicationClerkBribeLineCount;
vector<clerkState> ApplicationClerkState;
//variables for picture clerks. Need to be initialzed
vector<Lock*> PictureClerkLineLock;
vector<Condition*> PictureClerkLineCV;
vector<Condition*> PictureClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> PictureClerkBribeLineCV;
vector<Condition*> PictureClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> PictureClerkLineCount;
vector<int> PictureClerkBribeLineCount;
vector<clerkState> PictureClerkState;
int customerNum=-1;

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
        
        for(unsigned int i = 0;i < PictureClerkLineLock.size(); i++){//available picture clerk check
            if(PictureClerkBribeLineCount[i] < shortestPictureBribeLineSize){
                
                shortestPictureBribeLine = i;
                shortestPictureBribeLineSize = PictureClerkBribeLineCount[i];
                
            }
        }
        if(shortestApplicationBribeLineSize <= shortestPictureBribeLineSize){//determine my line and my line size
            myLine = shortestApplicationBribeLine;
            
           // if(ApplicationClerkState[myLine] == BUSY){
                //wait in the application clerk line
                ApplicationClerkBribeLineCount[myLine]++;
                cout << "customer[" << id << "] waiting in the bribe line for application clerk[" << myLine << "]" << endl;

                ApplicationClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
                ApplicationClerkBribeLineCount[myLine]--;
           // }
            ClerkLineLock.Release();
            ApplicationClerkLineLock[myLine]->Acquire();
            cout<<"give my"<<" customer["<<id<<"]'s "<< "data to application clerk["<<myLine<<"]"<<endl;

            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);

            //wait clerk to do their job
            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);

            ApplicationClerkLineLock[myLine]->Release();

        }
        else{
            myLine = shortestPictureBribeLine;
            
           // if(PictureClerkState[myLine] == BUSY){
                //wait in the picture clerk line
                PictureClerkBribeLineCount[myLine]++;
                cout << "customer["<< id << "] waiting in the bribe line for picture clerk[" << myLine << "]" << endl;
                PictureClerkBribeLineCV[myLine]->Wait(&ClerkLineLock);
                PictureClerkBribeLineCount[myLine]--;
           // }
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
        
        for(unsigned int i = 0; i < PictureClerkLineLock.size(); i++){
            if(PictureClerkLineCount[i] < shortestPictureLineSize){
                
                shortestPictureLine = i;
                shortestPictureLineSize = PictureClerkLineCount[i];
                
            }
            
        }
        if(shortestApplicationLineSize <= shortestPictureLineSize){
            myLine = shortestApplicationLine;
            
           // if(ApplicationClerkState[myLine] == BUSY){
                ApplicationClerkLineCount[myLine]++;
                cout << "customer[" << id << "] waiting in the line for application clerk[" << myLine << "]" << endl;
                ApplicationClerkLineCV[myLine]->Wait(&ClerkLineLock);
                ApplicationClerkLineCount[myLine]--;
            //}
            ClerkLineLock.Release();
            ApplicationClerkLineLock[myLine]->Acquire();
            cout<<"give my"<<" customer["<<id<<"]'s "<< "data to application clerk["<<myLine<<"]"<<endl;
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            //wait clerk to do their job
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
        }
        else{
            myLine = shortestPictureLine;
           // if(PictureClerkState[myLine] == BUSY){
                PictureClerkLineCount[myLine]++;
                cout << "customer[" << id << "] waiting in the line for picture clerk[" << myLine << "]" << endl;
                PictureClerkLineCV[myLine]->Wait(&ClerkLineLock);
                PictureClerkLineCount[myLine]--;
           // }
        }
    }
    
}

void ApplicationClerk(int myLine){

    int incoming=0;
    while(true){
    bool InBribeLine=false;
    ClerkLineLock.Acquire();
    if(ApplicationClerkBribeLineCount[myLine]>0){
        ApplicationClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
        ApplicationClerkState[myLine]=BUSY;
        InBribeLine=true;
    }
    else if(ApplicationClerkLineCount[myLine]>0){
        ApplicationClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
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
            cout<<"customer data received by application clerk["<<myLine<<"]"<<endl;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
        }
        else{//not in bribe line
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            //do my job customer now waiting
            cout<<"customer data received by application clerk["<<myLine<<"]"<<endl;
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
            
        }
        currentThread->Yield();//context switch
    }//while
}
void trySome(int i){
    
    cout<<"this is thread "<<i<<endl;
    
}


void PassportOffice(){
        cout<<"Passport Office Simulation."<<endl;
        cout<<"Test case 1: 2 customers and 1 application clerk "<<endl;
    
        Lock* l1=new Lock("lock1");
        ApplicationClerkLineLock.push_back(l1);
    
        Condition* c1=new Condition("Condition1");
        ApplicationClerkLineCV.push_back(c1);


        Condition* c2=new Condition("Condition2");
        ApplicationClerkBribeLineCV.push_back(c2);
    
        Condition* c3=new Condition("Condition3");
        ApplicationClerkLineWaitCV.push_back(c3);
    
        Condition* c4=new Condition("Condition4");
        ApplicationClerkBribeLineWaitCV.push_back(c4);

        ApplicationClerkLineCount.push_back(0);

        ApplicationClerkBribeLineCount.push_back(0);

        clerkState ct=AVAILABLE;
        ApplicationClerkState.push_back(ct);

    cout<<"c1"<<endl;
    Thread* t1;
    t1=new Thread("ApplicationClerk1");
    t1->Fork((VoidFunctionPtr)ApplicationClerk,0);
    t1=new Thread("Customer1");
    t1->Fork((VoidFunctionPtr)Customer,0);
    t1=new Thread("Customer2");
    t1->Fork((VoidFunctionPtr)Customer,0);
    
}



#endif
