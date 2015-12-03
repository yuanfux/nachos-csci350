#include "syscall.h"
#include "setup.h"

void main() {
    int id = 0;
    int printed = 0;
    int photoAcceptance;
    int cashierPunishment;
    int myLine;
    int data, count, bribeCount, lockData, cvData, money, status, senStatus, senData, has, rmCustomer;
    clerkState state = ONBREAK;
    setup();
    AcquireServer(incrementCount);
    data = GetMVServer(cashierNum);
    myLine = data + 1;
    data++;
    SetMVServer(cashierNum, data);
    ReleaseServer(incrementCount);

    while (1) {
        state = GetMVArrayServer(CashierStateArray, myLine);

        if (state == ONBREAK && !printed) {
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (state != ONBREAK) {
            printed = 0;
        }

        has = GetMVServer(hasSenator);
        senStatus = GetMVServer(senatorStatus);
        if (has == 1 && (myLine == 0) && senStatus <= 6) { /* if has senator and my index is 0 */

            state = GetMVArrayServer(CashierStateArray, myLine);
            if (state == ONBREAK) {
                SetMVArrayServer(CashierStateArray, myLine, BUSY);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = 0;
            }

            AcquireServer(senatorCashierWaitLock);
            AcquireServer(senatorWaitLock);

            senData = GetMVServer(senatorData);
            SetMVServer(senatorServiceId, myLine);
            SignalServer(senatorCashierWaitCV, senatorWaitLock);
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] has signalled a Senator to come to their counter.\n", sizeof("] has signalled a Senator to come to their counter.\n"), ConsoleOutput);

            WaitServer(senatorCashierWaitCV, senatorWaitLock);
            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(senData + 100);
            Write("] from Senator [", sizeof("] from Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            /****************************************
            *photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            *while (photoAcceptance <= 5) {
            *    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
            *    Printint(senData);
            *    Write("] has gone to Cashier [", sizeof("] has gone to Cashier ["), ConsoleOutput);
            *    Printint(myLine);
            *    Write("] too soon. They are going to the back of the line.\n", sizeof("] too soon. They are going to the back of the line.\n"), ConsoleOutput);
            *    photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            *}
            ****************************************/

            /* Collect Fee From Senator */
            AcquireServer(cashierMoneyLock);
            money = GetMVServer(MoneyFromCashier);
            money += 100;
            SetMVServer(MoneyFromCashier, money);
            ReleaseServer(cashierMoneyLock);


            Write("Senator [", sizeof("Senator ["), ConsoleOutput);
            Printint(senData);
            Write("] has given Cashier [", sizeof("] has given Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] $100.\n", sizeof("] $100.\n"), ConsoleOutput);

            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Senator [", sizeof("] has recorded a completed application for Senator ["), ConsoleOutput);
            Printint(senData);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            senStatus = GetMVServer(senatorStatus);
            senStatus += 4;
            SetMVServer(senatorStatus, senStatus);

            SignalServer(senatorCashierWaitCV, senatorWaitLock);
            ReleaseServer(senatorWaitLock);
            ReleaseServer(senatorCashierWaitLock);
        } else if (has == 1 && myLine != 0) {
            SetMVArrayServer(CashierStateArray, myLine, ONBREAK);
        }

        Yield();
        AcquireServer(ClerkLineLock);

        state = GetMVArrayServer(CashierStateArray, myLine);
        has = GetMVServer(hasSenator);
        if (state != ONBREAK && has == 0) {
            /* When CashierState != ONBREAK */
            count = GetMVArrayServer(CashierLineCountArray, myLine);
            if (count > 0) {
                cvData = GetMVArrayServer(CashierLineWaitCVArray, myLine);
                SignalServer(cvData, ClerkLineLock);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                SetMVArrayServer(CashierStateArray, myLine, BUSY);
            } else {
                ReleaseServer(ClerkLineLock);
                SetMVArrayServer(CashierStateArray, myLine, ONBREAK);
                Yield();
                rmCustomer = GetMVServer(remainingCustomer);
                if (rmCustomer == 0) break;
                continue;
            }
        } else { /* When CashierState == ONBREAK, Do Nothing */
            ReleaseServer(ClerkLineLock);
            Yield();
            rmCustomer = GetMVServer(remainingCustomer);
            if (rmCustomer == 0) break;
            continue;
        }

        lockData = GetMVArrayServer(CashierLineLockArray, myLine);
        AcquireServer(lockData);
        ReleaseServer(ClerkLineLock);


        cvData = GetMVArrayServer(CashierLineCVArray, myLine);
        lockData = GetMVArrayServer(CashierLineLockArray, myLine);
        WaitServer(cvData, lockData);
        id = GetMVArrayServer(CashierCustomerIdArray, myLine);
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
            AcquireServer(cashierMoneyLock);
            money = GetMVServer(MoneyFromCashier);
            money += 100;
            SetMVServer(MoneyFromCashier, money);
            ReleaseServer(cashierMoneyLock);


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

            data = GetMVArrayServer(customerApplicationStatusArray, id);
            data += 4;
            SetMVArrayServer(customerApplicationStatusArray, id, data);
            cvData = GetMVArrayServer(CashierLineCVArray, myLine);
            lockData = GetMVArrayServer(CashierLineLockArray, myLine);
            SignalServer(cvData, lockData);
        } else { /* Not yet Certified */
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received the $100 from Customer[", sizeof("] has received the $100 from Customer["), ConsoleOutput);
            Printint(id);
            Write("] before certification. They are to go to the back of my line.\n", sizeof("] before certification. They are to go to the back of my line.\n"), ConsoleOutput);
            Write("customerApplicationStatus[", sizeof("customerApplicationStatus["), ConsoleOutput);
            Printint(id);
            Write("] is: ", sizeof("] is: "), ConsoleOutput);

            data = GetMVArrayServer(customerApplicationStatusArray, id);
            Printint(data);
            Write("\n", sizeof("\n"), ConsoleOutput);
            cvData = GetMVArrayServer(CashierLineCVArray, myLine);
            lockData = GetMVArrayServer(CashierLineLockArray, myLine);
            SignalServer(cvData, lockData);

        }


        lockData = GetMVArrayServer(CashierLineLockArray, myLine);
        ReleaseServer(lockData);

        rmCustomer = GetMVServer(remainingCustomer);
        if (rmCustomer == 0) break;
    }   /* while loop */

    Exit(0);
}