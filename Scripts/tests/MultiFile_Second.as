
int SecondFileFunc(){

    // Circular calling methods between these files //
    if(RandomFuncInFirst() == 42)
        return 3;
    
    return 1;
}


