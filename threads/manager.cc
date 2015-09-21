int MoneyFromApplicationClerk = 0;
int MoneyFromPictureClerk = 0;
int MoneyFromPassportClerk = 0;
int MoneyFromCashier = 0;
int MoneyTotal = 0;
int count = 0;

//Lock for money




while (true){
    if (count == 1000){             //when it is the time to Print out all the money
        
        applicationMoenyLock.Acquire();
        pictureMoenyLock.Acquire();
        passportMoenyLock.Acquire();
        cashierMoenyLock.Acquire();
        
        cout << "Manager has counted a total of $" << MoneyFromApplicationClerk << " for ApplicationClerks" << endl;
        cout << "Manager has counted a total of $" << MoneyFromPictureClerk << " for PictureClerks" << endl;
        cout << "Manager has counted a total of $" << MoneyFromPassportClerk << " for PassportClerks" << endl;
        cout << "Manager has counted a total of $" << MoneyFromCashierClerk << " for Cashiers" << endl;
        
        MoneyTotal = MoneyFromApplicationClerk + MoneyFromPictureClerk + MoneyFromPassportClerk + MoneyFromCashier;
        cout << "Manager has counted a total of $" << MoneyTotal << " for The passport Office" << endl;
        
        applicationMoenyLock.Release();
        pictureMoenyLock.Release();
        passportMoenyLock.Release();
        cashierMoenyLock.Release();
        
        count = 0;
    }
        
        /*
        applicationMoneyLock.Acquire();
        MoneyFromApplicationClerk += 500;
        applicationMoneyLock.Release();
        */
        
        // A vector of clerkState
        // A vector of clerkCV
        
        clerkLineLock.Acquire();

            //Application Clerks
            for (int i = 0; i < ApplicationClerkLineLock.size(); i++){
                if (ApplicationClerkLineCount[i] + ApplicationClerkBribeLineCount[i] >= 3
                    && ApplicationClerkState[i] == ONBREAK){
                    
                    ApplicationClerkState[i] = AVAILABLE;
                    cout << "Manager has woken up an ApplicationClerk" << endl;
                }
                
            }
            
            //Picture Clerks
            for (int i = 0; i < pictureClerkLineLock.size(); i++){
                if (pictureClerkLineCount[i] + pictureClerkBribeLineCount[i] >= 3
                    && pictureClerkState[i] == ONBREAK){
                    
                    pictureClerkState[i] = AVAILABLE;
                    cout << "Manager has woken up a PictureClerk" << endl;
                }
                
            }
            
            //Passport Clerks
            for (int i = 0; i < passportClerkLineLock.size(); i++){
                if (passportClerkLineCount[i] + passportClerkBribeLineCount[i] >= 3
                    && passportClerkState[i] == ONBREAK){
                    
                    passportClerkState[i] = AVAILABLE;
                    cout << "Manager has woken up a PassportClerk" << endl;
                }

            }
            
            //Cashiers
            for (int i = 0; i < CashierLineLock.size(); i++){
                if (CashierLineCount[i] + CashierBribeLineCount[i] >= 3
                    && CashierState[i] == ONBREAK){
                    
                    CashierState[i] = AVAILABLE;
                    CashierCV[i]->Signal(CashierLock[i]);      //!!!!!!
                    cout << "Manager has woken up a Cashier" << endl;
                }
                
            }
        
        count++;
        clerkLineLock.Release();
    
    
    
}