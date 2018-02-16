// Tests ogre functions
bool TestAngleConversions(){

    Ogre::Radian rad = Ogre::Degree(180);

    if(rad.valueDegrees() != 180)
        return false;

    // This may not crash (that's the test)
    Ogre::Quaternion quat(Ogre::Degree(90), Float3(1, 0, 0));
    
    return true;
}

