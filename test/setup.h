#ifndef SYSCALLS_H
#define SYSCALLS_H
#include "syscall.h"

#define APPLICATIONCLERK_SIZE 5
#define PICTURECLERK_SIZE 5
#define PASSPORTCLERK_SIZE 5
#define CASHIER_SIZE 5
#define CUSTOMER_SIZE 20
#define SENATOR_SIZE 10
#define MANAGER_SIZE 1
#define RAND_UPPER_LIMIT 10000


typedef enum {AVAILABLE, BUSY, ONBREAK} clerkState;

int printLock;

int ClerkLineLock;
int incrementCount;
int customerApplicationStatus[CUSTOMER_SIZE];

/* variables for application clerk */
int ApplicationClerkLineLockArray;
int ApplicationClerkLineCVArray;
int ApplicationClerkLineWaitCVArray;
int ApplicationClerkBribeLineCVArray;
int ApplicationClerkBribeLineWaitCVArray;
int ApplicationClerkLineCountArray;
int ApplicationClerkBribeLineCountArray;
int ApplicationClerkStateArray;
int ApplicationClerkDataArray;

/* variables for picture clerks. */
int pictureClerkLineLockArray;
int pictureClerkLineCVArray;
int pictureClerkLineWaitCVArray;
int pictureClerkBribeLineCVArray;
int pictureClerkBribeLineWaitCVArray;
int pictureClerkLineCountArray;
int pictureClerkBribeLineCountArray;
int pictureClerkStateArray;
int pictureClerkDataArray;
int pictureAcceptanceArray;

/* variables for PassportClerk */
int passportClerkCustomerIdArray;
int passportClerkStateArray;
int passportClerkLineLockArray;
int passportClerkLineCVArray;
int passportClerkLineWaitCVArray;
int passportClerkBribeLineCVArray;
int passportClerkBribeLineWaitCVArray;
int passportClerkLineCountArray;
int passportClerkBribeLineCountArray;

/* variables for Cashier */
int CashierCustomerIdArray;
int CashierStateArray;
int CashierLineLockArray;
int CashierLineCVArray;
int CashierLineWaitCVArray;
int CashierLineCountArray;

/* variables for Manager */
/* each money variable needs a lock */
int applicationMoneyLock;
int pictureMoneyLock;
int passportMoneyLock;
int cashierMoneyLock;
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
int hasSenator = 0;


int customerNum = -1; /*  number of customers came into the office. */
int appClerkNum = -1;
int picClerkNum = -1;
int passClerkNum = -1;
int cashierNum = -1;

/*  it is also ssn of customers */
int remainingCustomer = 0; /*  number of customers still in the office */


int senatorNum = -1;
int randomNum;


/* Variables for PassportOffice() */
int applicationLock;
int applicationCV;
int applicationBribeCV;
int applicationWaitCV;
int applicationBribeWaitCV;

int pictureLock;
int pictureCV;
int pictureBribeCV;
int pictureWaitCV;
int pictureBribeWaitCV;

int passportLock;
int passportCV;
int passportBribeCV;
int passportWaitCV;
int passportBribeWaitCV;

int CashierLock;
int CashierCV;
int CashierWaitCV;

/* Variables for Customer */
int shortestPictureLine;
int shortestPictureBribeLine;
int shortestPictureBribeLineSize;
int shortestPictureLineSize;
int shortestApplicationBribeLine;
int shortestApplicationBribeLineSize;
int shortestApplicationLine;
int shortestApplicationLineSize;
int shortestPassportBribeLine;
int shortestPassportBribeLineSize;
int shortestPassportLine;
int shortestPassportLineSize;
int shortestCashierLine;
int shortestCashierLineSize;

char *applicationLockName = "ApplicationLock ";
char *applicationCVName = "applicationCV ";
char *applicationBribeCVName = "applicaitonBribeCV ";
char *applicationWaitCVName = "applicationWaitCV ";
char *applicationBribeWaitCVName = "applicationBribeWaitCV ";
char *pictureLockName = "PictureLock ";
char *pictureCVName = "pictureCV ";
char *pictureBribeCVName = "pictureBribeCV ";
char *pictureWaitCVName = "pictureWaitCV ";
char *pictureBribeWaitCVName = "pictureBribeWaitCV ";
char *passportLockName = "PassportLock ";
char *passportCVName = "passportCV ";
char *passportBribeCVName = "passportBribeCV ";
char *passportWaitCVName = "passportWaitCV ";
char *passportBribeWaitCVName = "passportBribeWaitCV ";
char *CashierLockName = "CashierLock ";
char *CashierCVName = "CashierCV ";
char *CashierWaitCVName = "CashierWaitCV ";

