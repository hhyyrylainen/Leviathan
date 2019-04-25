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

#include "OgreRoot.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT ImageWidget::ImageWidget(const std::string& image) : ImageName(image) {}

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
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsTextureManager* hlmsTextureManager = hlmsManager->getTextureManager();

    Ogre::HlmsUnlit* hlmsUnlit =
        static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    const auto datablockName = GetNameForDatablock();

    Ogre::HlmsBlendblock blendblock;
    // blendblock.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);
    blendblock.mSourceBlendFactor = Ogre::SBF_ONE;
    blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    // blendblock.mSourceBlendFactor = Ogre::SBF_SOURCE_ALPHA;
    // blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;

    Ogre::HlmsUnlitDatablock* datablock =
        static_cast<Ogre::HlmsUnlitDatablock*>(hlmsUnlit->createDatablock(datablockName,
            datablockName, Ogre::HlmsMacroblock(), blendblock, Ogre::HlmsParamVec()));

    Ogre::HlmsTextureManager::TextureLocation texLocation =
        hlmsTextureManager->createOrRetrieveTexture(
            ImageName, Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

    datablock->setTexture(0, texLocation.xIdx, texLocation.texture);

    Width = texLocation.texture->getWidth();
    Height = texLocation.texture->getHeight();

    QuadMesh = GeometryHelpers::CreateWidgetGeometry(
        "image_widget_" + std::to_string(ID) + "_mesh", Width, Height);

    QuadItem = ContainedIn->GetScene()->createItem(QuadMesh, Ogre::SCENE_DYNAMIC);
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

    const auto datablockName = GetNameForDatablock();

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsUnlit* hlmsUnlit =
        static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    hlmsUnlit->destroyDatablock(datablockName);
}

void ImageWidget::SetPosition(float x, float y)
{
    if(Node)
        Node->setPosition(x, -y, Z);
}

void ImageWidget::SetZ(float z)
{
    if(Node) {
        Z = z;
        auto pos = Node->getPosition();
        pos.z = Z;
        Node->setPosition(pos);
    }
}
