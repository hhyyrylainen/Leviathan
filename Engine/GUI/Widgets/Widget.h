// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

namespace Leviathan { namespace GUI {

class BaseGuiContainer;

//! \brief Base class for all Leviathan GUI widgets
class Widget : public ReferenceCounted {
protected:
    DLLEXPORT Widget();

public:
    DLLEXPORT virtual ~Widget();

    DLLEXPORT virtual void Tick(){};

    // These are called by the widget container when this is added or removed.
    // These are not protected to not have to do trickery to allow all container types to call
    // these.
    //! \protected
    DLLEXPORT virtual void OnAddedToContainer(BaseGuiContainer* container);
    //! \protected
    DLLEXPORT virtual void OnRemovedFromContainer(BaseGuiContainer* container);

protected:
    //! This is the recommended place to acquire rendering resources
    virtual void _AcquireRenderResources() = 0;
    virtual void _ReleaseRenderResources() = 0;

public:
    REFERENCE_COUNTED_PTR_TYPE(Widget);

protected:
    const int ID;
    BaseGuiContainer* ContainedIn = nullptr;
};

}} // namespace Leviathan::GUI
