vector<int> CashierCustomerId;
vector<bool> CashierCustomerWaiting;
vector<int> customerApplicationStatus;
vector<clerkState> CashierState;

void Cashier(int myLine){
    CashierState[myLine] = ONBREAK;
    int id = 0;
    
    while (true){
        
        ClerkLineLock.Acquire();
        bool inBribeLine = false;
        //int id = CashierCustomerId[myLine];
        
       
        if (CashierState[myLine] != ONBREAK){
            //When CashierState != ONBREAK
            if (CashierBribeLineCount[myLine] > 0){
                CashierBribeLineCV[myLine]->Signal(&ClerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
                inBribeLine = true;
            }   else if (CashierLineCount[myLine] > 0){
                CashierLineCV[myLine]->Signal(&ClerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
            }
            else {
                ClerkLineLock.Release();
                CashierState[myLine] = ONBREAK;     //!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                CashierCV[myLine]->Wait(CashierLock[myLine]);
                currentThread->Yield();
                continue;
            }
        }
        else {  //When CashierState == ONBREAK, Do Nothing
            ClerkLineLock.Release();
            currentThread.Yield();
            continue;
        }
        
        CashierLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        
        if (inBribeLine){
            //In BribeLine
            
            
            CashierBribeLineCV[myLine]->Wait(&CashierLineLock[myLine]);
            id = CashierCustomerId[myLine];
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
      
            
            
            if (customerApplicationStatus[id] == 6) {  // Passed All the tests (Certified)
                cout << "Cashier [" << myLine << "] has verified that Customer [" << id << "] has been certified by a PassportClerk" << endl;
                
                //Collect Fee From Customer
                cashierMoneyLock.Acquire();
                MoneyFromCashier += 100;
                cashierMoneyLock.Release();
                
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                cout << "Cashier [" << id << "] has provided Customer[identifier] their completed passport" << endl;
                cout << "Cashier [" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
                customerApplicationStatus[id] += 4;
                CashierBribeLineCV[myLine]->Signal(&CashierLineLock[myLine]);
            }
            else {  //Not yet certified
               
                //money += 100;
                //cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
                
                
                // To Get Verified?
                // Punish the Customer
                
            }
        }
        
        else {
            // NOT inBribeLine
         
            
            CashierLineCV[myLine]->Wait(&CashierLineLock[myLine]);
            id = CashierCustomerId[myLine];
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            
            if (customerApplicationStatus[id] == 6) {  // Passed All the tests (Certified)
                cout << "Cashier [" << myLine << "] has verified that Customer [" << id << "] has been certified by a PassportClerk" << endl;
                
                
                //Collect Fee From Customer
                cashierMoneyLock.Acquire();
                MoneyFromCashier += 100;
                cashierMoneyLock.Release();
                
                
                cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] after certification" << endl;
                
                // Give out the passport to the customer
                // TODO: Check if the passport has given out already (Check with id)
                // Notify the customer he is done
                cout << "Cashier [" << id << "] has provided Customer[identifier] their completed passport" << endl;
                cout << "Cashier [" << myLine << "] has recorded that Customer[" << id << "] has been given their completed passport" << endl;
                
                customerApplicationStatus[id] += 4;
                CashierLineCV[myLine]->Signal(&CashierLineLock[myLine]);
            }
            else {  //Not yet Certified
                //money += 100;
                //cout << "Cashier [" << myLine << "] has received the $100 from Customer[" << id << "] before certification. They are to go to the back of my line." << endl;
                
                
                // To Get Verified?
                // Punish the Customer
                
                
            }
        }
        
        CashierLineLock[myLine].Release();
    }   //while loop
}