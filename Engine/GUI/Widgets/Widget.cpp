// ------------------------------------ //
#include "Widget.h"

#include "GUI/BaseGuiContainer.h"
#include "Rendering/GeometryHelpers.h"

#include "Exceptions.h"
#include "Handlers/IDFactory.h"

#include "Hlms/Unlit/OgreHlmsUnlit.h"
#include "Hlms/Unlit/OgreHlmsUnlitDatablock.h"
#include "OgreItem.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT Widget::Widget() : ID(IDFactory::GetID()) {}
DLLEXPORT Widget::~Widget()
{
    LEVIATHAN_ASSERT(
        ContainedIn == nullptr, "Widget destroyed before being removed from container");
}
// ------------------------------------ //
DLLEXPORT void Widget::OnAddedToContainer(BaseGuiContainer* container)
{
    if(ContainedIn)
        throw InvalidState("Widget added to container while it was in a container");

    ContainedIn = container;
    _AcquireRenderResources();
}

DLLEXPORT void Widget::OnRemovedFromContainer(BaseGuiContainer* container)
{
    if(container != ContainedIn)
        throw InvalidArgument("Widget removed from container that it wasn't in");

    _ReleaseRenderResources();
    ContainedIn = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Widget::PerformOwnPositioning()
{
    if(PositionMode == POSITION_MODE::Manual)
        return;

    throw InvalidType(
        "This widget type can't be a top level widget (without manual positioning mode)");
}
// ------------------------------------ //
DLLEXPORT void Widget::SetLayer(uint8_t layer)
{
    Layer = layer;
    _ApplyLayer();
}
// ------------------------------------ //
// WidgetWithSceneNode
DLLEXPORT WidgetWithStandardResources::WidgetWithStandardResources() {}

DLLEXPORT WidgetWithStandardResources::~WidgetWithStandardResources()
{
    LEVIATHAN_ASSERT(Node == nullptr,
        "WidgetWithStandardResources has not been released before being destroyed");
}
// ------------------------------------ //
DLLEXPORT void WidgetWithStandardResources::SetPixelPosition(int x, int y)
{
    X = x;
    Y = y;

    if(Node) {
        Node->setPosition(X, -Y, 0);
    }
}

DLLEXPORT void WidgetWithStandardResources::SetAllocatedSize(int width, int height)
{
    if(!Node)
        return;

    if(width == NaturalWidth && height == NaturalHeight) {

        Node->setScale(1, 1, 1);
        return;
    }

    const float scaleWidth = width / static_cast<float>(NaturalWidth);
    const float scaleHeight = height / static_cast<float>(NaturalHeight);

    const auto scale = std::min(scaleWidth, scaleHeight);

    const int widthDifference = width - NaturalWidth * scale;
    const int heightDifference = height - NaturalHeight * scale;

    Node->setScale(scale, scale, 1);
    Node->setPosition(X + widthDifference / 2, -(Y + heightDifference / 2), 0);
}

DLLEXPORT void WidgetWithStandardResources::QueryPreferredSize(int& width, int& height) const
{
    width = NaturalWidth;
    height = NaturalHeight;
}
// ------------------------------------ //
DLLEXPORT void WidgetWithStandardResources::_AcquireRenderResources()
{
    Node = ContainedIn->GetParentForWidgets()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
}

DLLEXPORT void WidgetWithStandardResources::_ReleaseRenderResources()
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

    if(Node) {
        scene->destroySceneNode(Node);
        Node = nullptr;
    }

    _ReleaseDatablockIfCreated();
}

DLLEXPORT void WidgetWithStandardResources::_ReleaseDatablockIfCreated()
{
    if(DatablockCreated) {
        DatablockCreated = false;

        Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
        Ogre::HlmsUnlit* hlmsUnlit =
            static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

        hlmsUnlit->destroyDatablock(DatablockName);
    }
}
// ------------------------------------ //
DLLEXPORT void WidgetWithStandardResources::_ApplyLayer()
{
    if(Item)
        Item->setRenderQueueGroup(Layer);
}
// ------------------------------------ //
DLLEXPORT void WidgetWithStandardResources::_CreateDatablockWithTexture(
    int index, const Ogre::TexturePtr& texture)
{
    LEVIATHAN_ASSERT(
        DatablockCreated == false, "trying to make another datablock while one exists");

    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();

    Ogre::HlmsUnlit* hlmsUnlit =
        static_cast<Ogre::HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));

    const bool needsTransparency = texture->hasAlpha();

    Ogre::HlmsMacroblock macroBlock;

    // TODO: as sorting is already done with layers it might not be needed to have depth write
    // on at all

    if(needsTransparency) {
        macroBlock.mDepthWrite = false;
        // transparentMacro.mDepthCheck = false;
    }

    Ogre::HlmsMacroblock normalMacro;

    Ogre::HlmsBlendblock blendblock;
    if(needsTransparency) {
        blendblock.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);
        // blendblock.setBlendType(Ogre::SBT_ADD);
        // blendblock.mSourceBlendFactor = Ogre::SBF_ONE;
        // blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;

        // blendblock.mSourceBlendFactor = Ogre::SBF_ONE;
        // blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
        // blendblock.mSourceBlendFactor = Ogre::SBF_SOURCE_ALPHA;
        // blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    }

    Datablock = static_cast<Ogre::HlmsUnlitDatablock*>(hlmsUnlit->createDatablock(
        DatablockName, DatablockName, macroBlock, blendblock, Ogre::HlmsParamVec()));
    LEVIATHAN_ASSERT(Datablock, "datablock creation failed");

    // datablock->setAlphaTest(Ogre::)

    Datablock->setTexture(0, index, texture);

    DatablockCreated = true;
}

DLLEXPORT void WidgetWithStandardResources::_CreateDatablockWithTextureName(
    const std::string& texture, int* widthreceiver /*= nullptr*/,
    int* heightreceiver /*= nullptr*/, bool* hasalphareceiver /*= nullptr*/)
{
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsTextureManager* hlmsTextureManager = hlmsManager->getTextureManager();

    Ogre::HlmsTextureManager::TextureLocation texLocation =
        hlmsTextureManager->createOrRetrieveTexture(
            texture, Ogre::HlmsTextureManager::TEXTURE_TYPE_DIFFUSE);

    if(widthreceiver)
        *widthreceiver = texLocation.texture->getWidth();
    if(heightreceiver)
        *heightreceiver = texLocation.texture->getHeight();
    if(hasalphareceiver)
        *hasalphareceiver = texLocation.texture->hasAlpha();

    _CreateDatablockWithTexture(texLocation.xIdx, texLocation.texture);
}

DLLEXPORT void WidgetWithStandardResources::_CreateStandardMeshAndItem(
    const std::string& name, float width, float height)
{
    if(Mesh)
        LEVIATHAN_ASSERT(false,
            "duplicate call to WidgetWithStandardResources::_CreateStandardMeshAndItem");

    Mesh = GeometryHelpers::CreateWidgetGeometry(name, width, height);
    Item = ContainedIn->GetScene()->createItem(Mesh, Ogre::SCENE_DYNAMIC);
    if(Datablock)
        Item->setDatablock(Datablock);

    _ApplyLayer();

    if(Node)
        Node->attachObject(Item);
}
