// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/Types.h"
#include "Mesh.h"

namespace Leviathan {

//! \brief Helpers for generating procedural geometry
class GeometryHelpers {
public:
    struct QuadVertex {
    public:
        Float2 Pos;
        Float2 UV;
    };

    static_assert(sizeof(QuadVertex) == 4 * sizeof(float), "unexpected size");

public:
    //! \brief Creates a screen space plane with UV coordinates
    //!
    //! The coordinates are in screen space -1 - 1 where -1 is the left and top of the screen
    //! so a full screen quad would be CreateScreenSpaceQuad(-1, -1, 2, 2)
    DLLEXPORT static Mesh::pointer CreateScreenSpaceQuad(
        float x, float y, float width, float height);

    //! \brief Creates a quad with Float2 coordinates and Float2 UVs
    DLLEXPORT static Mesh::pointer CreateQuad(
        float left, float top, float width, float height);

    //! \brief Creates a plane with specific size and UV coordinates with 0, 0 on top left
    //!
    //! This plane is flat on the Y axis. The actual width of the mesh will be double of width
    //! to make it easier to calculate
    //! \todo Merge the shared code with CreateScreenSpaceQuad
    //! \todo This needs to be verified that this generates good geometry (currently crashes
    //! when recorded with RenderDoc). This might be outdated note after bsf
    DLLEXPORT static Mesh::pointer CreateXZPlane(float width, float height);

    GeometryHelpers() = delete;
};

} // namespace Leviathan
