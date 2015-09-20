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
        cout << "Manager has counted a total of $" << MoneyTotal << " for The Passport Office" << endl;
        
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
    
    
    
    
    
    
    
    count++;
}