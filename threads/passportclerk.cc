vector<int> passportClerkCustomerId;
vector<bool> passportClerkCustomerWaiting;
vector<int> customerApplicationStatus;
vector<clerkState> passportClerkState;

void PassportClerk(int myLine){
	int id = 0;
	passportClerkState[myLine] = ONBREAK;

	while(true){
		
		clerkLineLock.Acquire();
		bool inBribeLine = false;

		if (passportClerkState[myLine] != ONBREAK){
			if (passportClerkBribeLineCount[myLine] > 0){
				passportClerkBribeLineCV[myLine].Signal(&clerkLineLock);
				cout << "PassportClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
				passportClerkState[myLine] = BUSY;
				inBribeLine = true;
			} else if(passportClerkLineCount[myLine] > 0){
				passportClerkLineCV[myLine].Signal(&clerkLineLock);
				cout << "PassportClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
				passportClerkState[myLine] = BUSY;
			} else{
				passportClerkState[myLine] = ONBREAK;
				ClerkLineLock.Release();
				currentThread->Yield();//context switch
		      	continue;
			}
		}
		else{
			ClerkLineLock.Release();
			currentThread->Yield();//context switch
	      	continue;
		}

		passportClerkLineLock[myLine].Acquire();
		clerkLineLock.Release();

		if (inBribeLine){

			

			passportClerkBribeLineCV[myLine].Wait(&passportClerkLineLock[myLine]);
			id = passportClerkCustomerId[myLine];

            //Collect Bribe Money From Customer
            passportMoenyLock.Acquire();
            MoneyFromPassportClerk += 500;
            cout<<"PassportClerk["<<myLine<<"] has received $500 from Customer["<<id<<"]"<<endl;
            passportMoenyLock.Release();

			cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
			
			int passportClerkPunishment = rand() % 100;
			if (passportClerkPunishment > 5){

				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;
				
				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerApplicationStatus[id] += 3;
				cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
				passportClerkBribeLineCV[myLine].Signal(&passportClerkLineLock[myLine]);

			} else{

				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
				passportClerkBribeLineCV[myLine].Signal(&passportClerkLineLock[myLine]);

			}

		} else{

			passportClerkLineCV[myLine].Wait(&passportClerkLineLock[myLine]);
			cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;

			int passportClerkPunishment = rand() % 100;
			if (passportClerkPunishment > 5){

				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerApplicationStatus[id] += 3;
				cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
				passportClerkLineCV[myLine].Signal(&passportClerkLineLock[myLine]);

			} else{

				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
				passportClerkLineCV[myLine].Signal(&passportClerkLineLock[myLine]);

			}

		}
		
		passportClerkLineLock[myLine].Release();

	}
}