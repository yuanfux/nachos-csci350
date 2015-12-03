#include "syscall.h"
#include "setup.h"

void main() {
    int maxNumClerk = 0;
    unsigned int i;
    int data, count, bribeCount, lockData, cvData, money, status, senStatus, senData, has, rmCustomer;
    int monTotal, monFromCashier, monFromApplicationClerk, monFromPictureClerk, monFromPassportClerk;

    /* check the max number of the clerks */
    if (maxNumClerk < APPLICATIONCLERK_SIZE) maxNumClerk = APPLICATIONCLERK_SIZE;
    if (maxNumClerk < PICTURECLERK_SIZE) maxNumClerk = PICTURECLERK_SIZE;
    if (maxNumClerk < PASSPORTCLERK_SIZE) maxNumClerk = PASSPORTCLERK_SIZE;
    if (maxNumClerk < CASHIER_SIZE) maxNumClerk = CASHIER_SIZE;

    while (1) {
        Yield();
        Yield();
        Yield();
        Yield();
        Yield();
        Yield();
        Yield();
        Yield();
        Yield();
        Yield();
        /* acquire all the lock to print out the incoming statement */
        lockData = GetMVServer(applicationMoneyLock);
        AcquireServer(lockData);
        lockData = GetMVServer(pictureMoneyLock);
        AcquireServer(lockData);
        lockData = GetMVServer(passportMoneyLock);
        AcquireServer(lockData);
        lockData = GetMVServer(cashierMoneyLock);
        AcquireServer(lockData);

        monFromApplicationClerk = GetMVServer(MoneyFromApplicationClerk);
        monFromPictureClerk = GetMVServer(MoneyFromPictureClerk);
        monFromPassportClerk = GetMVServer(MoneyFromPassportClerk);
        monFromCashier = GetMVServer(MoneyFromCashier);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(monFromApplicationClerk);
        Write(" for ApplicationClerks\n", sizeof(" for ApplicationClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(monFromPictureClerk);
        Write(" for PictureClerks\n", sizeof(" for PictureClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(monFromPassportClerk);
        Write(" for PassportClerks\n", sizeof(" for PassportClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(monFromCashier);
        Write(" for Cashiers\n", sizeof(" for Cashiers\n"), ConsoleOutput);


        monTotal = monFromApplicationClerk + monFromPictureClerk + monFromPassportClerk + monFromCashier;
        SetMVServer(MoneyTotal, monTotal);
        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(monTotal);
        Write(" for The passport Office\n", sizeof(" for The passport Office\n"), ConsoleOutput);

        lockData = GetMVServer(applicationMoneyLock);
        ReleaseServer(lockData);
        lockData = GetMVServer(pictureMoneyLock);
        ReleaseServer(lockData);
        lockData = GetMVServer(passportMoneyLock);
        ReleaseServer(lockData);
        lockData = GetMVServer(cashierMoneyLock);
        ReleaseServer(lockData);


        Yield();
        lockData = GetMVServer(ClerkLineLock);
        AcquireServer(lockData);
        rmCustomer = GetMVServer(remainingCustomer);

        /* Application Clerks */
        for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {
            bribeCount = GetMVArrayServer(ApplicationClerkBribeLineCountArray, i);
            count = GetMVArrayServer(ApplicationClerkLineCountArray, i);
            state = GetMVArrayServer(ApplicationClerkStateArray, i);

            if (count + bribeCount >= 1 && state == ONBREAK) {

                SetMVArrayServer(ApplicationClerkStateArray, i, AVAILABLE);
                Write("Manager has woken up an ApplicationClerk\n", sizeof("Manager has woken up an ApplicationClerk\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (rmCustomer <= maxNumClerk * 3 && count + bribeCount > 0) {

                SetMVArrayServer(ApplicationClerkStateArray, i, AVAILABLE);
                Write("Manager has woken up an ApplicationClerk\n", sizeof("Manager has woken up an ApplicationClerk\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Picture Clerks */
        for (i = 0; i < PICTURECLERK_SIZE; i++) {
            bribeCount = GetMVArrayServer(pictureClerkBribeLineCountArray, i);
            count = GetMVArrayServer(pictureClerkLineCountArray, i);
            state = GetMVArrayServer(pictureClerkStateArray, i);
            if (count + bribeCount >= 1 && state == ONBREAK) {

                SetMVArrayServer(pictureClerkStateArray, i, AVAILABLE);
                Write("Manager has woken up a PictureClerk\n", sizeof("Manager has woken up a PictureClerk\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (rmCustomer <= maxNumClerk * 3 && count + bribeCount > 0) {

                SetMVArrayServer(pictureClerkStateArray, i, AVAILABLE);
                Write("Manager has woken up a PictureClerk\n", sizeof("Manager has woken up a PictureClerk\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Passport Clerks */
        for (i = 0; i < PASSPORTCLERK_SIZE; i++) {
            bribeCount = GetMVArrayServer(ApplicationClerkBribeLineCountArray, i);
            count = GetMVArrayServer(ApplicationClerkLineCountArray, i);
            state = GetMVArrayServer(ApplicationClerkStateArray, i);
            if (count + bribeCount >= 1 && state == ONBREAK) {

                SetMVArrayServer(passportClerkStateArray, i, AVAILABLE);
                Write("Manager has woken up a PassportClerk\n", sizeof("Manager has woken up a PassportClerk\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (rmCustomer <= maxNumClerk * 3 && count + bribeCount > 0) {

                SetMVArrayServer(passportClerkStateArray, i, AVAILABLE);
                Write("Manager has woken up a PassportClerk\n", sizeof("Manager has woken up a PassportClerk\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Cashiers */
        for (i = 0; i < CASHIER_SIZE; i++) {
            count = GetMVArrayServer(CashierLineCountArray, i);
            state = GetMVArrayServer(CashierStateArray, i);
            if (count >= 1 && state == ONBREAK) {

                SetMVArrayServer(CashierStateArray, i, AVAILABLE);
                Write("Manager has woken up a Cashier\n", sizeof("Manager has woken up a Cashier\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (rmCustomer <= maxNumClerk * 3 && count > 0) {

                SetMVArrayServer(CashierStateArray, i, AVAILABLE);
                Write("Manager has woken up a Cashier\n", sizeof("Manager has woken up a Cashier\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        lockData = GetMVServer(ClerkLineLock);
        ReleaseServer(lockData);

        rmCustomer = GetMVServer(remainingCustomer);
        if (rmCustomer == 0) {
            Write("No customer. Manager is closing the office\n", sizeof("No customer. Manager is closing the office\n"), ConsoleOutput);
            break;
        }

    }


    lockData = GetMVServer(printLock);
    AcquireServer(lockData);
    monFromApplicationClerk = GetMVServer(MoneyFromApplicationClerk);
    monFromPictureClerk = GetMVServer(MoneyFromPictureClerk);
    monFromPassportClerk = GetMVServer(MoneyFromPassportClerk);
    monFromCashier = GetMVServer(MoneyFromCashier);
    Write("\n", sizeof("\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation Finshed.\n", sizeof("Passport Office Simulation Finshed.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("\n\n--------------Simulation Summary---------------\n\n", sizeof("\n\n--------------Simulation Summary---------------\n\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(monFromApplicationClerk);
    Write(" for ApplicationClerks\n", sizeof(" for ApplicationClerks\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(monFromPictureClerk);
    Write(" for PictureClerks\n", sizeof(" for PictureClerks\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(monFromPassportClerk);
    Write(" for PassportClerks\n", sizeof(" for PassportClerks\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(monFromCashier);
    Write(" for Cashiers\n", sizeof(" for Cashiers\n"), ConsoleOutput);

    monTotal = monFromApplicationClerk + monFromPictureClerk + monFromPassportClerk + monFromCashier;
    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(monTotal);
    Write(" for The passport Office\n", sizeof(" for The passport Office\n"), ConsoleOutput);
    Write("\n\n--------------------------------------------\n\n", sizeof("\n\n--------------------------------------------\n\n"), ConsoleOutput);

    lockData = GetMVServer(printLock);
    ReleaseServer(lockData);

    Exit(0);
}