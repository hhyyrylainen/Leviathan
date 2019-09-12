// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

namespace Leviathan {
class GameWorld;

//! A base type for data classes that need to be associated with a GameWorld
class PerWorldData {
public:
    DLLEXPORT PerWorldData(GameWorld& world);
    virtual ~PerWorldData() = default;

    //! \brief Called when the world is cleared
    DLLEXPORT virtual void OnClear();

protected:
    GameWorld& InWorld;
};

} // namespace Leviathan
