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
    printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);
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
    printf("Got \"%s\" from %d, box %d\n", buffer, inPktHdr.from, inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

#define NUM_LOCK 1000
#define NUM_CONDITION 1000
#define NUM_MONITOR 1000


struct ServerLock {
    char* name;
    List* queue;
    int lockHolder;

    ServerLock() {
        name = NULL;
        queue = new List;
        lockHolder = -1;
    }

    ServerLock(char* lockName) {
        name = lockName;
        queue = new List;
        lockHolder = -1;
    }


};

struct ServerCondition {
    char* name;
    int waitingLock;
    List* queue;

    ServerCondition() {
        name = NULL;
        waitingLock = -1;
        queue = new List;
    }


    ServerCondition(char* conditionName) {
        name = conditionName;
        waitingLock = -1;
        queue = new List;
    }

};

struct ServerMonitorVariable {
    char* name;
    int data;

    ServerMonitorVariable() {
        name = NULL;
    }

    ServerMonitorVariable(char* monitorName) {
        name = monitorName;
    }


};


ServerLock serverLock[NUM_LOCK];

ServerCondition serverCondition[NUM_CONDITION];

ServerMonitorVariable serverMonitorVariable[NUM_MONITOR];

int numLock = 0;
int numCondition = 0;
int numMonitor = 0;

void sendInt(int replyTo, int value) {

    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = replyTo;
    outPktHdr.from = 0;
    outMailHdr.to = 0;
    outMailHdr.from = 0;

    char* send = new char[20];
    sprintf(send, "%d", numLock);
    outMailHdr.length = strlen(send) + 1;

    if (postOffice->Send(outPktHdr, outMailHdr, send) == false) {
        printf("Send failed from Sever CreateLock\n");
        return;
    }

}


void CreateLock(char* name, int replyTo) {

    serverLock[numLock].name = name;

    sendInt(replyTo, numLock);

    numLock++;

    return;
}

void Acquire(int lockIndex, int replyTo) {
    if (lockIndex >= numLock || lockIndex < 0) {
        printf("Acquire: out of Lock boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (serverLock[lockIndex].lockHolder == replyTo) {
        printf("Acquire: already the lock holder\n");
        sendInt(replyTo, -1);
        return;
    }
    else if (serverLock[lockIndex].lockHolder == -1) {
        serverLock[lockIndex].lockHolder = replyTo;
        sendInt(replyTo, lockIndex);
        return;
    }
    else {
        serverLock[lockIndex].queue->Append((void*)replyTo);
        return;
    }
}

void Release(int lockIndex, int replyTo) {
    if (lockIndex >= numLock || lockIndex < 0) {
        printf("Release: out of Lock boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (serverLock[lockIndex].lockHolder != replyTo) {
        printf("Release: not the lock holder\n");
        sendInt(replyTo, -1);
        return;
    }
    else {
        sendInt(replyTo, lockIndex);
        int nextReply = (int)serverLock[lockIndex].queue->Remove();
        serverLock[lockIndex].lockHolder = nextReply;
        sendInt(nextReply, lockIndex);
        return;
    }

}

void DestroyLock(int lockIndex, int replyTo) {
    if (lockIndex >= numLock || lockIndex < 0) {
        printf("DestroyLock: out of Lock boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    serverLock[lockIndex].name = NULL;
    serverLock[lockIndex].lockHolder = NULL;

    numLock--;

    sendInt(replyTo, lockIndex);

    return;
}

void CreateCondition(char* name, int replyTo) {

    serverCondition[numCondition].name = name;

    sendInt(replyTo, numCondition);

    numCondition++;

    return;

}

void Wait(int conditionIndex, int lockIndex, int replyTo) {

    if (lockIndex >= numLock || lockIndex < 0) {
        printf("Wait: out of Lock boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (conditionIndex >= numCondition || conditionIndex < 0) {
        printf("Wait: out of Condition boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (serverLock[lockIndex].lockHolder != replyTo) {
        printf("Wait: not the lock holder\n");
        sendInt(replyTo, -1);
        return;
    }

    if (serverCondition[conditionIndex].waitingLock == -1) {
        serverCondition[conditionIndex].waitingLock = lockIndex;
    }
    else if (serverCondition[conditionIndex].waitingLock != lockIndex) {
        printf("Wait: wrong condition to wait\n");
        sendInt(replyTo, -1);
        return;
    }

    serverCondition[conditionIndex].queue->Append((void*)replyTo);

    //no one is waiting
    if (serverLock[lockIndex].queue->IsEmpty()) {
        serverLock[lockIndex].lockHolder = -1;
        return;
    }
    else {
        int nextReply = (int)serverLock[lockIndex].queue->Remove();
        serverLock[lockIndex].lockHolder = nextReply;
        sendInt(nextReply, lockIndex);
        return;
    }

}

void Signal(int conditionIndex, int lockIndex, int replyTo) {

    if (lockIndex >= numLock || lockIndex < 0) {
        printf("Signal: out of Lock boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (conditionIndex >= numCondition || conditionIndex < 0) {
        printf("Signal: out of Condition boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (serverLock[lockIndex].lockHolder != replyTo) {
        printf("Signal: not the lock holder\n");
        sendInt(replyTo, -1);
        return;
    }
    if (serverCondition[conditionIndex].waitingLock != lockIndex) {
        printf("Signal: wrong condition to signal\n");
        sendInt(replyTo, -1);
        return;
    }
    if (serverLock[lockIndex].queue->IsEmpty()) {
        printf("Signal: nothing to signal\n");
        sendInt(replyTo, -1);
        return;
    }
    //?
    int nextReply = (int)serverCondition[conditionIndex].queue->Remove();
    if (serverCondition[conditionIndex].queue->IsEmpty()) {
        serverCondition[conditionIndex].waitingLock = -1;
    }
    if (serverLock[lockIndex].lockHolder == -1) {
        serverLock[lockIndex].lockHolder = nextReply;

        // SEND MESSAGE TO nextWaiting
        sendInt(nextReply, lockIndex);

    }
    else {
        serverLock[lockIndex].queue->Append((void*)nextReply);
    }

    sendInt(replyTo, lockIndex);
}

void Broadcast(int conditionIndex, int lockIndex, int replyTo) {
    if (lockIndex >= numLock || lockIndex < 0) {
        printf("Broadcast: out of Lock boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (conditionIndex >= numCondition || conditionIndex < 0) {
        printf("Broadcast: out of Condition boundary\n");
        sendInt(replyTo, -1);
        return;
    }

    if (serverLock[lockIndex].lockHolder != replyTo) {
        printf("Broadcast: not the lock holder\n");
        sendInt(replyTo, -1);
        return;
    }
    if (serverCondition[conditionIndex].waitingLock != lockIndex) {
        printf("Broadcast: wrong condition to signal\n");
        sendInt(replyTo, -1);
        return;
    }
    if (serverLock[lockIndex].queue->IsEmpty()) {
        printf("Broadcast: nothing to signal\n");
        sendInt(replyTo, -1);
        return;
    }
    //?
    while (!serverCondition[conditionIndex].queue->IsEmpty()) {

        int nextReply = (int)serverCondition[conditionIndex].queue->Remove();

        if (serverLock[lockIndex].lockHolder == -1) {
            serverLock[lockIndex].lockHolder = nextReply;

            // SEND MESSAGE TO nextWaiting
            sendInt(replyTo, lockIndex);
        }
        else {
            serverLock[lockIndex].queue->Append((void*)nextReply);
        }
    }

    // SEND MESSAGE BACK TO machineID
    sendInt(replyTo, 0);
    serverCondition[conditionIndex].waitingLock = -1;
}

void DestroyCondition(int conditionIndex, int replyTo) {
    if (conditionIndex >= numCondition || conditionIndex < 0) {
        printf("DestroyCondition: out of Condition boundary\n");
        sendInt(replyTo, -1);
        return;
    }
    serverCondition[conditionIndex].name = NULL;
    serverCondition[conditionIndex].waitingLock = -1;
    serverCondition[conditionIndex].queue = new List;
    numCondition--;

    sendInt(replyTo, conditionIndex);

    return;
}

void CreateMV(char* name, int replyTo) {

    serverMonitorVariable[numMonitor].name = name;

    sendInt(replyTo, numMonitor);

    numMonitor++;

    return;

}

void GetMV(int monitorIndex, int replyTo) {
    if (monitorIndex >= numMonitor || monitorIndex < 0 ) {
        printf("GetMV: out of boundary");

        sendInt(replyTo, -1);
    }

    sendInt(replyTo, serverMonitorVariable[monitorIndex].data);

    return;

}

void SetMV(int monitorIndex, int value, int replyTo) {
    if (monitorIndex >= numMonitor || monitorIndex < 0 ) {
        printf("GetMV: out of boundary");

        sendInt(replyTo, -1);
    }

    serverMonitorVariable[monitorIndex].data = value;

    sendInt(replyTo, monitorIndex);

}


