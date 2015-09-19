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
            CashierState[myLine] = BUSY;
            inBribeLine = true;
        }   else if (CashierLineCount[myLine] > 0){
            CashierLineCV[myLine].Signal(&clerkLineLock);
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
            
            if (customerStatus[id] == 6) {  // Passed All the tests (Certified)
                cout << "Customer[" << Id << "] (Bribe) Passport Certified and Delivered. Now Accepting Money From The Customer" << endl;
                // Money -100
                money += 100;               //collect fee from the customer
                cout << "Collected Money from Customer[" << Id << "]." << "Current Money is : " << money << "..." << endl;
                
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
            
            }
            else {
                cout << "Customer[" << id << "] passport materials haven't completed yet, customer go back to the line" << endl;
                
                //Force to wait
                int numCalls = rand() % 900 + 100;
                for (int i = 0; i < numCalls; i++)
                {
                    currentThread->Yield();
                }
                
                // To Get Verified?
                
                
            }
        }
        
        else {
            // NOT inBribeLine
         
            CashierLineCV[myLine].Wait(&CashierLineLock[myLine]);
            
            if (customerStatus[id] == 6) {  // Passed All the tests (Certified)
                cout << "Customer[" << Id << "] (Normal) Passport Certified and Delivered. Now Accepting Money From The Customer" << endl;
                // Money -100
                money += 100;               //collect fee from the customer
                cout << "Collected Money from Customer[" << Id << "]." << "Current Money is : " << money << "..." << endl;
                
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                
            }
            else {
                cout << "Customer[" << id << "] passport materials haven't completed yet, customer go back to the line" << endl;
                
                //Force to wait
                int numCalls = rand() % 900 + 100;
                for (int i = 0; i < numCalls; i++)
                {
                    currentThread->Yield();
                }
                
                // To Get Verified?
                
                
            }
        }
        
        CashierLineLock[myLine].Release();
    }   //while loop
}