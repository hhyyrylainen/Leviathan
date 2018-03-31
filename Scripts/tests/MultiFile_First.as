int CalledInListener = 0;


[@Listener="OnInit"]
int InitListener(){

    CalledInListener = 42;

    // Call stuff in the other file //
    if(SecondFileFunc() == 3)
        FirstTestRunSuccessFlag = 1;
    else
        FirstTestRunSuccessFlag = 0;

    CalledInListener = 0;
    return 0;
}

int RandomFuncInFirst(){

    return CalledInListener;
}


