// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

namespace Leviathan { namespace GUI {

class WidgetLayer;

//! \brief Base class for all Leviathan GUI widgets
class Widget : public ReferenceCounted {
    friend WidgetLayer;

protected:
    DLLEXPORT Widget();

public:
    DLLEXPORT virtual ~Widget();

    //! \brief Allows widget with animations or time keeping to update themselves
    DLLEXPORT virtual void Tick(float elapsed);

    //! \brief Called to render this widget
    DLLEXPORT virtual void Render() = 0;

protected:
    // These are called by the widget container when this is added or removed. This is the
    // recommended place to acquire rendering resources
    DLLEXPORT virtual void OnAddedToContainer(WidgetLayer* container);
    DLLEXPORT virtual void OnRemovedFromContainer(WidgetLayer* container);

public:
    REFERENCE_COUNTED_PTR_TYPE(Widget);

protected:
    const int ID;
    bool RequiresTick = false;
};

}} // namespace Leviathan::GUI
