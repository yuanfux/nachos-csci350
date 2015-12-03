#include "syscall.h"
#include "setup.h"

int shortestPictureLine;
int shortestPictureBribeLine;
int shortestPictureBribeLineSize;
int shortestPictureLineSize;
int shortestApplicationBribeLine;
int shortestApplicationBribeLineSize;
int shortestApplicationLine;
int shortestApplicationLineSize;
int shortestPassportBribeLine;
int shortestPassportBribeLineSize;
int shortestPassportLine;
int shortestPassportLineSize;
int shortestCashierLine;
int shortestCashierLineSize;

void main() {
    /* get ssn for each customer */
    int id;
    unsigned int i;
    int choseClerk;
    int randomNum;
    int myLine, data, count, bribeCount, lockData, cvData, money, status, has, rmCustomer;
    clerkState state = ONBREAK;
    setup();
    AcquireServer(incrementCount);
    data = GetMVServer(customerNum);
    id = data + 1;
    data++;
    SetMVServer(customerNum, data);
    ReleaseServer(incrementCount);


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
    SetMVArrayServer(numCustomerWaitingArray, id, id);
    while (1) {
        status = GetMVArrayServer(customerApplicationStatusArray, id);
        if (status == 10) {
            break;
        }
        AcquireServer(customerWaitLock);

        choseClerk = Random(RAND_UPPER_LIMIT) % 2; /* randomly choosing application or picture clerk */

        /* Goes to Picture Clerk. */
        if ((status == 1) || (status == 0 && choseClerk == 0)) { /* has finished applicaiton clerk */

            Yield();
            AcquireServer(ClerkLineLock);
            if (money > 500) { /* can bribe */
                money -= 500; /*  give out money */
                shortestPictureBribeLine = -1;
                shortestPictureBribeLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < PICTURECLERK_SIZE; i++) {
                    count = GetMVArrayServer(pictureClerkBribeLineCountArray, i);
                    if (count < shortestPictureBribeLineSize) {

                        shortestPictureBribeLine = i;
                        shortestPictureBribeLineSize = count;

                    }
                }
                myLine = shortestPictureBribeLine;

                /* wait in the picture clerk line */
                count = GetMVArrayServer(pictureClerkBribeLineCountArray, myLine);
                count++;
                SetMVArrayServer(pictureClerkBribeLineCountArray, myLine, count);
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for PictureClerk[", sizeof("] has gotten in bribe line for PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                ReleaseServer(customerWaitLock);
                cvData = GetMVArrayServer(pictureClerkBribeLineWaitCVArray, myLine);
                WaitServer(cvData, ClerkLineLock); /* wait for signal from clerk */
                count = GetMVArrayServer(pictureClerkBribeLineCountArray, myLine);
                count--;
                SetMVArrayServer(pictureClerkBribeLineCountArray, myLine, count);

                lockData = GetMVArrayServer(pictureClerkLineLockArray, myLine);
                AcquireServer(lockData);
                ReleaseServer(ClerkLineLock);

                SetMVArrayServer(pictureClerkDataArray, myLine, id); /* gives clerk the ssn */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PictureClerk[", sizeof("] to PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);

                cvData = GetMVArrayServer(pictureClerkBribeLineCVArray, myLine);
                SignalServer(cvData, lockData);

                /* wait for picture clerk to take picture */
                WaitServer(cvData, lockData);

                data = Random(RAND_UPPER_LIMIT) % 10; /*  customer decide whether to accept the picture */
                SetMVArrayServer(pictureAcceptanceArray, myLine, data);
                SignalServer(cvData, lockData);

                /* wait for picture clerk to tell customer leave or not */
                WaitServer(cvData, lockData);

                ReleaseServer(lockData);

            } else {
                shortestPictureLine = -1;
                shortestPictureLineSize = 10000;

                for (i = 0; i < PICTURECLERK_SIZE; i++) {
                    count = GetMVArrayServer(pictureClerkLineCountArray, i);
                    if (count < shortestPictureLineSize) {

                        shortestPictureLine = i;
                        shortestPictureLineSize = count;

                    }

                }

                myLine = shortestPictureLine;

                /* wait in the picture clerk line */
                count = GetMVArrayServer(pictureClerkLineCountArray, myLine);
                count++;
                SetMVArrayServer(pictureClerkLineCountArray, myLine, count);

                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for PictureClerk[", sizeof("] has gotten in regular line for PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                ReleaseServer(customerWaitLock);
                cvData = GetMVArrayServer(pictureClerkLineWaitCVArray, myLine);
                WaitServer(cvData, ClerkLineLock); /* wait for signal from clerk */
                count = GetMVArrayServer(pictureClerkLineCountArray, myLine);
                count--;
                SetMVArrayServer(pictureClerkLineCountArray, myLine, count);

                lockData = GetMVArrayServer(pictureClerkLineLockArray, myLine);
                AcquireServer(lockData);
                ReleaseServer(ClerkLineLock);
                SetMVArrayServer(pictureClerkDataArray, myLine, id); /* gives clerk the ssn */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PictureClerk[", sizeof("] to PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                cvData = GetMVArrayServer(pictureClerkLineCVArray, myLine);
                SignalServer(cvData, lockData);

                /* wait for picture clerk to take picture */
                WaitServer(cvData, lockData);

                data = Random(RAND_UPPER_LIMIT) % 10; /*  customer decide whether receive the picture */
                SetMVArrayServer(pictureAcceptanceArray, myLine, data);
                SignalServer(cvData, lockData);

                /* wait for picture clerk to tell customer leave or not */
                WaitServer(cvData, lockData);

                ReleaseServer(lockData);


            }

        }

        /* Goes to Application Clerk */
        else if ((status == 2) || (status == 0 && choseClerk == 1)) { /* has finished picture clerk */

            shortestApplicationBribeLine = -1;
            shortestApplicationBribeLineSize = 10000;
            Yield();
            AcquireServer(ClerkLineLock);
            if (money > 500) { /* has bribe money */
                money -= 500;
                shortestApplicationBribeLine = -1;
                shortestApplicationBribeLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {
                    count = GetMVArrayServer(ApplicationClerkBribeLineCountArray, i);
                    if (count < shortestApplicationBribeLineSize) {
                        shortestApplicationBribeLine = i;
                        shortestApplicationBribeLineSize = count;
                    }
                }
                myLine = shortestApplicationBribeLine;

                /* wait in the application clerk line */
                count = GetMVArrayServer(ApplicationClerkBribeLineCountArray, myLine);
                count++;
                SetMVArrayServer(ApplicationClerkBribeLineCountArray, myLine, count);
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for ApplicationClerk[", sizeof("] has gotten in bribe line for ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                ReleaseServer(customerWaitLock);
                /* wait to be signalled by clerk */
                cvData = GetMVArrayServer(ApplicationClerkBribeLineWaitCVArray, myLine);
                WaitServer(cvData, ClerkLineLock);
                count = GetMVArrayServer(ApplicationClerkBribeLineCountArray, myLine);
                count--;
                SetMVArrayServer(ApplicationClerkBribeLineCountArray, myLine, count);

                lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
                AcquireServer(lockData);
                ReleaseServer(ClerkLineLock);

                SetMVArrayServer(ApplicationClerkDataArray, myLine, id); /* give ssn to clerk */
                
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to ApplicationClerk[", sizeof("] to ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                
                cvData = GetMVArrayServer(ApplicationClerkBribeLineCVArray, myLine);
                SignalServer(cvData, lockData);

                /* wait for clerk to do the job */
                WaitServer(cvData, lockData);

                ReleaseServer(lockData);

            } else { /* does not have bribe money */
                shortestApplicationLine = -1;
                shortestApplicationLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {
                    count = GetMVArrayServer(ApplicationClerkLineCountArray, i);
                    if (count < shortestApplicationLineSize) {
                        shortestApplicationLine = i;
                        shortestApplicationLineSize = count;
                    }
                }

                myLine = shortestApplicationLine;

                /* wait in the application clerk line */
                count = GetMVArrayServer(ApplicationClerkLineCountArray, myLine);
                count++;
                SetMVArrayServer(ApplicationClerkLineCountArray, myLine, count);
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for ApplicationClerk[", sizeof("] has gotten in regular line for ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                ReleaseServer(customerWaitLock);
                /* wait to be signalled by clerk */
                cvData = GetMVArrayServer(ApplicationClerkLineWaitCVArray, myLine);
                WaitServer(cvData, ClerkLineLock);
                count = GetMVArrayServer(ApplicationClerkLineCountArray, myLine);
                count--;
                SetMVArrayServer(ApplicationClerkLineCountArray, myLine, count);
                
                lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
                AcquireServer(lockData);
                ReleaseServer(ClerkLineLock);

                SetMVArrayServer(ApplicationClerkDataArray, myLine, id); /* give ssn to clerk */
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to ApplicationClerk[", sizeof("] to ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                
                cvData = GetMVArrayServer(ApplicationClerkLineCVArray, myLine);
                SignalServer(cvData, lockData);

                /* wait for clerk to do the job */
                WaitServer(cvData, lockData);

                ReleaseServer(lockData);

            }

        }

        /* Goes to Passport Clerk */
        else if (status == 3) {

            Yield();
            AcquireServer(ClerkLineLock);

            if (money > 500) { /* has bribe money */
                money -= 500;
                shortestPassportBribeLine = -1;
                shortestPassportBribeLineSize = 10000;

                /* find shortest line */
                for (i = 0; i < PASSPORTCLERK_SIZE; i++) {
                    count = GetMVArrayServer(passportClerkBribeLineCountArray, i);
                    if (count < shortestPassportBribeLineSize) {
                        shortestPassportBribeLine = i;
                        shortestPassportBribeLineSize = count;
                    }
                }
                myLine = shortestPassportBribeLine;

                /* wait in the passport clerk line */
                count = GetMVArrayServer(passportClerkBribeLineCountArray, myLine);
                count++;
                SetMVArrayServer(passportClerkBribeLineCountArray, myLine, count);
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for PassportClerk[", sizeof("] has gotten in bribe line for PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                ReleaseServer(customerWaitLock);
                /* /wait to get signalled by passport clerk */
                cvData = GetMVArrayServer(passportClerkBribeLineWaitCVArray, myLine);
                WaitServer(cvData, ClerkLineLock);
                count = GetMVArrayServer(passportClerkBribeLineCountArray, myLine);
                count--;
                SetMVArrayServer(passportClerkBribeLineCountArray, myLine, count);
                

                lockData = GetMVArrayServer(passportClerkLineLockArray, myLine);
                AcquireServer(lockData);
                ReleaseServer(ClerkLineLock);

                /* give ssn to passport clerk */
                SetMVArrayServer(passportClerkCustomerIdArray, myLine, id);
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PassportClerk[", sizeof("] to PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                
                cvData = GetMVArrayServer(passportClerkBribeLineCVArray, myLine);
                SignalServer(cvData, lockData);

                /* wait for clerk to do the job */
                WaitServer(cvData, lockData);


                ReleaseServer(lockData);

            } else { /* does not have bribe money */
                shortestPassportLine = -1;
                shortestPassportLineSize = 10000;

                for (i = 0; i < PASSPORTCLERK_SIZE; i++) {
                    count = GetMVArrayServer(passportClerkLineCountArray, i);
                    if (count < shortestPassportLineSize) {

                        shortestPassportLine = i;
                        shortestPassportLineSize = count;

                    }
                }

                myLine = shortestPassportLine;

                count = GetMVArrayServer(passportClerkLineCountArray, myLine);
                count++;
                SetMVArrayServer(passportClerkLineCountArray, myLine, count);
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for PassportClerk[", sizeof("] has gotten in regular line for PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                ReleaseServer(customerWaitLock);
                /* wait to get signalled by passport clerk */
                cvData = GetMVArrayServer(passportClerkLineWaitCVArray, myLine);
                WaitServer(cvData, ClerkLineLock);
                count = GetMVArrayServer(passportClerkLineCountArray, myLine);
                count--;
                SetMVArrayServer(passportClerkLineCountArray, myLine, count);
                
                lockData = GetMVArrayServer(passportClerkLineLockArray, myLine);
                AcquireServer(lockData);
                ReleaseServer(ClerkLineLock);

                /* give ssn to passport clerk */
                SetMVArrayServer(passportClerkCustomerIdArray, myLine, id);

                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PassportClerk[", sizeof("] to PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                
                cvData = GetMVArrayServer(passportClerkLineCVArray, myLine);
                SignalServer(cvData, lockData);

                /* wait for clerk to do the job */
                WaitServer(cvData, lockData);

                ReleaseServer(lockData);

            }



        }

        /* Goes to Cashier counter */
        else if (status == 6) {

            shortestCashierLine;
            shortestCashierLineSize;
            Yield();
            AcquireServer(ClerkLineLock);

            shortestCashierLine = -1;
            shortestCashierLineSize = 10000;

            /* find shortest line */
            for (i = 0; i < CASHIER_SIZE; i++) {
                count = GetMVArrayServer(CashierLineCountArray, i);
                if (count < shortestCashierLineSize) {
                    shortestCashierLine = i;
                    shortestCashierLineSize = count;
                }
            }
            myLine = shortestCashierLine;

            /* get into cashier line */
            count = GetMVArrayServer(CashierLineCountArray, myLine);
            count++;
            SetMVArrayServer(CashierLineCountArray, myLine, count);
            Write("Customer[", sizeof("Customer["), ConsoleOutput);
            Printint(id);
            Write("] has gotten in regular line for Cashier[", sizeof("] has gotten in regular line for Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            ReleaseServer(customerWaitLock);
            cvData = GetMVArrayServer(CashierLineWaitCVArray, myLine);
            WaitServer(cvData, ClerkLineLock);
            count = GetMVArrayServer(CashierLineCountArray, myLine);
            count--;
            SetMVArrayServer(CashierLineCountArray, myLine, count);
            
            lockData = GetMVArrayServer(CashierLineLockArray, myLine);
            AcquireServer(lockData);
            ReleaseServer(ClerkLineLock);

            /* give cashier ssn */
            SetMVArrayServer(CashierCustomerIdArray, myLine, id);
            Write("Customer[", sizeof("Customer["), ConsoleOutput);
            Printint(id);
            Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
            Printint(id);
            Write("] to Cashier[", sizeof("] to Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            cvData = GetMVArrayServer(CashierLineCVArray, myLine);
            SignalServer(cvData, lockData);

            /* wait for cashier to do the job */
            WaitServer(cvData, lockData);
            ReleaseServer(lockData);

        }

    }

    AcquireServer(incrementCount);
    rmCustomer = GetMVServer(remainingCustomer);
    rmCustomer--;
    SetMVServer(remainingCustomer, rmCustomer);

    for (i = 0; i < CUSTOMER_SIZE; i++) {
        data = GetMVArrayServer(numCustomerWaitingArray, i);
        if (data == id) {

            SetMVArrayServer(numCustomerWaitingArray, i, -1);

        }
    }
    ReleaseServer(incrementCount);

    Exit(0);

}