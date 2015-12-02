#include "syscall.h"
#include "setup.h"

void main() {
    int id = 0;
    int printed = 0;
    unsigned int i;
    int InBribeLine = 0;
    int myLine;

    Acquire(incrementCount);
    myLine = appClerkNum + 1;
    appClerkNum++;
    Release(incrementCount);

    while (1) {
        InBribeLine = 0;
        if (ApplicationClerkState[myLine] == ONBREAK && !printed) {
            Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (ApplicationClerkState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator == 1 && myLine == 0 && senatorStatus == 0) { /* if there is a senator present and i am the index 0 clerk */

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
        } else if (hasSenator == 1 && myLine != 0) { /* if there is a senator present and i am not the index 0 clerk. Put myself on break */
            ApplicationClerkState[myLine] = ONBREAK;
        }

        Yield();
        Acquire(ClerkLineLock);/* acquire the line lock in case of line size change */

        if (ApplicationClerkState[myLine] != ONBREAK && hasSenator == 0) { /* no senator, not on break, deal with normal customers */
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
            Yield();
            Yield();
            Yield();
            Yield();
            Yield();
            Yield();
            Yield();
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

            Yield();
            Yield();
            Yield();
            Yield();
            Yield();
            Yield();
            Yield();

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