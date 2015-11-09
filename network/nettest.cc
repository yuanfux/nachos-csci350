// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "list.h"
#include "syscall.h"
#include <sstream>
// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

#define NUM_LOCK 1000
#define NUM_CONDITION 1000
#define NUM_MONITOR 1000

//
//struct Binder{
//    int replyTo;
//    int ThreadIndex;
//    
//    Binder(int r, int T){
//        replyTo = r;
//        ThreadIndex = T;
//    }
//};

struct ServerLock{
    char* name;
    List* queue;
    int lockHolder;
    int spaceHolder;
    ServerLock(){
        name = NULL;
        queue = new List;
        lockHolder = -1;
        spaceHolder = -1;
    }
    
};

struct ServerCondition{
    char* name;
    int waitingLock;
    List* queue;
    int spaceHolder;
    ServerCondition(){
        name = NULL;
        waitingLock = -1;
        queue = new List;
    }
    
};

struct ServerMonitorVariable{
    char* name;
    int data;
    int spaceHolder;
    ServerMonitorVariable(){
        name = NULL;
    }
    
    ServerMonitorVariable(char* monitorName){
        name = monitorName;
    }
    
    
};


ServerLock serverLock[NUM_LOCK];

ServerCondition serverCondition[NUM_CONDITION];

ServerMonitorVariable serverMonitorVariable[NUM_MONITOR];

int numLock = 0;
int numCondition = 0;
int numMonitor = 0;

void sendInt(int replyTo, int threadIndex, int value){
    
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    
    outPktHdr.to = replyTo;
    outPktHdr.from = 0;
    outMailHdr.to = threadIndex;
    outMailHdr.from = 0;
    
    char* send = new char[20];
    sprintf(send, "%d", value);
    outMailHdr.length = strlen(send) + 1;
    printf("send from server to %d : %d\n", replyTo, value);
    if(postOffice->Send(outPktHdr, outMailHdr, send) == false){
        printf("Send failed from Sever CreateLock\n");
        return;
    }
    
}


void CreateLock(char* name, int replyTo, int ThreadIndex){
    
    serverLock[numLock].name = name;
    serverLock[numLock].spaceHolder = replyTo;
    sendInt(replyTo,ThreadIndex, numLock);
    
    numLock++;
    
    return;
}

void Acquire(int lockIndex, int replyTo, int ThreadIndex){
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Acquire: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].spaceHolder != replyTo){
        printf("Acquire: cannot access lock from other machines\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
        
    }
    
    if(serverLock[lockIndex].spaceHolder == replyTo && serverLock[lockIndex].lockHolder == ThreadIndex){
        printf("Acquire: already the lock holder\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    else if(serverLock[lockIndex].spaceHolder == replyTo && serverLock[lockIndex].lockHolder == -1){
        printf("Acquire: no lock holder, hold the lock\n");
        serverLock[lockIndex].spaceHolder = replyTo;
        serverLock[lockIndex].lockHolder = ThreadIndex;
        sendInt(replyTo, ThreadIndex, lockIndex);
        return;
    }
    else{
        printf("Acquire: someone is holding the lock, wait\n");
        serverLock[lockIndex].queue->Append((void*)ThreadIndex);
        return;
    }
}

void Release(int lockIndex, int replyTo, int ThreadIndex){
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Release: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].spaceHolder != replyTo || serverLock[lockIndex].lockHolder != ThreadIndex){
        printf("Release: not the lock holder\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    else{
        
        sendInt(replyTo, ThreadIndex, lockIndex);
        if(serverLock[lockIndex].queue->IsEmpty()){
            serverLock[lockIndex].lockHolder = -1;
            return;
        }
        else{
            
            int  nextReply = (int )serverLock[lockIndex].queue->Remove();
            serverLock[lockIndex].lockHolder = nextReply;
            sendInt(replyTo, nextReply, lockIndex);
            return;
        }
    }
    
}

