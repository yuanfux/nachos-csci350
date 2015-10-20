#include "syscall.h"

#define APPLICATIONCLERK_SIZE 1
#define PICTURECLERK_SIZE 1
#define PASSPORTCLERK_SIZE 1
#define CASHIER_SIZE 1
#define CUSTOMER_SIZE 1
#define SENATOR_SIZE 0
#define MANAGER_SIZE 1

#define RAND_UPPER_LIMIT 10000


enum clerkState {AVAILABLE, BUSY, ONBREAK};

Lock ClerkLineLock("ClerkLineLock");
Lock incrementCount("incrementCount");
int customerApplicationStatus[];

//variables for application clerk
int ApplicationClerkLineLock[APPLICATIONCLERK_SIZE];
int ApplicationClerkLineCV[APPLICATIONCLERK_SIZE]; //cv for each line
int ApplicationClerkLineWaitCV[APPLICATIONCLERK_SIZE];//need new cv to prevent the different lock case
int ApplicationClerkBribeLineCV[APPLICATIONCLERK_SIZE];
int ApplicationClerkBribeLineWaitCV[APPLICATIONCLERK_SIZE];//need new cv to prevent the different lock case
int ApplicationClerkLineCount[APPLICATIONCLERK_SIZE]; //number of customers in each line
int ApplicationClerkBribeLineCount[APPLICATIONCLERK_SIZE];
int ApplicationClerkState[APPLICATIONCLERK_SIZE];
int ApplicationClerkData[APPLICATIONCLERK_SIZE]; //stores ssn's of customers for each clerk

//variables for picture clerks.
int pictureClerkLineLock[PICTURECLERK_SIZE];
int pictureClerkLineCV[PICTURECLERK_SIZE];
int pictureClerkLineWaitCV[PICTURECLERK_SIZE];//need new cv to prevent the different lock case
int pictureClerkBribeLineCV[PICTURECLERK_SIZE];
int pictureClerkBribeLineWaitCV[PICTURECLERK_SIZE];//need new cv to prevent the different lock case
int pictureClerkLineCount[PICTURECLERK_SIZE];
int pictureClerkBribeLineCount[PICTURECLERK_SIZE];
int pictureClerkState[PICTURECLERK_SIZE];
int pictureClerkData[PICTURECLERK_SIZE];
int pictureAcceptance[PICTURECLERK_SIZE]; //stores whether customer likes picture or not at counter

//variables for PassportClerk
int passportClerkCustomerId[PASSPORTCLERK_SIZE]; //stores ssn's of customers for each clerk
int passportClerkState[PASSPORTCLERK_SIZE];
int passportClerkLineLock[PASSPORTCLERK_SIZE];
int passportClerkLineCV[PASSPORTCLERK_SIZE];
int passportClerkLineWaitCV[PASSPORTCLERK_SIZE];//need new cv to prevent the different lock case
int passportClerkBribeLineCV[PASSPORTCLERK_SIZE];
int passportClerkBribeLineWaitCV[PASSPORTCLERK_SIZE];//need new cv to prevent the different lock case
int passportClerkLineCount[PASSPORTCLERK_SIZE];
int passportClerkBribeLineCount[PASSPORTCLERK_SIZE];

//variables for Cashier
int CashierCustomerId[CASHIER_SIZE];
int CashierState[CASHIER_SIZE];
int CashierLineLock[CASHIER_SIZE];
int CashierLineCV[CASHIER_SIZE];
int CashierLineWaitCV[CASHIER_SIZE];//need new cv to prevent the different lock case
int CashierLineCount[CASHIER_SIZE];

//variables for Manager
Lock applicationMoneyLock("applicationMoneyLock"); //each money variable needs a lock
Lock pictureMoneyLock("pictureMoneyLock");
Lock passportMoneyLock("passportMoneyLock");
Lock cashierMoneyLock("cashierMoneyLock");
int MoneyFromApplicationClerk = 0;
int MoneyFromPictureClerk = 0;
int MoneyFromPassportClerk = 0;
int MoneyFromCashier = 0;
int MoneyTotal = 0;

int senatorWaitLock;
int senatorApplicationWaitLock;
int senatorPictureWaitLock;
int senatorPassportWaitLock;
int senatorCashierWaitLock;
int senatorApplicationWaitCV;
int senatorPictureWaitCV;
int senatorPassportWaitCV;
int senatorCashierWaitCV;
int customerWaitCV;
int customerWaitLock;
int senatorStatus = 0;

int numCustomerWaiting[CUSTOMER_SIZE];

int senatorData;
int senatorServiceId;
bool hasSenator = false;


//bool hasSenator=false;
bool isFirst = true;
int customerNum = -1; // number of customers came into the office.
// it is also ssn of customers
int remainingCustomer = 0; // number of customers still in the office


int senatorNum = -1;

