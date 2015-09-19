vector<int> CashierCustomerId;
vector<bool> CashierCustomerWaiting;
vector<int> customerStatus;
vector<clerkState> CashierState;

void Cashier(int myLine){
    int money = 0;
    
    while (true){
        
        clerkLineLock.Acquire();
        bool inBribeLine = false;
        int id = CashierCustomerId[myLine];
        
        if (CashierBribeLineCount[myLine] > 0){
            CashierBribeLineCV[myLine].Signal(&clerkLineLock);
            cout << "Cashier [" << id << "] has signalled a Customer to come to their counter." << endl;
            CashierState[myLine] = BUSY;
            inBribeLine = true;
        }   else if (CashierLineCount[myLine] > 0){
            CashierLineCV[myLine].Signal(&clerkLineLock);
            cout << "Cashier [" << id << "] has signalled a Customer to come to their counter." << endl;
            CashierState[myLine] = BUSY;
        }
            else {
                CashierState[myLine] = AVAILABLE;
        }
        
        CashierLineLock[myLine].Acquire();
        clerkLineLock.Release();
        
        if (inBribeLine){
            //In BribeLine
            
            CashierBribeLineCV[myLine].Wait(&CashierLineLock[myLine]);
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            //Check Document Time
            int numCalls = rand() % 50 + 100;
            for (int i = 0; i < numCalls; i++)
            {
                currentThread->Yield();
            }
            
            
            if (customerStatus[id] == 6) {  // Passed All the tests (Certified)
                cout << "Cashier [" << myLine << "] has verified that Customer [" << id << "] has been certified by a PassportClerk" << endl;
                // Money -100
                money += 100;               //collect fee from the customer
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                cout << "Cashier [" << id << "] has provided Customer[identifier] their completed passport" << endl;
                cout << "Cashier [" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
            }
            else {
                money += 100;
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
                
                
                // To Get Verified?
                // Punish the Customer
                
            }
        }
        
        else {
            // NOT inBribeLine
         
            CashierLineCV[myLine].Wait(&CashierLineLock[myLine]);
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            //Check Document Time
            int numCalls = rand() % 50 + 100;
            for (int i = 0; i < numCalls; i++)
            {
                currentThread->Yield();
            }
            
            if (customerStatus[id] == 6) {  // Passed All the tests (Certified)
                cout << "Cashier [" << myLine << "] has verified that Customer [" << id << "] has been certified by a PassportClerk" << endl;
                // Money -100
                money += 100;               //collect fee from the customer
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
                
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                cout << "Cashier [" << id << "] has provided Customer[identifier] their completed passport" << endl;
                cout << "Cashier [" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
                
            }
            else {
                money += 100;
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
                
                
                // To Get Verified?
                // Punish the Customer
                
                
            }
        }
        
        CashierLineLock[myLine].Release();
    }   //while loop
}