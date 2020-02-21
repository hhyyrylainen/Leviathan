// ------------------------------------ //
#include "VideoPlayerWidget.h"

#include "Engine.h"
#include "GUI/GuiManager.h"
#include "GUI/GuiWidgetLayer.h"
#include "Rendering/GeometryHelpers.h"
#include "Rendering/Graphics.h"
#include "Rendering/Texture.h"


using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT VideoPlayerWidget::VideoPlayerWidget()
{
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
    if(!Player.Play(videofile))
        return false;

    CanCallCallback = true;

    // Create a texture for displaying the decoded video
    VideoTexture = Engine::Get()->GetGraphics()->CreateDynamicTexture(
        Player.GetVideoWidth(), Player.GetVideoHeight(), VIDEO_PLAYER_DILIGENT_PIXEL_FORMAT);

    LEVIATHAN_ASSERT(VideoTexture, "failed to create player widget texture");

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
DLLEXPORT void VideoPlayerWidget::Render()
{
    if(!VideoTexture)
        return;

    // Uses just a full screen quad to render for now
    if(!ResourcesDirty || !QuadMesh)
        QuadMesh = GeometryHelpers::CreateQuad(0, 0, 100.f, 100.f);


    // We use a dynamic texture so it needs to be written every frame
    Diligent::Box box;
    box.MinX = 0;
    box.MinY = 0;
    box.MaxX = Player.GetVideoWidth();
    box.MaxY = Player.GetVideoHeight();

    Diligent::TextureSubResData data;
    data.Stride = Player.GetVideoWidth() * VIDEO_PLAYER_BYTES_PER_PIXEL;
    data.pData = Player.GetTextureData().data();

    Engine::Get()->GetGraphics()->WriteDynamicTextureData(*VideoTexture, 0, 0, box, data);


    // No offset
    float x = 0;
    float y = 0;

    ContainedIn->GetGuiManager()->GetRenderer().DrawTransparentWithAlpha(
        *QuadMesh, *VideoTexture, x, y);
}
// ------------------------------------ //
DLLEXPORT void VideoPlayerWidget::OnAddedToContainer(WidgetLayer* container)
{
    ContainedIn = container;

    // LOG_WRITE("TODO: redo VideoPlayerWidget::OnAddedToContainer");

    // QuadMesh = GeometryHelpers::CreateScreenSpaceQuad(
    //     "videoplayer_widget_" + std::to_string(ID) + "_mesh", -1, -1, 2, 2);

    // // Duplicate the material
    // Ogre::MaterialPtr baseMaterial =
    //     Ogre::MaterialManager::getSingleton().getByName("GUIOverlay");

    // if(!baseMaterial)
    //     LOG_FATAL(
    //         "VideoPlayerWidget: GUIOverlay material doesn't exists! are the core Leviathan "
    //         "materials and shaders copied?");

    // Material = baseMaterial->clone("videoplayer_widget_" + std::to_string(ID) +
    // "_material");

    // Ogre::SceneManager* scene = ContainedIn->GetScene();

    // Node = scene->createSceneNode(Ogre::SCENE_STATIC);

    // // Setup render queue for it
    // // TODO: a proper system needs to be done for managing what is on top of what
    // scene->getRenderQueue()->setRenderQueueMode(2, Ogre::RenderQueue::FAST);
}

DLLEXPORT void VideoPlayerWidget::OnRemovedFromContainer(WidgetLayer* container)
{
    if(!ContainedIn)
        return;

    // LOG_WRITE("TODO: redo VideoPlayerWidget::OnRemovedFromContainer");

    // Ogre::SceneManager* scene = ContainedIn->GetScene();

    // if(Node) {
    //     scene->destroySceneNode(Node);
    //     Node = nullptr;
    // }

    // if(QuadItem) {
    //     scene->destroyItem(QuadItem);
    //     QuadItem = nullptr;
    // }

    // if(QuadMesh) {
    //     Ogre::MeshManager::getSingleton().remove(QuadMesh);
    //     QuadMesh.reset();
    // }

    // if(Material) {
    //     Ogre::MaterialManager::getSingleton().remove(Material);
    //     Material.reset();
    // }
}
