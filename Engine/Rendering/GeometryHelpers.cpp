// ------------------------------------ //
#include "GeometryHelpers.h"

#include <OgreManualObject.h>
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreRoot.h>
#include <OgreSubMesh2.h>

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Ogre::MeshPtr GeometryHelpers::CreateScreenSpaceQuad(
    const std::string& meshname, float x, float y, float width, float height)
{
    Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(
        meshname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    Ogre::SubMesh* subMesh = mesh->createSubMesh();

    Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::VaoManager* vaoManager = renderSystem->getVaoManager();

    Ogre::VertexElement2Vec vertexElements;
    vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
    vertexElements.push_back(
        Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));

    // This is a fullscreen quad in screenspace (so no transform matrix is used)
    float vertexData[] = {// First vertex
        x, y, 0, 0, 0,
        // Second
        x + width, y, 0, 1, 0,
        // Third
        x + width, y + width, 0, 1, 1,
        // Fourth
        x, y + width, 0, 0, 1};

    Ogre::VertexBufferPacked* vertexBuffer = vaoManager->createVertexBuffer(
        vertexElements, 4, Ogre::BT_IMMUTABLE, &vertexData, false);

    Ogre::VertexBufferPackedVec vertexBuffers;
    vertexBuffers.push_back(vertexBuffer);

    // 1 to 1 index buffer mapping
    Ogre::uint16 indices[] = {3, 0, 1, 1, 2, 3};

    // TODO: check if this is needed (when a 1 to 1 vertex and index mapping is
    // used)
    Ogre::IndexBufferPacked* indexBuffer = vaoManager->createIndexBuffer(
        Ogre::IndexBufferPacked::IT_16BIT, 6, Ogre::BT_IMMUTABLE, &indices, false);

    Ogre::VertexArrayObject* vao = vaoManager->createVertexArrayObject(
        vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

    subMesh->mVao[Ogre::VpNormal].push_back(vao);

    // This might be needed because we use a v2 mesh
    // Use the same geometry for shadow casting.
    // Because the material disables shadows this isn't needed
    // m_impl->m_microbeBackgroundSubMesh->mVao[Ogre::VpShadow].push_back( vao
    // );

    // Set the bounds to get frustum culling and LOD to work correctly.
    // To infinite to always render
    mesh->_setBounds(Ogre::Aabb::BOX_INFINITE /*, false*/);

    return mesh;
}
