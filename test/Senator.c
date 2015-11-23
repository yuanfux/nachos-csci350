void Senator() {
    /* senator ID and SSN */
    int id = senatorNum + 1;
    int ssn = id + 100;
    unsigned int i;

    /* acquire all the necessary locks to get started */
    Acquire(customerWaitLock);
    Acquire(senatorPictureWaitLock);
    Acquire(senatorPassportWaitLock);
    Acquire(senatorCashierWaitLock);
    Acquire(senatorApplicationWaitLock);
    Acquire(senatorWaitLock);
    senatorNum++;
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has came into passport office\n", sizeof("] has came into passport office\n"), ConsoleOutput);

    hasSenator = 1;
    for (i = 0; i < CUSTOMER_SIZE; i++) {
        if (numCustomerWaiting[i] != -1) {
            Write("Customer [", sizeof("Customer ["), ConsoleOutput);
            Printint(numCustomerWaiting[i]);
            Write("] is going outside the Passport Office because their is a Senator present.\n", sizeof("] is going outside the Passport Office because their is a Senator present.\n"), ConsoleOutput);
        }
    }

    Release(senatorApplicationWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for ApplicationClerk [", sizeof("] has gotten in regular line for ApplicationClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorApplicationWaitCV, senatorWaitLock);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to ApplicationClerk [", sizeof("] to ApplicationClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorApplicationWaitCV, senatorWaitLock);/* signal a clerk */

    Wait(senatorApplicationWaitCV, senatorWaitLock);/* wait for a filed application */


    Release(senatorPictureWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for PictureClerk [", sizeof("] has gotten in regular line for PictureClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorPictureWaitCV, senatorWaitLock);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to PictureClerk [", sizeof("] to PictureClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorPictureWaitCV, senatorWaitLock);/* signal a clerk */
    Wait(senatorPictureWaitCV, senatorWaitLock);/* wait for a filed application */


    Release(senatorPassportWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for PassportClerk [", sizeof("] has gotten in regular line for PassportClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorPassportWaitCV, senatorWaitLock);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to PassportClerk [", sizeof("] to PassportClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorPassportWaitCV, senatorWaitLock);/* signal a clerk */
    Wait(senatorPassportWaitCV, senatorWaitLock);/* wait for a filed application */


    Release(senatorCashierWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for Cashier [", sizeof("] has gotten in regular line for Cashier ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorCashierWaitCV, senatorWaitLock);/* wait for a clerk */

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to Cashier [", sizeof("] to Cashier ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorCashierWaitCV, senatorWaitLock);/* signal a clerk */
    Wait(senatorCashierWaitCV, senatorWaitLock);/* wait for a filed application */


    hasSenator = 0;
    senatorStatus = 0;
    Write("Senator[", sizeof("Senator["), ConsoleOutput);
    Printint(id);
    Write("] is leaving the Passport Office\n", sizeof("] is leaving the Passport Office\n"), ConsoleOutput); /* senator is leaving the passport office */
    Release(customerWaitLock);
    Release(senatorWaitLock);

    Exit(0);
}