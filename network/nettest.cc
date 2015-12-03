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
#include <deque>
// Test out message delivery, by doing the following:
//  1. send a message to the machine with ID "farAddr", at mail box #0
//  2. wait for the other machine's message to arrive (in our mailbox #0)
//  3. send an acknowledgment for the other machine's message
//  4. wait for an acknowledgement from the other machine to our 
//      original message
extern int getMachineID();

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
#define NUM_MONITOR_ARRAY 1000

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

struct ServerMonitorArray{
    char* name;
    std::vector<int> monitorVector;
    std::vector<Binder> spaceHolder;
    
    ServerMonitorArray(){
        
        name = NULL;
        
    }
    
};

struct Msg{
    int fromPktHeader;
    int fromMailHeader;
    char buffer[MaxMailSize];
    
    
    Msg(int fph, int fmh, char* content){
        fromPktHeader = fph;
        fromMailHeader =fmh;
        strcpy(buffer, content);
    }
};

//initialize all the variables with static arrays
ServerLock serverLock[NUM_LOCK];

ServerCondition serverCondition[NUM_CONDITION];

ServerMonitorVariable serverMonitorVariable[NUM_MONITOR];

ServerMonitorArray serverMonitorArray[NUM_MONITOR_ARRAY];

void initialize(){
    for (int i =0 ; i < NUM_LOCK ; i++ ){
        ServerLock tempLock;
        tempLock.name = NULL;
        tempLock.queue = new List;
        serverLock[i] = tempLock;
        
    }
    for (int i =0 ; i < NUM_CONDITION ; i++ ){
        ServerCondition tempLock;
        tempLock.name = NULL;
        tempLock.queue = new List;
        tempLock.waitingLock = -1;
        serverCondition[i] = tempLock;
        
    }
    for (int i =0 ; i < NUM_MONITOR ; i++ ){
        ServerMonitorVariable tempLock;
        tempLock.name = NULL;
        
        serverMonitorVariable[i] = tempLock;
        
    }
    for (int i =0 ; i < NUM_MONITOR_ARRAY ; i++ ){
        ServerMonitorArray tempLock;
        tempLock.name = NULL;
        serverMonitorArray[i] = tempLock;
        
    }
    
}


int numLock = 0;
int numCondition = 0;
int numMonitor = 0;
int numMonitorArray = 0;
//initialize msg queue

std::deque<Msg> msgQ;

