vector<int> pictureClerkCustomerId;
vector<int> pictureAcceptance;
vector<bool> pictureClerkCustomerWaiting;
vector<int> customerApplicationStatus;
vector<clerkState> pictureClerkState;

void PictureClerk(int myLine){
	cout << "pictureClerk debug" << endl;
	while(true){
    
	    ClerkLineLock.Acquire();
	    bool inBribeLine = false;
	    int id = 0; 

	    if (pictureClerkState[myLine] != ONBREAK){
		    if (pictureClerkBribeLineCount[myLine] > 0){
				pictureClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
				cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
				pictureClerkState[myLine] = BUSY;
				inBribeLine = true;
		    } else if(pictureClerkLineCount[myLine] > 0){
				pictureClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
				cout << "PictureClerk [" << myLine << "] has signalled a Customer to come to their counter." << endl;
				pictureClerkState[myLine] = BUSY;
		    } else{
				pictureClerkState[myLine] = ONBREAK;
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

	    pictureClerkLineLock[myLine]->Acquire();
	    ClerkLineLock.Release();
	    if (inBribeLine){

		    //Collect Bribe Money From Customer
		    pictureMoenyLock.Acquire();
		    MoneyFromPictureClerk += 500;
		    pictureMoenyLock.Release();

			pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
			id = pictureClerkData[myLine];
			cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
			cout << "PictureClerk [" << myLine << "] has taken a picture of Customer [" << id << "]" << endl;

			pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
			pictureClerkBribeLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

			if (pictureAcceptance[myLine] > 2){
		        cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

		        int numCalls = rand() % 80 + 20;
		        for (int i = 0; i < numCalls; i++){
		          	currentThread->Yield();
	        	}

		        customerApplicationStatus[id] += 2;
		        // if (pictureClerkCustomerWaiting[myLine] == true){
		        pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
		        // }
				} else{
					cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
					pictureClerkBribeLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
				}

	    } else{

			pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);
			id = pictureClerkData[myLine];

			cout << "PictureClerk [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
			cout << "PictureClerk [" << myLine << "] has taken a picture of Customer [" << id << "]" << endl;

			pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
			pictureClerkLineCV[myLine]->Wait(pictureClerkLineLock[myLine]);

			if (pictureAcceptance[myLine] > 2){
			    cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does like their picture" << endl;

			    int numCalls = rand() % 80 + 20;
			    for (int i = 0; i < numCalls; i++){
					currentThread->Yield();
			    }

		        customerApplicationStatus[id] += 2;
	        	// if (pictureClerkCustomerWaiting[myLine] == true){
				pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
	        	// }
			} else{
				cout << "PictureClerk [" << myLine << "] has been told that Customer[" << id << "] does not like their picture" << endl;
				pictureClerkLineCV[myLine]->Signal(pictureClerkLineLock[myLine]);
			}

	    }
	    
	    pictureClerkLineLock[myLine]->Release();

	}
}