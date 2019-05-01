// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"

namespace Ogre {
class HlmsUnlitDatablock;
}

namespace Leviathan { namespace GUI {

class BaseGuiContainer;

//! \brief The positioning mode for a widget. Controls how it should be moved around by the
//! parent container
enum class POSITION_MODE { Automatic = 1, Manual, Invalid };

//! \brief Base class for all Leviathan GUI widgets
class Widget : public ReferenceCounted {
protected:
    DLLEXPORT Widget();

public:
    DLLEXPORT virtual ~Widget();

    DLLEXPORT virtual void OnRender(float passed) {}

    // These are called by the widget container when this is added or removed.
    // These are not protected to not have to do trickery to allow all container types to call
    // these.
    //! \protected
    DLLEXPORT virtual void OnAddedToContainer(BaseGuiContainer* container);
    //! \protected
    DLLEXPORT virtual void OnRemovedFromContainer(BaseGuiContainer* container);

    // Positioning functions. These should be only called by the
    // container this widget is in unless this is in manual position mode
    //! \brief This is the main positioning function. This is only understood by "top level"
    //! widgets (Panel etc) and this is only used when this widget is in a container that does
    //! not position the children (for example WidgetLayer)
    //! \exception InvalidType if this Widget cannot be a top level widget. Won't be thrown if
    //! this is set into manual positioning mode
    DLLEXPORT virtual void PerformOwnPositioning();

    //! \brief Set the draw order. If more than 255 layers are needed a container with RTT
    //! enables need to be added
    DLLEXPORT virtual void SetLayer(uint8_t layer);

    //! \brief Sets the final pixel offset related to parent
    //! \note Due to the way the camera works this flips the Y coordinate in order to be from
    //! top to bottom
    DLLEXPORT virtual void SetPixelPosition(int x, int y) = 0;

    //! \brief Sets the size allocated for this widget
    //!
    //! The widget needs to cover the whole size if at all possible. The widget can tell the
    //! size it wants with QueryPreferredSize and related methods
    DLLEXPORT virtual void SetAllocatedSize(int width, int height) = 0;

    //! \brief This should return the preferred / natural size of this widget
    DLLEXPORT virtual void QueryPreferredSize(int& width, int& height) const = 0;

    //! \brief Some widgets can be squeezed. If so this returns the minimum size this widget
    //! can have
    DLLEXPORT virtual void QueryMinimumSize(int& width, int& height) const
    {
        QueryPreferredSize(width, height);
    }

    //! \returns True if this widget is allowed to be enlarged above the preferred size in
    //! order to fill up empty space
    DLLEXPORT virtual bool CanExpand() const
    {
        return false;
    }

    //! \brief Sets the positioning mode of this Widget
    //!
    //! This is most useful for temporary widgets that are manually placed instead of placed in
    //! a container like Panel for automatic positioning.
    DLLEXPORT virtual void SetPositionMode(POSITION_MODE mode)
    {
        PositionMode = mode;
    }

    DLLEXPORT inline auto GetPositionMode() const
    {
        return PositionMode;
    }

protected:
    //! This is the recommended place to acquire rendering resources
    virtual void _AcquireRenderResources() = 0;
    virtual void _ReleaseRenderResources() = 0;

    virtual void _ApplyLayer() = 0;

public:
    REFERENCE_COUNTED_PTR_TYPE(Widget);

protected:
    const int ID;
    BaseGuiContainer* ContainedIn = nullptr;

    uint8_t Layer = 0;
    POSITION_MODE PositionMode = POSITION_MODE::Automatic;
};

//! \brief Base class for widgets using an Ogre scene node, an item and a mesh for drawing
class WidgetWithStandardResources : public Widget {
protected:
    DLLEXPORT WidgetWithStandardResources();

public:
    DLLEXPORT ~WidgetWithStandardResources();

    // Default implementations for Widget methods
    DLLEXPORT virtual void SetPixelPosition(int x, int y) override;
    //! This function doesn't know if it is safe to change aspect
    //! ratio so what this does is scale the smaller, safe amount
    //! based on either the width or height difference and then
    //! positions this to be centered in the allocation area
    DLLEXPORT virtual void SetAllocatedSize(int width, int height) override;
    DLLEXPORT virtual void QueryPreferredSize(int& width, int& height) const override;

protected:
    // Default implementations for Widget methods
    DLLEXPORT virtual void _AcquireRenderResources() override;
    DLLEXPORT virtual void _ReleaseRenderResources() override;
    DLLEXPORT virtual void _ApplyLayer() override;


    DLLEXPORT virtual void _ReleaseDatablockIfCreated();

    //! \brief Helper for creating an unlit datablock with a single texture on it
    DLLEXPORT void _CreateDatablockWithTexture(int index, const Ogre::TexturePtr& texture);

    //! Variant with automatic texture fetching, also optionally returns the size
    DLLEXPORT void _CreateDatablockWithTextureName(const std::string& texture,
        int* widthreceiver = nullptr, int* heightreceiver = nullptr,
        bool* hasalphareceiver = nullptr);

    //! \brief Helper for making regular meshes and items from them easier
    DLLEXPORT void _CreateStandardMeshAndItem(
        const std::string& name, float width, float height);



protected:
    Ogre::SceneNode* Node = nullptr;
    Ogre::Item* Item = nullptr;
    Ogre::MeshPtr Mesh;

    //! The name of the datablock created by this widget or used by this widget
    //! \note This should not be touched after construction
    std::string DatablockName;

    //! If true automatically destroys datablock
    bool DatablockCreated = false;
    Ogre::HlmsUnlitDatablock* Datablock = nullptr;

    // For scaling the size
    int NaturalWidth = -1;
    int NaturalHeight = -1;

    // Stored position for centering
    int X = -1;
    int Y = -1;
};


}} // namespace Leviathan::GUI
