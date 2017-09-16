// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Include.h"
// ------------------------------------ //
#include <memory>

namespace Leviathan{

class GameWorld;

//! \brief Allows overwriting the used GameWorld class in the
//! engine. By default creates a StandardWorld
class GameWorldFactory{
public:
    GameWorldFactory();
    ~GameWorldFactory();

    //! \brief Creates a new world that can be used.
    //!
    //! This method can be skipped in places where the actual
    //!GameWorld class that should be created is known
    DLLEXPORT virtual std::shared_ptr<GameWorld> CreateNewWorld();

    DLLEXPORT static GameWorldFactory* Get();
    
protected:

    //! Global instance. Overwrite this in child class constructors
    static GameWorldFactory* StaticInstance;
};

}

