vector<int> pictureClerkCustomerId;
vector<int> pictureAcceptance;
vector<bool> pictureClerkCustomerWaiting;
vector<int> customerStatus;

void PictureClerk(int myLine){
	while(true){
		
		clerkLineLock.Acquire();
		bool inBribeLine = false;
		int id = pictureClerkCustomerId[myLine];

		if (pictureClerkBribeLineCount[myLine] > 0){
			pictureClerkBribeLineCV[myLine].Signal(&clerkLineLock);
			clerkState[myLine] = BUSY;
			inBribeLine = true;
		} else if(pictureClerkLineCount[myLine] > 0){
			pictureClerkLineCV[myLine].Signal(&clerkLineLock);
			clerkState[myLine] = BUSY;
		} else{
			clerkState[myLine] = AVAILABLE;
		}

		pictureClerkLineLock[myLine].Acquire();
		clerkLineLock.Release();

		if (inBribeLine){

			pictureClerkBribeLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			cout << "Customer[" << "] picture taken, wait to be accepted" << endl;

			pictureClerkBribeLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
			pictureClerkBribeLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			if (pictureAcceptance[myLine] > 2){
				cout << "Customer[" << id << "] picture accepted, but not filed" << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerStatus[id] += 2;
				cout << "Customer[" << id << "] picture filed" << endl;
				if (pictureClerkCustomerWaiting[myLine] == true){
					pictureClerkBribeLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
				}
			} else{
				cout << "Customer[" << id << "] picture not accepted, customer goes to the end of line." << endl;
			}

		} else{

			pictureClerkLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			cout << "Customer[" << "] picture taken, wait to be accepted" << endl;

			pictureClerkLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
			pictureClerkLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			if (pictureAcceptance[myLine] > 2){
				cout << "Customer[" << id << "] picture accepted, but not filed" << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerStatus[id] += 2;
				cout << "Customer[" << id << "] picture filed" << endl;
				if (pictureClerkCustomerWaiting[myLine] == true){
					pictureClerkLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
				}
			} else{
				cout << "Customer[" << id << "] picture not accepted, customer goes to the end of line." << endl;
			}

		}
		
		pictureClerkLineLock[myLine].Release();

	}
}