vector<int> pictureClerkCustomerId;
vector<int> pictureAcceptance;
vector<bool> pictureClerkCustomerWaiting;
vector<int> customerApplicationStatus;
vector<clerkState> pictureClerkState;

void PictureClerk(int myLine){
	while(true){
		
		clerkLineLock.Acquire();
		bool inBribeLine = false;
		int id = pictureClerkCustomerId[myLine];

		if (pictureClerkBribeLineCount[myLine] > 0){
			pictureClerkBribeLineCV[myLine].Signal(&clerkLineLock);
			cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
			pictureClerkState[myLine] = BUSY;
			inBribeLine = true;
		} else if(pictureClerkLineCount[myLine] > 0){
			pictureClerkLineCV[myLine].Signal(&clerkLineLock);
			cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
			pictureClerkState[myLine] = BUSY;
		} else{
			pictureClerkState[myLine] = AVAILABLE;
		}

		pictureClerkLineLock[myLine].Acquire();
		clerkLineLock.Release();

		if (inBribeLine){

			pictureClerkBribeLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
			cout << "PictureClerk [" << myLine << "] has taken a picture of Customer [" << id << "]" << endl;

			pictureClerkBribeLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
			pictureClerkBribeLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			if (pictureAcceptance[myLine] > 2){
				cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerApplicationStatus[id] += 2;
				if (pictureClerkCustomerWaiting[myLine] == true){
					pictureClerkBribeLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
				}
			} else{
				cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
			}

		} else{

			pictureClerkLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
			cout << "PictureClerk [" << myLine << "] has taken a picture of Customer [" << id << "]" << endl;

			pictureClerkLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
			pictureClerkLineCV[myLine].Wait(&pictureClerkLineLock[myLine]);

			if (pictureAcceptance[myLine] > 2){
				cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

				int numCalls = rand() % 80 + 20;
				for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
				}

				customerApplicationStatus[id] += 2;
				if (pictureClerkCustomerWaiting[myLine] == true){
					pictureClerkLineCV[myLine].Signal(&pictureClerkLineLock[myLine]);
				}
			} else{
				cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
			}

		}
		
		pictureClerkLineLock[myLine].Release();

	}
}