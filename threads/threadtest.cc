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

Lock ClerkLineLock("ClerkLineLock");
Lock incrementCount("incrementCount");
vector<int> customerApplicationStatus;

//variables for application clerk
vector<Lock*> ApplicationClerkLineLock;
vector<Condition*> ApplicationClerkLineCV; //cv for each line
vector<Condition*> ApplicationClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> ApplicationClerkBribeLineCV;
vector<Condition*> ApplicationClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> ApplicationClerkLineCount; //number of customers in each line
vector<int> ApplicationClerkBribeLineCount;
vector<clerkState> ApplicationClerkState;
vector<int> ApplicationClerkData; //stores ssn's of customers for each clerk

//variables for picture clerks.
vector<Lock*> pictureClerkLineLock;
vector<Condition*> pictureClerkLineCV;
vector<Condition*> pictureClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> pictureClerkBribeLineCV;
vector<Condition*> pictureClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> pictureClerkLineCount;
vector<int> pictureClerkBribeLineCount;
vector<clerkState> pictureClerkState;
vector<int> pictureClerkData;
vector<int> pictureAcceptance; //stores whether customer likes picture or not at counter

//variables for PassportClerk
vector<int> passportClerkCustomerId; //stores ssn's of customers for each clerk
vector<clerkState> passportClerkState;
vector<Lock*> passportClerkLineLock;
vector<Condition*> passportClerkLineCV;
vector<Condition*> passportClerkLineWaitCV;//need new cv to prevent the different lock case
vector<Condition*> passportClerkBribeLineCV;
vector<Condition*> passportClerkBribeLineWaitCV;//need new cv to prevent the different lock case
vector<int> passportClerkLineCount;
vector<int> passportClerkBribeLineCount;

//variables for Cashier
vector<int> CashierCustomerId;
vector<clerkState> CashierState;
vector<Lock*> CashierLineLock;
vector<Condition*> CashierLineCV;
vector<Condition*> CashierLineWaitCV;//need new cv to prevent the different lock case
vector<int> CashierLineCount;

//variables for Manager
Lock applicationMoneyLock("applicationMoenyLock"); //each money variable needs a lock
Lock pictureMoneyLock("pictureMoenyLock");
Lock passportMoneyLock("passportMoenyLock");
Lock cashierMoneyLock("cashierMoenyLock");
int MoneyFromApplicationClerk = 0;
int MoneyFromPictureClerk = 0;
int MoneyFromPassportClerk = 0;
int MoneyFromCashier = 0;
int MoneyTotal = 0;

Lock* senatorWaitLock;
Lock* senatorApplicationWaitLock;
Lock* senatorPictureWaitLock;
Lock* senatorPassportWaitLock;
Lock* senatorCashierWaitLock;
Condition* senatorApplicationWaitCV;
Condition* senatorPictureWaitCV;
Condition* senatorPassportWaitCV;
Condition* senatorCashierWaitCV;
Condition* customerWaitCV;
Lock* customerWaitLock;
int senatorStatus = 0;

vector<int> numCustomerWaiting;

int senatorData;
int senatorServiceId;
bool hasSenator = false;


//bool hasSenator=false;
bool isFirst=true;
int customerNum=-1; // number of customers came into the office. 
                    // it is also ssn of customers
int remainingCustomer = 0; // number of customers still in the office


int senatorNum=-1;

