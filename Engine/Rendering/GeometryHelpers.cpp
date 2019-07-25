// ------------------------------------ //
#include "GeometryHelpers.h"

#include "Engine.h"
#include "Rendering/Graphics.h"

#include "bsfCore/RenderAPI/BsVertexDataDesc.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT bs::HMesh GeometryHelpers::CreateScreenSpaceQuad(
    float x, float y, float width, float height, bool autoflipUV /*= true*/)
{
    static bool flippedY = Engine::Get()->GetGraphics()->IsVerticalUVFlipped();

    bs::MESH_DESC meshDesc;
    meshDesc.numVertices = 4;
    meshDesc.numIndices = 6;
    meshDesc.indexType = bs::IT_16BIT;
    meshDesc.usage = bs::MU_STATIC;
    meshDesc.subMeshes.push_back(bs::SubMesh(0, 6, bs::DOT_TRIANGLE_LIST));

    bs::SPtr<bs::VertexDataDesc> vertexDesc = bs::VertexDataDesc::create();
    vertexDesc->addVertElem(bs::VET_FLOAT2, bs::VES_POSITION);
    vertexDesc->addVertElem(bs::VET_FLOAT2, bs::VES_TEXCOORD);
    const auto stride = 4;
    meshDesc.vertexDesc = vertexDesc;

    bs::SPtr<bs::MeshData> meshData = bs::MeshData::create(4, 6, vertexDesc, bs::IT_16BIT);

    // Generate vertex data
    float* vertices = reinterpret_cast<float*>(meshData->getStreamData(0));
    size_t index = 0;

    float uvBottom = 1.f;
    float uvTop = 0.f;

    if(flippedY && autoflipUV) {
        uvBottom = 0.f;
        uvTop = 1.f;
    }

    {
        // First vertex
        index = 0;
        vertices[index * stride + 0] = x;
        vertices[index * stride + 1] = y;
        vertices[index * stride + 2] = 0;
        vertices[index * stride + 3] = uvTop;
    }

    {
        // Second
        index = 1;
        vertices[index * stride + 0] = x + width;
        vertices[index * stride + 1] = y;
        vertices[index * stride + 2] = 1;
        vertices[index * stride + 3] = uvTop;
    }

    {
        // Third
        index = 2;
        vertices[index * stride + 0] = x + width;
        vertices[index * stride + 1] = y + height;
        vertices[index * stride + 2] = 1;
        vertices[index * stride + 3] = uvBottom;
    }

    {
        // Fourth
        index = 3;
        vertices[index * stride + 0] = x;
        vertices[index * stride + 1] = y + height;
        vertices[index * stride + 2] = 0;
        vertices[index * stride + 3] = uvBottom;
    }

    // 1 to 1 index buffer mapping
    constexpr uint16_t indicesData[] = {3, 0, 1, 1, 2, 3};

    std::memcpy(meshData->getIndices16(), indicesData, sizeof(indicesData));

    return bs::Mesh::create(meshData, meshDesc);

    // // Set the bounds to get frustum culling and LOD to work correctly.
    // // To infinite to always render
    // mesh->_setBounds(Ogre::Aabb::BOX_INFINITE /*, false*/);
}

DLLEXPORT bs::HMesh GeometryHelpers::CreateXZPlane(float width, float height)
{
    static bool flippedY = Engine::Get()->GetGraphics()->IsVerticalUVFlipped();

    const auto x = -width / 2;
    const auto z = -height / 2;

    bs::MESH_DESC meshDesc;
    meshDesc.numVertices = 4;
    meshDesc.numIndices = 6;
    meshDesc.indexType = bs::IT_16BIT;
    meshDesc.usage = bs::MU_STATIC;
    meshDesc.subMeshes.push_back(bs::SubMesh(0, 6, bs::DOT_TRIANGLE_LIST));

    bs::SPtr<bs::VertexDataDesc> vertexDesc = bs::VertexDataDesc::create();
    vertexDesc->addVertElem(bs::VET_FLOAT3, bs::VES_POSITION);
    vertexDesc->addVertElem(bs::VET_FLOAT2, bs::VES_TEXCOORD);
    const auto stride = 5;
    meshDesc.vertexDesc = vertexDesc;

    bs::SPtr<bs::MeshData> meshData = bs::MeshData::create(4, 6, vertexDesc, bs::IT_16BIT);

    // Generate vertex data
    float* vertices = reinterpret_cast<float*>(meshData->getStreamData(0));
    size_t index = 0;

    {
        // First vertex
        index = 0;
        vertices[index * stride + 0] = x;
        vertices[index * stride + 1] = 0;
        vertices[index * stride + 2] = z;
        vertices[index * stride + 3] = 0;
        vertices[index * stride + 4] = 0;
    }

    {
        // Second
        index = 1;
        vertices[index * stride + 0] = x + width;
        vertices[index * stride + 1] = 0;
        vertices[index * stride + 2] = z;
        vertices[index * stride + 3] = 1;
        vertices[index * stride + 4] = 0;
    }

    {
        // Third
        index = 2;
        vertices[index * stride + 0] = x + width;
        vertices[index * stride + 1] = 1;
        vertices[index * stride + 2] = z + height;
        vertices[index * stride + 3] = 1;
        vertices[index * stride + 4] = 1;
    }

    {
        // Fourth
        index = 3;
        vertices[index * stride + 0] = x;
        vertices[index * stride + 1] = 0;
        vertices[index * stride + 2] = z + height;
        vertices[index * stride + 3] = 0;
        vertices[index * stride + 4] = 1;
    }

    // 1 to 1 index buffer mapping
    constexpr uint16_t indicesData[] = {3, 0, 1, 1, 2, 3};

    std::memcpy(meshData->getIndices16(), indicesData, sizeof(indicesData));

    return bs::Mesh::create(meshData, meshDesc);

    // // Set the bounds to get frustum culling and LOD to work correctly.
    // // To infinite to always render
    // mesh->_setBounds(Ogre::Aabb::BOX_INFINITE /*, false*/);
}
