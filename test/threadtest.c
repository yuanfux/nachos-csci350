#include "syscall.h"

#define APPLICATIONCLERK_SIZE 1
#define PICTURECLERK_SIZE 1
#define PASSPORTCLERK_SIZE 1
#define CASHIER_SIZE 1
#define CUSTOMER_SIZE 1
#define SENATOR_SIZE 0
#define MANAGER_SIZE 1

#define RAND_UPPER_LIMIT 10000


typedef enum {AVAILABLE, BUSY, ONBREAK} clerkState;

int ClerkLineLock;
int incrementCount;
int customerApplicationStatus[CUSTOMER_SIZE];

/* variables for application clerk */
int ApplicationClerkLineLock[APPLICATIONCLERK_SIZE];
int ApplicationClerkLineCV[APPLICATIONCLERK_SIZE]; /* cv for each line */
int ApplicationClerkLineWaitCV[APPLICATIONCLERK_SIZE];/* need new cv to prevent the different lock case */
int ApplicationClerkBribeLineCV[APPLICATIONCLERK_SIZE];
int ApplicationClerkBribeLineWaitCV[APPLICATIONCLERK_SIZE];/* need new cv to prevent the different lock case */
int ApplicationClerkLineCount[APPLICATIONCLERK_SIZE]; /* number of customers in each line */
int ApplicationClerkBribeLineCount[APPLICATIONCLERK_SIZE];
int ApplicationClerkState[APPLICATIONCLERK_SIZE];
int ApplicationClerkData[APPLICATIONCLERK_SIZE]; /* stores ssn's of customers for each clerk */

/* variables for picture clerks. */
int pictureClerkLineLock[PICTURECLERK_SIZE];
int pictureClerkLineCV[PICTURECLERK_SIZE];
int pictureClerkLineWaitCV[PICTURECLERK_SIZE];/* need new cv to prevent the different lock case */
int pictureClerkBribeLineCV[PICTURECLERK_SIZE];
int pictureClerkBribeLineWaitCV[PICTURECLERK_SIZE];/* need new cv to prevent the different lock case */
int pictureClerkLineCount[PICTURECLERK_SIZE];
int pictureClerkBribeLineCount[PICTURECLERK_SIZE];
int pictureClerkState[PICTURECLERK_SIZE];
int pictureClerkData[PICTURECLERK_SIZE];
int pictureAcceptance[PICTURECLERK_SIZE]; /* stores whether customer likes picture or not at counter */

/* variables for PassportClerk */
int passportClerkCustomerId[PASSPORTCLERK_SIZE]; /* stores ssn of customers for each clerk */
int passportClerkState[PASSPORTCLERK_SIZE];
int passportClerkLineLock[PASSPORTCLERK_SIZE];
int passportClerkLineCV[PASSPORTCLERK_SIZE];
int passportClerkLineWaitCV[PASSPORTCLERK_SIZE];/* need new cv to prevent the different lock case */
int passportClerkBribeLineCV[PASSPORTCLERK_SIZE];
int passportClerkBribeLineWaitCV[PASSPORTCLERK_SIZE];/* need new cv to prevent the different lock case */
int passportClerkLineCount[PASSPORTCLERK_SIZE];
int passportClerkBribeLineCount[PASSPORTCLERK_SIZE];

/* variables for Cashier */
int CashierCustomerId[CASHIER_SIZE];
int CashierState[CASHIER_SIZE];
int CashierLineLock[CASHIER_SIZE];
int CashierLineCV[CASHIER_SIZE];
int CashierLineWaitCV[CASHIER_SIZE];/* need new cv to prevent the different lock case */
int CashierLineCount[CASHIER_SIZE];

/* variables for Manager */
/* each money variable needs a lock */
int applicationMoneyLock;
int pictureMoneyLock;
int passportMoneyLock;
int cashierMoneyLock;
int MoneyFromApplicationClerk = 0;
int MoneyFromPictureClerk = 0;
int MoneyFromPassportClerk = 0;
int MoneyFromCashier = 0;
int MoneyTotal = 0;

int senatorWaitLock;
int senatorApplicationWaitLock;
int senatorPictureWaitLock;
int senatorPassportWaitLock;
int senatorCashierWaitLock;
int senatorApplicationWaitCV;
int senatorPictureWaitCV;
int senatorPassportWaitCV;
int senatorCashierWaitCV;
int customerWaitCV;
int customerWaitLock;
int senatorStatus = 0;

int numCustomerWaiting[CUSTOMER_SIZE];

int senatorData;
int senatorServiceId;
int hasSenator = 0;


int isFirst = 1;
int customerNum = -1; /*  number of customers came into the office. */
/*  it is also ssn of customers */
int remainingCustomer = 0; /*  number of customers still in the office */


int senatorNum = -1;
int randomNum;