//sendInt funciton to allow server to reply the clients with integers
void sendInt(int replyTo, int threadIndex, int value){
    
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    //send to specific machine and thread
    outPktHdr.to = replyTo;
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

void sendServerReply(int replyTo, int threadIndex, bool isYes){
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    //send to specific machine and thread
    outPktHdr.to = replyTo;
    outMailHdr.to = threadIndex;
    outMailHdr.from = 0;
    char* send = new char[10];
    
    if(isYes){
        sprintf(send, "s 1");
    }
    else{
        sprintf(send, "s 0");
    }
    outMailHdr.length = strlen(send) + 1;
    if(postOffice->Send(outPktHdr, outMailHdr, send) == false){
        printf("Server: Send failed\n");
        return;
    }
}

void forwardMsg(char* msg){
    
    for(int i = 0 ; i < NumServers ; i++ ){
        
        if(i != getMachineID()){
            PacketHeader outPktHdr;
            MailHeader outMailHdr;
            
            outPktHdr.to = i;
            outMailHdr.to = 0;
            outMailHdr.from = 0;
            outMailHdr.length = strlen(msg) + 1;
            postOffice->Send(outPktHdr, outMailHdr, msg);
        }
        
    }
    
}

void forwardMsgTo(int replyTo, int ThreadIndex, char* content){
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    //send to specific machine and thread
    outPktHdr.to = replyTo;
    outMailHdr.to = ThreadIndex;
    outMailHdr.from = 0;
    outMailHdr.length = strlen(content) + 1;
    postOffice->Send(outPktHdr, outMailHdr, content);
    
    
}

bool waitServerReply(){
    
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    std::vector<int> allServerReplys;
    
    while(allServerReplys.size() < NumServers-1){
    
    char* receive = new char[MaxMailSize];
    postOffice->Receive(0, &inPktHdr, &inMailHdr, receive);
    
    if(receive[0] == 's'){
        //server replys
        stringstream ss;
        ss << receive;
        
        char temp[20];
        ss >> temp;
        
        int confirm;
        
        ss >> confirm;
        
        allServerReplys.push_back(confirm);
        }
        
    else{
        //normal client requests
        Msg message(inPktHdr.from, inMailHdr.from, receive);
        
        msgQ.push_back(message);
        //need process later
        
    }
    }
    
    for(unsigned int i = 0; i< allServerReplys.size();i++){
        
        if(allServerReplys[i] == 1){
            
            return true;
            
        }
        
    }
    
    return false;
    
    
}

void Acquire(int lockIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    //handle some exceptions
    if(replyTo >= NumServers){//from client
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){//it's supposed on my machine
            int index = lockIndex % NUM_LOCK;
            
            if(index >= numLock || index < 0){
                printf("Acquire: out of Lock boundary\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                
                if(binderV[i].replyTo == replyTo){
                    //is the space holder, can possibly hold the lock
                    if(serverLock[index].lockHolder.replyTo == replyTo && serverLock[index].lockHolder.ThreadIndex == ThreadIndex){
                        printf("Acquire: already the lock holder\n");
                        sendInt(replyTo, ThreadIndex, -1);
                        return;
                    }
                    else if(serverLock[index].lockHolder.replyTo == -1 && serverLock[index].lockHolder.ThreadIndex == -1){
                        printf("Acquire: no lock holder, hold the lock\n");
                        serverLock[index].lockHolder.replyTo = replyTo;
                        serverLock[index].lockHolder.ThreadIndex = ThreadIndex;
                        sendInt(replyTo, ThreadIndex, lockIndex);
                        return;
                    }
                    else{
                        //someone is holding the lock. Wait here
                        printf("Acquire: someone is holding the lock, wait\n");
                        Binder* binder = new Binder(replyTo, ThreadIndex);
                        serverLock[index].queue->Append((Binder*)binder);
                        return;
                        
                    }
                }
            }
            
            printf("Acquire: not the space holder\n");
            sendInt(replyTo, ThreadIndex, -1);
            return;
            
        }
        else{//it's not on my machine
            char* send = new char[MaxMailSize];
            sprintf(send, "25 %d %d %d", lockIndex, replyTo, ThreadIndex);
            
            forwardMsg(send);
            
            bool isYes = waitServerReply();
            
            printf("Acquire: The result collected from other servers: %d\n", isYes);
            if(isYes){
                
                return;
                
            }
            
            else{
                printf("Acquire: error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;

            }
            
        }
        
        
    }
    else{//from server
        
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){//it's supposed on my machine
            int index = lockIndex % NUM_LOCK;
            
            if(index >= numLock || index < 0){
                printf("Acquire: out of Lock boundary\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyToClient){
                    //is the space holder, can possibly hold the lock
                    if(serverLock[index].lockHolder.replyTo == replyToClient && serverLock[index].lockHolder.ThreadIndex == ThreadIndexClient){
                        printf("Acquire: already the lock holder\n");
                        sendServerReply(replyTo, ThreadIndex, true);
                        return;
                    }
                    else if(serverLock[index].lockHolder.replyTo == -1 && serverLock[index].lockHolder.ThreadIndex == -1){
                        printf("Acquire: no lock holder, hold the lock\n");
                        serverLock[index].lockHolder.replyTo = replyToClient;
                        serverLock[index].lockHolder.ThreadIndex = ThreadIndexClient;
                        sendInt(replyToClient, ThreadIndexClient, lockIndex);
                        sendServerReply(replyTo, ThreadIndex, true);
                        return;
                    }
                    else{
                        //someone is holding the lock. Wait here
                        printf("Acquire: someone is holding the lock, wait\n");
                        Binder* binder = new Binder(replyToClient, ThreadIndexClient);
                        serverLock[index].queue->Append((Binder*)binder);
                        sendServerReply(replyTo, ThreadIndex, true);
                        return;
                        
                    }
                }
            }
            
            printf("Acquire: not the space holder\n");
            sendServerReply(replyTo, ThreadIndex, false);
            return;
            
        }
        else{
            sendServerReply(replyTo, ThreadIndex, false);
            return;
        }
    }
}

void Release(int lockIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    //handle some exceptions
    if(replyTo >= NumServers){//from clients
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){//it's supposed on my machine
            int index = lockIndex % NUM_LOCK;
            if(index >= numLock || index < 0){
                printf("Release: out of Lock boundary\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyTo){
                    //is the space holder, can possibly release the lock
                    if(serverLock[index].lockHolder.replyTo != replyTo || serverLock[index].lockHolder.ThreadIndex != ThreadIndex){
                        printf("Release: not the lock holder\n");
                        sendInt(replyTo, ThreadIndex, -1);
                        return;
                        
                    }
                    else{
                        //can actually release here
                        sendInt(replyTo, ThreadIndex, lockIndex);
                        if(serverLock[index].queue->IsEmpty()){
                            serverLock[index].lockHolder.replyTo = -1;
                            serverLock[index].lockHolder.ThreadIndex = -1;
                            return;
                        }
                        //some one is waiting
                        else{
                            
                            Binder*  nextReply = (Binder*)serverLock[index].queue->Remove();
                            serverLock[index].lockHolder.replyTo = nextReply->replyTo;
                            serverLock[index].lockHolder.ThreadIndex = nextReply->ThreadIndex;
                            sendInt(nextReply->replyTo, nextReply->ThreadIndex, lockIndex);
                            return;
                        }
                        
                    }
                    
                }
            }
            
            printf("Release: not the space holder\n");
            sendInt(replyTo, ThreadIndex, -1);
            return;

            
        }
        else{//it's not on my machine
            
            char* send = new char[MaxMailSize];
            sprintf(send, "26 %d %d %d", lockIndex, replyTo, ThreadIndex);
            
            forwardMsg(send);
            
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                printf("Release: error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
            }
        }
    }
    
    else{//from servers
        
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){//it's supposed on my machine
            int index = lockIndex % NUM_LOCK;
            if(index >= numLock || index < 0){
                printf("Release: out of Lock boundary\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
            for(unsigned int i = 0; i < binderV.size(); i++){
//                printf("In release: binderV[i].replyTo: %d, replytoClient: %d\n", binderV[i].replyTo, replyToClient);
                if(binderV[i].replyTo == replyToClient){
                    //is the space holder, can possibly release the lock
                    if(serverLock[index].lockHolder.replyTo != replyToClient || serverLock[index].lockHolder.ThreadIndex != ThreadIndexClient){
//                        printf("current lock holder replyto: %d, threadindex :%d\n", serverLock[index].lockHolder.replyTo, serverLock[index].lockHolder.ThreadIndex);
//                        printf("the one who wants to release replyto: %d, threadindex: %d \n", replyToClient, ThreadIndexClient);
                        printf("Release: not the lock holder\n");
                        sendServerReply(replyTo, ThreadIndex, false);
                        return;
                        
                    }
                    else{
                        //can actually release here
                        sendServerReply(replyTo, ThreadIndex, true);

                        sendInt(replyToClient, ThreadIndexClient, lockIndex);
                        if(serverLock[index].queue->IsEmpty()){
                            serverLock[index].lockHolder.replyTo = -1;
                            serverLock[index].lockHolder.ThreadIndex = -1;
                            return;
                        }
                        //some one is waiting
                        else{
                            
                            Binder*  nextReply = (Binder*)serverLock[index].queue->Remove();
                            serverLock[index].lockHolder.replyTo = nextReply->replyTo;
                            serverLock[index].lockHolder.ThreadIndex = nextReply->ThreadIndex;
                            sendInt(nextReply->replyTo, nextReply->ThreadIndex, lockIndex);
                            return;
                        }
                        
                    }
                    
                }
            }
            
            printf("Release: not the space holder\n");
            sendServerReply(replyTo, ThreadIndex, false);
            return;
            
            
        }
        else{
            
            sendServerReply(replyTo, ThreadIndex, false);
            return;
            
        }
        
        
        
    }
    
}


void Wait(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient, int conditionOrLock){
    
    if(replyTo >= NumServers){//from clients
        //check condiiton
        
        int machineC = conditionIndex/NUM_CONDITION;
        if(machineC == getMachineID()){
            int Cindex = conditionIndex % NUM_CONDITION;
            
            if(Cindex >= numCondition || Cindex < 0){
                printf("Wait: out of Condition boundary\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            //condition
            if(serverCondition[Cindex].waitingLock == -1){
                serverCondition[Cindex].waitingLock = lockIndex;
            }
            
            else if(serverCondition[Cindex].waitingLock != -1 && serverCondition[Cindex].waitingLock != lockIndex){
                printf("Wait: wrong condition to wait\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            //condition
            
            Binder* binder = new Binder(replyTo, ThreadIndex);
            serverCondition[Cindex].queue->Append((Binder*)binder);     //condition
            
            
        }
        else{//condition on other machines
            char* send = new char[MaxMailSize];
            sprintf(send, "27 %d %d %d %d 0", conditionIndex,lockIndex, replyTo, ThreadIndex);
            
            forwardMsg(send);
            
            bool isYes = waitServerReply();
            
            if(isYes){
                
                
            }
            
            else{
                printf("Wait: error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
            }
            
        }
        
        //check lock
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){//it's supposed on my machine
            int index = lockIndex % NUM_LOCK;
            if(index >= numLock || index < 0){
                printf("Wait: out of Lock boundary\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyTo){
                    //is the space holder, can possibly wait
                    if(serverLock[index].lockHolder.replyTo != replyTo || serverLock[index].lockHolder.ThreadIndex != ThreadIndex){
                        printf("Wait: not the lock holder\n");
                        sendInt(replyTo, ThreadIndex, -1);
                        return;
                        
                    }
                    
                    //lock
                    if(!(serverLock[index].queue->IsEmpty())){
                        Binder* nextReply = (Binder*)serverLock[index].queue->Remove();
                        serverLock[index].lockHolder.replyTo = nextReply->replyTo;
                        serverLock[index].lockHolder.ThreadIndex = nextReply->ThreadIndex;
                        
                        sendInt(nextReply->replyTo, nextReply->ThreadIndex, lockIndex);
                        return;
                    }
                    //if none is waiting
                    else{
                        serverLock[index].lockHolder.replyTo = -1;
                        serverLock[index].lockHolder.ThreadIndex = -1;
                        return;
                        
                    }
                    
                }
            }
            
            printf("Wait: not the space holder\n");
            sendInt(replyTo, ThreadIndex, -1);
            return;
            
            
        }
        else{
            char* send = new char[MaxMailSize];
            sprintf(send, "27 %d %d %d %d 1", conditionIndex, lockIndex, replyTo, ThreadIndex);
            
            forwardMsg(send);
            
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                printf("Wait: error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
            }
            
        }
        
        
        
    }
    else{//from servers
        if(conditionOrLock == 0){ // condition case
        int machineC = conditionIndex/NUM_CONDITION;
        if(machineC == getMachineID()){
            int Cindex = conditionIndex % NUM_CONDITION;
            
            if(Cindex >= numCondition || Cindex < 0){
                printf("Wait: out of Condition boundary\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            //condition
            if(serverCondition[Cindex].waitingLock == -1){
                serverCondition[Cindex].waitingLock = lockIndex;
            }
            
            else if(serverCondition[Cindex].waitingLock != -1 && serverCondition[Cindex].waitingLock != lockIndex){
                printf("Wait: wrong condition to wait\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            //condition
            sendServerReply(replyTo, ThreadIndex, true);
            Binder* binder = new Binder(replyToClient, ThreadIndexClient);
            serverCondition[Cindex].queue->Append((Binder*)binder);     //condition
            
            
        }
        else{
            sendServerReply(replyTo, ThreadIndex, false);
            return;
        }
            
            
        }
        if(conditionOrLock == 1){
            int machinel = lockIndex/NUM_LOCK;
            
            if(machinel == getMachineID()){//it's supposed on my machine
                int index = lockIndex % NUM_LOCK;
                if(index >= numLock || index < 0){
                    printf("Wait: out of Lock boundary\n");
                    sendServerReply(replyTo, ThreadIndex, false);
                    return;
                }
                
                std::vector<Binder> binderV = serverLock[index].spaceHolder;
                for(unsigned int i = 0; i < binderV.size(); i++){
                    if(binderV[i].replyTo == replyToClient){
                        //is the space holder, can possibly wait
                        if(serverLock[index].lockHolder.replyTo != replyToClient || serverLock[index].lockHolder.ThreadIndex != ThreadIndexClient){
                            printf("Release: not the lock holder\n");
                            sendServerReply(replyTo, ThreadIndex, false);
                            return;
                            
                        }
                        
                        //lock
                        sendServerReply(replyTo, ThreadIndex, true);

                        if(!(serverLock[index].queue->IsEmpty())){
                            Binder* nextReply = (Binder*)serverLock[index].queue->Remove();
                            serverLock[index].lockHolder.replyTo = nextReply->replyTo;
                            serverLock[index].lockHolder.ThreadIndex = nextReply->ThreadIndex;
                            
                            sendInt(nextReply->replyTo, nextReply->ThreadIndex, lockIndex);
                            return;
                        }
                        //if none is waiting
                        else{
                            serverLock[index].lockHolder.replyTo = -1;
                            serverLock[index].lockHolder.ThreadIndex = -1;
                            return;
                            
                        }
                        
                    }
                }
                printf("Wait: not the space holder\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
                
                
                
                
            }
            else{
                
                sendServerReply(replyTo, ThreadIndex, false);
                return;
                
            }
        }
    }
    
}

void Signal(int conditionIndex, int lockIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    
    if(replyTo >= NumServers){//from other clients
        //check condiiton
        int machineC = conditionIndex/NUM_CONDITION;
        if(machineC == getMachineID()){
            int Cindex = conditionIndex % NUM_CONDITION;
            int machineL = lockIndex / NUM_LOCK;
            if(Cindex >= numCondition || Cindex < 0){
                printf("Signal: out of Condition boundary\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            if (serverCondition[Cindex].waitingLock != lockIndex){
                printf("Signal: wrong condition to signal\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            if(serverCondition[Cindex].queue->IsEmpty()){
                printf("Signal: nothing to signal\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            Binder* nextReply = (Binder*)serverCondition[Cindex].queue->Remove();
            if (serverCondition[Cindex].queue->IsEmpty()) {
                //no one is waiting
                serverCondition[Cindex].waitingLock = -1;
            }
            //send to machine can process lock part
            char* send = new char[MaxMailSize];
            sprintf(send, "280 %d %d %d %d %d", lockIndex, nextReply->replyTo, nextReply->ThreadIndex, replyTo, ThreadIndex);
            forwardMsgTo(machineL, 0, send);
            return;
            
        }
        else{//condition not on my machine, cannot process request, forward to other machine
            
            char* send = new char[MaxMailSize];
            sprintf(send, "28 %d %d %d %d", conditionIndex, lockIndex, replyTo, ThreadIndex);
            forwardMsgTo(machineC, 0, send);
            return;
            
        }
    }
    else{//from other servers
        //check condiiton
        int machineC = conditionIndex/NUM_CONDITION;
        if(machineC == getMachineID()){
            int Cindex = conditionIndex % NUM_CONDITION;
            int machineL = lockIndex / NUM_LOCK;
            if(Cindex >= numCondition || Cindex < 0){
                printf("Signal: out of Condition boundary\n");
                sendInt(replyToClient, ThreadIndexClient, -1);
                return;
            }
            
            if (serverCondition[Cindex].waitingLock != lockIndex){
                printf("Signal: wrong condition to signal\n");
                sendInt(replyToClient, ThreadIndexClient, -1);
                return;
            }
            
            if(serverCondition[Cindex].queue->IsEmpty()){
                printf("Signal: nothing to signal\n");
                sendInt(replyToClient, ThreadIndexClient, -1);
                return;
            }
            
            Binder* nextReply = (Binder*)serverCondition[Cindex].queue->Remove();
            if (serverCondition[Cindex].queue->IsEmpty()) {
                //no one is waiting
                serverCondition[Cindex].waitingLock = -1;
            }
            //send to machine can process lock part
            char* send = new char[MaxMailSize];
            sprintf(send, "280 %d %d %d %d %d", lockIndex, nextReply->replyTo, nextReply->ThreadIndex, replyToClient, ThreadIndexClient);
            forwardMsgTo(machineL, 0, send);
            return;
        }
        
    }
    
}

//the lock part of signal syscall
void processSignal(int lockIndex, int nextReplyTo, int nextThreadIndex, int replyToClient, int ThreadIndexClient){
    
    int machinel = lockIndex/NUM_LOCK;
    
    if(machinel == getMachineID()){//it's supposed on my machine
        int index = lockIndex % NUM_LOCK;
        if(index >= numLock || index < 0){
            printf("Signal: out of Lock boundary\n");
            sendInt(replyToClient, ThreadIndexClient, -1);
            return;
        }
    
    Binder* binder = new Binder(nextReplyTo, nextThreadIndex);
    
    std::vector<Binder> binderV = serverLock[index].spaceHolder;
    for(unsigned int i = 0; i < binderV.size(); i++){
        printf("In Signal: binderV[i].replyTo: %d, replytoClient: %d\n", binderV[i].replyTo, replyToClient);

        if(binderV[i].replyTo == replyToClient){
            //is the space holder
            if(serverLock[index].lockHolder.replyTo != replyToClient || serverLock[index].lockHolder.ThreadIndex != ThreadIndexClient){
                printf("Signal: not the lock holder\n");
                sendInt(replyToClient, ThreadIndexClient, -1);
                return;
                
            }
            
            serverLock[index].queue->Append((Binder*)binder);//note mix here
            sendInt(nextReplyTo, nextThreadIndex, lockIndex);
            sendInt(replyToClient, ThreadIndexClient, lockIndex);
            return;
        }
    }
    
    printf("Signal: not the lock sapce holder\n");
    sendInt(replyToClient, ThreadIndexClient, -1);
    return;
        
    }
    else{
        sendInt(replyToClient, ThreadIndexClient, -1);
        return;
        
    }

    
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

void CreateLock(char* name, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    //create Lock syscall
    Binder binder(replyTo, ThreadIndex);
    
    if( replyTo >= NumServers){ //receive msg from clients
    for(int i = 0 ; i < numLock ; i++){
        printf("CreateLock: from client name: %s, in server name: %s\n", name, serverLock[i].name);
        printf("CreateLock: Current Lock size: %d\n", numLock);
        if(strcmp(serverLock[i].name, name) == 0){//name already exists -> share
            
            printf("CreateLock: name already exists\n");
            
            serverLock[i].spaceHolder.push_back(binder);
            
            printf("in createlcok: binder.replyto: %d , binder.threadindex: %d \n", binder.replyTo, binder.ThreadIndex);
            sendInt(replyTo, ThreadIndex, NUM_LOCK*getMachineID()+i);//send back the lock index
            
            return;
        }
    }
        
    //not in my machine, check other machines
    char* send = new char[MaxMailSize];
    sprintf(send, "30 %s %d %d", name, replyTo, ThreadIndex);
    
        //forward this request to all other machines
        forwardMsg(send);
    
      bool isYes = waitServerReply();
        printf("CreateLock: The result collected from other servers: %d\n", isYes);

        if(isYes){
            
            return;
            
        }
        
        else{
            //create new

            serverLock[numLock].name = new char[strlen(name)];
            strcpy(serverLock[numLock].name, name);
            
            
            strcpy( serverLock[numLock].name, name);
            serverLock[numLock].spaceHolder.push_back(binder);

//            printf("in createlcok: binder.replyto: %d , binder.threadindex: %d \n", binder.replyTo, binder.ThreadIndex);


    
            sendInt(replyTo,ThreadIndex, NUM_LOCK*getMachineID()+numLock);
            printf("5\n");

            numLock++;
        
            return;
        }
    
    }
    
    else{// this is a forwarded msg from other servers
        Binder binder2(replyToClient, ThreadIndexClient);

        for(int i = 0 ; i < numLock ; i++){
            
            if(strcmp(serverLock[i].name, name) == 0){//name already exists -> share
                
                printf("CreateLock: name already exists\n");
                
                serverLock[i].spaceHolder.push_back(binder2);
                
//                printf("in createlcok: binder2.replyto: %d , binder2.threadindex: %d \n", binder2.replyTo, binder2.ThreadIndex);

                
                sendInt(replyToClient, ThreadIndexClient, NUM_LOCK*getMachineID()+i);//send client the index
                
                sendServerReply(replyTo, ThreadIndex, true);//send back the server reply
                
                return;
                
            }
        }
        
        sendServerReply(replyTo, ThreadIndex, false);
        return;
        
    }
    
    
}

void DestroyLock(int lockIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    //handle some exceptions
    if(replyTo >= NumServers){//clients send
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){//it's supposed on my machine
            int index = lockIndex % NUM_LOCK;
            
            if(index >= numLock || index < 0){
                printf("DestroyLock: out of Lock boundary\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
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
        else{//on other machine
            char* send = new char[MaxMailSize];
            sprintf(send, "31 %d %d %d", lockIndex, replyTo, ThreadIndex);
            
            forwardMsg(send);
            
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                
                printf("DestroyLock: error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
            }
            
            
        }
        
        
    }
    else{//server send
        int machinel = lockIndex/NUM_LOCK;
        if(machinel == getMachineID()){
            int index = lockIndex % NUM_LOCK;
            
            if(index >= numLock || index < 0){
                printf("DestroyLock: out of Lock boundary\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            
            std::vector<Binder> binderV = serverLock[index].spaceHolder;
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyToClient){
                    //is the space holder
                    serverLock[lockIndex].name = NULL;
                    serverLock[lockIndex].lockHolder.replyTo = -1;
                    serverLock[lockIndex].lockHolder.ThreadIndex = -1;
                    serverLock[lockIndex].spaceHolder.clear();
                    numLock--;
                    
                    sendInt(replyToClient, ThreadIndexClient, lockIndex);
                    sendServerReply(replyTo, ThreadIndex, true);
                    return;
                }
            }
            
            printf("DestroyLock: not the space holder\n");
            
            sendServerReply(replyTo, ThreadIndex, false);
            
            return;

            
        }
        else{
            sendServerReply(replyTo, ThreadIndex, false);
            return;
        }
    }
    
//    if(serverLock[lockIndex].spaceHolder != replyTo){
//        printf("DestroyLock: cannot access lock from other machines\n");
//        sendInt(replyTo, ThreadIndex, -1);
//        return;
//        
//    }

}

void CreateCondition(char* name, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    
    Binder binder(replyTo, ThreadIndex);
    printf("11\n");
    if(replyTo >= NumServers){//client msg
        printf("12\n");
    for(int i = 0 ; i < numCondition ; i++){
        printf("in loop: %d\n ", i);
        printf("CreateCondition: from client name: %s, in server name: %s\n", name, serverCondition[i].name);
        printf("CreateCondition: Current Lock size: %d\n", numCondition);
        if(strcmp(serverCondition[i].name, name) == 0){//name already exists -> share
            printf("CreateCondition: name already exists\n");
            serverLock[i].spaceHolder.push_back(binder);
            
            sendInt(replyTo, ThreadIndex, NUM_CONDITION*getMachineID() + i);//send back the condition index
            
            return;
        }
    }
        //not in my machine, check other machines
        char* send = new char[MaxMailSize];
        sprintf(send, "32 %s %d %d", name, replyTo, ThreadIndex);
        
        //forward this request to all other machines
        forwardMsg(send);
        //wait other servers to reply
        bool isYes = waitServerReply();
        
        if(isYes){
            
            return;
            
        }
        else{
            //create new
            printf("1\n");
            serverCondition[numCondition].name = new char[strlen(name)];
            printf("2\n");

            strcpy(serverCondition[numCondition].name, name);
            printf("3\n");

            
            serverCondition[numCondition].spaceHolder.push_back(binder);
            printf("4\n");

            
            sendInt(replyTo, ThreadIndex, NUM_CONDITION*getMachineID() + numCondition);
            printf("5\n");

            
            numCondition++;
            
            return;
            
        }
    
    }
    else{//server msg
        Binder binder2(replyToClient, ThreadIndexClient);

        
        for(int i = 0 ; i < numCondition ; i++){
        if(strcmp(serverCondition[i].name, name) == 0){//name already exists -> share
            printf("CreateCondition: name already exists\n");
            serverLock[i].spaceHolder.push_back(binder2);
            sendInt(replyToClient, ThreadIndexClient, NUM_CONDITION*getMachineID()+i);//send client the index
            sendServerReply(replyTo, ThreadIndex, true);//send back the condition index
            
            return;
            }
        }
        
        sendServerReply(replyTo, ThreadIndex, false);
        return;
    }
    
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

void CreateMV(char* name, int replyTo, int ThreadIndex, int data, int replyToClient, int ThreadIndexClient){
    
    Binder binder(replyTo, ThreadIndex);
    
    if(replyTo >= NumServers){//clients
        
    for(int i = 0 ; i < numMonitor ;i++){
        if(strcmp(serverMonitorVariable[i].name, name) == 0) {//name already exists ->share
            
          serverMonitorVariable[i].spaceHolder.push_back(binder);
        
            sendInt(replyTo, ThreadIndex, NUM_MONITOR*getMachineID() + i);
            
            return;
            
        }
    }
    
    //name does not exit
        char* send = new char[MaxMailSize];
        sprintf(send, "34 %s %d %d %d", name, data, replyTo, ThreadIndex);
        
        //forward this request to all other machines
        forwardMsg(send);
        //wait other servers to reply
        bool isYes = waitServerReply();
        
        if(isYes){
            
            return;
            
        }
        else{
            //create new
            
            serverMonitorVariable[numMonitor].name = new char[strlen(name)];
            
            strcpy(serverMonitorVariable[numMonitor].name, name);
            
            serverMonitorVariable[numMonitor].data = data;
            
            serverMonitorVariable[numMonitor].spaceHolder.push_back(binder);
            
            sendInt(replyTo, ThreadIndex, NUM_MONITOR*getMachineID() + numMonitor);
            
            numMonitor++;
            
            return;
        }
    
    }
    else{ //servers
        Binder binder2(replyToClient, ThreadIndexClient);

        
        for(int i = 0 ; i < numMonitor ;i++){
            if(strcmp(serverMonitorVariable[i].name, name) == 0) {//name already exists ->share
                
                serverMonitorVariable[i].spaceHolder.push_back(binder2);
                
                sendInt(replyToClient, ThreadIndexClient, NUM_MONITOR*getMachineID() + i);
                
                sendServerReply(replyTo, ThreadIndex, true);
                
                return;
                
            }
        }
        
        sendServerReply(replyTo, ThreadIndex, false);
        return;
    }
    
}

void GetMV(int monitorIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    //handle some exceptions
    if(replyTo >= NumServers){//from clients
        int machineM = monitorIndex / NUM_MONITOR;
        if(machineM == getMachineID()){//on my machine
            int index = monitorIndex % NUM_MONITOR;
            
            if(index >= numMonitor || index < 0 ){
                printf("GetMV: out of boundary\n");
                
                sendInt(replyTo, ThreadIndex, -1);
                
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorVariable[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyTo){
                    
                    sendInt(replyTo, ThreadIndex, serverMonitorVariable[index].data);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("GetMV: not the space holder\n");
            
            sendInt(replyTo, ThreadIndex, -1);
            
            return;
            
        }
        else{//on other machine
            char* send = new char[MaxMailSize];
            sprintf(send, "35 %d %d %d", monitorIndex, replyTo, ThreadIndex);
            
            //forward this request to all other machines
            forwardMsg(send);
            //wait other servers to reply
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                
                printf("GetMV: Error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
                
            }
            
        }
        
    }
    else{//from servers
            int machineM = monitorIndex / NUM_MONITOR;
            if(machineM == getMachineID()){//on my machine
                int index = monitorIndex % NUM_MONITOR;
                
                if(index >= numMonitor || index < 0 ){
                    printf("GetMV: out of boundary\n");
                    
                    sendServerReply(replyTo, ThreadIndex, false);
                    return;
                }
                
                std::vector<Binder> binderV = serverMonitorVariable[index].spaceHolder;
                
                for(unsigned int i = 0; i < binderV.size(); i++){
                    if(binderV[i].replyTo == replyToClient){
                        
                        sendServerReply(replyTo, ThreadIndex, true);

                        sendInt(replyToClient, ThreadIndexClient, serverMonitorVariable[index].data);
                        
                        return;
                        
                    }
                }
                
                //wrong case
                printf("GetMV: not the space holder\n");
                
                sendServerReply(replyTo, ThreadIndex, false);
                
                return;
                
            }
        
            else{
                
                sendServerReply(replyTo, ThreadIndex, false);

                return;
                
            }
        
        
        
        
        
    }
    
}

void SetMV(int monitorIndex, int value, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    //handle some exceptions
    if(replyTo >= NumServers){
        int machineM = monitorIndex / NUM_MONITOR;
        if(machineM == getMachineID()){//on my machine
            int index = monitorIndex % NUM_MONITOR;
            
            if(index >= numMonitor || index < 0 ){
                printf("SetMV: out of boundary\n");
                
                sendInt(replyTo, ThreadIndex, -1);
                
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorVariable[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyTo){
                    
                    serverMonitorVariable[index].data = value;
                    
                    sendInt(replyTo, ThreadIndex, monitorIndex);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("SetMV: not the space holder\n");
            
            sendInt(replyTo, ThreadIndex, -1);
            
            return;

            
            
            
        }
        else{ //not on my machine
            char* send = new char[MaxMailSize];
            sprintf(send, "36 %d %d %d %d", monitorIndex, value, replyTo, ThreadIndex);
            
            //forward this request to all other machines
            forwardMsg(send);
            //wait other servers to reply
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                
                printf("SetMV: Error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
                
            }
        }
        
        
        
    }
    else{//from other servers
        int machineM = monitorIndex / NUM_MONITOR;
        if(machineM == getMachineID()){//on my machine
            int index = monitorIndex % NUM_MONITOR;
            
            if(index >= numMonitor || index < 0 ){
                printf("SetMV: out of boundary\n");
                
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorVariable[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyToClient){
                    
                    serverMonitorVariable[index].data = value;
                    
                    sendServerReply(replyTo, ThreadIndex, true);

                    sendInt(replyToClient, ThreadIndexClient, monitorIndex);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("SetMV: not the space holder\n");
            
            sendServerReply(replyTo, ThreadIndex, false);
            
            return;
            
            
            
            
        }
        else{
            sendServerReply(replyTo, ThreadIndex, false);
            return;

        }
        
        
    }
}

void CreateMVArray(char* name, int length, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    
    Binder binder(replyTo, ThreadIndex);

    if(replyTo >= NumServers){
        
        for(int i = 0 ; i < numMonitorArray ;i++){
            if(strcmp(serverMonitorArray[i].name, name) == 0) {//name already exists ->share
                
                serverMonitorArray[i].spaceHolder.push_back(binder);
                
                sendInt(replyTo, ThreadIndex, NUM_MONITOR_ARRAY*getMachineID() + i);
                
                return;
                
            }
        }
        
        //name does not exit
        char* send = new char[MaxMailSize];
        sprintf(send, "37 %s %d %d %d", name, length, replyTo, ThreadIndex);
        
        //forward this request to all other machines
        forwardMsg(send);
        //wait other servers to reply
        bool isYes = waitServerReply();
        
        if(isYes){
            
            return;
            
        }
        
        else{
            //create new
            
            serverMonitorArray[numMonitorArray].name = new char[strlen(name)];
            strcpy(serverMonitorArray[numMonitorArray].name, name);
            
            serverMonitorArray[numMonitorArray].spaceHolder.push_back(binder);
            
            for(int i = 0 ; i< length ;i++){
            
                serverMonitorArray[numMonitorArray].monitorVector.push_back(-1);//initialize all data to -1;
                
            }
            
            sendInt(replyTo, ThreadIndex, NUM_MONITOR_ARRAY*getMachineID() + numMonitorArray);
            
            numMonitorArray++;
            
            return;
        }
        
        
    }
    else{//from other servers
        Binder binder2(replyToClient, ThreadIndexClient);
            
            for(int i = 0 ; i < numMonitorArray ;i++){
                
                if(strcmp(serverMonitorArray[i].name, name) == 0) {//name already exists ->share
                    
                    serverMonitorArray[i].spaceHolder.push_back(binder2);
                    
                    sendInt(replyToClient, ThreadIndexClient, NUM_MONITOR_ARRAY*getMachineID() + i);
                    sendServerReply(replyTo, ThreadIndex, true);

                    
                    return;
                    
                }
            }
            
            sendServerReply(replyTo, ThreadIndex, false);
            return;
        
    }
    
}

void GetMVArray(int monitorArrayIndex, int InnerIndex, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    if(replyTo >= NumServers){
        int machineMA = monitorArrayIndex / NUM_MONITOR_ARRAY;
        if(machineMA == getMachineID()){//on my machine
            int index = monitorArrayIndex % NUM_MONITOR_ARRAY;
        
            if(index >= numMonitorArray || index < 0 ){
                printf("GetMVArray: out of boundary\n");
                
                sendInt(replyTo, ThreadIndex, -1);
                
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorArray[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyTo){
                    
                    sendInt(replyTo, ThreadIndex, serverMonitorArray[index].monitorVector[InnerIndex]);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("GetMVArray: not the space holder\n");
            
            sendInt(replyTo, ThreadIndex, -1);
            
            return;

            
        }
        else{//not on my machine
            char* send = new char[MaxMailSize];
            sprintf(send, "38 %d %d %d %d", monitorArrayIndex, InnerIndex,replyTo, ThreadIndex);
            
            //forward this request to all other machines
            forwardMsg(send);
            //wait other servers to reply
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                
                printf("GetMVArray: Error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
                
            }
            
        }
    }
    else{
        
        int machineMA = monitorArrayIndex / NUM_MONITOR_ARRAY;
        if(machineMA == getMachineID()){//on my machine
            int index = monitorArrayIndex % NUM_MONITOR_ARRAY;
            
            if(index >= numMonitorArray || index < 0 ){
                printf("GetMVArray: out of boundary\n");
                sendServerReply(replyTo, ThreadIndex, false);
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorArray[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyToClient){
                    
                    sendServerReply(replyTo, ThreadIndex, true);

                    sendInt(replyToClient, ThreadIndexClient, serverMonitorArray[index].monitorVector[InnerIndex]);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("GetMVArray: not the space holder\n");
            
            sendServerReply(replyTo, ThreadIndex, false);
            
            return;
            
            
        }
        else{
            sendServerReply(replyTo, ThreadIndex, false);
            
            return;

            
        }

    }
    
}

void SetMVArray(int monitorArrayIndex, int InnerIndex, int data, int replyTo, int ThreadIndex, int replyToClient, int ThreadIndexClient){
    if(replyTo >= NumServers){
        int machineMA = monitorArrayIndex / NUM_MONITOR_ARRAY;
        if(machineMA == getMachineID()){//on my machine
            int index = monitorArrayIndex % NUM_MONITOR_ARRAY;
            
            if(index >= numMonitorArray || index < 0 ){
                printf("SetMVArray: out of boundary\n");
                
                sendInt(replyTo, ThreadIndex, -1);
                
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorArray[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyTo){
                    
                    serverMonitorArray[index].monitorVector[InnerIndex] = data;
                    
                    sendInt(replyTo, ThreadIndex, monitorArrayIndex);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("SetMVArray: not the space holder\n");
            
            sendInt(replyTo, ThreadIndex, -1);
            
            return;
            
            
        }
        else{
            char* send = new char[MaxMailSize];
            sprintf(send, "39 %d %d %d %d %d", monitorArrayIndex, InnerIndex, data, replyTo, ThreadIndex);
            
            //forward this request to all other machines
            forwardMsg(send);
            //wait other servers to reply
            bool isYes = waitServerReply();
            
            if(isYes){
                
                return;
                
            }
            
            else{
                
                printf("SetMVArray: Error occurs\n");
                sendInt(replyTo, ThreadIndex, -1);
                return;
                
                
            }
            
        }
        
        
    }
    else{
        
        int machineMA = monitorArrayIndex / NUM_MONITOR_ARRAY;
        if(machineMA == getMachineID()){//on my machine
            int index = monitorArrayIndex % NUM_MONITOR_ARRAY;
            
            if(index >= numMonitorArray || index < 0 ){
                printf("SetMVArray: out of boundary\n");
                
                sendServerReply(replyTo, ThreadIndex ,false);
                
                
                
                return;
            }
            
            std::vector<Binder> binderV = serverMonitorArray[index].spaceHolder;
            
            for(unsigned int i = 0; i < binderV.size(); i++){
                if(binderV[i].replyTo == replyToClient){
                    
                    serverMonitorArray[index].monitorVector[InnerIndex] = data;
                    
                    sendServerReply(replyTo, ThreadIndex ,true);

                    
                    sendInt(replyToClient, ThreadIndexClient, monitorArrayIndex);
                    
                    return;
                    
                }
            }
            
            //wrong case
            printf("SetMVArray: not the space holder\n");
            
            sendServerReply(replyTo, ThreadIndex ,false);
            
            return;
            
            
        }
        else{
            
            sendServerReply(replyTo, ThreadIndex ,false);
            
            return;

            
            
        }
        
    }
    
    
}





void Server(){
    PacketHeader serverOutPktHdr;
    PacketHeader serverInPktHdr;
    MailHeader serverOutMailHdr;
    MailHeader serverInMailHdr;
    initialize();
    while(true){
        //printf("inside Server while loop\n");
        
        
        char receive[MaxMailSize];
        int replyTo;
        if(msgQ.size() == 0){
        postOffice->Receive(0, &serverInPktHdr, &serverInMailHdr, receive);
        //need delete
        printf("Server Gets the msg: \"%s\" from Packet Header: %d, Mail Header %d\n", receive, serverInPktHdr.from, serverInMailHdr.from);
        fflush(stdout);
        //handle the incoming msg
        
        replyTo = serverInPktHdr.from;
        }
        else{
            printf("therer is something in the waiting queue\n");
            replyTo = msgQ[0].fromPktHeader;
            serverInMailHdr.from = msgQ[0].fromMailHeader;
            strcpy(receive, msgQ[0].buffer);
            msgQ.pop_front();
            
        }
        
        std::stringstream ss;
        ss << receive;
        int syscall;
        ss >> syscall;
        if(syscall == 25){
            int lockIndex;
            ss >> lockIndex;
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
            }
            
            Acquire(lockIndex, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            //printf("Server: Acquire syscall\n");
            
        }
        else if(syscall == 26){
            int lockIndex;
            ss >> lockIndex;
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
            }
            
            
            Release(lockIndex, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            //printf("Server: Release syscall\n");
        }
        else if(syscall == 27){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            
            int replyToClient;
            int ThreadIndexClient;
            
            int conditionOrLock;
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                ss >> conditionOrLock;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                conditionOrLock = -1;
            }
            
            
            Wait(conditionIndex, lockIndex, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient, conditionOrLock);
            //printf("Server: Wait syscall\n");
        }
        else if(syscall == 28){
            int conditionIndex;
            ss >> conditionIndex;
            int lockIndex;
            ss >> lockIndex;
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
            }
            

            
            Signal(conditionIndex, lockIndex, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
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
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
            }
            
            CreateLock(lockName, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
           // printf("Server: CreateLock syscall\n");
        }
        else if(syscall == 31){
            int lockIndex;
            ss >> lockIndex;
            
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
            }
            
            DestroyLock(lockIndex, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
           // printf("Server: DestroyLock syscall\n");
            
        }
        else if(syscall == 32){
            char conditionName[100];
            ss >> conditionName;
            
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                ss >> replyToClient;
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
            }
            
            CreateCondition(conditionName, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
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
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
            
            ss >> replyToClient;
            
            ss >> ThreadIndexClient;
            
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                
            }
            
            CreateMV(monitorName, replyTo, serverInMailHdr.from, data, replyToClient, ThreadIndexClient);
           // printf("Server: CreateMonitor syscall\n");
        }
        else if(syscall == 35){
            int monitorIndex;
            ss >> monitorIndex;
            
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                
                ss >> replyToClient;
                
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                
            }
            
            GetMV(monitorIndex, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            //printf("Server: GetMV syscall\n");
        }
        else if(syscall == 36){
            int monitorIndex;
            int data;
            ss >> monitorIndex;
            ss >> data;
            
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                
                ss >> replyToClient;
                
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                
            }

            
            SetMV(monitorIndex, data, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            //printf("Server: SetMV syscall\n");
        }
        
        //array
        else if(syscall == 37){
            char monitorName[100];
            int length;
            ss >> monitorName;
            // printf("monitor name: %s \n", monitorName);
            ss >> length;
            // printf("monitor val: %d \n", data);
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                
                ss >> replyToClient;
                
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                
            }
            
            CreateMVArray(monitorName, length, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            // printf("Server: CreateMonitorArray syscall\n");
        }
        else if(syscall == 38){
            int monitorArrayIndex;
            ss >> monitorArrayIndex;
            int index;
            ss >> index;
            
            
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                
                ss >> replyToClient;
                
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                
            }
            
            GetMVArray(monitorArrayIndex, index, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            //printf("Server: GetMVArray syscall\n");
        }
        else if(syscall == 39){
            int monitorArrayIndex;
            int index;
            int data;
            
            ss >> monitorArrayIndex;
            ss >> index;
            ss >> data;
            
            int replyToClient;
            int ThreadIndexClient;
            
            if(replyTo < NumServers){
                
                ss >> replyToClient;
                
                ss >> ThreadIndexClient;
                
            }
            else{
                replyToClient = -1;
                ThreadIndexClient = -1;
                
            }
            
            
            SetMVArray(monitorArrayIndex, index, data, replyTo, serverInMailHdr.from, replyToClient, ThreadIndexClient);
            //printf("Server: SetMVArray syscall\n");
        }
        
        
        else if(syscall == 280){
            int lockIndex;
            int nextReplyTo;
            int nextThreadIndex;
            int replyToClient;
            int ThreadIndexClient;
            
            ss >> lockIndex;
            ss >> nextReplyTo;
            ss >> nextThreadIndex;
            ss >> replyToClient;
            ss >> ThreadIndexClient;
            
            processSignal(lockIndex, nextReplyTo, nextThreadIndex, replyToClient, ThreadIndexClient);
            
        }
        
        
        
    }
    
}