void DestroyLock(int lockIndex, int replyTo, int ThreadIndex){
    if(lockIndex >= numLock || lockIndex < 0){
        printf("DestroyLock: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].spaceHolder != replyTo){
        printf("DestroyLock: cannot access lock from other machines\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
        
    }
    
    serverLock[lockIndex].name = NULL;
    serverLock[lockIndex].lockHolder = NULL;
    serverLock[lockIndex].spaceHolder = NULL;
    numLock--;
    
    sendInt(replyTo, ThreadIndex, lockIndex);
    
    return;
}

void CreateCondition(char* name, int replyTo, int ThreadIndex){
    
    serverCondition[numCondition].name = name;
    serverCondition[numCondition].spaceHolder = replyTo;
    sendInt(replyTo, ThreadIndex, numCondition);
    
    numCondition++;
    
    return;
    
}

void Wait(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex){
    
    
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Wait: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(conditionIndex >= numCondition || conditionIndex < 0){
        printf("Wait: out of Condition boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].spaceHolder != replyTo || serverCondition[conditionIndex].spaceHolder != replyTo){
        printf("Wait: cannot access the lock or condition from other machine\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].lockHolder != ThreadIndex){
        printf("Wait: not the lock holder\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverCondition[conditionIndex].waitingLock == -1){
        serverCondition[conditionIndex].waitingLock = lockIndex;
    }
    
    else if (serverCondition[conditionIndex].waitingLock != lockIndex){
        printf("Wait: wrong condition to wait\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    serverCondition[conditionIndex].queue->Append((void*)ThreadIndex);
    
    if(!serverLock[lockIndex].queue->IsEmpty()){
        int nextReply = (int)serverLock[lockIndex].queue->Remove();
        serverLock[lockIndex].lockHolder = nextReply;
        sendInt(replyTo, nextReply, lockIndex);
        return;
    }
    else{
        serverLock[lockIndex].lockHolder = -1;
        return;
    }
}

void Signal(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex){
    
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Signal: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(conditionIndex >= numCondition || conditionIndex < 0){
        printf("Signal: out of Condition boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].spaceHolder != replyTo || serverCondition[conditionIndex].spaceHolder != replyTo){
        printf("Signal: cannot access the lock or condition from other machine\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].lockHolder != ThreadIndex ){
        printf("Signal: not the lock holder\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    if (serverCondition[conditionIndex].waitingLock != lockIndex){
        printf("Signal: wrong condition to signal\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    if(serverCondition[conditionIndex].queue->IsEmpty()){
        printf("Signal: nothing to signal\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    //?
    int nextReply = (int)serverCondition[conditionIndex].queue->Remove();
    if (serverCondition[conditionIndex].queue->IsEmpty()) {
        //no one is waiting
        serverCondition[conditionIndex].waitingLock = -1;
    }
    
    //if (serverLock[lockIndex].lockHolder != -1) {
        serverLock[lockIndex].queue->Append((void*)nextReply);
        
    //}
    //else{
      //  serverLock[lockIndex].lockHolder = nextReply;
       // printf("in side signal: lockHolder: %d \n", nextReply);
    
      //  sendInt(replyTo, nextReply, lockIndex);
    //}
    
    sendInt(replyTo, ThreadIndex, lockIndex);
}

void Broadcast(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex){
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Broadcast: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(conditionIndex >= numCondition || conditionIndex < 0){
        printf("Broadcast: out of Condition boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].spaceHolder != replyTo || serverCondition[conditionIndex].spaceHolder != replyTo){
        printf("Broadcast: cannot access the lock or condition from other machine\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    if(serverLock[lockIndex].lockHolder != ThreadIndex ){
        printf("Broadcast: not the lock holder\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    if (serverCondition[conditionIndex].waitingLock != lockIndex){
        printf("Broadcast: wrong condition to signal\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    if(serverCondition[conditionIndex].queue->IsEmpty()){
        printf("Broadcast: nothing to signal\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    //?
    while (!serverCondition[conditionIndex].queue->IsEmpty()){
        
        int nextReply = (int)serverCondition[conditionIndex].queue->Remove();
        
            serverLock[lockIndex].queue->Append((void*)nextReply);
        
    }
    
   serverCondition[conditionIndex].waitingLock = -1;
    
   sendInt(replyTo, ThreadIndex, 0);
}

void DestroyCondition(int conditionIndex, int replyTo, int ThreadIndex){
    if(conditionIndex >= numCondition || conditionIndex < 0){
        printf("DestroyCondition: out of Condition boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    if(serverCondition[conditionIndex].spaceHolder != replyTo){
        printf("DestroyCondition: cannot access the lock or condition from other machine\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    serverCondition[conditionIndex].name = NULL;
    serverCondition[conditionIndex].waitingLock = -1;
    serverCondition[conditionIndex].queue = new List;
    numCondition--;
    
    sendInt(replyTo, ThreadIndex, conditionIndex);
    
    return;
}

void CreateMV(char* name, int replyTo, int ThreadIndex){
    
    serverMonitorVariable[numMonitor].name = name;
    
    sendInt(replyTo, ThreadIndex, numMonitor);
    
    numMonitor++;
    
    return;
    
}

void GetMV(int monitorIndex, int replyTo, int ThreadIndex){
    if(monitorIndex >= numMonitor || monitorIndex < 0 ){
        printf("GetMV: out of boundary");
        
        sendInt(replyTo, ThreadIndex, -1);
        
        return;
    }
    
    sendInt(replyTo, ThreadIndex, serverMonitorVariable[monitorIndex].data);
    
    return;
    
}

void SetMV(int monitorIndex, int value, int replyTo, int ThreadIndex){
    if(monitorIndex >= numMonitor || monitorIndex < 0 ){
        printf("GetMV: out of boundary");
        
        sendInt(replyTo, ThreadIndex, -1);
        
        return;
    }
    
    serverMonitorVariable[monitorIndex].data = value;
    
    sendInt(replyTo, ThreadIndex, monitorIndex);
    
    return;
}

void Server(){
    PacketHeader serverOutPktHdr;
    PacketHeader serverInPktHdr;
    MailHeader serverOutMailHdr;
    MailHeader serverInMailHdr;
    
    while(true){
        printf("inside Server while loop\n");
        char receive[MaxMailSize];
        postOffice->Receive(0, &serverInPktHdr, &serverInMailHdr, receive);
        //need delete
        printf("Server: Got \"%s\" from %d, box %d\n", receive, serverInPktHdr.from, serverInMailHdr.from);
        fflush(stdout);
        
        
        int replyTo = serverInPktHdr.from;
        std::stringstream ss;
        ss << receive;
        int syscall;
        ss >> syscall;
        if(syscall == 25){
            int lockIndex;
            ss >> lockIndex;
            
            Acquire(lockIndex, replyTo, serverInMailHdr.from);
            printf("Server: Acquire syscall\n");
            
        }
        else if(syscall == 26){
            int lockIndex;
            ss >> lockIndex;
            
            Release(lockIndex, replyTo, serverInMailHdr.from);
            printf("Server: Release syscall\n");
        }
        else if(syscall == 27){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            Wait(conditionIndex, lockIndex, replyTo, serverInMailHdr.from);
            printf("Server: Wait syscall\n");
        }
        else if(syscall == 28){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            Signal(conditionIndex, lockIndex, replyTo, serverInMailHdr.from);
            printf("Server: Signal syscall\n");
            
        }
        else if(syscall == 29){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            Broadcast(conditionIndex, lockIndex, replyTo, serverInMailHdr.from);
            printf("Server: Broadcast syscall\n");
        }
        else if(syscall == 30){
            char lockName[100];
            ss >> lockName;
            
            CreateLock(lockName, replyTo, serverInMailHdr.from);
            printf("Server: CreateLock syscall\n");
        }
        else if(syscall == 31){
            int lockIndex;
            ss >> lockIndex;
            
            DestroyLock(lockIndex, replyTo, serverInMailHdr.from);
            printf("Server: DestroyLock syscall\n");
            
        }
        else if(syscall == 32){
            char conditionName[100];
            ss >> conditionName;
            
            CreateCondition(conditionName, replyTo, serverInMailHdr.from);
            printf("Server: CreateCondition syscall\n");
        }
        else if(syscall == 33){
            int conditionIndex;
            ss >> conditionIndex;
            
            DestroyCondition(conditionIndex, replyTo, serverInMailHdr.from);
            printf("Server: DestroyCondition syscall\n");
        }
        else if(syscall == 34){
            char monitorName[100];
            ss >> monitorName;
            
            CreateMV(monitorName, replyTo, serverInMailHdr.from);
            printf("Server: CreateMonitor syscall\n");
        }
        else if(syscall == 35){
            int monitorIndex;
            ss >> monitorIndex;
            
            GetMV(monitorIndex, replyTo, serverInMailHdr.from);
            printf("Server: GetMV syscall\n");
        }
        else if(syscall == 36){
            int monitorIndex;
            int data;
            ss >> monitorIndex;
            ss >> data;
            
            SetMV(monitorIndex, data, replyTo, serverInMailHdr.from);
            printf("Server: SetMV syscall\n");
        }
        
        
    }
    
}