void Customer() {
    //get ssn for each customer
    Acquire(incrementCount);
    int id = customerNum + 1;
    customerNum++;
    Release(incrementCount);

    //determine amount of money customer has
    int money;
    int randomNum = Random(RAND_UPPER_LIMIT) % 4;

    if (randomNum == 0) {
        money = 100;
    } else if (randomNum == 1) {
        money = 600;
    } else if (randomNum == 2) {
        money = 1100;
    } else if (randomNum == 3) {
        money = 1600;
    }
    Write("Customer[", sizeof("Customer["), ConsoleOutput);
    Printint(id);
    Write("] has $", sizeof("] has $"), ConsoleOutput);
    Printint(money);
    Write("]\n", sizeof("]\n"), ConsoleOutput);

    //each customer needs to go through all the counters before leaving
    numCustomerWaiting.push_back(id);
    while (customerApplicationStatus[id] != 10) {
        Acquire(customerWaitLock);

        int choseClerk = Random(RAND_UPPER_LIMIT) % 2; //randomly choosing application or picture clerk

        //Goes to Picture Clerk.
        if ((customerApplicationStatus[id] == 1) || (customerApplicationStatus[id] == 0 && choseClerk == 0)) { //has finished applicaiton clerk
            Acquire(ClerkLineLock);
            if (money > 500) { //can bribe
                money -= 500; // give out money
                int myLine;
                int shortestPictureBribeLine = -1;
                int shortestPictureBribeLineSize = INT_MAX;

                //find shortest line
                for (unsigned int i = 0; i < pictureClerkLineLock.size(); i++) {
                    if (pictureClerkBribeLineCount[i] < shortestPictureBribeLineSize) {

                        shortestPictureBribeLine = i;
                        shortestPictureBribeLineSize = pictureClerkBribeLineCount[i];

                    }
                }
                myLine = shortestPictureBribeLine;

                //wait in the picture clerk line
                pictureClerkBribeLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for PictureClerk[", sizeof("] has gotten in bribe line for PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                Wait(pictureClerkBribeLineWaitCV[myLine], ClerkLineLock); //wait for signal from clerk
                pictureClerkBribeLineCount[myLine]--; //leave the line and go to the counter
                Release(ClerkLineLock);

                Acquire(pictureClerkLineLock[myLine]);
                pictureClerkData[myLine] = id; //gives clerk the ssn
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PictureClerk[", sizeof("] to PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                //wait for picture clerk to take picture
                Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                pictureAcceptance[myLine] = Random(RAND_UPPER_LIMIT) % 10; // customer decide whether to accept the picture
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                //wait for picture clerk to tell customer leave or not
                Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

                Release(pictureClerkLineLock[myLine]);

            } else {
                int myLine;
                int shortestPictureLine = -1;
                int shortestPictureLineSize = INT_MAX;

                for (unsigned int i = 0; i < pictureClerkLineLock.size(); i++) {
                    if (pictureClerkLineCount[i] < shortestPictureLineSize) {

                        shortestPictureLine = i;
                        shortestPictureLineSize = pictureClerkLineCount[i];

                    }

                }

                myLine = shortestPictureLine;

                //wait in the picture clerk line
                pictureClerkLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for PictureClerk[", sizeof("] has gotten in regular line for PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                Wait(pictureClerkLineWaitCV[myLine], ClerkLineLock); //wait for signal from clerk
                pictureClerkLineCount[myLine]--;
                Release(ClerkLineLock);

                Acquire(pictureClerkLineLock[myLine]);
                pictureClerkData[myLine] = id; //gives clerk the ssn
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PictureClerk[", sizeof("] to PictureClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                //wait for picture clerk to take picture
                Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                pictureAcceptance[myLine] = Random(RAND_UPPER_LIMIT) % 10; // customer decide whether receive the picture
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                //wait for picture clerk to tell customer leave or not
                Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

                Release(pictureClerkLineLock[myLine]);


            }

        }

        //Goes to Application Clerk
        else if ((customerApplicationStatus[id] == 2) || (customerApplicationStatus[id] == 0 && choseClerk == 1)) { //has finished picture clerk
            Acquire(ClerkLineLock);
            if (money > 500) { //has bribe money
                money -= 500;
                int myLine;
                int shortestApplicationBribeLine = -1;
                int shortestApplicationBribeLineSize = INT_MAX;

                //find shortest line
                for (unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++) {
                    if (ApplicationClerkBribeLineCount[i] < shortestApplicationBribeLineSize) {
                        shortestApplicationBribeLine = i;
                        shortestApplicationBribeLineSize = ApplicationClerkBribeLineCount[i];
                    }
                }
                myLine = shortestApplicationBribeLine;

                //wait in the application clerk line
                ApplicationClerkBribeLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for ApplicationClerk[", sizeof("] has gotten in bribe line for ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                //wait to be signalled by clerk
                Wait(ApplicationClerkBribeLineWaitCV[myLine], ClerkLineLock);
                ApplicationClerkBribeLineCount[myLine]--;
                Release(ClerkLineLock);

                Acquire(ApplicationClerkLineLock[myLine]);
                ApplicationClerkData[myLine] = id; //give ssn to clerk
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to ApplicationClerk[", sizeof("] to ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);

                //wait for clerk to do the job
                Wait(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);

                Release(ApplicationClerkLineLock[myLine]);

            } else { //does not have bribe money
                int myLine;
                int shortestApplicationLine = -1;
                int shortestApplicationLineSize = INT_MAX;

                //find shortest line
                for (unsigned int i = 0; i < ApplicationClerkLineLock.size(); i++) {
                    if (ApplicationClerkLineCount[i] < shortestApplicationLineSize) {
                        shortestApplicationLine = i;
                        shortestApplicationLineSize = ApplicationClerkLineCount[i];
                    }
                }

                myLine = shortestApplicationLine;

                //wait in the application clerk line
                ApplicationClerkLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for ApplicationClerk[", sizeof("] has gotten in regular line for ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                //wait to be signalled by clerk
                Wait(ApplicationClerkLineWaitCV[myLine], ClerkLineLock);
                ApplicationClerkLineCount[myLine]--;
                Release(ClerkLineLock);

                Acquire(ApplicationClerkLineLock[myLine]);
                ApplicationClerkData[myLine] = id; //give ssn to clerk
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to ApplicationClerk[", sizeof("] to ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);

                //wait for clerk to do the job
                Wait(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);

                Release(ApplicationClerkLineLock[myLine]);

            }

        }

        //Goes to Passport Clerk
        else if (customerApplicationStatus[id] == 3) {
            Acquire(ClerkLineLock);

            if (money > 500) { //has bribe money
                money -= 500;
                int myLine;
                int shortestPassportBribeLine = -1;
                int shortestPassportBribeLineSize = INT_MAX;

                //find shortest line
                for (unsigned int i = 0; i < passportClerkLineLock.size(); i++) {
                    if (passportClerkBribeLineCount[i] < shortestPassportBribeLineSize) {
                        shortestPassportBribeLine = i;
                        shortestPassportBribeLineSize = passportClerkBribeLineCount[i];
                    }
                }
                myLine = shortestPassportBribeLine;

                //wait in the passport clerk line
                passportClerkBribeLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in bribe line for PassportClerk[", sizeof("] has gotten in bribe line for PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                ///wait to get signalled by passport clerk
                Wait(passportClerkBribeLineWaitCV[myLine], ClerkLineLock);
                passportClerkBribeLineCount[myLine]--;
                Release(ClerkLineLock);

                //give ssn to passport clerk
                Acquire(passportClerkLineLock[myLine]);
                passportClerkCustomerId[myLine] = id;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PassportClerk[", sizeof("] to PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);

                //wait for clerk to do the job
                Wait(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);


                Release(passportClerkLineLock[myLine]);

            } else { //does not have bribe money
                int myLine;
                int shortestPassportLine = -1;
                int shortestPassportLineSize = INT_MAX;

                for (unsigned int i = 0; i < passportClerkLineLock.size(); i++) {
                    if (passportClerkLineCount[i] < shortestPassportLineSize) {

                        shortestPassportLine = i;
                        shortestPassportLineSize = passportClerkLineCount[i];

                    }
                }

                myLine = shortestPassportLine;

                passportClerkLineCount[myLine]++;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has gotten in regular line for PassportClerk[", sizeof("] has gotten in regular line for PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Release(customerWaitLock);
                //wait to get signalled by passport clerk
                Wait(passportClerkLineWaitCV[myLine], ClerkLineLock);
                passportClerkLineCount[myLine]--;
                Release(ClerkLineLock);

                //give ssn to passport clerk
                Acquire(passportClerkLineLock[myLine]);
                passportClerkCustomerId[myLine] = id;
                Write("Customer[", sizeof("Customer["), ConsoleOutput);
                Printint(id);
                Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
                Printint(id);
                Write("] to PassportClerk[", sizeof("] to PassportClerk["), ConsoleOutput);
                Printint(myLine);
                Write("]\n", sizeof("]\n"), ConsoleOutput);
                Signal(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);

                //wait for clerk to do the job
                Wait(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);

                Release(passportClerkLineLock[myLine]);

            }



        }

        //Goes to Cashier counter
        else if (customerApplicationStatus[id] == 6) {
            Acquire(ClerkLineLock);

            int myLine;
            int shortestCashierLine = -1;
            int shortestCashierLineSize = INT_MAX;

            //find shortest line
            for (unsigned int i = 0; i < CashierLineLock.size(); i++) {
                if (CashierLineCount[i] < shortestCashierLineSize) {
                    shortestCashierLine = i;
                    shortestCashierLineSize = passportClerkLineCount[i];
                }
            }
            myLine = shortestCashierLine;

            //get into cashier's line
            CashierLineCount[myLine]++;
            Write("Customer[", sizeof("Customer["), ConsoleOutput);
            Printint(id);
            Write("] has gotten in regular line for Cashier[", sizeof("] has gotten in regular line for Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Release(customerWaitLock);
            Wait(CashierLineWaitCV[myLine], ClerkLineLock);
            CashierLineCount[myLine]--;
            Release(ClerkLineLock);

            //give cashier ssn
            Acquire(CashierLineLock[myLine]);
            CashierCustomerId[myLine] = id;
            Write("Customer[", sizeof("Customer["), ConsoleOutput);
            Printint(id);
            Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
            Printint(id);
            Write("] to Cashier[", sizeof("] to Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Signal(CashierLineCV[myLine], CashierLineLock[myLine]);

            //wait for cashier to do the job
            Wait(CashierLineCV[myLine], CashierLineLock[myLine]);
            Release(CashierLineLock[myLine]);

        }

    }

    Acquire(incrementCount);
    remainingCustomer--;

    for (unsigned int i = 0; i < numCustomerWaiting.size(); i++) {
        if (numCustomerWaiting[i] == id) {

            numCustomerWaiting.erase(numCustomerWaiting.begin() + i);

        }


    }
    Release(incrementCount);

}

void ApplicationClerk(int myLine) {
    int id = 0;
    bool printed = false;

    while (true) {
        bool InBribeLine = false;

        if (ApplicationClerkState[myLine] == ONBREAK && !printed) {
            Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = true;
        } else if (ApplicationClerkState[myLine] != ONBREAK) {
            printed = false;
        }

        if (hasSenator && (myLine == 0) && senatorStatus == 0) { //if there is a senator present and i am the index 0 clerk

            if (ApplicationClerkState[myLine] == ONBREAK) {
                ApplicationClerkState[myLine] = BUSY;
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("ApplicationClerk [", sizeof("ApplicationClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = false;
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
        } else if (hasSenator && myLine != 0) { //if there is a senator present and i am not the index 0 clerk. Put myself on break
            ApplicationClerkState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);//acquire the line lock in case of line size change

        if (ApplicationClerkState[myLine] != ONBREAK && !hasSenator) { //no senator, not on break, deal with normal customers
            if (ApplicationClerkBribeLineCount[myLine] > 0) { //bribe line customer first
                Signal(ApplicationClerkBribeLineWaitCV[myLine], ClerkLineLock);
                Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                ApplicationClerkState[myLine] = BUSY;
                InBribeLine = true;
            } else if (ApplicationClerkLineCount[myLine] > 0) { //regular line customer next
                Signal(ApplicationClerkLineWaitCV[myLine], ClerkLineLock);
                Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                ApplicationClerkState[myLine] = BUSY;

            } else { //no customer present
                ApplicationClerkState[myLine] = ONBREAK;
                Release(ClerkLineLock);
                currentThread->Yield();//context switch
                if (remainingCustomer == 0) break;
                continue;
            }
        } else { //if there is no customers, put myself on break
            Release(ClerkLineLock);
            currentThread->Yield();//context switch
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(ApplicationClerkLineLock[myLine]);
        Release(ClerkLineLock);
        //wait for customer data

        if (InBribeLine) { //in bribe line

            Wait(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);
            id = ApplicationClerkData[myLine];

            //Collect Bribe Money From Customer
            Acquire(applicationMoneyLock);
            MoneyFromApplicationClerk += 500;
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received $500 from Customer[", sizeof("] has received $500 from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            Release(applicationMoneyLock);

            //do my job customer now waiting
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);
            for (int i = 0; i < 20; i++) {
                currentThread->Yield();
            }
            customerApplicationStatus[id]++;
            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has recorded a completed application for Customer[", sizeof("] has recorded a completed application for Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(ApplicationClerkBribeLineCV[myLine], ApplicationClerkLineLock[myLine]);
        } else { //not in bribe line
            Wait(ApplicationClerkLineCV[myLine], ApplicationClerkLineLock[myLine]);
            //do my job customer now waiting
            id = ApplicationClerkData[myLine];

            Write("ApplicationClerk[", sizeof("ApplicationClerk["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            for (int i = 0; i < 20; i++) {
                currentThread->Yield();
            }

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
    }//while
}

void PictureClerk(int myLine) {
    int id = 0;
    bool printed = false;

    while (true) {

        bool inBribeLine = false;

        if (pictureClerkState[myLine] == ONBREAK && !printed) {
            Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = true;
        } else if (pictureClerkState[myLine] != ONBREAK) {
            printed = false;
        }

        if (hasSenator && (myLine == 0) && senatorStatus <= 1) { //if there is a senator present and i am the index 0 clerk

            if (pictureClerkState[myLine] == ONBREAK) {
                pictureClerkState[myLine] = BUSY;
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = false;
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

            int photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            while (photoAcceptance <= 5) {
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has taken a picture of Senator[");
                Printint(senatorData);
                Write("]\n", sizeof("]\n"), ConsoleOutput);

                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senatorData);
                Write("] does not like their picture from PictureClerk [");
                Printint(myLine);
                Write("].\n", sizeof("].\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }

            Write("Senator [", sizeof("Senator ["), ConsoleOutput);
            Printint(senatorData);
            Write("] does like their picture from PictureClerk [");
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
        } else if (hasSenator && myLine != 0) { //if there is a senator present and i am not the index 0 clerk. Put myself on break
            pictureClerkState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);//acquire the line lock in case of line size change
        if (pictureClerkState[myLine] != ONBREAK && !hasSenator) { //no senator, not on break, deal with normal customers

            if (pictureClerkBribeLineCount[myLine] > 0) { //bribe line customer first
                Signal(pictureClerkBribeLineWaitCV[myLine], ClerkLineLock);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                pictureClerkState[myLine] = BUSY;
                inBribeLine = true;
            } else if (pictureClerkLineCount[myLine] > 0) { //regular line next
                Signal(pictureClerkLineWaitCV[myLine], ClerkLineLock);
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                pictureClerkState[myLine] = BUSY;
            } else { //no customer present
                pictureClerkState[myLine] = ONBREAK;
                Release(ClerkLineLock);
                currentThread->Yield();//context switch
                if (remainingCustomer == 0) break;
                continue;
            }
        } else { //if there is no customers, put myself on break
            Release(ClerkLineLock);
            currentThread->Yield();//context switch
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(pictureClerkLineLock[myLine]);//acquire the clerk's lock to serve a customer
        Release(ClerkLineLock);
        //if in bribe line
        if (inBribeLine) {
            //customer service starts
            Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);
            id = pictureClerkData[myLine];

            //Collect Bribe Money From Customer
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
            Write("] has taken a picture of Customer[");
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);
            Wait(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

            if (pictureAcceptance[myLine] > 2) { //if customer likes the picture
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[");
                Printint(id);
                Write("] does like their picture\n");

                int numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;//delay for clerk to complete the service
                for (int i = 0; i < numCalls; i++) {
                    currentThread->Yield();
                }

                customerApplicationStatus[id] += 2;
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);

            } else { //if customer does not like the picture
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[");
                Printint(id);
                Write("] does not like their picture\n");
                Signal(pictureClerkBribeLineCV[myLine], pictureClerkLineLock[myLine]);
            }

        }
        //if in regular line
        else {
            //customer service starts
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
            Write("] has taken a picture of Customer[");
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);
            Wait(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

            if (pictureAcceptance[myLine] > 2) { //if customer likes the picture
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[");
                Printint(id);
                Write("] does like their picture\n");

                int numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (int i = 0; i < numCalls; i++) {
                    currentThread->Yield();
                }

                customerApplicationStatus[id] += 2;
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);

            } else { //if customer does not like the picture
                Write("PictureClerk [", sizeof("PictureClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has been told that Customer[");
                Printint(id);
                Write("] does not like their picture\n");
                Signal(pictureClerkLineCV[myLine], pictureClerkLineLock[myLine]);
            }

        }
        //customer service ends
        Release(pictureClerkLineLock[myLine]);

        if (remainingCustomer == 0) break;
    }
}

void PassportClerk(int myLine) {
    int id = 0;
    bool printed = false;

    while (true) {

        bool inBribeLine = false;

        if (passportClerkState[myLine] == ONBREAK && !printed) {
            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = true;
        } else if (passportClerkState[myLine] != ONBREAK) {
            printed = false;
        }

        if (hasSenator && (myLine == 0) && senatorStatus <= 3) { //if there is a senator present and I am the index 0 clerk

            if (passportClerkState[myLine] == ONBREAK) {
                passportClerkState[myLine] = BUSY;
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = false;
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

            int photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            while (photoAcceptance <= 5) { //randomness to make senator go back of the line
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
        } else if (hasSenator && myLine != 0) { //if there is no senator present and I am not the index 0 clerk. Put myself on break
            passportClerkState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);

        if (passportClerkState[myLine] != ONBREAK && !hasSenator) { //if there is no senator present and I am not on break, deal with the normal customers
            if (passportClerkBribeLineCount[myLine] > 0) { //bribe line first
                Signal(passportClerkBribeLineWaitCV[myLine], ClerkLineLock);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                passportClerkState[myLine] = BUSY;
                inBribeLine = true;
            } else if (passportClerkLineCount[myLine] > 0) { //regular line next
                Signal(passportClerkLineWaitCV[myLine], ClerkLineLock);
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                passportClerkState[myLine] = BUSY;
            } else { //put myself on break if there is no customers
                passportClerkState[myLine] = ONBREAK;
                Release(ClerkLineLock);
                currentThread->Yield();//context switch
                if (remainingCustomer == 0) break;
                continue;
            }
        } else {
            Release(ClerkLineLock);
            currentThread->Yield();//context switch
            if (remainingCustomer == 0) break;
            continue;
        }

        Acquire(passportClerkLineLock[myLine]);
        Release(ClerkLineLock);

        if (inBribeLine) { //deal with bribe line customers
            //clerk service starts
            Wait(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];

            //Collect Bribe Money From Customer
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

            int passportClerkPunishment = Random(RAND_UPPER_LIMIT) % 100;//randomness to make customer go back of the line
            if (passportClerkPunishment > 5) { //customer has both their application and picture comleted

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] has both their application and picture completed\n", sizeof("] has both their application and picture completed\n"), ConsoleOutput);

                int numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (int i = 0; i < numCalls; i++) {
                    currentThread->Yield();
                }

                customerApplicationStatus[id] += 3;
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has recorded Customer[", sizeof("] has recorded Customer["), ConsoleOutput);
                Printint(id);
                Write("] passport documentation\n", sizeof("] passport documentation\n"), ConsoleOutput);
                Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);

            } else { //customer does not have both their applicaiton and picture completed

                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] does not have both their application and picture completed\n", sizeof("] does not have both their application and picture completed\n"), ConsoleOutput);
                Signal(passportClerkBribeLineCV[myLine], passportClerkLineLock[myLine]);

            }

        } else { //deal with regular line customers

            Wait(passportClerkLineCV[myLine], passportClerkLineLock[myLine]);
            id = passportClerkCustomerId[myLine];

            Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received SSN [", sizeof("] has received SSN ["), ConsoleOutput);
            Printint(id);
            Write("] from Customer[", sizeof("] from Customer["), ConsoleOutput);
            Printint(id);
            Write("]\n", sizeof("]\n"), ConsoleOutput);

            int passportClerkPunishment = Random(RAND_UPPER_LIMIT) % 100;
            if (passportClerkPunishment > 5) {
                Write("PassportClerk [", sizeof("PassportClerk ["), ConsoleOutput);
                Printint(myLine);
                Write("] has determined that Customer[", sizeof("] has determined that Customer["), ConsoleOutput);
                Printint(id);
                Write("] has both their application and picture completed\n", sizeof("] has both their application and picture completed\n"), ConsoleOutput);

                int numCalls = Random(RAND_UPPER_LIMIT) % 80 + 20;
                for (int i = 0; i < numCalls; i++) {
                    currentThread->Yield();
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
}

void Cashier(int myLine) {
    int id = 0;
    bool printed = false;

    while (true) {

        if (CashierState[myLine] == ONBREAK && !printed) {
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] is going on break\n", sizeof("] is going on break\n"), ConsoleOutput);
            printed = true;
        } else if (CashierState[myLine] != ONBREAK) {
            printed = false;
        }

        if (hasSenator && (myLine == 0) && senatorStatus <= 6) { //if has senator and my index is 0

            if (CashierState[myLine] == ONBREAK) {
                CashierState[myLine] = BUSY;
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] sees a senator\n", sizeof("] sees a senator\n"), ConsoleOutput);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] is coming off break\n", sizeof("] is coming off break\n"), ConsoleOutput);
                printed = false;
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

            int photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;//randomness to make senator back of the line
            while (photoAcceptance <= 5) {
                Write("Senator [", sizeof("Senator ["), ConsoleOutput);
                Printint(senatorData);
                Write("] has gone to Cashier [", sizeof("] has gone to Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] too soon. They are going to the back of the line.\n", sizeof("] too soon. They are going to the back of the line.\n"), ConsoleOutput);
                photoAcceptance = Random(RAND_UPPER_LIMIT) % 100;
            }

            //Collect Fee From Senator
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
        } else if (hasSenator && myLine != 0) {
            CashierState[myLine] = ONBREAK;
        }

        Acquire(ClerkLineLock);

        if (CashierState[myLine] != ONBREAK && !hasSenator) {
            //When CashierState != ONBREAK
            if (CashierLineCount[myLine] > 0) {
                Signal(CashierLineWaitCV[myLine], ClerkLineLock);
                Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
                Printint(myLine);
                Write("] has signalled a Customer to come to their counter.\n", sizeof("] has signalled a Customer to come to their counter.\n"), ConsoleOutput);
                CashierState[myLine] = BUSY;
            } else {
                Release(ClerkLineLock);
                CashierState[myLine] = ONBREAK;     //
                // Wait(CashierCV[myLine], CashierLock[myLine]);
                currentThread->Yield();
                if (remainingCustomer == 0) break;
                continue;
            }
        } else { //When CashierState == ONBREAK, Do Nothing
            Release(ClerkLineLock);
            currentThread->Yield();
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

        int cashierPunishment = Random(RAND_UPPER_LIMIT) % 100;
        if (cashierPunishment > 5) {  // Passed All the tests (Certified)
            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has verified that Customer[", sizeof("] has verified that Customer["), ConsoleOutput);
            Printint(id);
            Write("] has been certified by a PassportClerk\n", sizeof("] has been certified by a PassportClerk\n"), ConsoleOutput);


            //Collect Fee From Customer
            Acquire(cashierMoneyLock);
            MoneyFromCashier += 100;
            Release(cashierMoneyLock);


            Write("Cashier[", sizeof("Cashier["), ConsoleOutput);
            Printint(myLine);
            Write("] has received the $100 from Customer[", sizeof("] has received the $100 from Customer["), ConsoleOutput);
            Printint(id);
            Write("] after certification\n", sizeof("] after certification\n"), ConsoleOutput);

            // Give out the passport to the customer
            // Notify the customer he is done
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
        } else { //Not yet Certified
            Write("Cashier [", sizeof("Cashier ["), ConsoleOutput);
            Printint(myLine);
            Write("] has received the $100 from Customer[", sizeof("] has received the $100 from Customer["), ConsoleOutput);
            Printint(id);
            Write("] before certification. They are to go to the back of my line.\n". sizeof("] before certification. They are to go to the back of my line.\n"), ConsoleOutput);
            Write("customerApplicationStatus[", sizeof("customerApplicationStatus["), ConsoleOutput);
            Printint(id);
            Write("] is: ", sizeof("] is: "), ConsoleOutput); 
            Printint(customerApplicationStatus[id]);
            Write("\n", sizeof("\n"), ConsoleOutput);
            Signal(CashierLineCV[myLine], CashierLineLock[myLine]);

        }


        Release(CashierLineLock[myLine]);

        if (remainingCustomer == 0) break;
    }   //while loop
}

void Manager() {
    int count = 0;
    int maxNumClerk = 0;
    int numApplicationClerk = ApplicationClerkLineLock.size();
    int numPictureClerk = pictureClerkLineLock.size();
    int numPassportClerk = passportClerkLineLock.size();
    int numCashier = CashierLineLock.size();
    //check the max number of the clerks
    if (maxNumClerk < numApplicationClerk) maxNumClerk = numApplicationClerk;
    if (maxNumClerk < numPictureClerk) maxNumClerk = numPictureClerk;
    if (maxNumClerk < numPassportClerk) maxNumClerk = numPassportClerk;
    if (maxNumClerk < numCashier) maxNumClerk = numCashier;

    while (true) {
        for (int i = 0; i < 100; ++i) {
            currentThread->Yield();
        }
        //acquire all the lock to print out the incoming statement
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
        Write("Manager has counted a total of $", sizeof("Manager has counted a total of $"), ConsoleOutput << MoneyTotal << " for The passport Office\n");

        Release(applicationMoneyLock);
        Release(pictureMoneyLock);
        Release(passportMoneyLock);
        Release(cashierMoneyLock);

        count = 0;

        // A vector of clerkState
        // A vector of clerkCV

        Acquire(ClerkLineLock);

        //Application Clerks
        for ( int i = 0; i < numApplicationClerk; i++) {
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

        //Picture Clerks
        for ( int i = 0; i < numPictureClerk; i++) {
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

        //Passport Clerks
        for ( int i = 0; i < numPassportClerk; i++) {
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

        //Cashiers
        for ( int i = 0; i < numCashier; i++) {
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

    Write("\n", sizeof("\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation Finshed.\n", sizeof("Passport Office Simulation Finshed.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("\n", sizeof("\n"), ConsoleOutput);

}

void Senator() {
    //acquire all the necessary locks to get started
    Acquire(customerWaitLock);
    Acquire(senatorPictureWaitLock);
    Acquire(senatorPassportWaitLock);
    Acquire(senatorCashierWaitLock);
    Acquire(senatorApplicationWaitLock);
    Acquire(senatorWaitLock);
    //senator ID and SSN
    int id = senatorNum + 1;
    int ssn = id + 100;
    senatorNum++;
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has came into passport office\n", sizeof("] has came into passport office\n"), ConsoleOutput);

    hasSenator = TRUE;
    for (unsigned int i = 0; i < numCustomerWaiting.size(); i++) {
        Write("Customer [", sizeof("Customer ["), ConsoleOutput);
        Printint(numCustomerWaiting[i]);
        Write("] is going outside the Passport Office because their is a Senator present.\n", sizeof("] is going outside the Passport Office because their is a Senator present.\n"), ConsoleOutput);
    }

    Release(senatorApplicationWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for ApplicationClerk [", sizeof("] has gotten in regular line for ApplicationClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorApplicationWaitCV, senatorWaitLock);//wait for a clerk

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to ApplicationClerk [", sizeof("] to ApplicationClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorApplicationWaitCV, senatorWaitLock);//signal a clerk

    Wait(senatorApplicationWaitCV, senatorWaitLock);//wait for a filed application


    Release(senatorPictureWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for PictureClerk [", sizeof("] has gotten in regular line for PictureClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorPictureWaitCV, senatorWaitLock);//wait for a clerk

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to PictureClerk [", sizeof("] to PictureClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorPictureWaitCV, senatorWaitLock);//signal a clerk
    Wait(senatorPictureWaitCV, senatorWaitLock);//wait for a filed application


    Release(senatorPassportWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for PassportClerk [", sizeof("] has gotten in regular line for PassportClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorPassportWaitCV, senatorWaitLock);//wait for a clerk

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to PassportClerk [", sizeof("] to PassportClerk ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorPassportWaitCV, senatorWaitLock);//signal a clerk
    Wait(senatorPassportWaitCV, senatorWaitLock);//wait for a filed application


    Release(senatorCashierWaitLock);
    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has gotten in regular line for Cashier [", sizeof("] has gotten in regular line for Cashier ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    Wait(senatorCashierWaitCV, senatorWaitLock);//wait for a clerk

    Write("Senator [", sizeof("Senator ["), ConsoleOutput);
    Printint(id);
    Write("] has given SSN [", sizeof("] has given SSN ["), ConsoleOutput);
    Printint(ssn);
    Write("] to Cashier [", sizeof("] to Cashier ["), ConsoleOutput);
    Printint(senatorServiceId);
    Write("].\n", sizeof("].\n"), ConsoleOutput);
    senatorData = id;
    Signal(senatorCashierWaitCV, senatorWaitLock);//signal a clerk
    Wait(senatorCashierWaitCV, senatorWaitLock);//wait for a filed application


    hasSenator = FALSE;
    senatorStatus = 0;
    Write("Senator[", sizeof("Senator["), ConsoleOutput);
    Printint(id);
    Write("] is leaving the Passport Office\n", sizeof("] is leaving the Passport Office\n"), ConsoleOutput); //senator is leaving the passport office
    Release(customerWaitLock);
    Release(senatorWaitLock);
}


void PassportOffice() {

    int numApplicationClerk;
    int numPictureClerk;
    int numPassportClerk;
    int numCashier;
    int numCustomer;
    int numSenator;
    Thread * t1;
    char integer[32];

    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation started.\n", sizeof("Passport Office Simulation started.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("]\n", sizeof("]\n"), ConsoleOutput);

    Write("Number of Customers = ", sizeof("Number of Customers = "), ConsoleOutput);
    cin >> numCustomer;
    while (numCustomer > 50 || numCustomer < 0) {
        Write("Customer number should be less than 50.\n", sizeof("Customer number should be less than 50.\n"), ConsoleOutput);
        Write("Number of Customers = ", sizeof("Number of Customers = "), ConsoleOutput);
        cin >> numCustomer;
    }
    remainingCustomer = numCustomer;

    Write("Number of ApplicationClerks = ", sizeof("Number of ApplicationClerks = "), ConsoleOutput);
    cin >> numApplicationClerk;
    while (numApplicationClerk > 5 || numApplicationClerk < 1) {
        Write("Number of ApplicationClerks should be 1-5\n", sizeof("Number of ApplicationClerks should be 1-5\n"), ConsoleOutput);
        Write("Number of ApplicationClerks = \n", sizeof("Number of ApplicationClerks = \n"), ConsoleOutput);
        cin >> numApplicationClerk;
    }

    Write("Number of PictureClerks = ", sizeof("Number of PictureClerks = "), ConsoleOutput);
    cin >> numPictureClerk;
    while (numPictureClerk > 5 || numPictureClerk < 1) {
        Write("Number of PictureClerks should be 1-5\n", sizeof("Number of PictureClerks should be 1-5\n"), ConsoleOutput);
        Write("Number of PictureClerks = \n", sizeof("Number of PictureClerks = \n"), ConsoleOutput);
        cin >> numPictureClerk;
    }

    Write("Number of PassportClerks = ", sizeof("Number of PassportClerks = "), ConsoleOutput);
    cin >> numPassportClerk;
    while (numPassportClerk > 5 || numPassportClerk < 1) {
        Write("Number of PassportClerks should be 1-5\n", sizeof("Number of PassportClerks should be 1-5\n"), ConsoleOutput);
        Write("Number of PassportClerks = ", sizeof("Number of PassportClerks = "), ConsoleOutput);
        cin >> numPassportClerk;
    }

    Write("Number of Cashiers = ", sizeof("Number of Cashiers = "), ConsoleOutput);
    cin >> numCashier;
    while (numApplicationClerk > 5 || numApplicationClerk < 1) {
        Write("Number of Cashiers should be 1-5\n", sizeof("Number of Cashiers should be 1-5\n"), ConsoleOutput);
        Write("Number of Cashiers = ", sizeof("Number of Cashiers = "), ConsoleOutput);
        cin >> numApplicationClerk;
    }


    Write("Number of Senators = ", sizeof("Number of Senators = "), ConsoleOutput);
    cin >> numSenator;
    while (numSenator > 10 || numSenator < 0) {
        Write("Senator number should be less than 10.\n", sizeof("Senator number should be less than 10.\n"), ConsoleOutput);
        Write("Number of Senators = ", sizeof("Number of Senators = "), ConsoleOutput);
        cin >> numSenator;
    }


    //Initialize all variables for all clerks
    for (int i = 0; i < numApplicationClerk; i++) {
        char lockName[100] = "ApplicationLock";
        sprintf(integer, "%d", i );
        strcat(lockName, integer);

        //application lock initialize
        int applicationLock = CreateLock(lockName);
        ApplicationClerkLineLock.push_back(applicationLock);

        //aplication CV initialize
        char applicationCVName[100] = "applicationCV";
        strcat(applicationCVName, integer);
        int applicationCV = CreateCondition(applicationCVName);
        ApplicationClerkLineCV.push_back(applicationCV);

        //application bribe CV initialize
        char applicaitonBribeCVName[100] = "applicaitonBribeCV";
        strcat(applicaitonBribeCVName, integer);
        int applicationBribeCV = CreateCondition(applicaitonBribeCVName);
        ApplicationClerkBribeLineCV.push_back(applicationBribeCV);

        //application Wait CV initialize
        char applicationWaitCVName[100] = "applicationWaitCV";
        strcat(applicationWaitCVName, integer);
        int applicationWaitCV = CreateCondition(applicationWaitCVName);
        ApplicationClerkLineWaitCV.push_back(applicationWaitCV);

        //application Bribe Wait CV initialize
        char applicationBribeWaitCVName[100] = "applicationWaitCV";
        strcat(applicationBribeWaitCVName, integer);
        int applicationBribeWaitCV = CreateCondition(applicationBribeWaitCVName);
        ApplicationClerkBribeLineWaitCV.push_back(applicationBribeWaitCV);

        //application line size intialize
        ApplicationClerkLineCount.push_back(0);
        //application bribe line size initialize
        ApplicationClerkBribeLineCount.push_back(0);
        //application clerk state initialize
        clerkState ct = AVAILABLE;
        ApplicationClerkState.push_back(ct);
        //application data initialize
        ApplicationClerkData.push_back(0);

    }

    for (int i = 0; i < numPictureClerk; i++) {
        char lockName[100] = "PictureLock";
        sprintf(integer, "%d", i );
        strcat(lockName, integer);

        //picture lock initialize
        int pictureLock = CreateLock(lockName);
        pictureClerkLineLock.push_back(pictureLock);

        //picture CV initialize
        char pictureCVName[100] = "pictureCV";
        strcat(pictureCVName, integer);
        int pictureCV = CreateCondition(pictureCVName);
        pictureClerkLineCV.push_back(pictureCV);

        //picture bribe CV initialize
        char pictureBribeCVName[100] = "pictureBribeCV";
        strcat(pictureBribeCVName, integer);
        int pictureBribeCV = CreateCondition(pictureBribeCVName);
        pictureClerkBribeLineCV.push_back(pictureBribeCV);

        //picture Wait CV initialize
        char pictureWaitCVName[100] = "pictureWaitCV";
        strcat(pictureWaitCVName, integer);
        int pictureWaitCV = CreateCondition(pictureWaitCVName);
        pictureClerkLineWaitCV.push_back(pictureWaitCV);

        //picture Bribe Wait CV initialize
        char pictureBribeWaitCVName[100] = "pictureWaitCV";
        strcat(pictureBribeWaitCVName, integer);
        int pictureBribeWaitCV = CreateCondition(pictureBribeWaitCVName);
        pictureClerkBribeLineWaitCV.push_back(pictureBribeWaitCV);

        //picture line size intialize
        pictureClerkLineCount.push_back(0);
        //picture bribe line size initialize
        pictureClerkBribeLineCount.push_back(0);
        //picture clerk state initialize
        clerkState ct = AVAILABLE;
        pictureClerkState.push_back(ct);
        //picture clerk data initialize
        pictureClerkData.push_back(0);
        pictureAcceptance.push_back(0);
    }

    for (int i = 0; i < numPassportClerk; i++) {
        char lockName[100] = "PassportLock";
        sprintf(integer, "%d", i );
        strcat(lockName, integer);

        //passport lock initialize
        int passportLock = CreateLock(lockName);
        passportClerkLineLock.push_back(passportLock);

        //passport CV initialize
        char passportCVName[100] = "passportCV";
        strcat(passportCVName, integer);
        int passportCV = CreateCondition(passportCVName);
        passportClerkLineCV.push_back(passportCV);

        //passport bribe CV initialize
        char passportBribeCVName[100] = "passportBribeCV";
        strcat(passportBribeCVName, integer);
        int passportBribeCV = CreateCondition(passportBribeCVName);
        passportClerkBribeLineCV.push_back(passportBribeCV);

        //passport Wait CV initialize
        char passportWaitCVName[100] = "passportWaitCV";
        strcat(passportWaitCVName, integer);
        int passportWaitCV = CreateCondition(passportWaitCVName);
        passportClerkLineWaitCV.push_back(passportWaitCV);

        //passport Bribe Wait CV initialize
        char passportBribeWaitCVName[100] = "passportWaitCV";
        strcat(passportBribeWaitCVName, integer);
        int passportBribeWaitCV = CreateCondition(passportBribeWaitCVName);
        passportClerkBribeLineWaitCV.push_back(passportBribeWaitCV);

        //passport line size intialize
        passportClerkLineCount.push_back(0);
        //passport bribe line size initialize
        passportClerkBribeLineCount.push_back(0);
        //passport clerk state initialize
        clerkState ct = AVAILABLE;
        passportClerkState.push_back(ct);
        //passport data initialize
        passportClerkCustomerId.push_back(0);
    }

    for (int i = 0; i < numCashier; i++) {
        char lockName[100] = "CashierLock";
        sprintf(integer, "%d", i );
        strcat(lockName, integer);

        //cashier lock initialize
        int CashierLock = CreateLock(lockName);
        CashierLineLock.push_back(CashierLock);

        //cashier CV initialize
        char CashierCVName[100] = "cashierCV";
        strcat(CashierCVName, integer);
        int CashierCV = CreateCondition(CashierCVName);
        CashierLineCV.push_back(CashierCV);

        //cashier Wait CV initialize
        char CashierWaitCVName[100] = "cashierWaitCV";
        strcat(CashierWaitCVName, integer);
        int CashierWaitCV = CreateCondition(CashierWaitCVName);
        CashierLineWaitCV.push_back(CashierWaitCV);

        //cashier line size intialize
        CashierLineCount.push_back(0);
        //cashier state initialize
        clerkState ct = AVAILABLE;
        CashierState.push_back(ct);
        //cashier data initialize
        CashierCustomerId.push_back(0);
    }

    senatorWaitLock = CreateLock("senator");
    senatorApplicationWaitLock = CreateLock("senatorApplicationWaitLock");
    senatorPictureWaitLock = CreateLock("senatorPictureWaitLock");
    senatorPassportWaitLock = CreateLock("senatorPassportWaitLock");
    senatorCashierWaitLock = CreateLock("senatorCashierWaitLock");
    senatorApplicationWaitCV = CreateCondition("senatorApplicationWaitCV");
    senatorPictureWaitCV = CreateCondition("senatorPictureWaitCV");
    senatorPassportWaitCV = CreateCondition("senatorPassportWaitCV");
    senatorCashierWaitCV = CreateCondition("senatorCashierWaitCV");
    customerWaitCV = CreateCondition("customerCV");
    customerWaitLock = CreateLock("customerLock");

    for (int i = 0 ; i < numCustomer; i++) {
        customerApplicationStatus.push_back(0);

    }

    //Initialize all threads
    for (int i = 0; i < numApplicationClerk; i++) {
        char threadName[100] = "ApplicationClerk";
        sprintf(integer, "%d", i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)ApplicationClerk, i);

    }

    for (int i = 0; i < numPictureClerk; i++) {
        char threadName[100] = "PictureClerk";
        sprintf(integer, "%d", i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)PictureClerk, i);
    }

    for (int i = 0; i < numPassportClerk; i++) {
        char threadName[100] = "PassportClerk";
        sprintf(integer, "%d", i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)PassportClerk, i);
    }

    for (int i = 0; i < numCashier; i++) {
        char threadName[100] = "Cashier";
        sprintf(integer, "%d", i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)Cashier, i);
    }

    for (int i = 0; i < numCustomer; i++) {
        char threadName[100] = "Customer";
        sprintf(integer, "%d", i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)Customer, 0);
    }


    t1 = new Thread("Manager");
    t1->Fork((VoidFunctionPtr)Manager, 0);


    for (int i = 0; i < numSenator; i++) {
        char threadName[100] = "Senator";
        sprintf(integer, "%d", i );
        strcat(threadName, integer);
        t1 = new Thread(threadName);
        t1->Fork((VoidFunctionPtr)Senator, 0);
    }

    Exit(0);

}

int main() {
    Exec(PassportOffice);
    return 0;

}

