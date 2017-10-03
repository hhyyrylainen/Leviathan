// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Component.h"

namespace Leviathan{

//! \brief Alternative base class for Component that creates distinct state objects
template<class StateT>
class ComponentWithStates : public Component{
public:

    inline ComponentWithStates(COMPONENT_TYPE type) : Component(type){}

    //! True when there are states in a StateHolder for this entity, which haven't been
    //! shown yet.
    bool StateMarked = false;

    // Data for currently interpolating state
    // Some child classes might not use this if interpolating is not done
    // maybe some of these could be moved to a new class
    float InterpolatingStartTime = 0.f;
    int InterpolatingRemoteStartTick;
    StateT* InterpolatingStartState = nullptr;
    StateT* InterpolatingEndState = nullptr;

    // Current time can be calculated from the game world tick and engine clock
    // once the time - InterpolatingStartTime is >= TICKSPEED
    // InterpolatingStartTick is incremented and if there are no state
    // with that tick number StateMarked will be set false and
    // interpolation stops until a new state is a added when we are
    // marked by the state creation and the interpolation continues
    // from the newest tick (number > InterpolatingStartTick)
    
};

//! \brief Base for all component state classes
class BaseComponentState{
public:

    
    
};


}


