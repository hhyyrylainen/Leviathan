// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/VideoPlayer.h"
#include "Widget.h"

namespace Leviathan { namespace GUI {

//! \brief Hosts a Leviathan::VideoPlayer in a Widget for diplay in the GUI
//! \todo This is currenlty hardcoded to work with GuiManager::PlayCutscene if the plan for a
//! custom GUI system is to go forward this needs to be generalized
class VideoPlayerWidget : public Widget {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT VideoPlayerWidget();

public:
    DLLEXPORT ~VideoPlayerWidget();

    DLLEXPORT void Tick() override {}

    //! \copydoc VideoPlayer::Play
    DLLEXPORT bool Play(const std::string& videofile);

    //! \brief Stops the currently playing video.
    //!
    //! This will call Callback.
    //! \todo The player should be changed to call our callback when it is stopped
    DLLEXPORT void Stop();

    //! \brief Sets callback to call when video has ended
    DLLEXPORT void SetEndCallback(std::function<void()> callback);


    REFERENCE_COUNTED_PTR_TYPE(VideoPlayerWidget);

protected:
    DLLEXPORT void OnAddedToContainer(WidgetLayer* container) override;
    DLLEXPORT void OnRemovedFromContainer(WidgetLayer* container) override;

private:
    void _DoCallback();

private:
    VideoPlayer Player;

    //! Used to call callback only once
    bool CanCallCallback = false;
    std::function<void()> Callback;

    WidgetLayer* ContainedIn = nullptr;

    // Ogre::MaterialPtr Material;
    // Ogre::SceneNode* Node = nullptr;
    // Ogre::Item* QuadItem = nullptr;
    // Ogre::MeshPtr QuadMesh;
};

}} // namespace Leviathan::GUI
