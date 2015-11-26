// nettest.cc 
//  Test out message delivery between two "Nachos" machines,
//  using the Post Office to coordinate delivery.
//
//  Two caveats:
//    1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//      ./nachos -m 0 -o 1 &
//      ./nachos -m 1 -o 0 &
//
//    2. You need an implementation of condition variables,
//       which is *not* provided as part of the baseline threads 
//       implementation.  The Post Office won't work without
//       a correct implementation of condition variables.
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
#include <vector>
// Test out message delivery, by doing the following:
//  1. send a message to the machine with ID "farAddr", at mail box #0
//  2. wait for the other machine's message to arrive (in our mailbox #0)
//  3. send an acknowledgment for the other machine's message
//  4. wait for an acknowledgement from the other machine to our 
//      original message

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

//binder class to handle a request from a specific machine and thread
struct Binder{
    int replyTo;
    int ThreadIndex;
    
    Binder(){
        replyTo = -1;
        ThreadIndex = -1;
    }
    
    Binder(int r, int T){
        replyTo = r;
        ThreadIndex = T;
    }
};
//server lock struct
struct ServerLock{
    char* name;
    List* queue;
    Binder lockHolder;
    std::vector<Binder> spaceHolder;
    ServerLock(){
        name = NULL;
        queue = new List;
    }
    
};
//server condition struct
struct ServerCondition{
    char* name;
    int waitingLock;
    List* queue;
    std::vector<Binder> spaceHolder;
    ServerCondition(){
        name = NULL;
        waitingLock = -1;
        queue = new List;
    }
    
};
//sevrer monitor variable struct
struct ServerMonitorVariable{
    char* name;
    int data;
    std::vector<Binder> spaceHolder;
    ServerMonitorVariable(){
        name = NULL;
    }
    
};

//initialize all the variables with static arrays
ServerLock serverLock[NUM_LOCK];

ServerCondition serverCondition[NUM_CONDITION];

ServerMonitorVariable serverMonitorVariable[NUM_MONITOR];

int numLock = 0;
int numCondition = 0;
int numMonitor = 0;

//sendInt funciton to allow server to reply the clients with integers
void sendInt(int replyTo, int threadIndex, int value){
    
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    //send to specific machine and thread
    outPktHdr.to = replyTo;
    outPktHdr.from = 0;
    outMailHdr.to = threadIndex;
    outMailHdr.from = 0;
    
    char* send = new char[10];
    sprintf(send, "%d", value);
    outMailHdr.length = strlen(send) + 1;
    //printf("send from server to machine %d, thread %d, value %d\n", replyTo, threadIndex,value);
    if(postOffice->Send(outPktHdr, outMailHdr, send) == false){
        printf("Server: Send failed\n");
        return;
    }
    
}

