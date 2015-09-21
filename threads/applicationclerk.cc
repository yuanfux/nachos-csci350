void ApplicationClerk(int myLine){
<<<<<<< HEAD
    cout << "ApplicationClerk debug" << endl;
    int incoming=0;
=======
    int id = 0;
    ApplicationClerkState[myLine] = ONBREAK;

>>>>>>> origin/rex
    while(true){
//cout<<"m3"<<endl;
        bool InBribeLine=false;
        senatorWaitLock->Acquire();
<<<<<<< HEAD
=======
        
>>>>>>> origin/rex
        if(hasSenator){
            if(isFirst){
                isFirst=FALSE;
// cout<<"m2"<<endl;
                cout<<"senator data received by applicaiton clerk["<<myLine<<"]"<<endl;
                senatorWaitCV->Signal(senatorWaitLock);
            }
        }
<<<<<<< HEAD
        senatorWaitLock->Release();
        ClerkLineLock.Acquire();
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
=======

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
                ApplicationClerkState[myLine]=ONBREAK;
                ClerkLineLock.Release();
                currentThread->Yield();//context switch
                continue;
            }
        }
        else{
>>>>>>> origin/rex
            ClerkLineLock.Release();
            currentThread->Yield();//context switch
            continue;
        }
<<<<<<< HEAD
        ApplicationClerkLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        //wait for customer data
        if(InBribeLine){//in bribe line
            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
        //do my job customer now waiting
            cout<<"ApplicationClerk["<<myLine<<"] has received SSN ["<<ApplicationClerkData[myLine]<<"] from Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            cout<<"ApplicationClerk["<<myLine<<"] has recorded a completed application for Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            cout<<"ApplicationClerk["<<myLine<<"] has received $500 from Customer["<<ApplicationClerkData[myLine]<<"]"<<endl;
            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();
=======

        ApplicationClerkLineLock[myLine]->Acquire();
        ClerkLineLock.Release();
        //wait for customer data

        if(InBribeLine){//in bribe line

            ApplicationClerkBribeLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
            id = ApplicationClerkData[myLine];

            //Collect Bribe Money From Customer
            applicationMoenyLock.Acquire();
            MoneyFromApplicationClerk += 500;
            cout<<"ApplicationClerk["<<myLine<<"] has received $500 from Customer["<<id<<"]"<<endl;
            applicationMoenyLock.Release();

        //do my job customer now waiting
            cout<<"ApplicationClerk[" << myLine << "] has received SSN [" << id << "] from Customer [" << id << "]" << endl;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            customerApplicationStatus[id]++;
            cout<<"ApplicationClerk[" << myLine << "] has recorded a completed application for Customer [" << id << "]" << endl;
            
            ApplicationClerkBribeLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
>>>>>>> origin/rex
        }
        else{//not in bribe line
            ApplicationClerkLineCV[myLine]->Wait(ApplicationClerkLineLock[myLine]);
        //do my job customer now waiting
<<<<<<< HEAD
            cout<<"ApplicationClerk["<<myLine<<"] has received SSN ["<<ApplicationClerkData[myLine]<<"] from Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            cout<<"ApplicationClerk["<<myLine<<"] has recorded a completed application for Customer ["<<ApplicationClerkData[myLine]<<"]"<<endl;
            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);
            ApplicationClerkLineLock[myLine]->Release();

        }
        currentThread->Yield();//context switch
=======
            id = ApplicationClerkData[myLine];

            cout<<"ApplicationClerk["<< myLine <<"] has received SSN [" << id << "] from Customer [" << id << "]" << endl;

            for(int i = 0; i < 20; i++){
                currentThread->Yield();
            }

            customerApplicationStatus[ApplicationClerkData[myLine]]++;
            cout<<"ApplicationClerk["<< myLine <<"] has recorded a completed application for Customer [" << id << "]" << endl;
            
            ApplicationClerkLineCV[myLine]->Signal(ApplicationClerkLineLock[myLine]);

        }
            
        ApplicationClerkLineLock[myLine]->Release();
        // currentThread->Yield();//context switch
>>>>>>> origin/rex
    }//while
}