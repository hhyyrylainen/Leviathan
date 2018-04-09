// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "OgreMesh2.h"

namespace Leviathan {

//! \brief Helpers for generating procedural geometry
class GeometryHelpers {
public:
    //! \brief Creates a screen space plane with UV coordinates
    //!
    //! The coordinates are in screen space -1 - 1 where -1 is the left and top of the screen
    //! so a full screen quad would be CreateScreenSpaceQuad(-1, -1, 2, 2)
    //! \param meshname The name of the created mesh, must be unique
    //! \todo Parameter to specify if shadow geometry is needed
    DLLEXPORT static Ogre::MeshPtr CreateScreenSpaceQuad(
        const std::string& meshname, float x, float y, float width, float height);
};

} // namespace Leviathan