void setup(){

    unsigned int i;

    clerkState ct;

    printLock = CreateLockServer("printLock", sizeof("printLock"));

    ClerkLineLock = CreateLockServer("ClerkLineLock", sizeof("ClerkLineLock"));
    incrementCount = CreateLockServer("incrementCount", sizeof("incrementCount"));
    applicationMoneyLock = CreateLockServer("applicationMoneyLock", sizeof("applicationMoneyLock"));
    pictureMoneyLock = CreateLockServer("pictureMoneyLock", sizeof("pictureMoneyLock"));
    passportMoneyLock = CreateLockServer("passportMoneyLock", sizeof("passportMoneyLock"));
    cashierMoneyLock = CreateLockServer("cashierMoneyLock", sizeof("cashierMoneyLock"));




	ApplicationClerkLineLockArray = CreateMVArrayServer("ApplicationClerkLineLockArray", sizeof("ApplicationClerkLineLockArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkLineCVArray = CreateMVArrayServer("ApplicationClerkLineCVArray", sizeof("ApplicationClerkLineCVArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkLineWaitCVArray = CreateMVArrayServer("ApplicationClerkLineWaitCVArray", sizeof("ApplicationClerkLineWaitCVArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkBribeLineCVArray = CreateMVArrayServer("ApplicationClerkBribeLineCVArray", sizeof("ApplicationClerkBribeLineCVArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkBribeLineWaitCVArray = CreateMVArrayServer("ApplicationClerkBribeLineWaitCVArray", sizeof("ApplicationClerkBribeLineWaitCVArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkLineCountArray = CreateMVArrayServer("ApplicationClerkLineCountArray", sizeof("ApplicationClerkLineCountArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkBribeLineCountArray = CreateMVArrayServer("ApplicationClerkBribeLineCountArray", sizeof("ApplicationClerkBribeLineCountArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkStateArray = CreateMVArrayServer("ApplicationClerkStateArray", sizeof("ApplicationClerkStateArray"), APPLICATIONCLERK_SIZE);
	ApplicationClerkDataArray = CreateMVArrayServer("ApplicationClerkDataArray", sizeof("ApplicationClerkDataArray"), APPLICATIONCLERK_SIZE);

	pictureClerkLineLockArray = CreateMVArrayServer("pictureClerkLineLockArray", sizeof("pictureClerkLineLockArray"), PICTURECLERK_SIZE);
	pictureClerkLineCVArray = CreateMVArrayServer("pictureClerkLineCVArray", sizeof("pictureClerkLineCVArray"), PICTURECLERK_SIZE);
	pictureClerkLineWaitCVArray = CreateMVArrayServer("pictureClerkLineWaitCVArray", sizeof("pictureClerkLineWaitCVArray"), PICTURECLERK_SIZE);
	pictureClerkBribeLineCVArray = CreateMVArrayServer("pictureClerkBribeLineCVArray", sizeof("pictureClerkBribeLineCVArray"), PICTURECLERK_SIZE);
	pictureClerkBribeLineWaitCVArray = CreateMVArrayServer("pictureClerkBribeLineWaitCVArray", sizeof("pictureClerkBribeLineWaitCVArray"), PICTURECLERK_SIZE);
	pictureClerkLineCountArray = CreateMVArrayServer("pictureClerkLineCountArray", sizeof("pictureClerkLineCountArray"), PICTURECLERK_SIZE);
	pictureClerkBribeLineCountArray = CreateMVArrayServer("pictureClerkBribeLineCountArray", sizeof("pictureClerkBribeLineCountArray"), PICTURECLERK_SIZE);
	pictureClerkStateArray = CreateMVArrayServer("pictureClerkStateArray", sizeof("pictureClerkStateArray"), PICTURECLERK_SIZE);
	pictureClerkDataArray = CreateMVArrayServer("pictureClerkDataArray", sizeof("pictureClerkDataArray"), PICTURECLERK_SIZE);
	pictureAcceptanceArray = CreateMVArrayServer("pictureAcceptanceArray", sizeof("pictureAcceptanceArray"), PICTURECLERK_SIZE);

	passportClerkCustomerIdArray = CreateMVArrayServer("passportClerkCustomerIdArray", sizeof("passportClerkCustomerIdArray"), PASSPORTCLERK_SIZE);
	passportClerkStateArray = CreateMVArrayServer("passportClerkStateArray", sizeof("passportClerkStateArray"), PASSPORTCLERK_SIZE);
	passportClerkLineLockArray = CreateMVArrayServer("passportClerkLineLockArray", sizeof("passportClerkLineLockArray"), PASSPORTCLERK_SIZE);
	passportClerkLineCVArray = CreateMVArrayServer("passportClerkLineCVArray", sizeof("passportClerkLineCVArray"), PASSPORTCLERK_SIZE);
	passportClerkLineWaitCVArray = CreateMVArrayServer("passportClerkLineWaitCVArray", sizeof("passportClerkLineWaitCVArray"), PASSPORTCLERK_SIZE);
	passportClerkBribeLineCVArray = CreateMVArrayServer("passportClerkBribeLineCVArray", sizeof("passportClerkBribeLineCVArray"), PASSPORTCLERK_SIZE);
	passportClerkBribeLineWaitCVArray = CreateMVArrayServer("passportClerkBribeLineWaitCVArray", sizeof("passportClerkBribeLineWaitCVArray"), PASSPORTCLERK_SIZE);
	passportClerkLineCountArray = CreateMVArrayServer("passportClerkLineCountArray", sizeof("passportClerkLineCountArray"), PASSPORTCLERK_SIZE);
	passportClerkBribeLineCountArray = CreateMVArrayServer("passportClerkBribeLineCountArray", sizeof("passportClerkBribeLineCountArray"), PASSPORTCLERK_SIZE);

	CashierCustomerIdArray = CreateMVArrayServer("CashierCustomerIdArray", sizeof("CashierCustomerIdArray"), CASHIER_SIZE);
	CashierStateArray = CreateMVArrayServer("CashierStateArray", sizeof("CashierStateArray"), CASHIER_SIZE);
	CashierLineLockArray = CreateMVArrayServer("CashierLineLockArray", sizeof("CashierLineLockArray"), CASHIER_SIZE);
	CashierLineCVArray = CreateMVArrayServer("CashierLineCVArray", sizeof("CashierLineCVArray"), CASHIER_SIZE);
	CashierLineWaitCVArray = CreateMVArrayServer("CashierLineWaitCVArray", sizeof("CashierLineWaitCVArray"), CASHIER_SIZE);
	CashierLineCountArray = CreateMVArrayServer("CashierLineCountArray", sizeof("CashierLineCountArray"), CASHIER_SIZE);



    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("Passport Office Simulation started.\n", sizeof("Passport Office Simulation started.\n"), ConsoleOutput);
    Write("===================================\n", sizeof("===================================\n"), ConsoleOutput);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of Customers = ", sizeof("Number of Customers = "), ConsoleOutput);
    Printint(CUSTOMER_SIZE);
    Write("\n", sizeof("\n"), ConsoleOutput);
    remainingCustomer = CUSTOMER_SIZE;

    Write("Number of ApplicationClerks = ", sizeof("Number of ApplicationClerks = "), ConsoleOutput);
    Printint(APPLICATIONCLERK_SIZE);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of PictureClerks = ", sizeof("Number of PictureClerks = "), ConsoleOutput);
    Printint(PICTURECLERK_SIZE);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of PassportClerks = ", sizeof("Number of PassportClerks = "), ConsoleOutput);
    Printint(PASSPORTCLERK_SIZE);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of Cashiers = ", sizeof("Number of Cashiers = "), ConsoleOutput);
    Printint(CASHIER_SIZE);
    Write("\n", sizeof("\n"), ConsoleOutput);

    Write("Number of Senators = ", sizeof("Number of Senators = "), ConsoleOutput);
    Printint(SENATOR_SIZE);
    Write("\n", sizeof("\n"), ConsoleOutput);

    for (i = 0; i < CUSTOMER_SIZE; i++) {
        numCustomerWaiting[i] = -1;
    }


    /* Initialize all variables for all clerks */
    for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {

        /* application lock initialize */
        applicationLockName[15] = '0' + i;
        ApplicationClerkLineLock[i] = applicationLock;

        /* aplication CV initialize */
        applicationCVName[13] = '0' + i;
        ApplicationClerkLineCV[i] = applicationCV;

        /* application bribe CV initialize */
        applicationBribeCVName[18] = '0' + i;
        ApplicationClerkBribeLineCV[i] = applicationBribeCV;

        /* application Wait CV initialize */
        applicationWaitCVName[17] = '0' + i;
        ApplicationClerkLineWaitCV[i] = applicationWaitCV;

        /* application Bribe Wait CV initialize */
        applicationBribeWaitCVName[22] = '0' + i;
        ApplicationClerkBribeLineWaitCV[i] = applicationBribeWaitCV;

        /* application line size intialize */
        ApplicationClerkLineCount[i] = 0;
        /* application bribe line size initialize */
        ApplicationClerkBribeLineCount[i] = 0;
        /* application clerk state initialize */
        ct = AVAILABLE;
        ApplicationClerkState[i] = ct;
        /* application data initialize */
        ApplicationClerkData[i] = 0;

    }

    for ( i = 0; i < PICTURECLERK_SIZE; i++) {

        /* picture lock initialize */
        pictureLockName[11] = '0' + i;
        pictureClerkLineLock[i] = pictureLock;

        /* picture CV initialize */
        pictureCVName[9] = '0' + i;
        pictureClerkLineCV[i] = pictureCV;

        /* picture bribe CV initialize */
        pictureBribeCVName[14] = '0' + i;
        pictureClerkBribeLineCV[i] = pictureBribeCV;

        /* picture Wait CV initialize */
        pictureWaitCVName[13] = '0' + i;
        pictureClerkLineWaitCV[i] = pictureWaitCV;

        /* picture Bribe Wait CV initialize */
        pictureBribeWaitCVName[18] = '0' + i;
        pictureClerkBribeLineWaitCV[i] = pictureBribeWaitCV;

        /* picture line size intialize */
        pictureClerkLineCount[i] = 0;
        /* picture bribe line size initialize */
        pictureClerkBribeLineCount[i] = 0;
        /* picture clerk state initialize */
        ct = AVAILABLE;
        pictureClerkState[i] = ct;
        /* picture clerk data initialize */
        pictureClerkData[i] = 0;
        pictureAcceptance[i] = 0;
    }

    for ( i = 0; i < PASSPORTCLERK_SIZE; i++) {

        /* passport lock initialize */
        passportLockName[12] = '0' + i;
        passportClerkLineLock[i] = passportLock;

        /* passport CV initialize */
        passportCVName[10] = '0' + i;
        passportClerkLineCV[i] = passportCV;

        /* passport bribe CV initialize */
        passportBribeCVName[15] = '0' + i;
        passportClerkBribeLineCV[i] = passportBribeCV;

        /* passport Wait CV initialize */
        passportWaitCVName[14] = '0' + i;
        passportClerkLineWaitCV[i] = passportWaitCV;

        /* passport Bribe Wait CV initialize */
        passportBribeWaitCVName[19] = '0' + i;
        passportClerkBribeLineWaitCV[i] = passportBribeWaitCV;

        /* passport line size intialize */
        passportClerkLineCount[i] = 0;
        /* passport bribe line size initialize */
        passportClerkBribeLineCount[i] = 0;
        /* passport clerk state initialize */
        ct = AVAILABLE;
        passportClerkState[i] = ct;
        /* passport data initialize */
        passportClerkCustomerId[i] = 0;
    }

    for ( i = 0; i < CASHIER_SIZE; i++) {

        /* cashier lock initialize */
        CashierLockName[11] = '0' + i;
        CashierLineLock[i] = CashierLock;

        /* cashier CV initialize */
        CashierCVName[9] = '0' + i;
        CashierLineCV[i] = CashierCV;

        /* cashier Wait CV initialize */
        CashierWaitCVName[13] = '0' + i;
        CashierLineWaitCV[i] = CashierWaitCV;

        /* cashier line size intialize */
        CashierLineCount[i] = 0;
        /* cashier state initialize */
        ct = AVAILABLE;
        CashierState[i] = ct;
        /* cashier data initialize */
        CashierCustomerId[i] = 0;
    }

    senatorWaitLock = CreateLockServer("senator", sizeof("senator"));
    senatorApplicationWaitLock = CreateLockServer("senatorApplicationWaitLock", sizeof("senatorApplicationWaitLock"));
    senatorPictureWaitLock = CreateLockServer("senatorPictureWaitLock", sizeof("senatorPictureWaitLock"));
    senatorPassportWaitLock = CreateLockServer("senatorPassportWaitLock", sizeof("senatorPassportWaitLock"));
    senatorCashierWaitLock = CreateLockServer("senatorCashierWaitLock", sizeof("senatorCashierWaitLock"));
    senatorApplicationWaitCV = CreateConditionServer("senatorApplicationWaitCV", sizeof("senatorApplicationWaitCV"));
    senatorPictureWaitCV = CreateConditionServer("senatorPictureWaitCV", sizeof("senatorPictureWaitCV"));
    senatorPassportWaitCV = CreateConditionServer("senatorPassportWaitCV", sizeof("senatorPassportWaitCV"));
    senatorCashierWaitCV = CreateConditionServer("senatorCashierWaitCV", sizeof("senatorCashierWaitCV"));
    customerWaitCV = CreateConditionServer("customerCV", sizeof("customerCV"));
    customerWaitLock = CreateLockServer("customerLock", sizeof("customerLock"));

    Exit(0);
}
#endif