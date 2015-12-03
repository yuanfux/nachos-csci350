#ifndef SETUP_H
#define SETUP_H
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
int customerApplicationStatusArray;

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

int numCustomerWaitingArray;


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

/* Money Variables */
int MoneyFromApplicationClerk;
int MoneyFromPictureClerk;
int MoneyFromPassportClerk;
int MoneyFromCashier;
int MoneyTotal;

/* Other Variables */
int hasSenator;

int customerNum;
int appClerkNum;
int picClerkNum;
int passClerkNum;
int cashierNum;

int remainingCustomer;

int senatorData;
int senatorServiceId;
int senatorStatus;
int senatorNum;

/* Name Variables */
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

char *ApplicationClerkLineLockName = "ApplicationClerkLineLock";
char *ApplicationClerkLineCVName = "ApplicationClerkLineCV";
char *ApplicationClerkBribeLineCVName = "ApplicationClerkBribeLineCV";
char *ApplicationClerkLineWaitCVName = "ApplicationClerkLineWaitCV";
char *ApplicationClerkBribeLineWaitCVName = "ApplicationClerkBribeLineWaitCV";
char *pictureClerkLineLockName = "pictureClerkLineLock";
char *pictureClerkLineCVName = "pictureClerkLineCV";
char *pictureClerkBribeLineCVName = "pictureClerkBribeLineCV";
char *pictureClerkLineWaitCVName = "pictureClerkLineWaitCV";
char *pictureClerkBribeLineWaitCVName = "pictureClerkBribeLineWaitCV";
char *passportClerkLineLockName = "passportClerkLineLock";
char *passportClerkLineCVName = "passportClerkLineCV";
char *passportClerkBribeLineCVName = "passportClerkBribeLineCV";
char *passportClerkLineWaitCVName = "passportClerkLineWaitCV";
char *passportClerkBribeLineWaitCVName = "passportClerkBribeLineWaitCV";
char *CashierLineLockName = "CashierLineLock";
char *CashierLineCVName = "CashierLineCV";
char *CashierLineWaitCVName = "CashierLineWaitCV";