void Customer(){
    //get ssn for each customer
    incrementCount.Acquire();
    int id = customerNum + 1;
    customerNum++;
    incrementCount.Release();

    //determine amount of money customer has
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
    cout <<"Customer[" << id << "] has $" << money << endl;
    
    //each customer needs to go through all the counters before leaving
    numCustomerWaiting.push_back(id);
    while(customerApplicationStatus[id]!=10){
        customerWaitLock->Acquire();
        
        int choseClerk=rand() % 2; //randomly choosing application or picture clerk
        
        //Goes to Picture Clerk.
        if((customerApplicationStatus[id]==1)|| (customerApplicationStatus[id]==0 && choseClerk==0)){//has finished applicaiton clerk
            ClerkLineLock.Acquire();
            if(money>500){//can bribe
                money -= 500; // give out money
                int myLine;
                int shortestPictureBribeLine = -1;
                int shortestPictureBribeLineSize = INT_MAX;
                
                //find shortest line
                for(unsigned int i = 0;i < pictureClerkLineLock.size(); i++){
                    if(pictureClerkBribeLineCount[i] < shortestPictureBribeLineSize){
                        
                        shortestPictureBribeLine = i;
                        shortestPictureBribeLineSize = pictureClerkBribeLineCount[i];
                        
                    }
                }
                myLine = shortestPictureBribeLine;
                
                //wait in the picture clerk line
                pictureClerkBribeLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in bribe line for PictureClerk[" << myLine << "]" << endl;
                customerWaitLock->Release();
                pictureClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock); //wait for signal from clerk
                pictureClerkBribeLineCount[myLine]--; //leave the line and go to the counter
                ClerkLineLock.Release();

                pictureClerkLineLock[myLine]->Acquire();
                pictureClerkData[myLine]=id; //gives clerk the ssn
                cout << "Customer[" << id << "] has given SSN [" << id << "] to PictureClerk["<<myLine<<"]" << endl;
                pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);

                //wait for picture clerk to take picture
                pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
                
                pictureAcceptance[myLine] = rand() % 10; // customer decide whether to accept the picture
                pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
                
                //wait for picture clerk to tell customer leave or not
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

                //wait in the picture clerk line
                pictureClerkLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in regular line for PictureClerk[" << myLine << "]" << endl;
                customerWaitLock->Release();
                pictureClerkLineWaitCV[myLine]->Wait(&ClerkLineLock); //wait for signal from clerk
                pictureClerkLineCount[myLine]--; 
                ClerkLineLock.Release();

                pictureClerkLineLock[myLine]->Acquire();
                pictureClerkData[myLine]=id; //gives clerk the ssn
                cout << "Customer[" << id << "] has given SSN [" << id << "] to PictureClerk["<<myLine<<"]" << endl;
                pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]); 

                //wait for picture clerk to take picture
                pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

                pictureAcceptance[myLine] = rand() % 10; // customer decide whether receive the picture
                pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);

                //wait for picture clerk to tell customer leave or not
                pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
                
                pictureClerkLineLock[myLine]->Release();
                
                
            }
            
        }
        
        //Goes to Application Clerk
        else if((customerApplicationStatus[id]==2) || (customerApplicationStatus[id]==0 && choseClerk==1)){//has finished picture clerk
            ClerkLineLock.Acquire();
            if(money>500){//has bribe money
                money -= 500;
                int myLine;
                int shortestApplicationBribeLine = -1;
                int shortestApplicationBribeLineSize = INT_MAX;
                
                //find shortest line
                for(unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++){
                    if(ApplicationClerkBribeLineCount[i] < shortestApplicationBribeLineSize){
                        shortestApplicationBribeLine = i;
                        shortestApplicationBribeLineSize = ApplicationClerkBribeLineCount[i];
                    }
                }
                myLine = shortestApplicationBribeLine;
                
                //wait in the application clerk line
                ApplicationClerkBribeLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in bribe line for ApplicationClerk[" << myLine << "]" << endl;
                customerWaitLock->Release();
                //wait to be signalled by clerk
                ApplicationClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
                ApplicationClerkBribeLineCount[myLine]--;
                ClerkLineLock.Release();

                ApplicationClerkLineLock[myLine]->Acquire();
                ApplicationClerkData[myLine]=id; //give ssn to clerk
                cout << "Customer[" << id << "] has given SSN [" << id << "] to ApplicationClerk["<<myLine<<"]" << endl;
                ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
                
                //wait for clerk to do the job
                ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
                
                ApplicationClerkLineLock[myLine]->Release();
                
            }
            else{//does not have bribe money
                int myLine;
                int shortestApplicationLine = -1;
                int shortestApplicationLineSize = INT_MAX;
                
                //find shortest line
                for(unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++){
                    if(ApplicationClerkLineCount[i] < shortestApplicationLineSize){
                        shortestApplicationLine = i;
                        shortestApplicationLineSize = ApplicationClerkLineCount[i];
                    }
                }
                
                myLine = shortestApplicationLine;
                
                //wait in the application clerk line
                ApplicationClerkLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in regular line for ApplicationClerk[" << myLine << "]" << endl;
                customerWaitLock->Release();
                //wait to be signalled by clerk
                ApplicationClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
                ApplicationClerkLineCount[myLine]--;
                ClerkLineLock.Release();
                
                ApplicationClerkLineLock[myLine]->Acquire();
                ApplicationClerkData[myLine]=id; //give ssn to clerk
                cout << "Customer[" << id << "] has given SSN [" << id << "] to ApplicationClerk["<<myLine<<"]" << endl;
                ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
                
                //wait for clerk to do the job
                ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
                
                ApplicationClerkLineLock[myLine]->Release();
                
            }
            
        }
        
        //Goes to Passport Clerk
        else if(customerApplicationStatus[id]==3){
            ClerkLineLock.Acquire();
            
            if(money>500){//has bribe money
              money -= 500;
                int myLine;
                int shortestPassportBribeLine = -1;
                int shortestPassportBribeLineSize = INT_MAX;
                
                //find shortest line
                for(unsigned int i = 0; i < passportClerkLineLock.size(); i++){
                    if(passportClerkBribeLineCount[i] < shortestPassportBribeLineSize){
                        shortestPassportBribeLine = i;
                        shortestPassportBribeLineSize = passportClerkBribeLineCount[i];
                    }
                }
                myLine = shortestPassportBribeLine;
                
                //wait in the passport clerk line
                passportClerkBribeLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in bribe line for PassportClerk[" << myLine << "]" << endl;
                customerWaitLock->Release();
                ///wait to get signalled by passport clerk
                passportClerkBribeLineWaitCV[myLine]->Wait(&ClerkLineLock);
                passportClerkBribeLineCount[myLine]--;
                ClerkLineLock.Release();

                //give ssn to passport clerk
                passportClerkLineLock[myLine]->Acquire();
                passportClerkCustomerId[myLine]=id;
                cout << "Customer[" << id << "] has given SSN [" << id << "] to PassportClerk["<<myLine<<"]" << endl;
                passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
                //wait for clerk to do the job
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
                
                passportClerkLineCount[myLine]++;
                cout << "Customer[" << id << "] has gotten in regular line for PassportClerk[" << myLine << "]" << endl;
                customerWaitLock->Release();
                //wait to get signalled by passport clerk
                passportClerkLineWaitCV[myLine]->Wait(&ClerkLineLock);
                passportClerkLineCount[myLine]--;
                ClerkLineLock.Release();

                //give ssn to passport clerk
                passportClerkLineLock[myLine]->Acquire();
                passportClerkCustomerId[myLine]=id;
                cout << "Customer[" << id << "] has given SSN [" << id << "] to PassportClerk["<<myLine<<"]" << endl;
                passportClerkLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
                //wait for clerk to do the job
                passportClerkLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
                
                passportClerkLineLock[myLine]->Release();
                
            }

            
            
        }

        //Goes to Cashier counter
        else if(customerApplicationStatus[id]==6){
            ClerkLineLock.Acquire();
        
            int myLine;
            int shortestCashierLine = -1;
            int shortestCashierLineSize = INT_MAX;
            
            //find shortest line
            for(unsigned int i = 0; i < CashierLineLock.size(); i++){
                if(CashierLineCount[i] < shortestCashierLineSize){
                    shortestCashierLine = i;
                    shortestCashierLineSize = passportClerkLineCount[i];
                }
            }
            myLine = shortestCashierLine;
            
            //get into cashier's line
            CashierLineCount[myLine]++;
            cout << "Customer[" << id << "] has gotten in regular line for Cashier[" << myLine << "]" << endl;
            customerWaitLock->Release();
            CashierLineWaitCV[myLine]->Wait(&ClerkLineLock);
            CashierLineCount[myLine]--;
            ClerkLineLock.Release();

            //give cashier ssn
            CashierLineLock[myLine]->Acquire();
            CashierCustomerId[myLine]=id;
            cout << "Customer[" << id << "] has given SSN [" << id << "] to Cashier["<<myLine<<"]" << endl;
            CashierLineCV[myLine]->Signal(CashierLineLock[myLine]);
            
            //wait for cashier to do the job
            CashierLineCV[myLine]->Wait(CashierLineLock[myLine]);
            CashierLineLock[myLine]->Release();
                
        }
        
    }
     
    incrementCount.Acquire();
    remainingCustomer--;
    
    for(unsigned int i=0;i<numCustomerWaiting.size();i++){
        if(numCustomerWaiting[i]==id){
            
            numCustomerWaiting.erase(numCustomerWaiting.begin()+i);
            
        }
        
        
    }
    incrementCount.Release();
    
}

