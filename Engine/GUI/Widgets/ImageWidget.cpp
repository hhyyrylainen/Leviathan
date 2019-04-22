// ------------------------------------ //
#include "ImageWidget.h"

#include "GUI/BaseGuiContainer.h"
#include "Rendering/GeometryHelpers.h"

#include "Hlms/Unlit/OgreHlmsUnlit.h"
#include "Hlms/Unlit/OgreHlmsUnlitDatablock.h"
#include "OgreItem.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"
#include "OgreSubMesh2.h"
#include "OgreTechnique.h"

#include "OgreRoot.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT ImageWidget::ImageWidget(const std::string& image, int width, int height) :
    ImageName(image), Width(width), Height(height)
{}

DLLEXPORT ImageWidget::~ImageWidget() {}
// ------------------------------------ //
DLLEXPORT void ImageWidget::SetImage(const std::string& imagename)
{
    ImageName = imagename;

    // Update the rendering if this widget is already displayed
    DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT void ImageWidget::_AcquireRenderResources()
{
    QuadMesh = GeometryHelpers::CreateScreenSpaceQuad(
        "image_widget_" + std::to_string(ID) + "_mesh", 0, 0, Width, Height);

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsTextureManager* hlmsTextureManager = hlmsManager->getTextureManager();

    Ogre::HlmsUnlit* hlmsUnlit =
        static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    const auto datablockName = "image_widget_" + std::to_string(ID);

    Ogre::HlmsUnlitDatablock* datablock = static_cast<Ogre::HlmsUnlitDatablock*>(
        hlmsUnlit->createDatablock(datablockName, datablockName, Ogre::HlmsMacroblock(),
            Ogre::HlmsBlendblock(), Ogre::HlmsParamVec()));

    // Ogre::HlmsTextureManager::TextureLocation texLocation =
    //     hlmsTextureManager->createOrRetrieveTexture(
    //         ImageName, Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);
    Ogre::HlmsTextureManager::TextureLocation texLocation =
        hlmsTextureManager->createOrRetrieveTexture(
            "patchy_cement1_Base_Color.png", Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

    datablock->setTexture(0, texLocation.xIdx, texLocation.texture);

    QuadItem = ContainedIn->GetScene()->createItem(QuadMesh, Ogre::SCENE_DYNAMIC);
    // QuadItem = ContainedIn->GetScene()->createItem("UnitCube.mesh",
    //     Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
    QuadItem->setDatablock(datablock);

    Node = ContainedIn->GetParentForWidgets()->createChildSceneNode(Ogre::SCENE_DYNAMIC);

    Node->attachObject(QuadItem);
}

DLLEXPORT void ImageWidget::_ReleaseRenderResources()
{
    Ogre::SceneManager* scene = ContainedIn->GetScene();

    if(Node) {
        scene->destroySceneNode(Node);
        Node = nullptr;
    }

    if(QuadItem) {
        scene->destroyItem(QuadItem);
        QuadItem = nullptr;
    }

    if(QuadMesh) {
        Ogre::MeshManager::getSingleton().remove(QuadMesh);
        QuadMesh.reset();
    }

    const auto datablockName = "image_widget_" + std::to_string(ID);

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsUnlit* hlmsUnlit =
        static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    hlmsUnlit->destroyDatablock(datablockName);
}

void ImageWidget::SetPosition(float x, float y)
{
    // if(Node)
    //     Node->setPosition(x, y, 0);

    Node->setPosition(0, 0, 0);
}
