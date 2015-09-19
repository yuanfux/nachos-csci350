vector<int> passportClerkCustomerId;
vector<bool> passportClerkCustomerWaiting;
vector<int> customerApplicationStatus;
vector<clerkState> passportClerkState;

void PassportClerk(int myLine){
	while(true){
		
		clerkLineLock.Acquire();
		bool inBribeLine = false;
		int id = passportClerkCustomerId[myLine];

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
			passportClerkState[myLine] = AVAILABLE;
		}

		passportClerkLineLock[myLine].Acquire();
		clerkLineLock.Release();

		if (inBribeLine){

			//TODO: add $500 to money received
			passportClerkBribeLineCV[myLine].Wait(&passportClerkLineLock[myLine]);
			cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;

			if (customerApplicationStatus[id] == 3){

				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;
				
				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerApplicationStatus[id] += 3;
				cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
				if (passportClerkCustomerWaiting[myLine] == true){
					passportClerkBribeLineCV[myLine].Signal(&passportClerkLineLock[myLine]);
				}
			} else{
				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
			}

		} else{

			passportClerkLineCV[myLine].Wait(&passportClerkLineLock[myLine]);
			cout << "PassportClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;

			if (customerApplicationStatus[id] == 3){

				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] has both their application and picture completed" << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerApplicationStatus[id] += 3;
				cout << "PassportClerk [" << myLine << "] has recorded Customer[" << id << "] passport documentation" << endl;
				if (passportClerkCustomerWaiting[myLine] == true){
					passportClerkLineCV[myLine].Signal(&passportClerkLineLock[myLine]);
				}
			} else{
				cout << "PassportClerk [" << myLine << "] has determined that Customer[" << id << "] does not have both their application and picture completed" << endl;
				//TODO: customer should be punished
			}

		}
		
		passportClerkLineLock[myLine].Release();

	}
}