// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Widget.h"

#include "OgreMesh2.h"

namespace Leviathan { namespace GUI {

//! \brief A simple image showing widget
class ImageWidget : public Widget {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT ImageWidget(const std::string& image);

public:
    DLLEXPORT ~ImageWidget();

    //! \brief Sets the image shown by this widget
    DLLEXPORT void SetImage(const std::string& imagename);


    void SetPosition(float x, float y);


    REFERENCE_COUNTED_PTR_TYPE(ImageWidget);

protected:
    DLLEXPORT virtual void _AcquireRenderResources() override;
    DLLEXPORT virtual void _ReleaseRenderResources() override;

private:
    std::string ImageName;

    int Width;
    int Height;

    Ogre::SceneNode* Node = nullptr;
    Ogre::Item* QuadItem = nullptr;
    Ogre::MeshPtr QuadMesh;
};

}} // namespace Leviathan::GUI
