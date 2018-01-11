int CalledInListener = 0;


[@Listener="OnInit"]
void InitListener(){

    CalledInListener = 42;

    // Call stuff in the other file //
    if(SecondFileFunc() == 3)
        FirstTestRunSuccessFlag = 1;
    else
        FirstTestRunSuccessFlag = 0;

    CalledInListener = 0;
}

int RandomFuncInFirst(){

    return CalledInListener;
}


