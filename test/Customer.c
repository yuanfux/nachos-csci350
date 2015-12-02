#include "syscall.h"
#include "setup.h"

void main() {
    /* get ssn for each customer */
    int id;
    unsigned int i;
    int money;
    int choseClerk;
    int randomNum;
    int myLine;

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
    Write("\n", sizeof("\n"), ConsoleOutput);

    /* each customer needs to go through all the counters before leaving */
    numCustomerWaiting[id] = id;
    while (customerApplicationStatus[id] != 10) {
        Acquire(customerWaitLock);

        choseClerk = Random(RAND_UPPER_LIMIT) % 2; /* randomly choosing application or picture clerk */

        /* Goes to Picture Clerk. */
        if ((customerApplicationStatus[id] == 1) || (customerApplicationStatus[id] == 0 && choseClerk == 0)) { /* has finished applicaiton clerk */

            Yield();
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
                shortestPictureLine = -1;
                shortestPictureLineSize = 10000;

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

            shortestApplicationBribeLine = -1;
            shortestApplicationBribeLineSize = 10000;
            Yield();
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
                shortestApplicationLine = -1;
                shortestApplicationLineSize = 10000;

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

            shortestPassportBribeLine;
            shortestPassportBribeLineSize;
            Yield();
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
                shortestPassportLine = -1;
                shortestPassportLineSize = 10000;

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

            shortestCashierLine;
            shortestCashierLineSize;
            Yield();
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