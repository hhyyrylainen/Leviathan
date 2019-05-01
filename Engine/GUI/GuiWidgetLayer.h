// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "BaseGuiContainer.h"
#include "GUI/Widgets/Widget.h"
#include "GuiLayer.h"

#include "OgreSceneManager.h"

namespace Leviathan { namespace GUI {

//! \brief All Leviathan Widget objects need to be contained in a container for rendering
//! \todo This is currenlty hardcoded to work with GuiManager::PlayCutscene if the plan for a
//! custom GUI system is to go forward this needs to be generalized
class WidgetLayer final : public Layer, public BaseGuiContainer {
public:
    DLLEXPORT WidgetLayer(GuiManager* owner, Window* window, int renderorder);
    DLLEXPORT ~WidgetLayer();

    DLLEXPORT void OnRender(float passed) override;

    DLLEXPORT void RemoveAllWidgets();

    // BaseGuiContainer interface
    DLLEXPORT bool AddWidget(const boost::intrusive_ptr<Widget>& widget) override;
    DLLEXPORT bool RemoveWidget(Widget* widget) override;

    DLLEXPORT void OnSizeChanged() override;

    DLLEXPORT inline Ogre::SceneNode* GetParentForWidgets() override
    {
        return Layer::GetScene()->getRootSceneNode(Ogre::SCENE_DYNAMIC);
    }

    DLLEXPORT inline Ogre::SceneManager* GetScene() override
    {
        return Layer::GetScene();
    }

    DLLEXPORT void GetInnerSize(int& width, int& height) override
    {
        width = Width;
        height = Height;
    }

protected:
    // Layer interface
    DLLEXPORT void _DoReleaseResources() override;
    DLLEXPORT void _OnWindowResized() override;
    DLLEXPORT void _OnFocusChanged() override;

private:
    std::vector<boost::intrusive_ptr<Widget>> Widgets;

    //! Used to defer layout updates until next render
    bool LayoutDirty = true;
};

}} // namespace Leviathan::GUI
