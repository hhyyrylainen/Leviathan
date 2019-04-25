// ------------------------------------ //
#include "VideoPlayerWidget.h"

#include "GUI/BaseGuiContainer.h"
#include "Rendering/GeometryHelpers.h"

#include "Hlms/Unlit/OgreHlmsUnlit.h"
#include "Hlms/Unlit/OgreHlmsUnlitDatablock.h"
#include "OgreItem.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"

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
    _ReleaseSingleRunResources();

    if(!Player.Play(videofile))
        return false;

    CanCallCallback = true;

    // The Play method creates the texture we want to display
    // So we do the item and material setup here
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsTextureManager* hlmsTextureManager = hlmsManager->getTextureManager();

    Ogre::HlmsUnlit* hlmsUnlit =
        static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    const auto datablockName = GetNameForDatablock();

    Ogre::HlmsBlendblock blendblock;
    // blendblock.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);
    blendblock.mSourceBlendFactor = Ogre::SBF_ONE;
    blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;

    Ogre::HlmsUnlitDatablock* datablock =
        static_cast<Ogre::HlmsUnlitDatablock*>(hlmsUnlit->createDatablock(datablockName,
            datablockName, Ogre::HlmsMacroblock(), blendblock, Ogre::HlmsParamVec()));

    // Ogre::HlmsTextureManager::TextureLocation texLocation =
    //     hlmsTextureManager->createOrRetrieveTexture(
    //         "flagella_texture.png", Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

    // datablock->setTexture(0, texLocation.xIdx, texLocation.texture);

    datablock->setTexture(0, 0, Player.GetTexture());

    // datablock->setAlphaTest(Ogre::)

    QuadMesh =
        GeometryHelpers::CreateWidgetGeometry("video_widget_" + std::to_string(ID) + "_mesh",
            Player.GetVideoWidth(), Player.GetVideoHeight());
    // 200, 200);

    QuadItem = ContainedIn->GetScene()->createItem(QuadMesh, Ogre::SCENE_DYNAMIC);
    QuadItem->setDatablock(datablock);

    Ogre::SceneManager* scene = ContainedIn->GetScene();

    QuadItem = scene->createItem(QuadMesh, Ogre::SCENE_DYNAMIC);

    Node->attachObject(QuadItem);

    // Center in container
    LOG_WRITE("TODO: Center in container");
    // DEBUG_BREAK;
    Node->setPosition(0, 0, 2);

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
DLLEXPORT void VideoPlayerWidget::_AcquireRenderResources()
{
    Node = ContainedIn->GetParentForWidgets()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
}

void VideoPlayerWidget::_ReleaseSingleRunResources()
{
    Ogre::SceneManager* scene = ContainedIn->GetScene();

    if(QuadItem) {
        scene->destroyItem(QuadItem);
        QuadItem = nullptr;
    }

    if(QuadMesh) {
        Ogre::MeshManager::getSingleton().remove(QuadMesh);
        QuadMesh.reset();
    }

    if(DatablockCreated) {
        DatablockCreated = false;

        const auto datablockName = GetNameForDatablock();

        Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
        Ogre::HlmsUnlit* hlmsUnlit =
            static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

        hlmsUnlit->destroyDatablock(datablockName);
    }
}

DLLEXPORT void VideoPlayerWidget::_ReleaseRenderResources()
{
    Ogre::SceneManager* scene = ContainedIn->GetScene();

    if(Node) {
        scene->destroySceneNode(Node);
        Node = nullptr;
    }

    _ReleaseSingleRunResources();
}
