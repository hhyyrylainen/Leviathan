// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "EntityCommon.h"
#include "Common/ObjectPool.h"

#include "boost/pool/pool.hpp"

namespace Leviathan{

//! Number of states that are kept. Corresponds to time span of TICKSPEED * KEPT_STATES_COUNT
constexpr auto KEPT_STATES_COUNT = 5;

template<class StateT>
class ObjectsComponentStates{
public:

    DLLEXPORT ObjectsComponentStates(){

        for(size_t i = 0; i < StoredStates.size(); ++i){

            StoredStates[i] = nullptr;
        }
    }
    

    std::array<StateT*, KEPT_STATES_COUNT> StoredStates;
};

//! \brief Holds state objects of type for quick access by ObjectID
template<class StateT>
class StateHolder{
public:

    DLLEXPORT StateHolder(){

    }
    
    DLLEXPORT ~StateHolder(){

    }
    
protected:

    template<typename... Args>
    DLLEXPORT StateT* CreateNewState(Args&&... args){
        
        return StatePool.ConstructNew(std::forward<Args>(args)...);
    }

    DLLEXPORT void DestroyStateObject(StateT* state){

        StatePool.Destroy(state);
    }

private:
    
    //! Keeps track of states associated with an object
    ObjectPool<ObjectsComponentStates<StateT>, ObjectID> StateObjects;

    //! All of the created StateT objects are held by this pool and
    //! only referenced from StateObjects
    BasicPool<StateT> StatePool;
};

}

