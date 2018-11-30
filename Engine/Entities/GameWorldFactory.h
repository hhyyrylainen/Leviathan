// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Include.h"
// ------------------------------------ //
#include <memory>

namespace Leviathan {

class PhysicsMaterialManager;
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
    DLLEXPORT virtual std::shared_ptr<GameWorld> CreateNewWorld(int worldtype,
        const std::shared_ptr<PhysicsMaterialManager>& physicsMaterials, int overrideid = -1);

    DLLEXPORT static GameWorldFactory* Get();

protected:
    //! Global instance. Overwrite this in child class constructors
    DLLEXPORT static GameWorldFactory* StaticInstance;
};

//! \brief Types of inbuilt world types
enum class INBUILT_WORLD_TYPE : int32_t { Standard = 1024 };

//! \brief Factory for inbuilt world types
class InbuiltWorldFactory {
public:
    static std::shared_ptr<GameWorld> CreateNewWorld(INBUILT_WORLD_TYPE worldtype,
        const std::shared_ptr<PhysicsMaterialManager>& physicsMaterials, int overrideid = -1);
};

} // namespace Leviathan
