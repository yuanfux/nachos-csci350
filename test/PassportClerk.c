#include "syscall.h"
#include "setup.h"

void main() {
    int id = 0;
    int i;
    int printed = 0;
    int photoAcceptance;
    int numCalls;
    int passportClerkPunishment;
    int inBribeLine = 0;
    int myLine;

    Acquire(incrementCount);
    myLine = passClerkNum + 1;
    passClerkNum++;
    Release(incrementCount);
    while (1) {
        inBribeLine = 0;

        if (passportClerkState[myLine] == ONBREAK && !printed) {
            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (passportClerkState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator == 1 && (myLine == 0) && senatorStatus <= 3) { /* if there is a senator present and I am the index 0 clerk */

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
        } else if (hasSenator == 1 && myLine != 0) { /* if there is no senator present and I am not the index 0 clerk. Put myself on break */
            passportClerkState[myLine] = ONBREAK;
        }

        Yield();
        Acquire(ClerkLineLock);

        if (passportClerkState[myLine] != ONBREAK && hasSenator == 0) { /* if there is no senator present and I am not on break, deal with the normal customers */
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