void Acquire(int lockIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Acquire: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
//    if(serverLock[lockIndex].spaceHolder != replyTo){
//        printf("Acquire: cannot access lock from other machines\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//        
//    }
    
    std::vector<Binder> binderV = serverLock[lockIndex].spaceHolder;
    
    for(unsigned int i = 0; i < binderV.size(); i++){
        
        if(binderV[i].replyTo == replyTo){
            //is the space holder, can possibly hold the lock
            if(serverLock[lockIndex].lockHolder.replyTo == replyTo && serverLock[lockIndex].lockHolder.ThreadIndex == ThreadIndex){
                printf("Acquire: already the lock holder\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            else if(serverLock[lockIndex].lockHolder.replyTo == -1 && serverLock[lockIndex].lockHolder.ThreadIndex == -1){
                printf("Acquire: no lock holder, hold the lock\n");
                serverLock[lockIndex].lockHolder.replyTo = replyTo;
                serverLock[lockIndex].lockHolder.ThreadIndex = ThreadIndex;
                sendInt(replyTo, ThreadIndex, lockIndex);
                return;
            }
            else{
                //someone is holding the lock. Wait here
                printf("Acquire: someone is holding the lock, wait\n");
                Binder* binder = new Binder(replyTo, ThreadIndex);
                serverLock[lockIndex].queue->Append((Binder*)binder);
                return;
                
            }
            
            
            sendInt(replyTo, ThreadIndex, lockIndex);
            
            return;
            
        }
    }
    
    printf("Acquire: not the space holder\n");
    sendInt(replyTo, ThreadIndex, -1);
    return;
    
    
    
//    if(serverLock[lockIndex].spaceHolder == replyTo && serverLock[lockIndex].lockHolder == ThreadIndex){
//        //printf("Acquire: already the lock holder\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//    else if(serverLock[lockIndex].spaceHolder == replyTo && serverLock[lockIndex].lockHolder == -1){
//        //printf("Acquire: no lock holder, hold the lock\n");
//        serverLock[lockIndex].spaceHolder = replyTo;
//        serverLock[lockIndex].lockHolder = ThreadIndex;
//        sendInt(replyTo, ThreadIndex, lockIndex);
//        return;
//    }
//    else{
//        //wait here
//        //printf("Acquire: someone is holding the lock, wait\n");
//        serverLock[lockIndex].queue->Append((void*)ThreadIndex);
//        return;
//    }
}

void Release(int lockIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
    if(lockIndex >= numLock || lockIndex < 0){
        printf("Release: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
    std::vector<Binder> binderV = serverLock[lockIndex].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
         //is the space holder, can possibly release the lock
            if(serverLock[lockIndex].lockHolder.replyTo != replyTo || serverLock[lockIndex].lockHolder.ThreadIndex != ThreadIndex){
                printf("Release: not the lock holder\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;

            }
            else{
                //can actually release here
                sendInt(replyTo, ThreadIndex, lockIndex);
                if(serverLock[lockIndex].queue->IsEmpty()){
                    serverLock[lockIndex].lockHolder.replyTo = -1;
                    serverLock[lockIndex].lockHolder.ThreadIndex = -1;
                    return;
                }
                //some one is waiting
                else{
                    
                    Binder*  nextReply = (Binder*)serverLock[lockIndex].queue->Remove();
                    serverLock[lockIndex].lockHolder.replyTo = nextReply->replyTo;
                    serverLock[lockIndex].lockHolder.ThreadIndex = nextReply->ThreadIndex;
                    sendInt(nextReply->replyTo, nextReply->ThreadIndex, lockIndex);
                    return;
                }
                
            }
            
            
            
            
        }
    }
        
        printf("Release: not the space holder\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    
    
//    if(serverLock[lockIndex].spaceHolder != replyTo || serverLock[lockIndex].lockHolder != ThreadIndex){
//        printf("Release: not the lock holder\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    
    
//    
//    else{
//        // send back to client
//        sendInt(replyTo, ThreadIndex, lockIndex);
//        //if no one is waiting
//        if(serverLock[lockIndex].queue->IsEmpty()){
//            serverLock[lockIndex].lockHolder = -1;
//            return;
//        }
//        //some one is waiting
//        else{
//            
//            int  nextReply = (int )serverLock[lockIndex].queue->Remove();
//            serverLock[lockIndex].lockHolder = nextReply;
//            sendInt(replyTo, nextReply, lockIndex);
//            return;
//        }
//    }
    
}


void Wait(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex){
    
    //handle some exceptions
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
    
//    if(serverLock[lockIndex].spaceHolder != replyTo || serverCondition[conditionIndex].spaceHolder != replyTo){
//        printf("Wait: cannot access the lock or condition from other machine\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//
    std::vector<Binder> binderV = serverLock[lockIndex].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
            //is the space holder, can possibly wait
            if(serverLock[lockIndex].lockHolder.replyTo != replyTo || serverLock[lockIndex].lockHolder.ThreadIndex != ThreadIndex){
                printf("Release: not the lock holder\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
            }
            
            if(serverCondition[conditionIndex].waitingLock == -1){
                serverCondition[conditionIndex].waitingLock = lockIndex;
            }
            
            else if(serverCondition[conditionIndex].waitingLock != -1 && serverCondition[conditionIndex].waitingLock != lockIndex){
                printf("Wait: wrong condition to wait\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            Binder* binder = new Binder(replyTo, ThreadIndex);
            serverCondition[conditionIndex].queue->Append((Binder*)binder);
            //if someone is waiting
            if(!(serverLock[lockIndex].queue->IsEmpty())){
                Binder* nextReply = (Binder*)serverLock[lockIndex].queue->Remove();
                serverLock[lockIndex].lockHolder.replyTo = nextReply->replyTo;
                serverLock[lockIndex].lockHolder.ThreadIndex = nextReply->ThreadIndex;

                sendInt(nextReply->replyTo, nextReply->ThreadIndex, lockIndex);
                return;
            }
            //if none is waiting
            else{
                serverLock[lockIndex].lockHolder.replyTo = -1;
                serverLock[lockIndex].lockHolder.ThreadIndex = -1;
                return;

            }
            
        }
    }
    
    printf("Wait: not the lock sapce holder\n");
    sendInt(replyTo, ThreadIndex, -1);
    return;

    
//    if(serverLock[lockIndex].lockHolder != ThreadIndex){
//        printf("Wait: not the lock holder\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    
//    if(serverCondition[conditionIndex].waitingLock == -1){
//        serverCondition[conditionIndex].waitingLock = lockIndex;
//    }
//    
//    else if (serverCondition[conditionIndex].waitingLock != lockIndex){
//        printf("Wait: wrong condition to wait\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//    
//    serverCondition[conditionIndex].queue->Append((void*)ThreadIndex);
//    //if someone is waiting
//    if(!(serverLock[lockIndex].queue->IsEmpty())){
//        int nextReply = (int)serverLock[lockIndex].queue->Remove();
//        serverLock[lockIndex].lockHolder = nextReply;
//        sendInt(replyTo, nextReply, lockIndex);
//        return;
//    }
//    //if none is waiting
//    else{
//        serverLock[lockIndex].lockHolder = -1;
//        return;
//    }
}

void Signal(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
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
    
//    if(serverLock[lockIndex].spaceHolder != replyTo || serverCondition[conditionIndex].spaceHolder != replyTo){
//        printf("Signal: cannot access the lock or condition from other machine\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    
    std::vector<Binder> binderV = serverLock[lockIndex].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
        //is the space holder
            if(serverLock[lockIndex].lockHolder.replyTo != replyTo || serverLock[lockIndex].lockHolder.ThreadIndex != ThreadIndex){
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
            Binder* nextReply = (Binder*)serverCondition[conditionIndex].queue->Remove();
            if (serverCondition[conditionIndex].queue->IsEmpty()) {
                //no one is waiting
                serverCondition[conditionIndex].waitingLock = -1;
            }
            
            serverLock[lockIndex].queue->Append((Binder*)nextReply);
            sendInt(replyTo, ThreadIndex, lockIndex);
            return;
        }
    }
    
    printf("Signal: not the lock sapce holder\n");
    sendInt(replyTo, ThreadIndex, -1);
    return;

    
//    if(serverLock[lockIndex].lockHolder != ThreadIndex ){
//        printf("Signal: not the lock holder\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//    if (serverCondition[conditionIndex].waitingLock != lockIndex){
//        printf("Signal: wrong condition to signal\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//    if(serverCondition[conditionIndex].queue->IsEmpty()){
//        printf("Signal: nothing to signal\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//    int nextReply = (int)serverCondition[conditionIndex].queue->Remove();
//    if (serverCondition[conditionIndex].queue->IsEmpty()) {
//        //no one is waiting
//        serverCondition[conditionIndex].waitingLock = -1;
//    }
//    serverLock[lockIndex].queue->Append((void*)nextReply);
//    sendInt(replyTo, ThreadIndex, lockIndex);
}




void Broadcast(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
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
    
//    if(serverLock[lockIndex].spaceHolder != replyTo || serverCondition[conditionIndex].spaceHolder != replyTo){
//        printf("Broadcast: cannot access the lock or condition from other machine\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    
//    if(serverLock[lockIndex].lockHolder != ThreadIndex ){
//        printf("Broadcast: not the lock holder\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    
    std::vector<Binder> binderV = serverLock[lockIndex].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
            //is the space holder
            if(serverLock[lockIndex].lockHolder.replyTo != replyTo || serverLock[lockIndex].lockHolder.ThreadIndex != ThreadIndex){
                printf("Signal: not the lock holder\n");
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
            
            while (!serverCondition[conditionIndex].queue->IsEmpty()){
                
                Binder* nextReply = (Binder*)serverCondition[conditionIndex].queue->Remove();
                
                serverLock[lockIndex].queue->Append((Binder*)nextReply);
                
            }
            
            serverCondition[conditionIndex].waitingLock = -1;
            
            sendInt(replyTo, ThreadIndex, 0);
            
            return;
            
        }
    }
    
    printf("Broadcast: not the lock sapce holder\n");
    sendInt(replyTo, ThreadIndex, -1);
    return;
    
    
    
//    if (serverCondition[conditionIndex].waitingLock != lockIndex){
//        printf("Broadcast: wrong condition to signal\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
//    if(serverCondition[conditionIndex].queue->IsEmpty()){
//        printf("Broadcast: nothing to signal\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    //while loop to signal each waiting client
//    while (!serverCondition[conditionIndex].queue->IsEmpty()){
//        
//        int nextReply = (int)serverCondition[conditionIndex].queue->Remove();
//        
//        serverLock[lockIndex].queue->Append((void*)nextReply);
//        
//    }
//    
//    serverCondition[conditionIndex].waitingLock = -1;
//    
//    sendInt(replyTo, ThreadIndex, 0);
}

void CreateLock(char* name, int replyTo, int ThreadIndex){
    //create Lock syscall
    Binder binder(replyTo, ThreadIndex);
    for(int i = 0 ; i < numLock ; i++){
        
        if(strcmp(serverLock[i].name, name) == 0){//name already exists -> share
            
            printf("CreateLock: name already exists\n");
            
            serverLock[i].spaceHolder.push_back(binder);
            
            
            sendInt(replyTo, ThreadIndex, i);//send back the lock index
            

            
            return;
        }
    }
    //name does not exist

    serverLock[numLock].name = name;

    serverLock[numLock].spaceHolder.push_back(binder);

    
    sendInt(replyTo,ThreadIndex, numLock);

    numLock++;
    

    
    return;
}

void DestroyLock(int lockIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
    if(lockIndex >= numLock || lockIndex < 0){
        printf("DestroyLock: out of Lock boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
    
//    if(serverLock[lockIndex].spaceHolder != replyTo){
//        printf("DestroyLock: cannot access lock from other machines\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//        
//    }
    
    std::vector<Binder> binderV = serverLock[lockIndex].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
            //is the space holder
            serverLock[lockIndex].name = NULL;
            serverLock[lockIndex].lockHolder.replyTo = -1;
            serverLock[lockIndex].lockHolder.ThreadIndex = -1;
            serverLock[lockIndex].spaceHolder.clear();
            numLock--;
            
            sendInt(replyTo, ThreadIndex, lockIndex);
            
            return;
        }
    }
    
    printf("DestroyLock: not the space holder\n");
    
    sendInt(replyTo, ThreadIndex, -1);
    
    return;

}

void CreateCondition(char* name, int replyTo, int ThreadIndex){
    
    Binder binder(replyTo, ThreadIndex);
    for(int i = 0 ; i < numCondition ; i++){
        
        if(strcmp(serverCondition[i].name, name) == 0){//name already exists -> share
            printf("CreateCondition: name already exists\n");
            serverLock[i].spaceHolder.push_back(binder);
            
            sendInt(replyTo, ThreadIndex, i);//send back the condition index
            
            return;
        }
    }
    
    
    
    serverCondition[numCondition].name = name;
    
    serverCondition[numCondition].spaceHolder.push_back(binder);
    
    sendInt(replyTo, ThreadIndex, numCondition);
    
    numCondition++;
    
    return;
    
}


void DestroyCondition(int conditionIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
    if(conditionIndex >= numCondition || conditionIndex < 0){
        printf("DestroyCondition: out of Condition boundary\n");
        sendInt(replyTo, ThreadIndex, -1);
        return;
    }
//    if(serverCondition[conditionIndex].spaceHolder != replyTo){
//        printf("DestroyCondition: cannot access the lock or condition from other machine\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//    }
    
    std::vector<Binder> binderV = serverCondition[conditionIndex].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
            //is the space holder
            serverCondition[conditionIndex].name = NULL;
            serverCondition[conditionIndex].waitingLock = -1;
            serverCondition[conditionIndex].queue = new List;
            serverCondition[conditionIndex].spaceHolder.clear();
            numCondition--;
            
            sendInt(replyTo, ThreadIndex, conditionIndex);
            
            return;
            
        }
        
    }
    
    printf("DestroyCondition: not the space holder\n");
    sendInt(replyTo, ThreadIndex, -1);
    return;

}

