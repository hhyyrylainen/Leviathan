// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Include.h"
// ------------------------------------ //
#include <memory>

namespace Leviathan {

class GameWorld;

//! \brief Allows overwriting the used GameWorld class in the
//! engine. By default creates a StandardWorld
class GameWorldFactory {
public:
    DLLEXPORT GameWorldFactory();
    DLLEXPORT ~GameWorldFactory();

    //! \brief Creates a new world that can be used.
    //!
    //! This method can be skipped in places where the actual
    //! GameWorld class that should be created is known
    //! \param worldtype Application specific type that can be used to pass the wanted type of
    //! world around
    DLLEXPORT virtual std::shared_ptr<GameWorld> CreateNewWorld(int worldtype);

    DLLEXPORT static GameWorldFactory* Get();

protected:
    //! Global instance. Overwrite this in child class constructors
    DLLEXPORT static GameWorldFactory* StaticInstance;
};

} // namespace Leviathan