void setup() {

	unsigned int i;

	clerkState ct;
	ct = AVAILABLE;

	printLock = CreateLockServer("printLock", sizeof("printLock"));

	ClerkLineLock = CreateLockServer("ClerkLineLock", sizeof("ClerkLineLock"));
	incrementCount = CreateLockServer("incrementCount", sizeof("incrementCount"));
	applicationMoneyLock = CreateLockServer("applicationMoneyLock", sizeof("applicationMoneyLock"));
	pictureMoneyLock = CreateLockServer("pictureMoneyLock", sizeof("pictureMoneyLock"));
	passportMoneyLock = CreateLockServer("passportMoneyLock", sizeof("passportMoneyLock"));
	cashierMoneyLock = CreateLockServer("cashierMoneyLock", sizeof("cashierMoneyLock"));


	ApplicationClerkLineLockArray = CreateMVArrayServer("ApplicationClerkLineLock", sizeof("ApplicationClerkLineLock"), APPLICATIONCLERK_SIZE);
	ApplicationClerkLineCVArray = CreateMVArrayServer("ApplicationClerkLineCV", sizeof("ApplicationClerkLineCV"), APPLICATIONCLERK_SIZE);
	ApplicationClerkLineWaitCVArray = CreateMVArrayServer("ApplicationClerkLineWaitCV", sizeof("ApplicationClerkLineWaitCV"), APPLICATIONCLERK_SIZE);
	ApplicationClerkBribeLineCVArray = CreateMVArrayServer("ApplicationClerkBribeLineCV", sizeof("ApplicationClerkBribeLineCV"), APPLICATIONCLERK_SIZE);
	ApplicationClerkBribeLineWaitCVArray = CreateMVArrayServer("ApplicationClerkBribeLineWaitCV", sizeof("ApplicationClerkBribeLineWaitCV"), APPLICATIONCLERK_SIZE);
	ApplicationClerkLineCountArray = CreateMVArrayServer("ApplicationClerkLineCount", sizeof("ApplicationClerkLineCount"), APPLICATIONCLERK_SIZE);
	ApplicationClerkBribeLineCountArray = CreateMVArrayServer("ApplicationClerkBribeLineCount", sizeof("ApplicationClerkBribeLineCount"), APPLICATIONCLERK_SIZE);
	ApplicationClerkStateArray = CreateMVArrayServer("ApplicationClerkState", sizeof("ApplicationClerkState"), APPLICATIONCLERK_SIZE);
	ApplicationClerkDataArray = CreateMVArrayServer("ApplicationClerkData", sizeof("ApplicationClerkData"), APPLICATIONCLERK_SIZE);

	pictureClerkLineLockArray = CreateMVArrayServer("pictureClerkLineLock", sizeof("pictureClerkLineLock"), PICTURECLERK_SIZE);
	pictureClerkLineCVArray = CreateMVArrayServer("pictureClerkLineCV", sizeof("pictureClerkLineCV"), PICTURECLERK_SIZE);
	pictureClerkLineWaitCVArray = CreateMVArrayServer("pictureClerkLineWaitCV", sizeof("pictureClerkLineWaitCV"), PICTURECLERK_SIZE);
	pictureClerkBribeLineCVArray = CreateMVArrayServer("pictureClerkBribeLineCV", sizeof("pictureClerkBribeLineCV"), PICTURECLERK_SIZE);
	pictureClerkBribeLineWaitCVArray = CreateMVArrayServer("pictureClerkBribeLineWaitCV", sizeof("pictureClerkBribeLineWaitCV"), PICTURECLERK_SIZE);
	pictureClerkLineCountArray = CreateMVArrayServer("pictureClerkLineCount", sizeof("pictureClerkLineCount"), PICTURECLERK_SIZE);
	pictureClerkBribeLineCountArray = CreateMVArrayServer("pictureClerkBribeLineCount", sizeof("pictureClerkBribeLineCount"), PICTURECLERK_SIZE);
	pictureClerkStateArray = CreateMVArrayServer("pictureClerkState", sizeof("pictureClerkState"), PICTURECLERK_SIZE);
	pictureClerkDataArray = CreateMVArrayServer("pictureClerkData", sizeof("pictureClerkData"), PICTURECLERK_SIZE);
	pictureAcceptanceArray = CreateMVArrayServer("pictureAcceptance", sizeof("pictureAcceptance"), PICTURECLERK_SIZE);


	passportClerkCustomerIdArray = CreateMVArrayServer("passportClerkCustomerId", sizeof("passportClerkCustomerId"), PASSPORTCLERK_SIZE);
	passportClerkStateArray = CreateMVArrayServer("passportClerkState", sizeof("passportClerkState"), PASSPORTCLERK_SIZE);
	passportClerkLineLockArray = CreateMVArrayServer("passportClerkLineLock", sizeof("passportClerkLineLock"), PASSPORTCLERK_SIZE);
	passportClerkLineCVArray = CreateMVArrayServer("passportClerkLineCV", sizeof("passportClerkLineCV"), PASSPORTCLERK_SIZE);
	passportClerkLineWaitCVArray = CreateMVArrayServer("passportClerkLineWaitCV", sizeof("passportClerkLineWaitCV"), PASSPORTCLERK_SIZE);
	passportClerkBribeLineCVArray = CreateMVArrayServer("passportClerkBribeLineCV", sizeof("passportClerkBribeLineCV"), PASSPORTCLERK_SIZE);
	passportClerkBribeLineWaitCVArray = CreateMVArrayServer("passportClerkBribeLineWaitCV", sizeof("passportClerkBribeLineWaitCV"), PASSPORTCLERK_SIZE);
	passportClerkLineCountArray = CreateMVArrayServer("passportClerkLineCount", sizeof("passportClerkLineCount"), PASSPORTCLERK_SIZE);
	passportClerkBribeLineCountArray = CreateMVArrayServer("passportClerkBribeLineCount", sizeof("passportClerkBribeLineCount"), PASSPORTCLERK_SIZE);


	CashierCustomerIdArray = CreateMVArrayServer("CashierCustomerId", sizeof("CashierCustomerId"), CASHIER_SIZE);
	CashierStateArray = CreateMVArrayServer("CashierState", sizeof("CashierState"), CASHIER_SIZE);
	CashierLineLockArray = CreateMVArrayServer("CashierLineLock", sizeof("CashierLineLock"), CASHIER_SIZE);
	CashierLineCVArray = CreateMVArrayServer("CashierLineCV", sizeof("CashierLineCV"), CASHIER_SIZE);
	CashierLineWaitCVArray = CreateMVArrayServer("CashierLineWaitCV", sizeof("CashierLineWaitCV"), CASHIER_SIZE);
	CashierLineCountArray = CreateMVArrayServer("CashierLineCount", sizeof("CashierLineCount"), CASHIER_SIZE);

	numCustomerWaitingArray = CreateMVArrayServer("numCustomerWaiting", sizeof("numCustomerWaiting"), CUSTOMER_SIZE);
	customerApplicationStatusArray = CreateMVArrayServer("customerApplicationStatus", sizeof("customerApplicationStatus"), CUSTOMER_SIZE);

	MoneyFromApplicationClerk = CreateMVServer("MoneyFromApplicationClerk", sizeof("MoneyFromApplicationClerk"), 0);
	MoneyFromPictureClerk = CreateMVServer("MoneyFromPictureClerk", sizeof("MoneyFromPictureClerk"), 0);
	MoneyFromPassportClerk = CreateMVServer("MoneyFromPassportClerk", sizeof("MoneyFromPassportClerk"), 0);
	MoneyFromCashier = CreateMVServer("MoneyFromCashier", sizeof("MoneyFromCashier"), 0);
	MoneyTotal = CreateMVServer("MoneyTotal", sizeof("MoneyTotal"), 0);

	hasSenator = CreateMVServer("hasSenator", sizeof("hasSenator"), 0);

	customerNum = CreateMVServer("customerNum", sizeof("customerNum"), -1);
	appClerkNum = CreateMVServer("appClerkNum", sizeof("appClerkNum"), -1);
	picClerkNum = CreateMVServer("picClerkNum", sizeof("picClerkNum"), -1);
	passClerkNum = CreateMVServer("passClerkNum", sizeof("passClerkNum"), -1);
	cashierNum = CreateMVServer("cashierNum", sizeof("cashierNum"), -1);

	remainingCustomer = CreateMVServer("remainingCustomer", sizeof("remainingCustomer"), 0);

	senatorData; CreateMVServer("senatorData", sizeof("senatorData"), -1);
	senatorStatus = CreateMVServer("senatorStatus", sizeof("senatorStatus"), 0);
	senatorNum = CreateMVServer("senatorNum", sizeof("senatorNum"), -1);

	for (i = 0; i < CUSTOMER_SIZE; i++) {
		SetMVArrayServer(numCustomerWaitingArray, i, -1);
	}

	for (i = 0 ; i < CUSTOMER_SIZE; i++) {
		SetMVArrayServer(customerApplicationStatusArray, i, 0);

	}

	/* Initialize all variables for all clerks */
	for (i = 0; i < APPLICATIONCLERK_SIZE; i++) {

		/* application lock initialize */
		applicationLockName[15] = '0' + i;
		applicationLock = CreateLockServer(applicationLockName, 16);
		SetMVArrayServer(ApplicationClerkLineLockArray, i, applicationLock);

		/* aplication CV initialize */
		applicationCVName[13] = '0' + i;
		applicationCV = CreateConditionServer(applicationCVName, 14);
		SetMVArrayServer(ApplicationClerkLineCVArray, i, applicationCV);

		/* application bribe CV initialize */
		applicationBribeCVName[18] = '0' + i;
		applicationBribeCV = CreateConditionServer(applicationBribeCVName, 19);
		SetMVArrayServer(ApplicationClerkBribeLineCVArray, i, applicationBribeCV);

		/* application Wait CV initialize */
		applicationWaitCVName[17] = '0' + i;
		applicationWaitCV = CreateConditionServer(applicationWaitCVName, 18);
		SetMVArrayServer(ApplicationClerkLineWaitCVArray, i, applicationWaitCV);

		/* application Bribe Wait CV initialize */
		applicationBribeWaitCVName[22] = '0' + i;
		applicationBribeWaitCV = CreateConditionServer(applicationBribeWaitCVName, 23);
		SetMVArrayServer(ApplicationClerkBribeLineWaitCVArray, i, applicationBribeWaitCV);

		/* application line size intialize */
		SetMVArrayServer(ApplicationClerkLineCountArray, i, 0);
		/* application bribe line size initialize */
		SetMVArrayServer(ApplicationClerkBribeLineCountArray, i, 0);
		/* application clerk state initialize */
		SetMVArrayServer(ApplicationClerkStateArray, i, ct);
		/* application data initialize */
		SetMVArrayServer(ApplicationClerkDataArray, i, 0);

	}

	for ( i = 0; i < PICTURECLERK_SIZE; i++) {

		/* picture lock initialize */
		pictureLockName[11] = '0' + i;
		pictureLock = CreateLockServer(pictureLockName, 12);
		SetMVArrayServer(pictureClerkLineLockArray, i, pictureLock);

		/* picture CV initialize */
		pictureCVName[9] = '0' + i;
		pictureCV = CreateConditionServer(pictureCVName, 10);
		SetMVArrayServer(pictureClerkLineCVArray, i, pictureCV);

		/* picture bribe CV initialize */
		pictureBribeCVName[14] = '0' + i;
		pictureBribeCV = CreateConditionServer(pictureBribeCVName, 15);
		SetMVArrayServer(pictureClerkBribeLineCVArray, i, pictureBribeCV);

		/* picture Wait CV initialize */
		pictureWaitCVName[13] = '0' + i;
		pictureWaitCV = CreateConditionServer(pictureWaitCVName, 14);
		SetMVArrayServer(pictureClerkLineWaitCVArray, i, pictureWaitCV);

		/* picture Bribe Wait CV initialize */
		pictureBribeWaitCVName[18] = '0' + i;
		pictureBribeWaitCV = CreateConditionServer(pictureBribeWaitCVName, 19);
		SetMVArrayServer(pictureClerkBribeLineWaitCVArray, i, pictureBribeWaitCV);

		/* picture line size intialize */
		SetMVArrayServer(pictureClerkLineCountArray, i, 0);
		/* picture bribe line size initialize */
		SetMVArrayServer(pictureClerkBribeLineCountArray, i, 0);
		/* picture clerk state initialize */
		SetMVArrayServer(pictureClerkStateArray, i, ct);
		/* picture clerk data initialize */
		SetMVArrayServer(pictureClerkDataArray, i, 0);
		SetMVArrayServer(pictureAcceptanceArray, i, 0);
	}

	for ( i = 0; i < PASSPORTCLERK_SIZE; i++) {

		/* passport lock initialize */
		passportLockName[12] = '0' + i;
		passportLock = CreateLockServer(passportLockName, 13);
		SetMVArrayServer(passportClerkLineLockArray, i, passportLock);

		/* passport CV initialize */
		passportCVName[10] = '0' + i;
		passportCV = CreateConditionServer(passportCVName, 11);
		SetMVArrayServer(passportClerkLineCVArray, i, passportCV);

		/* passport bribe CV initialize */
		passportBribeCVName[15] = '0' + i;
		passportBribeCV = CreateConditionServer(passportBribeCVName, 16);
		SetMVArrayServer(passportClerkBribeLineCVArray, i, passportBribeCV);

		/* passport Wait CV initialize */
		passportWaitCVName[14] = '0' + i;
		passportWaitCV = CreateConditionServer(passportWaitCVName, 15);
		SetMVArrayServer(passportClerkLineWaitCVArray, i, passportWaitCV);

		/* passport Bribe Wait CV initialize */
		passportBribeWaitCVName[19] = '0' + i;
		passportBribeWaitCV = CreateConditionServer(passportBribeWaitCVName, 20);
		SetMVArrayServer(passportClerkBribeLineWaitCVArray, i, passportBribeWaitCV);

		/* passport line size intialize */
		SetMVArrayServer(passportClerkLineCountArray, i, 0);
		/* passport bribe line size initialize */
		SetMVArrayServer(passportClerkBribeLineCountArray, i, 0);
		/* passport clerk state initialize */
		SetMVArrayServer(passportClerkStateArray, i, ct);
		/* passport data initialize */
		SetMVArrayServer(passportClerkCustomerIdArray, i, 0);
	}

	for ( i = 0; i < CASHIER_SIZE; i++) {

		/* cashier lock initialize */
		CashierLockName[11] = '0' + i;
		CashierLock = CreateLockServer(CashierLockName, 12);
		SetMVArrayServer(CashierLineLockArray, i, CashierLock);

		/* cashier CV initialize */
		CashierCVName[9] = '0' + i;
		CashierCV = CreateConditionServer(CashierCVName, 10);
		SetMVArrayServer(CashierLineCVArray, i, CashierCV);

		/* cashier Wait CV initialize */
		CashierWaitCVName[13] = '0' + i;
		CashierWaitCV = CreateConditionServer(CashierWaitCVName, 14);
		SetMVArrayServer(CashierLineWaitCVArray, i, CashierWaitCV);

		/* cashier line size intialize */
		SetMVArrayServer(CashierLineCountArray, i, 0);
		/* cashier state initialize */
		SetMVArrayServer(CashierStateArray, i, ct);
		/* cashier data initialize */
		SetMVArrayServer(CashierCustomerIdArray, i, 0);
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