void ApplicationClerk(int myLine){
    int id = 0;
    bool printed = false;

    while(true){
      bool InBribeLine = false;
        
        if (ApplicationClerkState[myLine] == ONBREAK && !printed){
          cout << "ApplicationClerk [" << myLine << "] is going on break" << endl;
          printed = true;
        }
        else if (ApplicationClerkState[myLine] != ONBREAK){
          printed = false;
        }

        if(hasSenator && (myLine==0) && senatorStatus == 0){//if there is a senator present and i am the index 0 clerk

           if (ApplicationClerkState[myLine] == ONBREAK){
              ApplicationClerkState[myLine] = BUSY;
              cout << "ApplicationClerk [" << myLine << "] sees a senator" << endl;
              cout << "ApplicationClerk [" << myLine << "] is coming off break" << endl;
              printed = false;
           }
           
            senatorApplicationWaitLock->Acquire();
            senatorWaitLock->Acquire();

                senatorServiceId=myLine;
                senatorApplicationWaitCV->Signal(senatorWaitLock);
                cout<<"ApplicationClerk ["<<myLine<<"] has signalled a Senator to come to their counter."<<endl;
                senatorApplicationWaitCV->Wait(senatorWaitLock);
                cout << "ApplicationClerk[" << myLine << "] has received SSN [" << senatorData + 100 << "] from Senator [" << senatorData << "]" << endl;
                cout << "ApplicationClerk[" << myLine << "] has recorded a completed application for Senator [" << senatorData << "]" << endl;
                senatorStatus++;
                senatorApplicationWaitCV->Signal(senatorWaitLock);

            senatorApplicationWaitLock->Release();
            senatorWaitLock->Release();
        }
        else if(hasSenator && myLine != 0){//if there is a senator present and i am not the index 0 clerk. Put myself on break
            ApplicationClerkState[myLine] = ONBREAK;
        }

        ClerkLineLock.Acquire();//acquire the line lock in case of line size change

        if (ApplicationClerkState[myLine] != ONBREAK && !hasSenator){//no senator, not on break, deal with normal customers
            if(ApplicationClerkBribeLineCount[myLine]>0){//bribe line customer first
                ApplicationClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "ApplicationClerk["<<myLine<<"] has signalled a Customer to come to their counter." << endl;
                ApplicationClerkState[myLine]=BUSY;
                InBribeLine=true;
            }
            else if(ApplicationClerkLineCount[myLine]>0){//regular line customer next
                ApplicationClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "ApplicationClerk["<<myLine<<"] has signalled a Customer to come to their counter." << endl;
                ApplicationClerkState[myLine]=BUSY;

            }
            else{//no customer present
                ApplicationClerkState[myLine]=ONBREAK;
                ClerkLineLock.Release();
                currentThread->Yield();//context switch
                continue;
            }
        }
        else{//if there is no customers, put myself on break
            ClerkLineLock.Release();
            currentThread->Yield();//context switch
            continue;
        }

        ApplicationClerkLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        //wait for customer data

        if(InBribeLine){//in bribe line

            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            id = ApplicationClerkData[myLine];

            //Collect Bribe Money From Customer
            applicationMoneyLock.Acquire();
            MoneyFromApplicationClerk += 500;
            cout << "ApplicationClerk["<<myLine<<"] has received $500 from Customer[" << id << "]" << endl;
            applicationMoneyLock.Release();

            //do my job customer now waiting
            cout << "ApplicationClerk[" << myLine << "] has received SSN [" << id << "] from Customer[" << id << "]" << endl;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            customerApplicationStatus[id]++;
            cout << "ApplicationClerk[" << myLine << "] has recorded a completed application for Customer[" << id << "]" << endl;
            
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
        }
        else{//not in bribe line
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            //do my job customer now waiting
            id = ApplicationClerkData[myLine];

            cout << "ApplicationClerk["<< myLine <<"] has received SSN [" << id << "] from Customer[" << id << "]" << endl;

            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }

            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            cout << "ApplicationClerk["<< myLine <<"] has recorded a completed application for Customer[" << id << "]" << endl;
            
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);

        }
            
        ApplicationClerkLineLock[myLine]->Release();

    }//while
}

