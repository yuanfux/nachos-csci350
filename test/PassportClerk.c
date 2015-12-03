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
    int lockData, cvData;
    int data, count, bribeCount, lockData, cvData, money, status, senStatus, senData, has, rmCustomer;
    clerkState state = ONBREAK;

    AcquireServer(incrementCount);
    data = GetMVServer(passClerkNum);
    myLine = data + 1;
    data++;
    SetMVServer(passClerkNum, data);
    ReleaseServer(incrementCount);
    while (1) {
        inBribeLine = 0;

        state = GetMVArrayServer(passportClerkStateArray, myLine);
        if (state == ONBREAK && !printed) {
            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (state != ONBREAK) {
            printed = 0;
        }

        has = GetMVServer(hasSenator);
        senStatus = GetMVServer(senatorStatus);
        if (has == 1 && (myLine == 0) && senStatus <= 3) { /* if there is a senator present and I am the index 0 clerk */

            state = GetMVArrayServer(passportClerkStateArray, myLine);
            if (state == ONBREAK) {
                SetMVArrayServer(passportClerkStateArray, myLine, BUSY);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            AcquireServer(senatorPassportWaitLock);
            AcquireServer(senatorWaitLock);

            senData = GetMVServer(senatorData);
            SignalServer(senatorPassportWaitCV, senatorWaitLock);
            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            WaitServer(senatorPassportWaitCV, senatorWaitLock);
            Write("PassportClerk[", sizeof("PassportClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            while (photoAcceptance <= 5) { /* randomness to make senator go back of the line */
                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senData);
                Write("] has gone to PassportClerk [", sizeof("] has gone to PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] too soon. They are going to the back of the line.\n", sizeof("] too soon. They are going to the back of the line.\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }

            Write("PassportClerk[", sizeof("PassportClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            senStatus = GetMVServer(senatorStatus);
            senStatus += 3;
            SetMVServer(senatorStatus, senStatus);
            SignalServer(senatorPassportWaitCV, senatorWaitLock);
            ReleaseServer(senatorWaitLock);
            ReleaseServer(senatorPassportWaitLock);
        } else if (has == 1 && myLine != 0) { /* if there is no senator present and I am not the index 0 clerk. Put myself on break */
            SetMVArrayServer(passportClerkStateArray, myLine, ONBREAK);
        }

        Yield();
        AcquireServer(ClerkLineLock);

        state = GetMVArrayServer(passportClerkStateArray, myLine);
        has = GetMVServer(hasSenator);
        if (state != ONBREAK && has == 0) { /* if there is no senator present and I am not on break, deal with the normal customers */
            bribeCount = GetMVArrayServer(passportClerkBribeLineCountArray, myLine);
            count = GetMVArrayServer(passportClerkLineCountArray, myLine);
            if (bribeCount > 0) { /* bribe line first */
                cvData = GetMVArrayServer(passportClerkBribeLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(passportClerkStateArray, myLine, BUSY);
                inBribeLine = 1;
            } else if (count > 0) { /* regular line next */
                cvData = GetMVArrayServer(passportClerkLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(passportClerkStateArray, myLine, BUSY);
            } else { /* put myself on break if there is no customers */
                SetMVArrayServer(passportClerkStateArray, myLine, ONBREAK);
                ReleaseServer(ClerkLineLock);
                Yield();/* context switch */
                rmCustomer = GetMVServer(remainingCustomer);
                if (rmCustomer == 0) break;
                continue;
            }
        } else {
            ReleaseServer(ClerkLineLock);
            Yield();/* context switch */
            rmCustomer = GetMVServer(remainingCustomer);
            if (rmCustomer == 0) break;
            continue;
        }

        lockData = GetMVArrayServer(passportClerkLineLockArray, myLine);
        AcquireServer(lockData);
        ReleaseServer(ClerkLineLock);

        if (inBribeLine) { /* deal with bribe line customers */
            /* clerk service starts */
            cvData = GetMVArrayServer(passportClerkBribeLineCVArray, myLine);
            WaitServer(cvData, lockData);
            id = GetMVArrayServer(passportClerkCustomerIdArray, myLine);

            /* Collect Bribe Money From Customer */
            AcquireServer(passportMoneyLock);
            money = GetMVServer(MoneyFromPassportClerk);
            money += 500;
            SetMVServer(MoneyFromPassportClerk, money);
            Write("PassportClerk[", sizeof("PassportClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            ReleaseServer(passportMoneyLock);

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

                data = GetMVArrayServer(customerApplicationStatusArray, id);
                data += 3;
                SetMVArrayServer(customerApplicationStatusArray, id, data);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has recorded Customer[", sizeof("] has recorded Customer["), ConsoleOutput);
                Printint(id);
                Write("] passport documentation\n", sizeof("] passport documentation\n"), ConsoleOutput);
                SignalServer(cvData, lockData);

            } else { /* customer does not have both their applicaiton and picture completed */

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not have both their application and picture completed\n", sizeof("] does not have both their application and picture completed\n"), ConsoleOutput);
                SignalServer(cvData, lockData);

            }

        } else { /* deal with regular line customers */

            cvData = GetMVArrayServer(passportClerkLineCVArray, myLine);
            WaitServer(cvData, lockData);
            id = GetMVArrayServer(passportClerkCustomerIdArray, myLine);

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

                data = GetMVArrayServer(customerApplicationStatusArray, id);
                data += 3;
                SetMVArrayServer(customerApplicationStatusArray, id, data);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has recorded Customer[", sizeof("] has recorded Customer["), ConsoleOutput);
                Printint(id);
                Write("] passport documentation\n", sizeof("] passport documentation\n"), ConsoleOutput);
                SignalServer(cvData, lockData);

            } else {

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not have both their application and picture completed\n", sizeof("] does not have both their application and picture completed\n"), ConsoleOutput);
                SignalServer(cvData, lockData);

            }

        }

        ReleaseServer(lockData);

        rmCustomer = GetMVServer(remainingCustomer);
        if (rmCustomer == 0) break;
    }

    Exit(0);
}