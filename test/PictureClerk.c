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
    int picAcc, data, count, bribeCount, lockData, cvData, money, status, senStatus, senData, has, rmCustomer;
    clerkState state = ONBREAK;

    AcquireServer(incrementCount);
    data = GetMVServer(picClerkNum);
    myLine = data + 1;
    data++;
    SetMVServer(picClerkNum, data);
    ReleaseServer(incrementCount);

    while (1) {

        inBribeLine = 0;
        state = GetMVArrayServer(pictureClerkStateArray, myLine);
        if (state == ONBREAK && !printed) {
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (state != ONBREAK) {
            printed = 0;
        }

        has = GetMVServer(hasSenator);
        senStatus = GetMVServer(senatorStatus);
        if (has == 1 && (myLine == 0) && senStatus <= 1) { /* if there is a senator present and i am the index 0 clerk */

            state = GetMVArrayServer(pictureClerkStateArray, myLine);
            if (state == ONBREAK) {
                SetMVArrayServer(pictureClerkStateArray, myLine, BUSY);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            AcquireServer(senatorPictureWaitLock);
            AcquireServer(senatorWaitLock);

            SetMVServer(senatorServiceId, myLine);
            senData = GetMVServer(senatorData);
            SignalServer(senatorPictureWaitCV, senatorWaitLock);
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);
            
            WaitServer(senatorPictureWaitCV, senatorWaitLock);
            Write("PictureClerk[", sizeof("PictureClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            while (photoAcceptance <= 5) {
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has taken a picture of Senator[", sizeof("] has taken a picture of Senator["), ConsoleOutput);
                Printint(senData);
                Write("]\n", sizeof("]\n"), ConsoleOutput);

                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senData);
                Write("] does not like their picture from PictureClerk [", sizeof("] does not like their picture from PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("].\n", sizeof("].\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }


            Write("Senator [", sizeof("Senator ["), ConsoleOutput);
            Printint(senData);
            Write("] does like their picture from PictureClerk [", sizeof("] does like their picture from PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("].\n", sizeof("].\n"), ConsoleOutput);
            Write("PictureClerk[", sizeof("PictureClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            senStatus = GetMVServer(senatorStatus);
            senStatus += 2;
            SetMVServer(senatorStatus, senStatus);
            SignalServer(senatorPictureWaitCV, senatorWaitLock);
            ReleaseServer(senatorWaitLock);
            ReleaseServer(senatorPictureWaitLock);
        } else if (has == 1 && myLine != 0) { /* if there is a senator present and i am not the index 0 clerk. Put myself on break */
            SetMVArrayServer(pictureClerkStateArray, myLine, ONBREAK);
        }

        Yield();
        AcquireServer(ClerkLineLock);/* acquire the line lock in case of line size change */
        state = GetMVArrayServer(pictureClerkStateArray, myLine);
        has = GetMVServer(hasSenator);
        if (state != ONBREAK && has == 0) { /* no senator, not on break, deal with normal customers */

            bribeCount = GetMVArrayServer(pictureClerkBribeLineCountArray, myLine);
            count = GetMVArrayServer(pictureClerkLineCountArray, myLine);
            if (bribeCount > 0) { /* bribe line customer first */
                cvData = GetMVArrayServer(pictureClerkBribeLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(pictureClerkStateArray, myLine, BUSY);
                inBribeLine = 1;
            } else if (count > 0) { /* regular line next */
                cvData = GetMVArrayServer(pictureClerkLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(pictureClerkStateArray, myLine, BUSY);
            } else { /* no customer present */
                SetMVArrayServer(pictureClerkStateArray, myLine, ONBREAK);
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

        lockData = GetMVArrayServer(pictureClerkLineLockArray, myLine);
        AcquireServer(lockData);/* acquire the clerk lock to serve a customer */
        ReleaseServer(ClerkLineLock);
        /* if in bribe line */
        if (inBribeLine) {
            /* customer service starts */
            cvData = GetMVArrayServer(pictureClerkBribeLineCVArray, myLine);
            WaitServer(cvData, lockData);
            id = GetMVArrayServer(pictureClerkDataArray, myLine);

            /* Collect Bribe Money From Customer */
            AcquireServer(pictureMoneyLock);
            money = GetMVServer(MoneyFromPictureClerk);
            money += 500;
            SetMVServer(MoneyFromPictureClerk, money);
            Write("PictureClerk[", sizeof("PictureClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            ReleaseServer(pictureMoneyLock);

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

            SignalServer(cvData, lockData);
            WaitServer(cvData, lockData);

            picAcc = GetMVArrayServer(pictureAcceptanceArray, myLine);
            if (picAcc > 2) { /* if customer likes the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does like their picture\n", sizeof("] does like their picture\n"), ConsoleOutput);

                numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;/* delay for clerk to complete the service */
                for ( i = 0; i < numCalls; i++) {
                    Yield();
                }

                data = GetMVArrayServer(customerApplicationStatusArray, id);
                data += 2;
                SetMVArrayServer(customerApplicationStatusArray, id, data);

                SignalServer(cvData, lockData);

            } else { /* if customer does not like the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not like their picture\n", sizeof("] does not like their picture\n"), ConsoleOutput);
                SignalServer(cvData, lockData);
            }

        }
        /* if in regular line */
        else {
            /* customer service starts */
            cvData = GetMVArrayServer(pictureClerkLineCVArray, myLine);
            WaitServer(cvData, lockData);
            id = GetMVArrayServer(pictureClerkDataArray, myLine);

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

            SignalServer(cvData, lockData);
            WaitServer(cvData, lockData);

            picAcc = GetMVArrayServer(pictureAcceptanceArray, myLine);
            if (picAcc > 2) { /* if customer likes the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does like their picture\n", sizeof("] does like their picture\n"), ConsoleOutput);

                numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (i = 0; i < numCalls; i++) {
                    Yield();
                }

                data = GetMVArrayServer(customerApplicationStatusArray, id);
                data += 2;
                SetMVArrayServer(customerApplicationStatusArray, id, data);

                SignalServer(cvData, lockData);

            } else { /* if customer does not like the picture */
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[", sizeof("] has been told that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not like their picture\n", sizeof("] does not like their picture\n"), ConsoleOutput);
                SignalServer(cvData, lockData);
            }

        }
        /* customer service ends */
        ReleaseServer(lockData);

        rmCustomer = GetMVServer(remainingCustomer);
        if (rmCustomer == 0) break;
    }

    Exit(0);
}