// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //

namespace Leviathan{

class StandardWorld;
}

namespace Demos {

//! \brief Base class for all the samples to let them get stuff from the DemosApplication class
class SampleCommon {
public:

    //! \note The child class destructor needs to clean up to allow sample switching
    virtual ~SampleCommon(){}
    
    //! \brief Sets up the sample
    virtual void Start(Leviathan::StandardWorld& world) = 0;
    
};

} // namespace Demos
