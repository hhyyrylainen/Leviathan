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

float MAX_CHANCE_SCORE = 13;

// Tests small engine parts
bool TestFunction2(){

    {
        int i = GetEngine().GetRandom().GetNumber(0, 13);
        
        if(i < 0 || i > 13){
            LOG_WRITE("invalid value, i = " + i);
            return false;
        }
    }

    {
        float i = GetEngine().GetRandom().GetNumber(0.f, MAX_CHANCE_SCORE);
        
        if(i < 0.f || i > MAX_CHANCE_SCORE){
            LOG_WRITE("invalid value, i = " + i);
            return false;
        }
    }
    
    return true;
}

