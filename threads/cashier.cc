vector<int> CashierCustomerId;
vector<bool> CashierCustomerWaiting;
vector<int> customerStatus;
vector<clerkState> CashierState;

void Cashier(int myLine){
    CashierState[myLine] = ONBREAK;
    
    while (true){
        
        clerkLineLock.Acquire();
        bool inBribeLine = false;
        int id = 0;
        //int id = CashierCustomerId[myLine];
        
       
        if (CashierState[myLine] != ONBREAK){
            //When CashierState != ONBREAK
            if (CashierBribeLineCount[myLine] > 0){
                CashierBribeLineCV[myLine]->Signal(&clerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
                inBribeLine = true;
            }   else if (CashierLineCount[myLine] > 0){
                CashierLineCV[myLine]->Signal(&clerkLineLock);
                cout << "Cashier [" << myLine << "] has signalled a Customer to come to their counter." << endl;
                CashierState[myLine] = BUSY;
            }
                else {
                    clerkLineLock.Release();
                    CashierState[myLine] = ONBREAK;     //!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    CashierCV[myLine] ->Wait(CashierLock[myLine]);
                    currentThread->Yield();
                    continue;
            }
        }
        else {  //When CashierState == ONBREAK, Do Nothing
            clerkLineLock.Release();
            currentThread.Yield();
            continue;
        }
        
        CashierLineLock[myLine]->Acquire();
        clerkLineLock.Release();
        
        if (inBribeLine){
            //In BribeLine
            
            id = CashierCustomerId[myLine];
            
            CashierBribeLineCV[myLine]->Wait(&CashierLineLock[myLine]);
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
      
            
            
            if (customerStatus[id] == 6) {  // Passed All the tests (Certified)
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
         
            id = CashierCustomerId[myLine];
            
            CashierLineCV[myLine]->Wait(&CashierLineLock[myLine]);
            cout << "Cashier [" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            
            
            if (customerStatus[id] == 6) {  // Passed All the tests (Certified)
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