void Customer() {
    /* get ssn for each customer */
    int id;
    unsigned int i;
    int money;
    int choseClerk;

    Acquire(incrementCount);
    id = customerNum + 1;
    customerNum++;
    Release(incrementCount);

    /* determine amount of money customer has */
    randomNum = Random(RAND_UPPER_LIMIT) % 4;

    if (randomNum == 0) {
        money = 100;
    } else if (randomNum == 1) {
        money = 600;
    } else if (randomNum == 2) {
        money = 1100;
    } else if (randomNum == 3) {
        money = 1600;
    }
    Write("Customer[", sizeof("Customer["), ConsoleOutput);
    Printint(id);
    Write("] has $", sizeof("] has $"), ConsoleOutput);
    Printint(money);
    Write("]\n", sizeof("]\n"), ConsoleOutput);

    /* each customer needs to go through all the counters before leaving */
    numCustomerWaiting[id] = id;
    while (customerApplicationStatus[id] != 10) {
        Acquire(customerWaitLock);

        choseClerk = Random(RAND_UPPER_LIMIT) % 2; /* randomly choosing application or picture clerk */

        /* Goes to Picture Clerk. */
        if ((customerApplicationStatus[id] == 1) || (customerApplicationStatus[id] == 0 && choseClerk == 0)) { /* has finished applicaiton clerk */
            int myLine;
            int shortestPictureBribeLine;
            int shortestPictureBribeLineSize;
            Acquire(ClerkLineLock);
            if (money > 500) { /* can bribe */
                money -= 500; /*  give out money */
                shortestPictureBribeLine = -1;
                shortestPictureBribeLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < PICTURECLERK_SIZE; i++) {
                    if (pictureClerkBribeLineCount[i] < shortestPictureBribeLineSize) {

                        shortestPictureBribeLine = i;
                        shortestPictureBribeLineSize = pictureClerkBribeLineCount[i];

                    }
                }
                myLine = shortestPictureBribeLine;

                /* wait in the picture clerk line */
                pictureClerkBribeLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for PictureClerk[", sizeof("] has gotten in bribe line for PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                Wait(pictureClerkBribeLineWaitCV[myLine], ClerkLineLock); /* wait for signal from clerk */
                pictureClerkBribeLineCount[myLine]--; /* leave the line and go to the counter */
                Release(ClerkLineLock);

                Acquire(pictureClerkLineLock[myLine]);
                pictureClerkData[myLine] = id; /* gives clerk the ssn */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PictureClerk[", sizeof("] to PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                /* wait for picture clerk to take picture */
                Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                pictureAcceptance[myLine] = Random(RAND_UPPER_LIMIT) % 10; /*  customer decide whether to accept the picture */
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                /* wait for picture clerk to tell customer leave or not */
                Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                Release(pictureClerkLineLock[myLine]);

            } else {
                int myLine;
                int shortestPictureLine = -1;
                int shortestPictureLineSize = 10000;

                for (i = 0; i < PICTURECLERK_SIZE; i++) {
                    if (pictureClerkLineCount[i] < shortestPictureLineSize) {

                        shortestPictureLine = i;
                        shortestPictureLineSize = pictureClerkLineCount[i];

                    }

                }

                myLine = shortestPictureLine;

                /* wait in the picture clerk line */
                pictureClerkLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for PictureClerk[", sizeof("] has gotten in regular line for PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                Wait(pictureClerkLineWaitCV[myLine], ClerkLineLock); /* wait for signal from clerk */
                pictureClerkLineCount[myLine]--;
                Release(ClerkLineLock);

                Acquire(pictureClerkLineLock[myLine]);
                pictureClerkData[myLine] = id; /* gives clerk the ssn */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PictureClerk[", sizeof("] to PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                /* wait for picture clerk to take picture */
                Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                pictureAcceptance[myLine] = Random(RAND_UPPER_LIMIT) % 10; /*  customer decide whether receive the picture */
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                /* wait for picture clerk to tell customer leave or not */
                Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                Release(pictureClerkLineLock[myLine]);


            }

        }

        /* Goes to Application Clerk */
        else if ((customerApplicationStatus[id] == 2) || (customerApplicationStatus[id] == 0 && choseClerk == 1)) { /* has finished picture clerk */

            int myLine;
            int shortestApplicationBribeLine = -1;
            int shortestApplicationBribeLineSize = 10000;
            Acquire(ClerkLineLock);
            if (money > 500) { /* has bribe money */
                money -= 500;
                shortestApplicationBribeLine = -1;
                shortestApplicationBribeLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {
                    if (ApplicationClerkBribeLineCount[i] < shortestApplicationBribeLineSize) {
                        shortestApplicationBribeLine = i;
                        shortestApplicationBribeLineSize = ApplicationClerkBribeLineCount[i];
                    }
                }
                myLine = shortestApplicationBribeLine;

                /* wait in the application clerk line */
                ApplicationClerkBribeLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for ApplicationClerk[", sizeof("] has gotten in bribe line for ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                /* wait to be signalled by clerk */
                Wait(ApplicationClerkBribeLineWaitCV[myLine], ClerkLineLock);
                ApplicationClerkBribeLineCount[myLine]--;
                Release(ClerkLineLock);

                Acquire(ApplicationClerkLineLock[myLine]);
                ApplicationClerkData[myLine] = id; /* give ssn to clerk */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to ApplicationClerk[", sizeof("] to ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);

                /* wait for clerk to do the job */
                Wait(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);

                Release(ApplicationClerkLineLock[myLine]);

            } else { /* does not have bribe money */
                int myLine;
                int shortestApplicationLine = -1;
                int shortestApplicationLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {
                    if (ApplicationClerkLineCount[i] < shortestApplicationLineSize) {
                        shortestApplicationLine = i;
                        shortestApplicationLineSize = ApplicationClerkLineCount[i];
                    }
                }

                myLine = shortestApplicationLine;

                /* wait in the application clerk line */
                ApplicationClerkLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for ApplicationClerk[", sizeof("] has gotten in regular line for ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                /* wait to be signalled by clerk */
                Wait(ApplicationClerkLineWaitCV[myLine], ClerkLineLock);
                ApplicationClerkLineCount[myLine]--;
                Release(ClerkLineLock);

                Acquire(ApplicationClerkLineLock[myLine]);
                ApplicationClerkData[myLine] = id; /* give ssn to clerk */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to ApplicationClerk[", sizeof("] to ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);

                /* wait for clerk to do the job */
                Wait(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);

                Release(ApplicationClerkLineLock[myLine]);

            }

        }

        /* Goes to Passport Clerk */
        else if (customerApplicationStatus[id] == 3) {

            int myLine;
            int shortestPassportBribeLine;
            int shortestPassportBribeLineSize;
            Acquire(ClerkLineLock);

            if (money > 500) { /* has bribe money */
                money -= 500;
                shortestPassportBribeLine = -1;
                shortestPassportBribeLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < PASSPORTCLERK_SIZE; i++) {
                    if (passportClerkBribeLineCount[i] < shortestPassportBribeLineSize) {
                        shortestPassportBribeLine = i;
                        shortestPassportBribeLineSize = passportClerkBribeLineCount[i];
                    }
                }
                myLine = shortestPassportBribeLine;

                /* wait in the passport clerk line */
                passportClerkBribeLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for PassportClerk[", sizeof("] has gotten in bribe line for PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                /* /wait to get signalled by passport clerk */
                Wait(passportClerkBribeLineWaitCV[myLine], ClerkLineLock);
                passportClerkBribeLineCount[myLine]--;
                Release(ClerkLineLock);

                /* give ssn to passport clerk */
                Acquire(passportClerkLineLock[myLine]);
                passportClerkCustomerId[myLine] = id;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PassportClerk[", sizeof("] to PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);

                /* wait for clerk to do the job */
                Wait(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);


                Release(passportClerkLineLock[myLine]);

            } else { /* does not have bribe money */
                int myLine;
                int shortestPassportLine = -1;
                int shortestPassportLineSize = 10000;

                for (i = 0; i < PASSPORTCLERK_SIZE; i++) {
                    if (passportClerkLineCount[i] < shortestPassportLineSize) {

                        shortestPassportLine = i;
                        shortestPassportLineSize = passportClerkLineCount[i];

                    }
                }

                myLine = shortestPassportLine;

                passportClerkLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for PassportClerk[", sizeof("] has gotten in regular line for PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                /* wait to get signalled by passport clerk */
                Wait(passportClerkLineWaitCV[myLine], ClerkLineLock);
                passportClerkLineCount[myLine]--;
                Release(ClerkLineLock);

                /* give ssn to passport clerk */
                Acquire(passportClerkLineLock[myLine]);
                passportClerkCustomerId[myLine] = id;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PassportClerk[", sizeof("] to PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);

                /* wait for clerk to do the job */
                Wait(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);

                Release(passportClerkLineLock[myLine]);

            }



        }

        /* Goes to Cashier counter */
        else if (customerApplicationStatus[id] == 6) {

            int myLine;
            int shortestCashierLine;
            int shortestCashierLineSize;
            Acquire(ClerkLineLock);

            shortestCashierLine = -1;
            shortestCashierLineSize = 10000;

            /* find shortest line */
            for (i = 0; i < CASHIER_SIZE; i++) {
                if (CashierLineCount[i] < shortestCashierLineSize) {
                    shortestCashierLine = i;
                    shortestCashierLineSize = passportClerkLineCount[i];
                }
            }
            myLine = shortestCashierLine;

            /* get into cashier line */
            CashierLineCount[myLine]++;
            Write("Customer[", sizeof("Customer["), ConsoleOutput);
            Printint(id);
            Write("] has gotten in regular line for Cashier[", sizeof("] has gotten in regular line for Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Release(customerWaitLock);
            Wait(CashierLineWaitCV[myLine], ClerkLineLock);
            CashierLineCount[myLine]--;
            Release(ClerkLineLock);

            /* give cashier ssn */
            Acquire(CashierLineLock[myLine]);
            CashierCustomerId[myLine] = id;
            Write("Customer[", sizeof("Customer["), ConsoleOutput);
            Printint(id);
            Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
            Printint(id);
            Write("] to Cashier[", sizeof("] to Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Signal(CashierLineCV[myLine], CashierLineLock[myLine]);

            /* wait for cashier to do the job */
            Wait(CashierLineCV[myLine], CashierLineLock[myLine]);
            Release(CashierLineLock[myLine]);

        }

    }

    Acquire(incrementCount);
    remainingCustomer--;

    for (i = 0; i < CUSTOMER_SIZE; i++) {
        if (numCustomerWaiting[i] == id) {

            numCustomerWaiting[i] = -1;

        }
    }
    Release(incrementCount);

    Exit(0);

}

void ApplicationClerk(int myLine) {
    int id = 0;
    int printed = 0;
    unsigned int i;

    while (1) {
        int InBribeLine = 0;

        if (ApplicationClerkState[myLine] == ONBREAK && !printed) {
            Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (ApplicationClerkState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator && (myLine == 0) && senatorStatus == 0) { /* if there is a senator present and i am the index 0 clerk */

            if (ApplicationClerkState[myLine] == ONBREAK) {
                ApplicationClerkState[myLine] = BUSY;
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            Acquire(senatorApplicationWaitLock);
            Acquire(senatorWaitLock);

            senatorServiceId = myLine;
            Signal(senatorApplicationWaitCV, senatorWaitLock);
            Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            Wait(senatorApplicationWaitCV, senatorWaitLock);
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senatorData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            senatorStatus++;
            Signal(senatorApplicationWaitCV, senatorWaitLock);

            Release(senatorApplicationWaitLock);
            Release(senatorWaitLock);
        } else if (hasSenator && myLine != 0) { /* if there is a senator present and i am not the index 0 clerk. Put myself on break */
            ApplicationClerkState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);/* acquire the line lock in case of line size change */

        if (ApplicationClerkState[myLine] != ONBREAK && !hasSenator) { /* no senator, not on break, deal with normal customers */
            if (ApplicationClerkBribeLineCount[myLine] > 0) { /* bribe line customer first */
                Signal(ApplicationClerkBribeLineWaitCV[myLine], ClerkLineLock);
                Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                ApplicationClerkState[myLine] = BUSY;
                InBribeLine = 1;
            } else if (ApplicationClerkLineCount[myLine] > 0) { /* regular line customer next */
                Signal(ApplicationClerkLineWaitCV[myLine], ClerkLineLock);
                Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                ApplicationClerkState[myLine] = BUSY;

            } else { /* no customer present */
                ApplicationClerkState[myLine] = ONBREAK;
                Release(ClerkLineLock);
                Yield();/* context switch */
                if (remainingCustomer == 0) break;
                continue;
            }
        } else { /* if there is no customers, put myself on break */
            Release(ClerkLineLock);
            Yield();/* context switch */
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(ApplicationClerkLineLock[myLine]);
        Release(ClerkLineLock);
        /* wait for customer data */

        if (InBribeLine) { /* in bribe line */

            Wait(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);
            id = ApplicationClerkData[myLine];

            /* Collect Bribe Money From Customer */
            Acquire(applicationMoneyLock);
            MoneyFromApplicationClerk += 500;
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Release(applicationMoneyLock);

            /* do my job customer now waiting */
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            for ( i = 0; i < 20; i++) {
                Yield();
            }
            customerApplicationStatus[id]++;
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Customer[", sizeof("] has recorded a completed application for Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);
        } else { /* not in bribe line */
            Wait(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);
            /* do my job customer now waiting */
            id = ApplicationClerkData[myLine];

            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            for ( i = 0; i < 20; i++) {
                Yield();
            }

            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Customer[", sizeof("] has recorded a completed application for Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);

        }

        Release(ApplicationClerkLineLock[myLine]);

        if (remainingCustomer == 0) break;
    }/* while */

    Exit(0);
}

void PictureClerk(int myLine) {
    int id = 0;
    int printed = 0;
    unsigned int i;
    int photoAcceptance;
    int numCalls;
    while (1) {

        int inBribeLine = 0;

        if (pictureClerkState[myLine] == ONBREAK && !printed) {
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (pictureClerkState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator && (myLine == 0) && senatorStatus <= 1) { /* if there is a senator present and i am the index 0 clerk */

            if (pictureClerkState[myLine] == ONBREAK) {
                pictureClerkState[myLine] = BUSY;
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            Acquire(senatorPictureWaitLock);
            Acquire(senatorWaitLock);

            senatorServiceId = myLine;
            Signal(senatorPictureWaitCV, senatorWaitLock);
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            Wait(senatorPictureWaitCV, senatorWaitLock);
            Write("PictureClerk[", sizeof("PictureClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senatorData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            while (photoAcceptance <= 5) {
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has taken a picture of Senator[", sizeof("] has taken a picture of Senator["), ConsoleOutput);
                Printint(senatorData);
                Write("]\n", sizeof("]\n"), ConsoleOutput);

                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senatorData);
                Write("] does not like their picture from PictureClerk [", sizeof("] does not like their picture from PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("].\n", sizeof("].\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }

            Write("Senator [", sizeof("Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("] does like their picture from PictureClerk [", sizeof("] does like their picture from PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("].\n", sizeof("].\n"), ConsoleOutput);
            Write("PictureClerk[", sizeof("PictureClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            senatorStatus += 2;
            Signal(senatorPictureWaitCV, senatorWaitLock);
            Release(senatorWaitLock);
            Release(senatorPictureWaitLock);
        } else if (hasSenator && myLine != 0) { /* if there is a senator present and i am not the index 0 clerk. Put myself on break */
            pictureClerkState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);/* acquire the line lock in case of line size change */
        if (pictureClerkState[myLine] != ONBREAK && !hasSenator) { /* no senator, not on break, deal with normal customers */

            if (pictureClerkBribeLineCount[myLine] > 0) { /* bribe line customer first */
                Signal(pictureClerkBribeLineWaitCV[myLine], ClerkLineLock);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                pictureClerkState[myLine] = BUSY;
                inBribeLine = 1;
            } else if (pictureClerkLineCount[myLine] > 0) { /* regular line next */
                Signal(pictureClerkLineWaitCV[myLine], ClerkLineLock);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                pictureClerkState[myLine] = BUSY;
            } else { /* no customer present */
                pictureClerkState[myLine] = ONBREAK;
                Release(ClerkLineLock);
                Yield();/* context switch */
                if (remainingCustomer == 0) break;
                continue;
            }
        } else { /* if there is no customers, put myself on break */
            Release(ClerkLineLock);
            Yield();/* context switch */
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(pictureClerkLineLock[myLine]);/* acquire the clerk lock to serve a customer */
        Release(ClerkLineLock);
        /* if in bribe line */
        if (inBribeLine) {
            /* customer service starts */
            Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);
            id = pictureClerkData[myLine];

            /* Collect Bribe Money From Customer */
            Acquire(pictureMoneyLock);
            MoneyFromPictureClerk += 500;
            Write("PictureClerk[", sizeof("PictureClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Release(pictureMoneyLock);

            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has taken a picture of Customer[", sizeof("] has taken a picture of Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);
            Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

            if (pictureAcceptance[myLine] > 2) { /* if customer likes the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does like their picture\n", sizeof("] does like their picture\n"), ConsoleOutput);

                numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;/* delay for clerk to complete the service */
                for ( i = 0; i < numCalls; i++) {
                    Yield();
                }

                customerApplicationStatus[id] += 2;
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

            } else { /* if customer does not like the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not like their picture\n", sizeof("] does not like their picture\n"), ConsoleOutput);
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);
            }

        }
        /* if in regular line */
        else {
            /* customer service starts */
            Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);
            id = pictureClerkData[myLine];

            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has taken a picture of Customer[", sizeof("] has taken a picture of Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);
            Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

            if (pictureAcceptance[myLine] > 2) { /* if customer likes the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does like their picture\n", sizeof("] does like their picture\n"), ConsoleOutput);

                numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (i = 0; i < numCalls; i++) {
                    Yield();
                }

                customerApplicationStatus[id] += 2;
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

            } else { /* if customer does not like the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not like their picture\n", sizeof("] does not like their picture\n"), ConsoleOutput);
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);
            }

        }
        /* customer service ends */
        Release(pictureClerkLineLock[myLine]);

        if (remainingCustomer == 0) break;
    }

    Exit(0);
}

void PassportClerk(int myLine) {
    int id = 0;
    int i;
    int printed = 0;
    int photoAcceptance;
    int numCalls;
    int passportClerkPunishment;
    while (1) {

        int inBribeLine = 0;

        if (passportClerkState[myLine] == ONBREAK && !printed) {
            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (passportClerkState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator && (myLine == 0) && senatorStatus <= 3) { /* if there is a senator present and I am the index 0 clerk */

            if (passportClerkState[myLine] == ONBREAK) {
                passportClerkState[myLine] = BUSY;
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            Acquire(senatorPassportWaitLock);
            Acquire(senatorWaitLock);

            senatorServiceId = myLine;
            Signal(senatorPassportWaitCV, senatorWaitLock);
            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            Wait(senatorPassportWaitCV, senatorWaitLock);
            Write("PassportClerk[", sizeof("PassportClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senatorData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            while (photoAcceptance <= 5) { /* randomness to make senator go back of the line */
                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senatorData);
                Write("] has gone to PassportClerk [", sizeof("] has gone to PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] too soon. They are going to the back of the line.\n", sizeof("] too soon. They are going to the back of the line.\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }

            Write("PassportClerk[", sizeof("PassportClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            senatorStatus += 3;
            Signal(senatorPassportWaitCV, senatorWaitLock);
            Release(senatorWaitLock);
            Release(senatorPassportWaitLock);
        } else if (hasSenator && myLine != 0) { /* if there is no senator present and I am not the index 0 clerk. Put myself on break */
            passportClerkState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);

        if (passportClerkState[myLine] != ONBREAK && !hasSenator) { /* if there is no senator present and I am not on break, deal with the normal customers */
            if (passportClerkBribeLineCount[myLine] > 0) { /* bribe line first */
                Signal(passportClerkBribeLineWaitCV[myLine], ClerkLineLock);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                passportClerkState[myLine] = BUSY;
                inBribeLine = 1;
            } else if (passportClerkLineCount[myLine] > 0) { /* regular line next */
                Signal(passportClerkLineWaitCV[myLine], ClerkLineLock);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                passportClerkState[myLine] = BUSY;
            } else { /* put myself on break if there is no customers */
                passportClerkState[myLine] = ONBREAK;
                Release(ClerkLineLock);
                Yield();/* context switch */
                if (remainingCustomer == 0) break;
                continue;
            }
        } else {
            Release(ClerkLineLock);
            Yield();/* context switch */
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(passportClerkLineLock[myLine]);
        Release(ClerkLineLock);

        if (inBribeLine) { /* deal with bribe line customers */
            /* clerk service starts */
            Wait(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];

            /* Collect Bribe Money From Customer */
            Acquire(passportMoneyLock);
            MoneyFromPassportClerk += 500;
            Write("PassportClerk[", sizeof("PassportClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Release(passportMoneyLock);

            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            passportClerkPunishment = Random(RAND_UPPER_LIMIT) % 100;/* randomness to make customer go back of the line */
            if (passportClerkPunishment > 5) { /* customer has both their application and picture comleted */

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] has both their application and picture completed\n", sizeof("] has both their application and picture completed\n"), ConsoleOutput);

                numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (i = 0; i < numCalls; i++) {
                    Yield();
                }

                customerApplicationStatus[id] += 3;
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has recorded Customer[", sizeof("] has recorded Customer["), ConsoleOutput);
                Printint(id);
                Write("] passport documentation\n", sizeof("] passport documentation\n"), ConsoleOutput);
                Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);

            } else { /* customer does not have both their applicaiton and picture completed */

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not have both their application and picture completed\n", sizeof("] does not have both their application and picture completed\n"), ConsoleOutput);
                Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);

            }

        } else { /* deal with regular line customers */

            Wait(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];

            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            passportClerkPunishment = Random(RAND_UPPER_LIMIT) % 100;
            if (passportClerkPunishment > 5) {
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] has both their application and picture completed\n", sizeof("] has both their application and picture completed\n"), ConsoleOutput);

                numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (i = 0; i < numCalls; i++) {
                    Yield();
                }

                customerApplicationStatus[id] += 3;
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has recorded Customer[", sizeof("] has recorded Customer["), ConsoleOutput);
                Printint(id);
                Write("] passport documentation\n", sizeof("] passport documentation\n"), ConsoleOutput);
                Signal(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);

            } else {

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not have both their application and picture completed\n", sizeof("] does not have both their application and picture completed\n"), ConsoleOutput);
                Signal(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);

            }

        }

        Release(passportClerkLineLock[myLine]);

        if (remainingCustomer == 0) break;
    }

    Exit(0);
}

void Cashier(int myLine) {
    int id = 0;
    int printed = 0;
    int photoAcceptance;
    int cashierPunishment;

    while (1) {

        if (CashierState[myLine] == ONBREAK && !printed) {
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (CashierState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator && (myLine == 0) && senatorStatus <= 6) { /* if has senator and my index is 0 */

            if (CashierState[myLine] == ONBREAK) {
                CashierState[myLine] = BUSY;
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            Acquire(senatorCashierWaitLock);
            Acquire(senatorWaitLock);

            senatorServiceId = myLine;
            Signal(senatorCashierWaitCV, senatorWaitLock);
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            Wait(senatorCashierWaitCV, senatorWaitLock);
            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senatorData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;/* randomness to make senator back of the line */
            while (photoAcceptance <= 5) {
                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senatorData);
                Write("] has gone to Cashier [", sizeof("] has gone to Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] too soon. They are going to the back of the line.\n", sizeof("] too soon. They are going to the back of the line.\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }

            /* Collect Fee From Senator */
            Acquire(cashierMoneyLock);
            MoneyFromCashier += 100;
            Release(cashierMoneyLock);


            Write("Senator [", sizeof("Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("] has given Cashier [", sizeof("] has given Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] $100.\n", sizeof("] $100.\n"), ConsoleOutput);

            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            senatorStatus += 4;
            Signal(senatorCashierWaitCV, senatorWaitLock);
            Release(senatorWaitLock);
            Release(senatorCashierWaitLock);
        } else if (hasSenator && myLine != 0) {
            CashierState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);

        if (CashierState[myLine] != ONBREAK && !hasSenator) {
            /* When CashierState != ONBREAK */
            if (CashierLineCount[myLine] > 0) {
                Signal(CashierLineWaitCV[myLine], ClerkLineLock);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                CashierState[myLine] = BUSY;
            } else {
                Release(ClerkLineLock);
                CashierState[myLine] = ONBREAK;     /*  */
                /*  Wait(CashierCV[myLine], CashierLock[myLine]); */
                Yield();
                if (remainingCustomer == 0) break;
                continue;
            }
        } else { /* When CashierState == ONBREAK, Do Nothing */
            Release(ClerkLineLock);
            Yield();
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(CashierLineLock[myLine]);
        Release(ClerkLineLock);


        Wait(CashierLineCV[myLine], CashierLineLock[myLine]);
        id = CashierCustomerId[myLine];
        Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
        Printint(myLine);
        Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
        Printint(id);
        Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
        Printint(id);
        Write("]\n", sizeof("]\n"), ConsoleOutput);

        cashierPunishment = Random(RAND_UPPER_LIMIT) % 100;
        if (cashierPunishment > 5) {  /*  Passed All the tests (Certified) */
            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has verified that Customer[", sizeof("] has verified that Customer["), ConsoleOutput);
            Printint(id);
            Write("] has been certified by a PassportClerk\n", sizeof("] has been certified by a PassportClerk\n"), ConsoleOutput);


            /* Collect Fee From Customer */
            Acquire(cashierMoneyLock);
            MoneyFromCashier += 100;
            Release(cashierMoneyLock);


            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has received the $100 from Customer[", sizeof("] has received the $100 from Customer["), ConsoleOutput);
            Printint(id);
            Write("] after certification\n", sizeof("] after certification\n"), ConsoleOutput);

            /*  Give out the passport to the customer */
            /*  Notify the customer he is done */
            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has provided Customer[", sizeof("] has provided Customer["), ConsoleOutput);
            Printint(id);
            Write("] their completed passport\n", sizeof("] their completed passport\n"), ConsoleOutput);
            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded that Customer[", sizeof("] has recorded that Customer["), ConsoleOutput);
            Printint(id);
            Write("] has been given their completed passport\n", sizeof("] has been given their completed passport\n"), ConsoleOutput);

            customerApplicationStatus[id] += 4;
            Signal(CashierLineCV[myLine], CashierLineLock[myLine]);
        } else { /* Not yet Certified */
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received the $100 from Customer[", sizeof("] has received the $100 from Customer["), ConsoleOutput);
            Printint(id);
            Write("] before certification. They are to go to the back of my line.\n", sizeof("] before certification. They are to go to the back of my line.\n"), ConsoleOutput);
            Write("customerApplicationStatus[", sizeof("customerApplicationStatus["), ConsoleOutput);
            Printint(id);
            Write("] is: ", sizeof("] is: "), ConsoleOutput);
            Printint(customerApplicationStatus[id]);
            Write("\n", sizeof("\n"), ConsoleOutput);
            Signal(CashierLineCV[myLine], CashierLineLock[myLine]);

        }


        Release(CashierLineLock[myLine]);

        if (remainingCustomer == 0) break;
    }   /* while loop */

    Exit(0);
}

void Manager() {
    int count = 0;
    int maxNumClerk = 0;
    int numApplicationClerk = APPLICATIONCLERK_SIZE;
    int numPictureClerk = PICTURECLERK_SIZE;
    int numPassportClerk = PASSPORTCLERK_SIZE;
    int numCashier = CASHIER_SIZE;
    unsigned int i;

    /* check the max number of the clerks */
    if (maxNumClerk < numApplicationClerk) maxNumClerk = numApplicationClerk;
    if (maxNumClerk < numPictureClerk) maxNumClerk = numPictureClerk;
    if (maxNumClerk < numPassportClerk) maxNumClerk = numPassportClerk;
    if (maxNumClerk < numCashier) maxNumClerk = numCashier;

    while (1) {
        for (i = 0; i < 100; ++i) {
            Yield();
        }
        /* acquire all the lock to print out the incoming statement */
        Acquire(applicationMoneyLock);
        Acquire(pictureMoneyLock);
        Acquire(passportMoneyLock);
        Acquire(cashierMoneyLock);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromApplicationClerk);
        Write(" for ApplicationClerks\n", sizeof(" for ApplicationClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromPictureClerk);
        Write(" for PictureClerks\n", sizeof(" for PictureClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromPassportClerk);
        Write(" for PassportClerks\n", sizeof(" for PassportClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromCashier);
        Write(" for Cashiers\n", sizeof(" for Cashiers\n"), ConsoleOutput);

        MoneyTotal = MoneyFromApplicationClerk + MoneyFromPictureClerk + MoneyFromPassportClerk + MoneyFromCashier;
        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyTotal);
        Write(" for The passport Office\n", sizeof(" for The passport Office\n"), ConsoleOutput);

        Release(applicationMoneyLock);
        Release(pictureMoneyLock);
        Release(passportMoneyLock);
        Release(cashierMoneyLock);

        count = 0;

        /*  A vector of clerkState */
        /*  A vector of clerkCV */

        Acquire(ClerkLineLock);

        /* Application Clerks */
        for (i = 0; i < numApplicationClerk; i++) {
            if (ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] >= 1
                    && ApplicationClerkState[i] == ONBREAK) {

                ApplicationClerkState[i] = AVAILABLE;
                Write("Manager has woken up an ApplicationClerk\n", sizeof("Manager has woken up an ApplicationClerk\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] > 0) {

                ApplicationClerkState[i] = AVAILABLE;
                Write("Manager has woken up an ApplicationClerk\n", sizeof("Manager has woken up an ApplicationClerk\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Picture Clerks */
        for (i = 0; i < numPictureClerk; i++) {
            if (pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] >= 1
                    && pictureClerkState[i] == ONBREAK) {

                pictureClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PictureClerk\n", sizeof("Manager has woken up a PictureClerk\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] > 0) {

                pictureClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PictureClerk\n", sizeof("Manager has woken up a PictureClerk\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Passport Clerks */
        for (i = 0; i < numPassportClerk; i++) {
            if (passportClerkLineCount[i] + passportClerkBribeLineCount[i] >= 1
                    && passportClerkState[i] == ONBREAK) {

                passportClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PassportClerk\n", sizeof("Manager has woken up a PassportClerk\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       passportClerkLineCount[i] + passportClerkBribeLineCount[i] > 0) {

                passportClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PassportClerk\n", sizeof("Manager has woken up a PassportClerk\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Cashiers */
        for (i = 0; i < numCashier; i++) {
            if (CashierLineCount[i] >= 1
                    && CashierState[i] == ONBREAK) {

                CashierState[i] = AVAILABLE;
                Write("Manager has woken up a Cashier\n", sizeof("Manager has woken up a Cashier\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       CashierLineCount[i] > 0) {

                CashierState[i] = AVAILABLE;
                Write("Manager has woken up a Cashier\n", sizeof("Manager has woken up a Cashier\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        Release(ClerkLineLock);

        count++;

        if (remainingCustomer == 0) {
            Write("No customer. Manager is closing the office\n", sizeof("No customer. Manager is closing the office\n"), ConsoleOutput);
            break;
        }

    }

    Write("\n", sizeof("\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation Finshed.\n", sizeof("Passport Office Simulation Finshed.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Exit(0);
}

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


void PassportOffice() {

    int numApplicationClerk;
    int numPictureClerk;
    int numPassportClerk;
    int numCashier;
    int numCustomer;
    int numSenator;
    unsigned int i;

    int applicationLock;
    int applicationCV;
    int applicationBribeCV;
    int applicationWaitCV;
    int applicationBribeWaitCV;

    int pictureLock;
    int pictureCV;
    int pictureBribeCV;
    int pictureWaitCV;
    int pictureBribeWaitCV;

    int passportLock;
    int passportCV;
    int passportBribeCV;
    int passportWaitCV;
    int passportBribeWaitCV;

    int CashierLock;
    int CashierCV;
    int CashierWaitCV;

    char *applicationCVName = "applicationCV";
    char *applicaitonBribeCVName = "applicaitonBribeCV";
    char *applicationWaitCVName = "applicationWaitCV";
    char *applicationBribeWaitCVName = "applicationWaitCV";

    char *pictureCVName = "pictureCV";
    char *pictureBribeCVName = "pictureBribeCV";
    char *pictureWaitCVName = "pictureWaitCV";
    char *pictureBribeWaitCVName = "pictureWaitCV";

    char *passportCVName = "passportCV";
    char *passportBribeCVName = "passportBribeCV";
    char *passportWaitCVName = "passportWaitCV";
    char *passportBribeWaitCVName = "passportWaitCV";

    char *CashierCVName = "cashierCV";
    char *CashierWaitCVName = "cashierWaitCV";

    clerkState ct;


    ClerkLineLock = CreateLock("ClerkLineLock");
    incrementCount = CreateLock("incrementCount");
    applicationMoneyLock = CreateLock("applicationMoneyLock");
    pictureMoneyLock = CreateLock("pictureMoneyLock");
    passportMoneyLock = CreateLock("passportMoneyLock");
    cashierMoneyLock = CreateLock("cashierMoneyLock");

    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation started.\n", sizeof("Passport Office Simulation started.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("]\n", sizeof("]\n"), ConsoleOutput);

    Write("Number of Customers = ", sizeof("Number of Customers = "), ConsoleOutput);
    numCustomer = CUSTOMER_SIZE;
    Printint(numCustomer);
    Write("\n", sizeof("\n"), ConsoleOutput);
    remainingCustomer = numCustomer;

    Write("Number of ApplicationClerks = ", sizeof("Number of ApplicationClerks = "), ConsoleOutput);
    numApplicationClerk = APPLICATIONCLERK_SIZE;
    Printint(numApplicationClerk);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of PictureClerks = ", sizeof("Number of PictureClerks = "), ConsoleOutput);
    numPictureClerk = PICTURECLERK_SIZE;
    Printint(numPictureClerk);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of PassportClerks = ", sizeof("Number of PassportClerks = "), ConsoleOutput);
    numPassportClerk = PASSPORTCLERK_SIZE;
    Printint(numPassportClerk);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of Cashiers = ", sizeof("Number of Cashiers = "), ConsoleOutput);
    numCashier = CASHIER_SIZE;
    Printint(numCashier);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of Senators = ", sizeof("Number of Senators = "), ConsoleOutput);
    numSenator = SENATOR_SIZE;
    Printint(numSenator);
    Write("\n", sizeof("\n"), ConsoleOutput);

    for (i = 0; i < CUSTOMER_SIZE; i++) {
        numCustomerWaiting[i] = -1;
    }


    /* Initialize all variables for all clerks */
    for (i = 0; i < numApplicationClerk; i++) {
        char *lockName = "ApplicationLock";

        /* application lock initialize */
        applicationLock = CreateLock(lockName);
        ApplicationClerkLineLock[i] = applicationLock;

        /* aplication CV initialize */
        applicationCV = CreateCondition(applicationCVName);
        ApplicationClerkLineCV[i] = applicationCV;

        /* application bribe CV initialize */
        applicationBribeCV = CreateCondition(applicaitonBribeCVName);
        ApplicationClerkBribeLineCV[i] = applicationBribeCV;

        /* application Wait CV initialize */
        applicationWaitCV = CreateCondition(applicationWaitCVName);
        ApplicationClerkLineWaitCV[i] = applicationWaitCV;

        /* application Bribe Wait CV initialize */
        applicationBribeWaitCV = CreateCondition(applicationBribeWaitCVName);
        ApplicationClerkBribeLineWaitCV[i] = applicationBribeWaitCV;

        /* application line size intialize */
        ApplicationClerkLineCount[i] = 0;
        /* application bribe line size initialize */
        ApplicationClerkBribeLineCount[i] = 0;
        /* application clerk state initialize */
        ct = AVAILABLE;
        ApplicationClerkState[i] = ct;
        /* application data initialize */
        ApplicationClerkData[i] = 0;

    }

    for ( i = 0; i < numPictureClerk; i++) {
        char *lockName = "PictureLock";

        /* picture lock initialize */
        pictureLock = CreateLock(lockName);
        pictureClerkLineLock[i] = pictureLock;

        /* picture CV initialize */
        pictureCV = CreateCondition(pictureCVName);
        pictureClerkLineCV[i] = pictureCV;

        /* picture bribe CV initialize */
        pictureBribeCV = CreateCondition(pictureBribeCVName);
        pictureClerkBribeLineCV[i] = pictureBribeCV;

        /* picture Wait CV initialize */
        pictureWaitCV = CreateCondition(pictureWaitCVName);
        pictureClerkLineWaitCV[i] = pictureWaitCV;

        /* picture Bribe Wait CV initialize */
        pictureBribeWaitCV = CreateCondition(pictureBribeWaitCVName);
        pictureClerkBribeLineWaitCV[i] = pictureBribeWaitCV;

        /* picture line size intialize */
        pictureClerkLineCount[i] = 0;
        /* picture bribe line size initialize */
        pictureClerkBribeLineCount[i] = 0;
        /* picture clerk state initialize */
        ct = AVAILABLE;
        pictureClerkState[i] = ct;
        /* picture clerk data initialize */
        pictureClerkData[i] = 0;
        pictureAcceptance[i] = 0;
    }

    for ( i = 0; i < numPassportClerk; i++) {
        char *lockName = "PassportLock";

        /* passport lock initialize */
        passportLock = CreateLock(lockName);
        passportClerkLineLock[i] = passportLock;

        /* passport CV initialize */
        passportCV = CreateCondition(passportCVName);
        passportClerkLineCV[i] = passportCV;

        /* passport bribe CV initialize */
        passportBribeCV = CreateCondition(passportBribeCVName);
        passportClerkBribeLineCV[i] = passportBribeCV;

        /* passport Wait CV initialize */
        passportWaitCV = CreateCondition(passportWaitCVName);
        passportClerkLineWaitCV[i] = passportWaitCV;

        /* passport Bribe Wait CV initialize */
        passportBribeWaitCV = CreateCondition(passportBribeWaitCVName);
        passportClerkBribeLineWaitCV[i] = passportBribeWaitCV;

        /* passport line size intialize */
        passportClerkLineCount[i] = 0;
        /* passport bribe line size initialize */
        passportClerkBribeLineCount[i] = 0;
        /* passport clerk state initialize */
        ct = AVAILABLE;
        passportClerkState[i] = ct;
        /* passport data initialize */
        passportClerkCustomerId[i] = 0;
    }

    for ( i = 0; i < numCashier; i++) {
        char *lockName = "CashierLock";

        /* cashier lock initialize */
        CashierLock = CreateLock(lockName);
        CashierLineLock[i] = CashierLock;

        /* cashier CV initialize */
        CashierCV = CreateCondition(CashierCVName);
        CashierLineCV[i] = CashierCV;

        /* cashier Wait CV initialize */
        CashierWaitCV = CreateCondition(CashierWaitCVName);
        CashierLineWaitCV[i] = CashierWaitCV;

        /* cashier line size intialize */
        CashierLineCount[i] = 0;
        /* cashier state initialize */
        ct = AVAILABLE;
        CashierState[i] = ct;
        /* cashier data initialize */
        CashierCustomerId[i] = 0;
    }

    senatorWaitLock = CreateLock("senator");
    senatorApplicationWaitLock = CreateLock("senatorApplicationWaitLock");
    senatorPictureWaitLock = CreateLock("senatorPictureWaitLock");
    senatorPassportWaitLock = CreateLock("senatorPassportWaitLock");
    senatorCashierWaitLock = CreateLock("senatorCashierWaitLock");
    senatorApplicationWaitCV = CreateCondition("senatorApplicationWaitCV");
    senatorPictureWaitCV = CreateCondition("senatorPictureWaitCV");
    senatorPassportWaitCV = CreateCondition("senatorPassportWaitCV");
    senatorCashierWaitCV = CreateCondition("senatorCashierWaitCV");
    customerWaitCV = CreateCondition("customerCV");
    customerWaitLock = CreateLock("customerLock");

    for (i = 0 ; i < numCustomer; i++) {
        customerApplicationStatus[i] = 0;

    }

    /* Initialize all threads */
    for (i = 0; i < numApplicationClerk; i++) {
        Fork(ApplicationClerk);

    }

    for (i = 0; i < numPictureClerk; i++) {
        Fork(PictureClerk);
    }

    for (i = 0; i < numPassportClerk; i++) {
        Fork(PassportClerk);
    }

    for (i = 0; i < numCashier; i++) {
        Fork(Cashier);
    }

    for (i = 0; i < numCustomer; i++) {
        Fork(Customer);
    }


    Fork(Manager);


    for (i = 0; i < numSenator; i++) {
        Fork(Senator);
    }

    Exit(0);

}

int main() {
    Fork(PassportOffice);

}

