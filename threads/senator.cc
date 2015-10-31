Lock* senatorWaitLock;
Condition* senatorApplicationWaitCV;
Condition* senatorPictureWaitCV;
Condition* senatorPassportWaitCV;
Condition* senatorCashierWaitCV;
Condition* customerWaitCV;
Lock* customerWaitLock;

void Senator(){
    customerWaitLock->Acquire();
    senatorWaitLock->Acquire();
    int id=senatorNum+1;
    senatorNum++;
    cout << "Senator ["<< id<<"] has came into passport office"<< endl;
    
    for(int i=0;i<numCustomerWaiting.size();i++){
        cout<<"Customer ["<<numCustomerWaiting[i]<<"] is going outside the Passport Office because their is a Senator present."<<endl;
    }
    hasSenator=TRUE;
   // cout << "mighty" << endl;
    cout << "Senator ["<<id<<"] has gotten in regular line for ApplicationClerk ["<< senatorServiceId << "]." << endl;
    senatorApplicationWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator ["<<id<<"] has given SSN ["<<id<<"] to ApplicationClerk ["<<senatorServiceId<<"]."<<endl;
    senatorData=id;
    senatorApplicationWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorApplicationWaitCV->Wait(senatorWaitLock);//wait for a filed application
    


    cout << "Senator ["<<id<<"] has gotten in regular line for PictureClerk ["<< senatorServiceId << "]." << endl;
    senatorPictureWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator ["<<id<<"] has given SSN ["<<id<<"] to PictureClerk ["<<senatorServiceId<<"]."<<endl;
    senatorData=id;
    senatorPictureWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorPictureWaitCV->Wait(senatorWaitLock);//wait for a filed application
    
    
    cout << "Senator ["<<id<<"] has gotten in regular line for PassportClerk ["<< senatorServiceId << "]." << endl;
    senatorPassportWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator ["<<id<<"] has given SSN ["<<id<<"] to PassportClerk ["<<senatorServiceId<<"]."<<endl;
    senatorData=id;
    senatorPassportWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorPassportWaitCV->Wait(senatorWaitLock);//wait for a filed application
    
    
    cout << "Senator ["<<id<<"] has gotten in regular line for Cashier ["<< senatorServiceId << "]." << endl;
    senatorCashierWaitCV->Wait(senatorWaitLock);//wait for a clerk
    
    cout << "Senator ["<<id<<"] has given SSN ["<<id<<"] to Cashier ["<<senatorServiceId<<"]."<<endl;
    senatorData=id;
    senatorCashierWaitCV->Signal(senatorWaitLock);//signal a clerk
    senatorCashierWaitCV->Wait(senatorWaitLock);//wait for a filed application

    
    hasSenator=FALSE;
    
    cout<<"senator finished"<<endl;
   // cout<<"33"<<endl;
    customerWaitLock->Release();
    //cout<<"44"<<endl;
    senatorWaitLock->Release();
     //cout<<"55"<<endl;
}