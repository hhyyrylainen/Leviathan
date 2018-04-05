// ------------------------------------ //
#include "Sample1.h"

#include "Generated/StandardWorld.h"

#include "Hlms/Pbs/OgreHlmsPbs.h"
#include "Hlms/Pbs/OgreHlmsPbsDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsTextureManager.h"
#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreSceneManager.h"

using namespace Demos;
// ------------------------------------ //
void Sample1::Start(Leviathan::StandardWorld& world)
{
    // This method contains code directly copied from Ogre see License.txt for the Ogre
    // license. This code has been modified
    Ogre::SceneManager* sceneManager = world.GetScene();

    const float armsLength = 2.5f;

    Ogre::v1::MeshPtr planeMeshV1 = Ogre::v1::MeshManager::getSingleton().createPlane(
        "Plane v1", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::Plane(Ogre::Vector3::UNIT_Y, 1.0f), 50.0f, 50.0f, 1, 1, true, 1, 4.0f, 4.0f,
        Ogre::Vector3::UNIT_Z, Ogre::v1::HardwareBuffer::HBU_STATIC,
        Ogre::v1::HardwareBuffer::HBU_STATIC);

    Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createManual(
        "Plane", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    planeMesh->importV1(planeMeshV1.get(), true, true, true);

    {
        Ogre::Item* item = sceneManager->createItem(planeMesh, Ogre::SCENE_DYNAMIC);
        item->setDatablock("Marble");
        Ogre::SceneNode* sceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)
                                         ->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        sceneNode->setPosition(0, -1, 0);
        sceneNode->attachObject(item);

        // Change the addressing mode of the roughness map to wrap via code.
        // Detail maps default to wrap, but the rest to clamp.
        LEVIATHAN_ASSERT(
            dynamic_cast<Ogre::HlmsPbsDatablock*>(item->getSubItem(0)->getDatablock()),
            "Datablock get failed");
        Ogre::HlmsPbsDatablock* datablock =
            static_cast<Ogre::HlmsPbsDatablock*>(item->getSubItem(0)->getDatablock());
        // Make a hard copy of the sampler block
        Ogre::HlmsSamplerblock samplerblock(*datablock->getSamplerblock(Ogre::PBSM_ROUGHNESS));
        samplerblock.mU = Ogre::TAM_WRAP;
        samplerblock.mV = Ogre::TAM_WRAP;
        samplerblock.mW = Ogre::TAM_WRAP;
        // Set the new samplerblock. The Hlms system will
        // automatically create the API block if necessary
        datablock->setSamplerblock(Ogre::PBSM_ROUGHNESS, samplerblock);
    }

    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            Ogre::String meshName;

            if(i == j)
                meshName = "Sphere1000.mesh";
            else
                meshName = "Cube_d.mesh";

            Ogre::Item* item = sceneManager->createItem(meshName,
                Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                Ogre::SCENE_DYNAMIC);
            if(i % 2 == 0)
                item->setDatablock("Rocks");
            else
                item->setDatablock("Marble");

            item->setVisibilityFlags(0x000000001);

            size_t idx = i * 4 + j;

            Ogre::SceneNode* node = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)
                                        ->createChildSceneNode(Ogre::SCENE_DYNAMIC);

            node->setPosition((i - 1.5f) * armsLength, 2.0f, (j - 1.5f) * armsLength);
            node->setScale(0.65f, 0.65f, 0.65f);

            node->roll(Ogre::Radian((Ogre::Real)idx));

            node->attachObject(item);
        }
    }

    // TODO: make this work
    // {
    //     int mNumSpheres = 0;
    //     Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    //     Ogre::HlmsTextureManager* hlmsTextureManager = hlmsManager->getTextureManager();

    //     assert(dynamic_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS)));

    //     Ogre::HlmsPbs* hlmsPbs =
    //         static_cast<Ogre::HlmsPbs*>(hlmsManager->getHlms(Ogre::HLMS_PBS));

    //     const int numX = 8;
    //     const int numZ = 8;

    //     const float armsLength = 1.0f;
    //     const float startX = (numX - 1) / 2.0f;
    //     const float startZ = (numZ - 1) / 2.0f;

    //     for(int x = 0; x < numX; ++x) {
    //         for(int z = 0; z < numZ; ++z) {
    //             Ogre::String datablockName =
    //                 "Test" + Ogre::StringConverter::toString(mNumSpheres++);
    //             Ogre::HlmsPbsDatablock* datablock = static_cast<Ogre::HlmsPbsDatablock*>(
    //                 hlmsPbs->createDatablock(datablockName, datablockName,
    //                     Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(),
    //                     Ogre::HlmsParamVec()));

    //             Ogre::HlmsTextureManager::TextureLocation texLocation =
    //                 hlmsTextureManager->createOrRetrieveTexture("SaintPetersBasilica.dds",
    //                     Ogre::HlmsTextureManager::TEXTURE_TYPE_ENV_MAP);

    //             datablock->setTexture(
    //                 Ogre::PBSM_REFLECTION, texLocation.xIdx, texLocation.texture);
    //             datablock->setDiffuse(Ogre::Vector3(0.0f, 1.0f, 0.0f));

    //             datablock->setRoughness(std::max(0.02f, x / Ogre::max(1, (float)(numX -
    //             1)))); datablock->setFresnel(
    //                 Ogre::Vector3(z / Ogre::max(1, (float)(numZ - 1))), false);

    //             Ogre::Item* item = sceneManager->createItem("Sphere1000.mesh",
    //                 Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
    //                 Ogre::SCENE_DYNAMIC);
    //             item->setDatablock(datablock);
    //             item->setVisibilityFlags(0x000000002);

    //             Ogre::SceneNode* sceneNode =
    //                 sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC)
    //                     ->createChildSceneNode(Ogre::SCENE_DYNAMIC);
    //             sceneNode->setPosition(
    //                 Ogre::Vector3(armsLength * x - startX, 1.0f, armsLength * z - startZ));
    //             sceneNode->attachObject(item);
    //         }
    //     }
    // }

    Ogre::SceneNode* rootNode = sceneManager->getRootSceneNode();

    Ogre::Light* light = sceneManager->createLight();
    Ogre::SceneNode* lightNode = rootNode->createChildSceneNode();
    lightNode->attachObject(light);
    light->setDiffuseColour(0.8f, 0.4f, 0.2f); // Warm
    light->setSpecularColour(0.8f, 0.4f, 0.2f);
    light->setPowerScale(Ogre::Math::PI);
    light->setType(Ogre::Light::LT_SPOTLIGHT);
    lightNode->setPosition(-10.0f, 10.0f, 10.0f);
    light->setDirection(Ogre::Vector3(1, -1, -1).normalisedCopy());
    light->setAttenuationBasedOnRadius(10.0f, 0.01f);

    // mLightNodes[1] = lightNode;

    light = sceneManager->createLight();
    lightNode = rootNode->createChildSceneNode();
    lightNode->attachObject(light);
    light->setDiffuseColour(0.2f, 0.4f, 0.8f); // Cold
    light->setSpecularColour(0.2f, 0.4f, 0.8f);
    light->setPowerScale(Ogre::Math::PI);
    light->setType(Ogre::Light::LT_SPOTLIGHT);
    lightNode->setPosition(10.0f, 10.0f, -10.0f);
    light->setDirection(Ogre::Vector3(-1, -1, 1).normalisedCopy());
    light->setAttenuationBasedOnRadius(10.0f, 0.01f);

    // mLightNodes[2] = lightNode;
}