void CreateMV(char* name, int replyTo, int ThreadIndex, int data){
    
    Binder binder(replyTo, ThreadIndex);
    
    for(int i = 0 ; i < numMonitor ;i++){
        if(strcmp(serverMonitorVariable[i].name, name) == 0) {//name already exists ->share
            
          serverMonitorVariable[i].spaceHolder.push_back(binder);
        
            sendInt(replyTo, ThreadIndex, i);
            
            return;
            
        }
    }
    
    //name does not exit
    
    serverMonitorVariable[numMonitor].name = name;
    
    serverMonitorVariable[numMonitor].data = data;
    
    serverMonitorVariable[numMonitor].spaceHolder.push_back(binder);
    
    sendInt(replyTo, ThreadIndex, numMonitor);
    
    numMonitor++;
    
    return;
    
}

void GetMV(int monitorIndex, int replyTo, int ThreadIndex){
    //handle some exceptions
    if(monitorIndex >= numMonitor || monitorIndex < 0 ){
        printf("GetMV: out of boundary\n");
        
        sendInt(replyTo, ThreadIndex, -1);
        
        return;
    }
    
    //make sure person getting MV is one of the space holders
    std::vector<Binder> binderV = serverMonitorVariable[monitorIndex].spaceHolder;
    
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
            
            sendInt(replyTo, ThreadIndex, serverMonitorVariable[monitorIndex].data);
            
            return;
            
        }
    }
    
    //wrong case
    printf("GetMV: not the space holder\n");
    
    sendInt(replyTo, ThreadIndex, -1);
    
    return;
    
}