void PictureClerk(int myLine){
    int id = 0;
    bool printed = false;
    
    while(true){
      
        bool inBribeLine = false;
        
        if (pictureClerkState[myLine] == ONBREAK && !printed){
          cout << "PictureClerk [" << myLine << "] is going on break" << endl;
          printed = true;
        }
        else if (pictureClerkState[myLine] != ONBREAK){
          printed = false;
        }

       if(hasSenator && (myLine==0) && senatorStatus <= 1){//if there is a senator present and i am the index 0 clerk
           
           if (pictureClerkState[myLine] == ONBREAK){
              pictureClerkState[myLine] = BUSY;
              cout << "PictureClerk [" << myLine << "] sees a senator" << endl;
              cout << "PictureClerk [" << myLine << "] is coming off break" << endl;
              printed = false;
           }
           
           senatorPictureWaitLock->Acquire();
           senatorWaitLock->Acquire();

           senatorServiceId=myLine;
           senatorPictureWaitCV->Signal(senatorWaitLock);
           cout<<"PictureClerk ["<<myLine<<"] has signalled a Senator to come to their counter."<<endl;
           senatorPictureWaitCV->Wait(senatorWaitLock);
           cout << "PictureClerk[" << myLine << "] has received SSN [" << senatorData + 100 << "] from Senator [" << senatorData << "]" << endl;
           
           int photoAcceptance = rand() % 100;
           while (photoAcceptance <= 5){
               cout << "PictureClerk [" << myLine << "] has taken a picture of Senator[" << senatorData << "]" << endl;

               cout << "Senator [" << senatorData << "] does not like their picture from PictureClerk [" << myLine << "]." << endl;
               photoAcceptance = rand() % 100;
           }

           cout << "Senator [" << senatorData << "] does like their picture from PictureClerk [" << myLine << "]." << endl;
           cout << "PictureClerk[" << myLine << "] has recorded a completed application for Senator [" << senatorData << "]" << endl;
           senatorStatus += 2;
           senatorPictureWaitCV->Signal(senatorWaitLock);
           senatorWaitLock->Release();
           senatorPictureWaitLock->Release();
       }
        else if(hasSenator && myLine != 0){//if there is a senator present and i am not the index 0 clerk. Put myself on break
            pictureClerkState[myLine] = ONBREAK;
        }

        ClerkLineLock.Acquire();//acquire the line lock in case of line size change
        if (pictureClerkState[myLine] != ONBREAK && !hasSenator){//no senator, not on break, deal with normal customers

            if (pictureClerkBribeLineCount[myLine] > 0){//bribe line customer first
                pictureClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                pictureClerkState[myLine] = BUSY;
                inBribeLine = true;
            } else if(pictureClerkLineCount[myLine] > 0){//regular line next
                pictureClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                pictureClerkState[myLine] = BUSY;
            } else{//no customer present
                pictureClerkState[myLine] = ONBREAK;
                ClerkLineLock.Release();
                currentThread->Yield();//context switch
                continue;
            }
        }
        else{//if there is no customers, put myself on break
            ClerkLineLock.Release();
            currentThread->Yield();//context switch
            continue;
        }

        pictureClerkLineLock[myLine]->Acquire();//acquire the clerk's lock to serve a customer
        ClerkLineLock.Release();
        //if in bribe line
        if (inBribeLine){
            //customer service starts
            pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            id = pictureClerkData[myLine];

            //Collect Bribe Money From Customer
            pictureMoneyLock.Acquire();
            MoneyFromPictureClerk += 500;
            cout << "PictureClerk["<<myLine<<"] has received $500 from Customer[" << id << "]" << endl;
            pictureMoneyLock.Release();
                  
            cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer[" << id << "]" << endl;
            cout << "PictureClerk [" << myLine << "] has taken a picture of Customer[" << id << "]" << endl;

            pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

            if (pictureAcceptance[myLine] > 2){//if customer likes the picture
                cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

                int numCalls = rand() % 80 + 20;//delay for clerk to complete the service
                for (int i = 0; i < numCalls; i++){
                    currentThread->Yield();
                }

                customerApplicationStatus[id] += 2;
                pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);

            } else{//if customer does not like the picture
                cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
                pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            }

        }
        //if in regular line
        else{
            //customer service starts
            pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
            id = pictureClerkData[myLine];

            cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer[" << id << "]" << endl;
            cout << "PictureClerk [" << myLine << "] has taken a picture of Customer[" << id << "]" << endl;

            pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

            if (pictureAcceptance[myLine] > 2){//if customer likes the picture
                cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

                int numCalls = rand() % 80 + 20;
                for (int i = 0; i < numCalls; i++){
                currentThread->Yield();
                }

                customerApplicationStatus[id] += 2;
                pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            
            } else{//if customer does not like the picture
                cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
                pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
            }

        }
        //customer service ends
        pictureClerkLineLock[myLine]->Release();

    }
}

