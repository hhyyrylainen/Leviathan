// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <boost/intrusive_ptr.hpp>

namespace Ogre {
class SceneManager;
class SceneNode;
} // namespace Ogre

namespace Leviathan { namespace GUI {

class Widget;

//! \brief Base class for all types that can contain Widget objects
class BaseGuiContainer {
public:
    //! \brief This recalculates the layout of child widgets.
    //!
    //! This is called when the parent changes size. Or a child widget changes the size it
    //! wants
    DLLEXPORT virtual void OnSizeChanged() = 0;

    //! \brief Adds a widget to the end of this container
    //! \returns False if can't be added. Any exceptions will be printed to the log
    DLLEXPORT virtual bool AddWidget(const boost::intrusive_ptr<Widget>& widget) = 0;

    //! \brief Removes a widget from this container
    DLLEXPORT virtual bool RemoveWidget(Widget* widget) = 0;

    //! \brief This is the scene node that Widget inside this container will use. This allows
    //! easily moveable panels
    DLLEXPORT virtual Ogre::SceneNode* GetParentForWidgets() = 0;

    //! \brief This is the scene where all widgets added to this container render themselves in
    //!
    //! This allows things like some containers being a RTT instead of a main scene on a window
    DLLEXPORT virtual Ogre::SceneManager* GetScene() = 0;
};


}} // namespace Leviathan::GUI
