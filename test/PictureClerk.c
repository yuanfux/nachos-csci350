#include "syscall.h"
#include "setup.h"

void main() {
    int id = 0;
    int printed = 0;
    unsigned int i;
    int photoAcceptance;
    int numCalls;
    int inBribeLine = 0;
    int myLine;

    Acquire(incrementCount);
    myLine = picClerkNum + 1;
    picClerkNum++;
    Release(incrementCount);

    while (1) {

        inBribeLine = 0;
        if (pictureClerkState[myLine] == ONBREAK && !printed) {
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (pictureClerkState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator == 1 && (myLine == 0) && senatorStatus <= 1) { /* if there is a senator present and i am the index 0 clerk */

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
        } else if (hasSenator == 1 && myLine != 0) { /* if there is a senator present and i am not the index 0 clerk. Put myself on break */
            pictureClerkState[myLine] = ONBREAK;
        }

        Yield();
        Acquire(ClerkLineLock);/* acquire the line lock in case of line size change */
        if (pictureClerkState[myLine] != ONBREAK && hasSenator == 0) { /* no senator, not on break, deal with normal customers */

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