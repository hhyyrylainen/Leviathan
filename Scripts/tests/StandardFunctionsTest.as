// Tests for bound standard functions

bool TestFunction1(){

    if(min(1, 2) != 1)
        return false;
    if(min(1.0, 2.0) != 1.0)
        return false;

    if(max(1, 2) != 2)
        return false;
    if(max(1.0, 2.0) != 2.0)
        return false;

    if(min(0.01f, 2.0) != 0.01f)
        return false;
    
    return true;
}

