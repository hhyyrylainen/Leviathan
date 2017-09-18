// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Entities/Components.h"
#include "Entities/GameWorld.h"

namespace Leviathan{

//! \brief Class to collect all entity creations to one class
//! \note All created objects are added to the world so that they are broadcast on
//! the network (if this is a server)
//! \todo Allow objects to be created that will be sent to clients only after the caller has
//! had the chance to create constraints etc.
class ObjectLoader{
    friend EntitySerializer;
public:

    ObjectLoader() = delete;

    //! \brief Creates a full camera entity that can be usedas the
    //! primary world camera and sound listener
    template<class TWorldClass>
        DLLEXPORT static ObjectID LoadCamera(TWorldClass &world, const Float3 &initialposition,
            const Float4 &initialrotation)
    {
        ObjectID newEntity = world.CreateEntity();
        
        world.Create_Position(newEntity, initialposition, initialrotation);

        world.Create_Camera(newEntity);

        return newEntity;
    }

};

}