void PassportClerk(int myLine){
    int id = 0;
    bool printed = false;
    
    while(true){
        
        bool inBribeLine = false;

        if (passportClerkState[myLine] == ONBREAK && !printed){
          cout << "PassportClerk [" << myLine << "] is going on break" << endl;
          printed = true;
        }
        else if (passportClerkState[myLine] != ONBREAK){
          printed = false;
        }

       if(hasSenator && (myLine==0) && senatorStatus <= 3){//if there is a senator present and I am the index 0 clerk
           
           if (passportClerkState[myLine] == ONBREAK){
              passportClerkState[myLine] = BUSY;
              cout << "PassportClerk [" << myLine << "] sees a senator" << endl;
              cout << "PassportClerk [" << myLine << "] is coming off break" << endl;
              printed = false;
           }

           senatorPassportWaitLock->Acquire();
           senatorWaitLock->Acquire();

           senatorServiceId=myLine;
           senatorPassportWaitCV->Signal(senatorWaitLock);
           cout<<"PassportClerk ["<<myLine<<"] has signalled a Senator to come to their counter."<<endl;
           senatorPassportWaitCV->Wait(senatorWaitLock);
           cout << "PassportClerk[" << myLine << "] has received SSN [" << senatorData + 100 << "] from Senator [" << senatorData << "]" << endl;
           
           int photoAcceptance = rand() % 100;
           while (photoAcceptance <= 5){//randomness to make senator go back of the line
               cout << "Senator [" << senatorData << "] has gone to PassportClerk [" << myLine << "] too soon. They are going to the back of the line." << endl;
               photoAcceptance = rand() % 100;
           }

           cout << "PassportClerk[" << myLine << "] has recorded a completed application for Senator [" << senatorData << "]" << endl;
           senatorStatus += 3;
           senatorPassportWaitCV->Signal(senatorWaitLock);
           senatorWaitLock->Release();
           senatorPassportWaitLock->Release();
       }
        else if(hasSenator && myLine != 0){//if there is no senator present and I am not the index 0 clerk. Put myself on break
            passportClerkState[myLine] = ONBREAK;
            currentThread->Yield();//context switch
            continue;
        }
        
        ClerkLineLock.Acquire();

        if (passportClerkState[myLine] != ONBREAK && !hasSenator){//if there is no senator present and I am not on break, deal with the normal customers
            if (passportClerkBribeLineCount[myLine] > 0){//bribe line first
                passportClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "PassportClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                passportClerkState[myLine] = BUSY;
                inBribeLine = true;
            } else if(passportClerkLineCount[myLine] > 0){//regular line next
                passportClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "PassportClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                passportClerkState[myLine] = BUSY;
            } else{//put myself on break if there is no customers
                passportClerkState[myLine] = ONBREAK;
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
        
        if (inBribeLine){//deal with bribe line customers
            //clerk service starts
            passportClerkBribeLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];
            
            //Collect Bribe Money From Customer
           passportMoneyLock.Acquire();
           MoneyFromPassportClerk += 500;
           cout << "PassportClerk["<<myLine<<"] has received $500 from Customer[" << id << "]" << endl;
           passportMoneyLock.Release();
            
            cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer[" << id << "]" << endl;
            
            int passportClerkPunishment = rand() % 100;//randomness to make customer go back of the line
            if (passportClerkPunishment > 5){//customer has both their application and picture comleted
                
                cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;
                
                int numCalls = rand() % 80 + 20;
                for (int i = 0; i < numCalls; i++){
                    currentThread->Yield();
                }
                
                customerApplicationStatus[id] += 3;
                cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
                passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
            } else{//customer does not have both their applicaiton and picture completed
                
                cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
                passportClerkBribeLineCV[myLine]->Signal(passportClerkLineLock[myLine]);
                
            }
            
        } else{//deal with regular line customers
            
            passportClerkLineCV[myLine]->Wait(passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];

            cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer[" << id << "]" << endl;
            
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
    int id = 0;
    bool printed = false;
    
    while (true){
        
        if (CashierState[myLine] == ONBREAK && !printed){
          cout << "Cashier [" << myLine << "] is going on break" << endl;
          printed = true;
        }
        else if (CashierState[myLine] != ONBREAK){
          printed = false;
        }

       if(hasSenator && (myLine==0) && senatorStatus <= 6){//if has senator and my index is 0

           if (CashierState[myLine] == ONBREAK){
              CashierState[myLine] = BUSY;
              cout << "Cashier [" << myLine << "] sees a senator" << endl;
              cout << "Cashier [" << myLine << "] is coming off break" << endl;
              printed = false;
           }

           senatorCashierWaitLock->Acquire();
           senatorWaitLock->Acquire();

           senatorServiceId=myLine;
           senatorCashierWaitCV->Signal(senatorWaitLock);
           cout<<"Cashier ["<<myLine<<"] has signalled a Senator to come to their counter."<<endl;
           senatorCashierWaitCV->Wait(senatorWaitLock);
           cout << "Cashier[" << myLine << "] has received SSN [" << senatorData + 100 << "] from Senator [" << senatorData << "]" << endl;
           
           int photoAcceptance = rand() % 100;//randomness to make senator back of the line
           while (photoAcceptance <= 5){
               cout << "Senator [" << senatorData << "] has gone to Cashier [" << myLine << "] too soon. They are going to the back of the line." << endl;
               photoAcceptance = rand() % 100;
           }

            //Collect Fee From Senator
            cashierMoneyLock.Acquire();
            MoneyFromCashier += 100;
            cashierMoneyLock.Release();
            
            
           cout << "Senator [" << senatorData << "] has given Cashier [" << myLine << "] $100." << endl;
            
           cout << "Cashier[" << myLine << "] has recorded a completed application for Senator [" << senatorData << "]" << endl;
           senatorStatus += 4;
           senatorCashierWaitCV->Signal(senatorWaitLock);
           senatorWaitLock->Release();
           senatorCashierWaitLock->Release();
       }
        else if(hasSenator && myLine != 0){
            CashierState[myLine] = ONBREAK;
        }

        ClerkLineLock.Acquire();

        if (CashierState[myLine] != ONBREAK && !hasSenator){
            //When CashierState != ONBREAK
            if (CashierLineCount[myLine] > 0){
                CashierLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
            }
            else {
                ClerkLineLock.Release();
                CashierState[myLine] = ONBREAK;     //
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
        
        
        CashierLineCV[myLine]->Wait(CashierLineLock[myLine]);
        id = CashierCustomerId[myLine];
        cout << "Cashier[" << myLine << "] has received SSN [" << id << "] from Customer[" << id << "]" << endl;
        
        int cashierPunishment = rand() % 100;
        if (cashierPunishment > 5) {  // Passed All the tests (Certified)
            cout << "Cashier[" << myLine << "] has verified that Customer[" << id << "] has been certified by a PassportClerk" << endl;
            
            
            //Collect Fee From Customer
            cashierMoneyLock.Acquire();
            MoneyFromCashier += 100;
            cashierMoneyLock.Release();
            
            
            cout << "Cashier[" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
            
            // Give out the passport to the customer
            // Notify the customer he is done
            cout << "Cashier[" << myLine << "] has provided Customer[" << id << "] their completed passport" << endl;
            cout << "Cashier[" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
            
            customerApplicationStatus[id] += 4;
            CashierLineCV[myLine]->Signal(CashierLineLock[myLine]);
        }
        else {  //Not yet Certified
            cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
            cout << "customerApplicationStatus[" << id << "] is: " << customerApplicationStatus[id] << endl;
            CashierLineCV[myLine]->Signal(CashierLineLock[myLine]);
            
        }
        
        
        CashierLineLock[myLine]->Release();
    }   //while loop
}

void Manager(){
    int count = 0;
    int maxNumClerk = 0;
    int numApplicationClerk = ApplicationClerkLineLock.size();
    int numPictureClerk = pictureClerkLineLock.size();
    int numPassportClerk = passportClerkLineLock.size();
    int numCashier = CashierLineLock.size();
    //check the max number of the clerks
    if (maxNumClerk < numApplicationClerk) maxNumClerk = numApplicationClerk;
    if (maxNumClerk < numPictureClerk) maxNumClerk = numPictureClerk;
    if (maxNumClerk < numPassportClerk) maxNumClerk = numPassportClerk;
    if (maxNumClerk < numCashier) maxNumClerk = numCashier;

    while (true){
        for (int i = 0; i < 100; ++i) {
            currentThread->Yield();
        } 
        //acquire all the lock to print out the incoming statement
        applicationMoneyLock.Acquire();
        pictureMoneyLock.Acquire();
        passportMoneyLock.Acquire();
        cashierMoneyLock.Acquire();
        
        cout << "Manager has counted a total of $" << MoneyFromApplicationClerk << " for ApplicationClerks" << endl;
        cout << "Manager has counted a total of $" << MoneyFromPictureClerk << " for PictureClerks" << endl;
        cout << "Manager has counted a total of $" << MoneyFromPassportClerk << " for PassportClerks" << endl;
        cout << "Manager has counted a total of $" << MoneyFromCashier << " for Cashiers" << endl;
        
        MoneyTotal = MoneyFromApplicationClerk + MoneyFromPictureClerk + MoneyFromPassportClerk + MoneyFromCashier;
        cout << "Manager has counted a total of $" << MoneyTotal << " for The passport Office" << endl;
        
        applicationMoneyLock.Release();
        pictureMoneyLock.Release();
        passportMoneyLock.Release();
        cashierMoneyLock.Release();
        
        count = 0;
    
        // A vector of clerkState
        // A vector of clerkCV
        
        ClerkLineLock.Acquire();

            //Application Clerks
        for ( int i = 0; i < numApplicationClerk; i++){
            if (ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] >= 1
                && ApplicationClerkState[i] == ONBREAK){
                
                ApplicationClerkState[i] = AVAILABLE;
                cout << "Manager has woken up an ApplicationClerk" << endl;
                cout << "ApplicationClerk [" << i << "] is coming off break" << endl;
            }
            else if (remainingCustomer <= maxNumClerk * 3 && 
                      ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] > 0){
                
                ApplicationClerkState[i] = AVAILABLE;
                cout << "Manager has woken up an ApplicationClerk" << endl;
                cout << "ApplicationClerk [" << i << "] is coming off break" << endl;

            }
            
        }
        
        //Picture Clerks
        for ( int i = 0; i < numPictureClerk; i++){
            if (pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] >= 1
                && pictureClerkState[i] == ONBREAK){
                
                pictureClerkState[i] = AVAILABLE;
                cout << "Manager has woken up a PictureClerk" << endl;
                cout << "PictureClerk [" << i << "] is coming off break" << endl;
            }
            else if (remainingCustomer <= maxNumClerk * 3 && 
                      pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] > 0){
                
                pictureClerkState[i] = AVAILABLE;
                cout << "Manager has woken up a PictureClerk" << endl;
                cout << "PictureClerk [" << i << "] is coming off break" << endl;

            }
            
        }
        
        //Passport Clerks
        for ( int i = 0; i < numPassportClerk; i++){
            if (passportClerkLineCount[i] + passportClerkBribeLineCount[i] >= 1
                && passportClerkState[i] == ONBREAK){
                
                passportClerkState[i] = AVAILABLE;
                cout << "Manager has woken up a PassportClerk" << endl;
                cout << "PassportClerk [" << i << "] is coming off break" << endl;
            }
            else if (remainingCustomer <= maxNumClerk * 3 && 
                      passportClerkLineCount[i] + passportClerkBribeLineCount[i] > 0){
                
                passportClerkState[i] = AVAILABLE;
                cout << "Manager has woken up a PassportClerk" << endl;
                cout << "PassportClerk [" << i << "] is coming off break" << endl;

            }

        }
        
        //Cashiers
        for ( int i = 0; i < numCashier; i++){
            if (CashierLineCount[i] >= 1
                && CashierState[i] == ONBREAK){
                
                CashierState[i] = AVAILABLE;
                cout << "Manager has woken up a Cashier" << endl;
                cout << "Cashier [" << i << "] is coming off break" << endl;
            }
            else if (remainingCustomer <= maxNumClerk * 3 && 
                      CashierLineCount[i] > 0){
                
                CashierState[i] = AVAILABLE;
                cout << "Manager has woken up a Cashier" << endl;
                cout << "Cashier [" << i << "] is coming off break" << endl;

            }
            
        }

        ClerkLineLock.Release();
        
        count++;
        
        if (remainingCustomer == 0) break;
        
    }

}

void Senator(){
    //acquire all the necessary locks to get started
    customerWaitLock->Acquire();
    senatorPictureWaitLock->Acquire();
    senatorPassportWaitLock->Acquire();
    senatorCashierWaitLock->Acquire();
    senatorApplicationWaitLock->Acquire();
    senatorWaitLock->Acquire();
    //senator ID and SSN
    int id = senatorNum + 1;
    int ssn = id + 100;
    senatorNum++;
    cout << "Senator ["<< id<<"] has came into passport office"<< endl;

	  hasSenator = TRUE;
    for(unsigned int i=0;i<numCustomerWaiting.size();i++){
        cout<<"Customer ["<<numCustomerWaiting[i]<<"] is going outside the Passport Office because their is a Senator present."<<endl;
    }
    
    senatorApplicationWaitLock->Release();
    cout << "Senator [" << id << "] has gotten in regular line for ApplicationClerk [" << senatorServiceId << "]." << endl;
    senatorApplicationWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator [" << id << "] has given SSN [" << ssn << "] to ApplicationClerk [" << senatorServiceId << "]."<<endl;
    senatorData=id;
    senatorApplicationWaitCV->Signal(senatorWaitLock);//signal a clerk

    senatorApplicationWaitCV->Wait(senatorWaitLock);//wait for a filed application
    

    senatorPictureWaitLock->Release();
    cout << "Senator [" << id << "] has gotten in regular line for PictureClerk [" << senatorServiceId << "]." << endl;
    senatorPictureWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator [" << id << "] has given SSN [" << ssn << "] to PictureClerk [" << senatorServiceId << "]."<<endl;
    senatorData=id;
    senatorPictureWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorPictureWaitCV->Wait(senatorWaitLock);//wait for a filed application
    
    
    senatorPassportWaitLock->Release();
    cout << "Senator [" << id << "] has gotten in regular line for PassportClerk [" << senatorServiceId << "]." << endl;
    senatorPassportWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator [" << id << "] has given SSN [" << ssn << "] to PassportClerk [" << senatorServiceId << "]."<<endl;
    senatorData=id;
    senatorPassportWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorPassportWaitCV->Wait(senatorWaitLock);//wait for a filed application
    
    
    senatorCashierWaitLock->Release();
    cout << "Senator [" << id << "] has gotten in regular line for Cashier [" << senatorServiceId << "]." << endl;
    senatorCashierWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator [" << id << "] has given SSN [" << ssn << "] to Cashier [" << senatorServiceId << "]."<<endl;
    senatorData=id;
    senatorCashierWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorCashierWaitCV->Wait(senatorWaitLock);//wait for a filed application

    
    hasSenator=FALSE;
    senatorStatus = 0;
    cout<<"Senator[" << id << "] is leaving the Passport Office"<<endl;//senator is leaving the passport office
    customerWaitLock->Release();
    senatorWaitLock->Release();
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

    cout << "Passport Office Simulation started." << endl;

    cout << "Enter the numebr of application clerks(1-5): " << endl;
    cin >> numApplicationClerk;
    while (numApplicationClerk > 5 || numApplicationClerk < 1){
        cout << "Invalid input." << endl;
        cout << "Enter the numebr of application clerks(1-5): " << endl;
        cin >> numApplicationClerk;
    }

    cout << "Enter the numebr of picture clerks(1-5): " << endl;
    cin >> numPictureClerk;
    while (numPictureClerk > 5 || numPictureClerk < 1){
        cout << "Invalid input." << endl;
        cout << "Enter the numebr of picture clerks(1-5): " << endl;
        cin >> numPictureClerk;
    }

    cout << "Enter the numebr of passport clerks(1-5): " << endl;
    cin >> numPassportClerk;
    while (numPassportClerk > 5 || numPassportClerk < 1){
        cout << "Invalid input." << endl;
        cout << "Enter the numebr of passport clerks(1-5): " << endl;
        cin >> numPassportClerk;
    }

    cout << "Enter the numebr of cashiers(1-5): " << endl;
    cin >> numCashier;
    while (numApplicationClerk > 5 || numApplicationClerk < 1){
        cout << "Invalid input. " << endl;
        cout << "Enter the numebr of cashiers(1-5): " << endl;
        cin >> numApplicationClerk;
    }

    cout << "Enter the numebr of customers: " << endl;
    cin >> numCustomer;
    while (numCustomer > 50 || numCustomer < 0){
        cout << "Customer number should be less than 50." << endl;
        cout << "Enter the numebr of customers: " << endl;
        cin >> numCustomer;
    }
    remainingCustomer = numCustomer;

    cout << "Enter the number of senators: " << endl;
    cin >> numSenator;
    while (numSenator > 10 || numSenator < 0){
        cout << "Senator number should be less than 10." << endl;
        cout << "Enter the number of senators: " << endl;
        cin >> numSenator;
    }
    
    
    //Initialize all variables for all clerks
    for(int i = 0; i < numApplicationClerk; i++){
        char lockName[100] = "ApplicationLock";
        sprintf(integer,"%d",i );
        strcat(lockName,integer);

        //application lock initialize
        Lock* applicationLock = new Lock(lockName);
        ApplicationClerkLineLock.push_back(applicationLock);
        
        //aplication CV initialize
        char applicationCVName[100] = "applicationCV";
        strcat(applicationCVName, integer);
        Condition* applicationCV = new Condition(applicationCVName);
        ApplicationClerkLineCV.push_back(applicationCV);

        //application bribe CV initialize
        char applicaitonBribeCVName[100] = "applicaitonBribeCV";
        strcat(applicaitonBribeCVName, integer);
        Condition* applicationBribeCV = new Condition(applicaitonBribeCVName);
        ApplicationClerkBribeLineCV.push_back(applicationBribeCV);

        //application Wait CV initialize
        char applicationWaitCVName[100] = "applicationWaitCV";
        strcat(applicationWaitCVName, integer);
        Condition* applicationWaitCV = new Condition(applicationWaitCVName);
        ApplicationClerkLineWaitCV.push_back(applicationWaitCV);

        //application Bribe Wait CV initialize
        char applicationBribeWaitCVName[100] = "applicationWaitCV";
        strcat(applicationBribeWaitCVName, integer);
        Condition* applicationBribeWaitCV = new Condition(applicationBribeWaitCVName);
        ApplicationClerkBribeLineWaitCV.push_back(applicationBribeWaitCV);

        //application line size intialize
        ApplicationClerkLineCount.push_back(0);
        //application bribe line size initialize
        ApplicationClerkBribeLineCount.push_back(0);
        //application clerk state initialize
        clerkState ct = AVAILABLE;
        ApplicationClerkState.push_back(ct);
        //application data initialize
        ApplicationClerkData.push_back(0);
      
    }
   
    for(int i = 0; i < numPictureClerk; i++){
        char lockName[100] = "PictureLock";
        sprintf(integer,"%d",i );
        strcat(lockName,integer);
        
        //picture lock initialize
        Lock* pictureLock = new Lock(lockName);
        pictureClerkLineLock.push_back(pictureLock);
        
        //picture CV initialize
        char pictureCVName[100] = "pictureCV";
        strcat(pictureCVName, integer);
        Condition* pictureCV = new Condition(pictureCVName);
        pictureClerkLineCV.push_back(pictureCV);

        //picture bribe CV initialize
        char pictureBribeCVName[100] = "pictureBribeCV";
        strcat(pictureBribeCVName, integer);
        Condition* pictureBribeCV = new Condition(pictureBribeCVName);
        pictureClerkBribeLineCV.push_back(pictureBribeCV);

        //picture Wait CV initialize
        char pictureWaitCVName[100] = "pictureWaitCV";
        strcat(pictureWaitCVName, integer);
        Condition* pictureWaitCV = new Condition(pictureWaitCVName);
        pictureClerkLineWaitCV.push_back(pictureWaitCV);

        //picture Bribe Wait CV initialize
        char pictureBribeWaitCVName[100] = "pictureWaitCV";
        strcat(pictureBribeWaitCVName, integer);
        Condition* pictureBribeWaitCV = new Condition(pictureBribeWaitCVName);
        pictureClerkBribeLineWaitCV.push_back(pictureBribeWaitCV);

        //picture line size intialize
        pictureClerkLineCount.push_back(0);
        //picture bribe line size initialize
        pictureClerkBribeLineCount.push_back(0);
        //picture clerk state initialize
        clerkState ct = AVAILABLE;
        pictureClerkState.push_back(ct);
        //picture clerk data initialize
        pictureClerkData.push_back(0);
        pictureAcceptance.push_back(0);
    }
   
    for(int i = 0; i < numPassportClerk; i++){
        char lockName[100] = "PassportLock";
        sprintf(integer,"%d",i );
        strcat(lockName,integer);
        
        //passport lock initialize
        Lock* passportLock = new Lock(lockName);
        passportClerkLineLock.push_back(passportLock);
        
        //passport CV initialize
        char passportCVName[100] = "passportCV";
        strcat(passportCVName, integer);
        Condition* passportCV = new Condition(passportCVName);
        passportClerkLineCV.push_back(passportCV);

        //passport bribe CV initialize
        char passportBribeCVName[100] = "passportBribeCV";
        strcat(passportBribeCVName, integer);
        Condition* passportBribeCV = new Condition(passportBribeCVName);
        passportClerkBribeLineCV.push_back(passportBribeCV);

        //passport Wait CV initialize
        char passportWaitCVName[100] = "passportWaitCV";
        strcat(passportWaitCVName, integer);
        Condition* passportWaitCV = new Condition(passportWaitCVName);
        passportClerkLineWaitCV.push_back(passportWaitCV);

        //passport Bribe Wait CV initialize
        char passportBribeWaitCVName[100] = "passportWaitCV";
        strcat(passportBribeWaitCVName, integer);
        Condition* passportBribeWaitCV = new Condition(passportBribeWaitCVName);
        passportClerkBribeLineWaitCV.push_back(passportBribeWaitCV);

        //passport line size intialize
        passportClerkLineCount.push_back(0);
        //passport bribe line size initialize
        passportClerkBribeLineCount.push_back(0);
        //passport clerk state initialize
        clerkState ct = AVAILABLE;
        passportClerkState.push_back(ct);
        //passport data initialize
        passportClerkCustomerId.push_back(0);
    }
   
    for(int i = 0; i < numCashier; i++){
        char lockName[100] = "CashierLock";
        sprintf(integer,"%d",i );
        strcat(lockName,integer);
        
        //cashier lock initialize
        Lock* CashierLock = new Lock(lockName);
        CashierLineLock.push_back(CashierLock);
        
        //cashier CV initialize
        char CashierCVName[100] = "cashierCV";
        strcat(CashierCVName, integer);
        Condition* CashierCV = new Condition(CashierCVName);
        CashierLineCV.push_back(CashierCV);

        //cashier Wait CV initialize
        char CashierWaitCVName[100] = "cashierWaitCV";
        strcat(CashierWaitCVName, integer);
        Condition* CashierWaitCV = new Condition(CashierWaitCVName);
        CashierLineWaitCV.push_back(CashierWaitCV);

        //cashier line size intialize
        CashierLineCount.push_back(0);
        //cashier state initialize
        clerkState ct = AVAILABLE;
        CashierState.push_back(ct);
        //cashier data initialize
        CashierCustomerId.push_back(0);
    }

    senatorWaitLock = new Lock("senator");
    senatorApplicationWaitLock = new Lock("senatorApplicationWaitLock");
    senatorPictureWaitLock = new Lock("senatorPictureWaitLock");
    senatorPassportWaitLock = new Lock("senatorPassportWaitLock");
    senatorCashierWaitLock = new Lock("senatorCashierWaitLock");
    senatorApplicationWaitCV = new Condition("senatorApplicationWaitCV");
    senatorPictureWaitCV = new Condition("senatorPictureWaitCV");
    senatorPassportWaitCV = new Condition("senatorPassportWaitCV");
    senatorCashierWaitCV = new Condition("senatorCashierWaitCV");
    customerWaitCV = new Condition("customerCV");
    customerWaitLock = new Lock("customerLock");

    for(int i =0 ;i < numCustomer; i++){
        customerApplicationStatus.push_back(0);

    }

    //Initialize all threads
    for(int i = 0; i < numApplicationClerk; i++){
        char threadName[100] = "ApplicationClerk";
        sprintf(integer,"%d",i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)ApplicationClerk, i);
      
    }
   
    for(int i = 0; i < numPictureClerk; i++){
        char threadName[100] = "PictureClerk";
        sprintf(integer,"%d",i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)PictureClerk, i);
    }
   
    for(int i = 0; i < numPassportClerk; i++){
        char threadName[100] = "PassportClerk";
        sprintf(integer,"%d",i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)PassportClerk, i);
    }
   
    for(int i = 0; i < numCashier; i++){
        char threadName[100] = "Cashier";
        sprintf(integer,"%d",i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)Cashier, i);
    }

    for (int i = 0; i < numCustomer; i++){
        char threadName[100] = "Customer";
        sprintf(integer,"%d",i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)Customer, 0);
    }


    t1 = new Thread("Manager");
    t1->Fork((VoidFunctionPtr)Manager, 0);


    for (int i = 0; i < numSenator; i++){
        char threadName[100] = "Senator";
        sprintf(integer,"%d",i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)Senator, 0);
    }
    

}


#endif
