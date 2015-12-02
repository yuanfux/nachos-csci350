#include "syscall.h"
#include "setup.h"

void main() {
    int id = 0;
    int printed = 0;
    int photoAcceptance;
    int cashierPunishment;
    int myLine;

    Acquire(incrementCount);
    myLine = cashierNum + 1;
    cashierNum++;
    Release(incrementCount);

    while (1) {

        if (CashierState[myLine] == ONBREAK && !printed) {
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = 1;
        } else if (CashierState[myLine] != ONBREAK) {
            printed = 0;
        }

        if (hasSenator == 1 && (myLine == 0) && senatorStatus <= 6) { /* if has senator and my index is 0 */

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
        } else if (hasSenator == 1 && myLine != 0) {
            CashierState[myLine] = ONBREAK;
        }

        Yield();
        Acquire(ClerkLineLock);
        if (CashierState[myLine] != ONBREAK && hasSenator == 0) {
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