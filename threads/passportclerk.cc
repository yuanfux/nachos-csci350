vector<int> passportClerkCustomerId;
vector<bool> passportClerkCustomerWaiting;
vector<int> customerStatus;
vector<clerkState> passportClerkState;

void PassportClerk(int myLine){
	while(true){
		
		clerkLineLock.Acquire();
		bool inBribeLine = false;
		int id = passportClerkCustomerId[myLine];

		if (passportClerkBribeLineCount[myLine] > 0){
			passportClerkBribeLineCV[myLine].Signal(&clerkLineLock);
			passportClerkState[myLine] = BUSY;
			inBribeLine = true;
		} else if(passportClerkLineCount[myLine] > 0){
			passportClerkLineCV[myLine].Signal(&clerkLineLock);
			passportClerkState[myLine] = BUSY;
		} else{
			passportClerkState[myLine] = AVAILABLE;
		}

		passportClerkLineLock[myLine].Acquire();
		clerkLineLock.Release();

		if (inBribeLine){

			passportClerkBribeLineCV[myLine].Wait(&passportClerkLineLock[myLine]);

			if (customerStatus[id] == 3){

				cout << "Customer[" << id << "] passport materials accepted." << endl;
				cout << "Not certified by PassportClerk[" << myLine << "]." << endl;

				int numCalls = rand() % 900 + 100;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerStatus[id] += 3;
				cout << "Customer[" << id << "] passport certified by PassportClerk[" << myLine << "]." << endl;
				if (passportClerkCustomerWaiting[myLine] == true){
					passportClerkBribeLineCV[myLine].Signal(&passportClerkLineLock[myLine]);
				}
			} else{
				cout << "Customer[" << id << "] passport materials not completed, customer goes to the back of a passport line" << endl;
			}

		} else{

			passportClerkLineCV[myLine].Wait(&passportClerkLineLock[myLine]);

			if (customerStatus[id] == 3){

				cout << "Customer[" << id << "] passport materials accepted." << endl;
				cout << "Not certified by PassportClerk[" << myLine << "]." << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerStatus[id] += 3;
				cout << "Customer[" << id << "] passport certified by PassportClerk[" << myLine << "]." << endl;
				if (passportClerkCustomerWaiting[myLine] == true){
					passportClerkLineCV[myLine].Signal(&passportClerkLineLock[myLine]);
				}
			} else{
				cout << "Customer[" << id << "] passport materials not completed, customer goes to the back of a passport line" << endl;
			}

		}
		
		passportClerkLineLock[myLine].Release();

	}
}