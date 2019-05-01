// ------------------------------------ //
#include "VideoPlayerWidget.h"

#include "GUI/BaseGuiContainer.h"

#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT VideoPlayerWidget::VideoPlayerWidget()
{
    DatablockName = "video_widget_" + std::to_string(ID);

    Player.OnPlayBackEnded.Register(LambdaDelegateSlot::MakeShared<LambdaDelegateSlot>(
        [=](const NamedVars::pointer& values) { _DoCallback(); }));
}

DLLEXPORT VideoPlayerWidget::~VideoPlayerWidget()
{
    Callback = nullptr;
}
// ------------------------------------ //
DLLEXPORT bool VideoPlayerWidget::Play(const std::string& videofile)
{
    _ReleaseSingleRunResources();

    if(!Player.Play(videofile))
        return false;

    CanCallCallback = true;

    NaturalWidth = Player.GetVideoWidth();
    NaturalHeight = Player.GetVideoHeight();

    // The Play method creates the texture we want to display
    // So we do the item and material setup here
    _CreateDatablockWithTexture(0, Player.GetTexture());

    _CreateStandardMeshAndItem(
        "video_widget_" + std::to_string(ID) + "_mesh", NaturalWidth, NaturalHeight);
    // 200, 200);

    ContainedIn->OnSizeChanged();
    return true;
}

DLLEXPORT void VideoPlayerWidget::Stop()
{
    Player.Stop();

    _DoCallback();
}
// ------------------------------------ //
DLLEXPORT void VideoPlayerWidget::SetEndCallback(std::function<void()> callback)
{
    Callback = callback;
}

void VideoPlayerWidget::_DoCallback()
{
    if(!CanCallCallback)
        return;

    CanCallCallback = false;

    if(Callback)
        Callback();
}
// ------------------------------------ //
void VideoPlayerWidget::_ReleaseSingleRunResources()
{
    Ogre::SceneManager* scene = ContainedIn->GetScene();

    if(Item) {
        scene->destroyItem(Item);
        Item = nullptr;
    }

    if(Mesh) {
        Ogre::MeshManager::getSingleton().remove(Mesh);
        Mesh.reset();
    }

    _ReleaseDatablockIfCreated();
}

DLLEXPORT void VideoPlayerWidget::PerformOwnPositioning()
{
    SetPixelPosition(0, 0);
    int width, height;
    ContainedIn->GetInnerSize(width, height);
    SetAllocatedSize(width, height);
}
