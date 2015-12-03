#include "syscall.h"
#include "setup.h"

void main() {
    /* senator ID and SSN */
    int id;
    int ssn = id + 100;
    int data, count, bribeCount, lockData, cvData, money, status, senStatus, senData, has, rmCustomer, serviceId;
    unsigned int i;

    /* acquire all the necessary locks to get started */
    lockData = GetMVServer(customerWaitLock);
    AcquireServer(lockData);
    lockData = GetMVServer(senatorPictureWaitLock);
    AcquireServer(lockData);
    lockData = GetMVServer(senatorPassportWaitLock);
    AcquireServer(lockData);
    lockData = GetMVServer(senatorCashierWaitLock);
    AcquireServer(lockData);
    lockData = GetMVServer(senatorApplicationWaitLock);
    AcquireServer(lockData);
    lockData = GetMVServer(senatorWaitLock);
    AcquireServer(lockData);

    data = GetMVServer(senatorNum);
    id = data + 1;
    data++;
    SetMVServer(senatorNum, data);

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has came into passport office\n", sizeof("] has came into passport office\n"), ConsoleOutput);

    SetMVServer(hasSenator, 1);
    for (i = 0; i < CUSTOMER_SIZE; i++) {
        data = GetMVArrayServer(numCustomerWaitingArray, i);
        if (data != -1) {
            Write("Customer [", sizeof("Customer ["), ConsoleOutput);
            Printint(data);
            Write("] is going outside the Passport Office because their is a Senator present.\n", sizeof("] is going outside the Passport Office because their is a Senator present.\n"), ConsoleOutput);
        }
    }

    lockData = GetMVServer(senatorApplicationWaitLock);
    ReleaseServer(lockData);
    serviceId = GetMVServer(senatorServiceId);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for ApplicationClerk [", sizeof("] has gotten in regular line for ApplicationClerk ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    cvData = GetMVServer(senatorApplicationWaitCV);
    lockData = GetMVServer(senatorWaitLock);
    WaitServer(cvData, lockData);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to ApplicationClerk [", sizeof("] to ApplicationClerk ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    SetMVServer(senatorData, id);
    SignalServer(cvData, lockData);/* signal a clerk */

    WaitServer(cvData, lockData);/* wait for a filed application */


    lockData = GetMVServer(senatorPictureWaitLock);
    ReleaseServer(lockData);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for PictureClerk [", sizeof("] has gotten in regular line for PictureClerk ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    cvData = GetMVServer(senatorPictureWaitCV);
    lockData = GetMVServer(senatorWaitLock);
    WaitServer(cvData, lockData);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to PictureClerk [", sizeof("] to PictureClerk ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    SetMVServer(senatorData, id);

    SignalServer(cvData, lockData);/* signal a clerk */

    WaitServer(cvData, lockData);/* wait for a filed application */


    lockData = GetMVServer(senatorPassportWaitLock);
    ReleaseServer(lockData);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for PassportClerk [", sizeof("] has gotten in regular line for PassportClerk ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    cvData = GetMVServer(senatorPassportWaitCV);
    lockData = GetMVServer(senatorWaitLock);
    WaitServer(cvData, lockData);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to PassportClerk [", sizeof("] to PassportClerk ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    SetMVServer(senatorData, id);

    SignalServer(cvData, lockData);/* signal a clerk */

    WaitServer(cvData, lockData);/* wait for a filed application */


    lockData = GetMVServer(senatorCashierWaitLock);
    ReleaseServer(lockData);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for Cashier [", sizeof("] has gotten in regular line for Cashier ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    cvData = GetMVServer(senatorCashierWaitCV);
    lockData = GetMVServer(senatorWaitLock);
    WaitServer(cvData, lockData);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to Cashier [", sizeof("] to Cashier ["), ConsoleOutput);
    Printint(serviceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    SetMVServer(senatorData, id);

    SignalServer(cvData, lockData);/* signal a clerk */

    WaitServer(cvData, lockData);/* wait for a filed application */


    SetMVServer(hasSenator, 0);
    SetMVServer(senatorStatus, 0);
    Write("Senator[", sizeof("Senator["), ConsoleOutput);
    Printint(id);
    Write("] is leaving the Passport Office\n", sizeof("] is leaving the Passport Office\n"), ConsoleOutput); /* senator is leaving the passport office */
    
    lockData = GetMVServer(customerWaitLock);
    ReleaseServer(lockData);
    
    lockData = GetMVServer(senatorWaitLock);
    ReleaseServer(lockData);

    Exit(0);
}