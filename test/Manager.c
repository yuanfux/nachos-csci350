void Manager() {
    int count = 0;
    int maxNumClerk = 0;
    unsigned int i;

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
        Acquire(applicationMoneyLock);
        Acquire(pictureMoneyLock);
        Acquire(passportMoneyLock);
        Acquire(cashierMoneyLock);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromApplicationClerk);
        Write(" for ApplicationClerks\n", sizeof(" for ApplicationClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromPictureClerk);
        Write(" for PictureClerks\n", sizeof(" for PictureClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromPassportClerk);
        Write(" for PassportClerks\n", sizeof(" for PassportClerks\n"), ConsoleOutput);

        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyFromCashier);
        Write(" for Cashiers\n", sizeof(" for Cashiers\n"), ConsoleOutput);

        MoneyTotal = MoneyFromApplicationClerk + MoneyFromPictureClerk + MoneyFromPassportClerk + MoneyFromCashier;
        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
        Printint(MoneyTotal);
        Write(" for The passport Office\n", sizeof(" for The passport Office\n"), ConsoleOutput);

        Release(applicationMoneyLock);
        Release(pictureMoneyLock);
        Release(passportMoneyLock);
        Release(cashierMoneyLock);

        count = 0;

        /*  A vector of clerkState */
        /*  A vector of clerkCV */

        Yield();
        Acquire(ClerkLineLock);

        /* Application Clerks */
        for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {
            if (ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] >= 1
                    && ApplicationClerkState[i] == ONBREAK) {

                ApplicationClerkState[i] = AVAILABLE;
                Write("Manager has woken up an ApplicationClerk\n", sizeof("Manager has woken up an ApplicationClerk\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] > 0) {

                ApplicationClerkState[i] = AVAILABLE;
                Write("Manager has woken up an ApplicationClerk\n", sizeof("Manager has woken up an ApplicationClerk\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Picture Clerks */
        for (i = 0; i < PICTURECLERK_SIZE; i++) {
            if (pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] >= 1
                    && pictureClerkState[i] == ONBREAK) {

                pictureClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PictureClerk\n", sizeof("Manager has woken up a PictureClerk\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] > 0) {

                pictureClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PictureClerk\n", sizeof("Manager has woken up a PictureClerk\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Passport Clerks */
        for (i = 0; i < PASSPORTCLERK_SIZE; i++) {
            if (passportClerkLineCount[i] + passportClerkBribeLineCount[i] >= 1
                    && passportClerkState[i] == ONBREAK) {

                passportClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PassportClerk\n", sizeof("Manager has woken up a PassportClerk\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       passportClerkLineCount[i] + passportClerkBribeLineCount[i] > 0) {

                passportClerkState[i] = AVAILABLE;
                Write("Manager has woken up a PassportClerk\n", sizeof("Manager has woken up a PassportClerk\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        /* Cashiers */
        for (i = 0; i < CASHIER_SIZE; i++) {
            if (CashierLineCount[i] >= 1
                    && CashierState[i] == ONBREAK) {

                CashierState[i] = AVAILABLE;
                Write("Manager has woken up a Cashier\n", sizeof("Manager has woken up a Cashier\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
            } else if (remainingCustomer <= maxNumClerk * 3 &&
                       CashierLineCount[i] > 0) {

                CashierState[i] = AVAILABLE;
                Write("Manager has woken up a Cashier\n", sizeof("Manager has woken up a Cashier\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(i);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);

            }

        }

        Release(ClerkLineLock);

        count++;

        if (remainingCustomer == 0) {
            Write("No customer. Manager is closing the office\n", sizeof("No customer. Manager is closing the office\n"), ConsoleOutput);
            break;
        }

    }


    Acquire(printLock);
    Write("\n", sizeof("\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation Finshed.\n", sizeof("Passport Office Simulation Finshed.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("\n\n--------------Simulation Summary---------------\n\n", sizeof("\n\n--------------Simulation Summary---------------\n\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(MoneyFromApplicationClerk);
    Write(" for ApplicationClerks\n", sizeof(" for ApplicationClerks\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(MoneyFromPictureClerk);
    Write(" for PictureClerks\n", sizeof(" for PictureClerks\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(MoneyFromPassportClerk);
    Write(" for PassportClerks\n", sizeof(" for PassportClerks\n"), ConsoleOutput);

    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(MoneyFromCashier);
    Write(" for Cashiers\n", sizeof(" for Cashiers\n"), ConsoleOutput);

    MoneyTotal = MoneyFromApplicationClerk + MoneyFromPictureClerk + MoneyFromPassportClerk + MoneyFromCashier;
    Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput);
    Printint(MoneyTotal);
    Write(" for The passport Office\n", sizeof(" for The passport Office\n"), ConsoleOutput);
    Write("\n\n--------------------------------------------\n\n", sizeof("\n\n--------------------------------------------\n\n"), ConsoleOutput);
    Release(printLock);

    Exit(0);
}