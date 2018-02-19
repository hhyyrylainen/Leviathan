// Tests ogre functions
bool TestAngleConversions(){

    Ogre::Radian rad = Ogre::Degree(180);
    int valueDeg = int(rad.valueDegrees());
    if(valueDeg != 180){
        // TODO: find a good way to run tests in as
        LOG_ERROR("Values don't match: " + formatInt(valueDeg) + " != " +
            formatInt(180));
        return false;
    }

    // This may not crash (that's the test)
    Ogre::Quaternion quat(Ogre::Degree(90), Float3(1, 0, 0));
    
    return true;
}

