// ------------------------------------ //
#include "GeometryHelpers.h"

#include "Engine.h"
#include "Rendering/Graphics.h"
#include "Rendering/Layout.h"
#include "Rendering/Mesh.h"

#include "bsfCore/Mesh/BsMesh.h"
#include "bsfCore/RenderAPI/BsVertexDataDesc.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h"

#include <climits>
using namespace Leviathan;
// ------------------------------------ //
//! Common static definition for the element layout for the quads
static const Diligent::LayoutElement QuadVertexElementDefinitions[] = { // Attribute 0 - 2D pos
    Diligent::LayoutElement{0, 0, 2, Diligent::VT_FLOAT32, Diligent::False},
    // Attribute 1 - UV
    Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, Diligent::False}};

static_assert(sizeof(float) == 4, "float is assumed to be 32 bits");
static_assert(CHAR_BIT == 8, "byte is assumed to be 8 bits");
// ------------------------------------ //
DLLEXPORT Mesh::pointer GeometryHelpers::CreateScreenSpaceQuad(
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

    DEBUG_BREAK;
    // return Mesh::MakeShared<Mesh>(bs::Mesh::create(meshData, meshDesc));

    // // Set the bounds to get frustum culling and LOD to work correctly.
    // // To infinite to always render
    // mesh->_setBounds(Ogre::Aabb::BOX_INFINITE /*, false*/);
}

DLLEXPORT Mesh::pointer GeometryHelpers::CreateXZPlane(float width, float height)
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

    constexpr uint16_t indicesData[] = {3, 0, 1, 1, 2, 3};

    std::memcpy(meshData->getIndices16(), indicesData, sizeof(indicesData));

    DEBUG_BREAK;
    // return Mesh::MakeShared<Mesh>(bs::Mesh::create(meshData, meshDesc));

    // // Set the bounds to get frustum culling and LOD to work correctly.
    // // To infinite to always render
    // mesh->_setBounds(Ogre::Aabb::BOX_INFINITE /*, false*/);
}
// ------------------------------------ //
DLLEXPORT Mesh::pointer GeometryHelpers::CreateQuad(
    float left, float top, float width, float height, bool autoflipUV /*= true*/)
{
    static bool flippedY = Engine::Get()->GetGraphics()->IsVerticalUVFlipped();

    // Generate vertex data
    float uvBottom = 1.f;
    float uvTop = 0.f;

    if(flippedY && autoflipUV) {
        uvBottom = 0.f;
        uvTop = 1.f;
    }

    const QuadVertex meshData[] = {// First vertex
        {Float2(left, top), Float2(0, uvTop)},

        // Second
        {Float2(left + width, top), Float2(1, uvTop)},

        // Third
        {Float2(left + width, top + height), Float2(1, uvBottom)},

        // Fourth
        {Float2(left, top + height), Float2(0, uvBottom)}};

    static_assert(sizeof(meshData) == 4 * 4 * sizeof(float), "mesh data size changed");

    // bs::MESH_DESC meshDesc;
    // meshDesc.numVertices = 4;
    // meshDesc.numIndices = 6;
    // meshDesc.indexType = bs::IT_16BIT;
    // meshDesc.usage = bs::MU_STATIC;
    // meshDesc.subMeshes.push_back(bs::SubMesh(0, 6, bs::DOT_TRIANGLE_LIST));

    // bs::SPtr<bs::VertexDataDesc> vertexDesc = bs::VertexDataDesc::create();
    // vertexDesc->addVertElem(bs::VET_FLOAT2, bs::VES_POSITION);
    // vertexDesc->addVertElem(bs::VET_FLOAT2, bs::VES_TEXCOORD);
    // const auto stride = 4;
    // meshDesc.vertexDesc = vertexDesc;

    // bs::SPtr<bs::MeshData> meshData = bs::MeshData::create(4, 6, vertexDesc, bs::IT_16BIT);

    constexpr uint16_t indicesData[] = {3, 0, 1, 1, 2, 3};

    auto graphics = Engine::Get()->GetGraphics();

    Diligent::BufferDesc vertexBufferDesc;
    vertexBufferDesc.Name = "quad vertex buffer";
    vertexBufferDesc.Usage = Diligent::USAGE_STATIC;
    vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    vertexBufferDesc.uiSizeInBytes = sizeof(meshData);
    Diligent::BufferData vertexBufferData;
    vertexBufferData.pData = meshData;
    vertexBufferData.DataSize = sizeof(meshData);

    auto vertexBuffer = graphics->CreateBuffer(vertexBufferDesc, &vertexBufferData);

    Diligent::BufferDesc indexBufferDesc;
    indexBufferDesc.Name = "quad index buffer";
    indexBufferDesc.Usage = Diligent::USAGE_STATIC;
    indexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    indexBufferDesc.uiSizeInBytes = sizeof(indicesData);
    Diligent::BufferData indexBufferData;
    indexBufferData.pData = indicesData;
    indexBufferData.DataSize = sizeof(indicesData);
    auto indexBuffer = graphics->CreateBuffer(indexBufferDesc, &indexBufferData);

    // QuadVertexElementDefinitions
    return Mesh::MakeShared<Mesh>(vertexBuffer,
        Rendering::LayoutElements(
            QuadVertexElementDefinitions, std::size(QuadVertexElementDefinitions), false),
        indexBuffer, Diligent::VT_UINT16, 6);
}