void SetMV(int monitorIndex, int value, int replyTo, int ThreadIndex){
    //handle some exceptions
    if(monitorIndex >= numMonitor || monitorIndex < 0 ){
        printf("SetMV: out of boundary\n");
        
        sendInt(replyTo, ThreadIndex, -1);
        
        return;
    }
    
    std::vector<Binder> binderV = serverMonitorVariable[monitorIndex].spaceHolder;
    
    for(unsigned int i = 0; i < binderV.size(); i++){
        if(binderV[i].replyTo == replyTo){
            
            serverMonitorVariable[monitorIndex].data = value;
            
            sendInt(replyTo, ThreadIndex, monitorIndex);
            
            return;
            
        }
    }
    
    //wrong case
    printf("SetMV: not the space holder\n");
    
    sendInt(replyTo, ThreadIndex, -1);
    
    return;
}

void Server(){
    PacketHeader serverOutPktHdr;
    PacketHeader serverInPktHdr;
    MailHeader serverOutMailHdr;
    MailHeader serverInMailHdr;
    
    while(true){
        //printf("inside Server while loop\n");
        char receive[MaxMailSize];
        postOffice->Receive(0, &serverInPktHdr, &serverInMailHdr, receive);
        //need delete
        printf("Server Gets the msg: \"%s\" from Packet Header: %d, Mail Header %d\n", receive, serverInPktHdr.from, serverInMailHdr.from);
        fflush(stdout);
        //handle the incoming msg
        
        int replyTo = serverInPktHdr.from;
        std::stringstream ss;
        ss << receive;
        int syscall;
        ss >> syscall;
        if(syscall == 25){
            int lockIndex;
            ss >> lockIndex;
            
            Acquire(lockIndex, replyTo, serverInMailHdr.from);
            //printf("Server: Acquire syscall\n");
            
        }
        else if(syscall == 26){
            int lockIndex;
            ss >> lockIndex;
            
            Release(lockIndex, replyTo, serverInMailHdr.from);
            //printf("Server: Release syscall\n");
        }
        else if(syscall == 27){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            Wait(conditionIndex, lockIndex, replyTo, serverInMailHdr.from);
            //printf("Server: Wait syscall\n");
        }
        else if(syscall == 28){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            Signal(conditionIndex, lockIndex, replyTo, serverInMailHdr.from);
           // printf("Server: Signal syscall\n");
            
        }
        else if(syscall == 29){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            Broadcast(conditionIndex, lockIndex, replyTo, serverInMailHdr.from);
            //printf("Server: Broadcast syscall\n");
        }
        else if(syscall == 30){
            char lockName[100];
            ss >> lockName;
            
            CreateLock(lockName, replyTo, serverInMailHdr.from);
           // printf("Server: CreateLock syscall\n");
        }
        else if(syscall == 31){
            int lockIndex;
            ss >> lockIndex;
            
            DestroyLock(lockIndex, replyTo, serverInMailHdr.from);
           // printf("Server: DestroyLock syscall\n");
            
        }
        else if(syscall == 32){
            char conditionName[100];
            ss >> conditionName;
            
            CreateCondition(conditionName, replyTo, serverInMailHdr.from);
            //printf("Server: CreateCondition syscall\n");
        }
        else if(syscall == 33){
            int conditionIndex;
            ss >> conditionIndex;
            
            DestroyCondition(conditionIndex, replyTo, serverInMailHdr.from);
           // printf("Server: DestroyCondition syscall\n");
        }
        else if(syscall == 34){
            char monitorName[100];
            int data;
            ss >> monitorName;
           // printf("monitor name: %s \n", monitorName);
            ss >> data;
           // printf("monitor val: %d \n", data);
            
            CreateMV(monitorName, replyTo, serverInMailHdr.from, data);
           // printf("Server: CreateMonitor syscall\n");
        }
        else if(syscall == 35){
            int monitorIndex;
            ss >> monitorIndex;
            
            GetMV(monitorIndex, replyTo, serverInMailHdr.from);
            //printf("Server: GetMV syscall\n");
        }
        else if(syscall == 36){
            int monitorIndex;
            int data;
            ss >> monitorIndex;
            ss >> data;
            
            SetMV(monitorIndex, data, replyTo, serverInMailHdr.from);
            //printf("Server: SetMV syscall\n");
        }
        
        
    }
    
}

