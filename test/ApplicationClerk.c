#include "syscall.h"
#include "setup.h"

void main() {
    int id = 0;
    int printed = 0;
    unsigned int i;
    int InBribeLine = 0;
    int myLine;
    int data, count, bribeCount, lockData, cvData, money, status, senStatus, senData, has, rmCustomer;
    clerkState state = ONBREAK;

    AcquireServer(incrementCount);
    data = GetMVServer(appClerkNum);
    myLine = data + 1;
    data++;
    SetMVServer(appClerkNum, data);
    ReleaseServer(incrementCount);

    while (1) {
        InBribeLine = 0;
        state = GetMVArrayServer(ApplicationClerkStateArray, myLine);
        if (state == ONBREAK && !printed) {
            Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (state != ONBREAK) {
            printed = 0;
        }

        has = GetMVServer(hasSenator);
        senStatus = GetMVServer(senatorStatus);
        if (has == 1 && myLine == 0 && senStatus == 0) { /* if there is a senator present and i am the index 0 clerk */

            state = GetMVArrayServer(ApplicationClerkStateArray, myLine);

            if (state == ONBREAK) {
                SetMVArrayServer(ApplicationClerkStateArray, myLine, BUSY);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            AcquireServer(senatorApplicationWaitLock);
            AcquireServer(senatorWaitLock);

            SignalServer(senatorApplicationWaitCV, senatorWaitLock);
            Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);

            senData = GetMVServer(senatorData);
            WaitServer(senatorApplicationWaitCV, senatorWaitLock);
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            senStatus = GetMVServer(senatorStatus);
            senStatus++;
            SetMVServer(senatorStatus, senStatus);

            SignalServer(senatorApplicationWaitCV, senatorWaitLock);

            ReleaseServer(senatorApplicationWaitLock);
            ReleaseServer(senatorWaitLock);
        } else if (has == 1 && myLine != 0) { /* if there is a senator present and i am not the index 0 clerk. Put myself on break */
            SetMVArrayServer(ApplicationClerkStateArray, myLine, ONBREAK);
        }

        Yield();
        AcquireServer(ClerkLineLock);/* acquire the line lock in case of line size change */

        state = GetMVArrayServer(ApplicationClerkStateArray, myLine);

        has = GetMVServer(hasSenator);
        if (state != ONBREAK && has == 0) { /* no senator, not on break, deal with normal customers */
            bribeCount = GetMVArrayServer(ApplicationClerkBribeLineCountArray, myLine);
            count = GetMVArrayServer(ApplicationClerkLineCountArray, myLine);
            if (bribeCount > 0) { /* bribe line customer first */
                cvData = GetMVArrayServer(ApplicationClerkBribeLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(ApplicationClerkStateArray, myLine, BUSY);
                InBribeLine = 1;
            } else if (count > 0) { /* regular line customer next */
                cvData = GetMVArrayServer(ApplicationClerkLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(ApplicationClerkStateArray, myLine, BUSY);

            } else { /* no customer present */
                SetMVArrayServer(ApplicationClerkStateArray, myLine, ONBREAK);
                ReleaseServer(ClerkLineLock);
                Yield();/* context switch */
                rmCustomer = GetMVServer(remainingCustomer);
                if (rmCustomer == 0) break;
                continue;
            }
        } else { /* if there is no customers, put myself on break */
            ReleaseServer(ClerkLineLock);
            Yield();/* context switch */
            rmCustomer = GetMVServer(remainingCustomer);
            if (rmCustomer == 0) break;
            continue;
        }

        lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
        AcquireServer(lockData);
        ReleaseServer(ClerkLineLock);
        /* wait for customer data */

        if (InBribeLine) { /* in bribe line */

            lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
            cvData = GetMVArrayServer(ApplicationClerkBribeLineCVArray, myLine);
            WaitServer(cvData, lockData);
            id = GetMVArrayServer(ApplicationClerkDataArray, myLine);

            /* Collect Bribe Money From Customer */
            AcquireServer(applicationMoneyLock);
            money = GetMVServer(MoneyFromApplicationClerk);
            money += 500;
            SetMVServer(MoneyFromApplicationClerk, money);

            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            ReleaseServer(applicationMoneyLock);

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

            data = GetMVArrayServer(customerApplicationStatusArray, id);
            data++;
            SetMVArrayServer(customerApplicationStatusArray, id, data);

            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Customer[", sizeof("] has recorded a completed application for Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
            cvData = GetMVArrayServer(ApplicationClerkBribeLineCVArray, myLine);
            SignalServer(cvData, lockData);
        } else { /* not in bribe line */
            lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
            cvData = GetMVArrayServer(ApplicationClerkLineCVArray, myLine);
            WaitServer(cvData, lockData);
            /* do my job customer now waiting */
            id = GetMVArrayServer(ApplicationClerkDataArray, myLine);

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

            data = GetMVArrayServer(customerApplicationStatusArray, id);
            data++;
            SetMVArrayServer(customerApplicationStatusArray, id, data);

            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Customer[", sizeof("] has recorded a completed application for Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
            cvData = GetMVArrayServer(ApplicationClerkLineCVArray, myLine);
            SignalServer(cvData, lockData);

        }

        lockData = GetMVArrayServer(ApplicationClerkLineLockArray, myLine);
        ReleaseServer(lockData);

        rmCustomer = GetMVServer(remainingCustomer);
        if (rmCustomer == 0) break;
    }/* while */

    Exit(0);
}