void ApplicationClerk(int myLine){
    int id = 0;
    bool InBribeLine = false;

    while(true){
        senatorWaitLock->Acquire();
        
        if(hasSenator){
            if(isFirst){
                isFirst=FALSE;
                cout<<"senator data received by applicaiton clerk["<<myLine<<"]"<<endl;
                senatorWaitCV->Signal(senatorWaitLock);
            }
        }

        senatorWaitLock->Release();
        ClerkLineLock.Acquire();
        if (ApplicationClerkState[myLine] != ONBREAK){
            if(ApplicationClerkBribeLineCount[myLine]>0){
                ApplicationClerkBribeLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout<<"ApplicationClerk["<<myLine<<"] has signalled a Customer to come to their counter."<<endl;
                ApplicationClerkState[myLine]=BUSY;
                InBribeLine=true;
            }
            else if(ApplicationClerkLineCount[myLine]>0){
                ApplicationClerkLineWaitCV[myLine]->Signal(&ClerkLineLock);
                cout<<"ApplicationClerk["<<myLine<<"] has signalled a Customer to come to their counter."<<endl;
                ApplicationClerkState[myLine]=BUSY;

            }
            else{
                ApplicationClerkState[myLine]=AVAILABLE;
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

        ApplicationClerkLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        //wait for customer data

        if(InBribeLine){//in bribe line

            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            id = ApplicationClerkData[myLine];

            //Collect Bribe Money From Customer
            applicationMoneyLock.Acquire();
            MoneyFromApplicationClerk += 500;
            cout<<"ApplicationClerk["<<myLine<<"] has received $500 from Customer["<<id<<"]"<<endl;
            applicationMoneyLock.Release();

        //do my job customer now waiting
            cout<<"ApplicationClerk[" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            customerApplicationStatus[id]++;
            cout<<"ApplicationClerk[" << myLine << "] has recorded a completed application for Customer [" << id << "]" << endl;
            
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
        }
        else{//not in bribe line
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
        //do my job customer now waiting
            id = ApplicationClerkData[myLine];

            cout<<"ApplicationClerk["<< myLine <<"] has received SSN [" << id << "] from Customer [" << id << "]" << endl;

            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }

            customerApplicationStatus[id]++;
            cout<<"ApplicationClerk["<< myLine <<"] has recorded a completed application for Customer [" << id << "]" << endl;
            
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);

        }
            
        ApplicationClerkLineLock[myLine]->Release();
        // currentThread->Yield();//context switch
    }//while
}