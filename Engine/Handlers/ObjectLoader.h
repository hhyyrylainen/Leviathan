// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Entities/Components.h"
#include "Entities/GameWorld.h"

namespace Leviathan {

//! \brief Class to collect all entity creations to one class
//! \note All created objects are added to the world so that they are broadcast on
//! the network (if this is a server)
//! \todo Allow objects to be created that will be sent to clients only after the caller has
//! had the chance to create constraints etc.
class ObjectLoader {
public:
    ObjectLoader() = delete;

    //! \brief Creates a full camera entity that can be usedas the
    //! primary world camera and sound listener
    template<class TWorldClass>
    static ObjectID LoadCamera(
        TWorldClass& world, const Float3& initialposition, const Float4& initialrotation)
    {
        ObjectID newEntity = world.CreateEntity();

        world.Create_Position(newEntity, initialposition, initialrotation);

        world.Create_Camera(newEntity);

        return newEntity;
    }

    // //! \brief Creates a plane that has a material applied to it
    // template<class TWorldClass>
    // static ObjectID LoadPlane(TWorldClass& world, const Float3& initialposition,
    //     const Float4& initialrotation, const std::string& materialname,
    //     const Ogre::Plane& plane, const Float2& planesize)
    // {
    //     ObjectID newEntity = world.CreateEntity();

    //     world.Create_Position(newEntity, initialposition, initialrotation);

    //     RenderNode& node = world.Create_RenderNode(newEntity);

    //     world.Create_Plane(newEntity, node.Node, materialname, plane, planesize);

    //     return newEntity;
    // }
};

} // namespace Leviathan
