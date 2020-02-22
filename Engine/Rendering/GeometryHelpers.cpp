// ------------------------------------ //
#include "GeometryHelpers.h"

#include "Engine.h"
#include "Rendering/Graphics.h"
#include "Rendering/Layout.h"
#include "Rendering/Mesh.h"

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

static const Diligent::LayoutElement PlaneVertexElementDefinitions[] =
    { // Attribute 0 - 3D pos
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - UV
        Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, Diligent::False}};

static_assert(sizeof(float) == 4, "float is assumed to be 32 bits");
static_assert(CHAR_BIT == 8, "byte is assumed to be 8 bits");
// ------------------------------------ //
DLLEXPORT Mesh::pointer GeometryHelpers::CreateScreenSpaceQuad(
    float x, float y, float width, float height)
{
    return CreateQuad(x, y, width, height);
}

DLLEXPORT Mesh::pointer GeometryHelpers::CreateXZPlane(float width, float height)
{
    const auto x = -width / 2;
    const auto z = -height / 2;

    // Generate vertex data
    const PlaneVertex meshData[] = {// First vertex
        {Float3(x, 0, z), Float2(0, 0)},

        // Second
        {Float3(x + width, 0, z), Float2(1, 0)},

        // Third
        {Float3(x + width, 0, z + height), Float2(1, 1)},

        // Fourth
        {Float3(x, 0, z + height), Float2(0, 1)}};

    static_assert(sizeof(meshData) == 5 * 4 * sizeof(float), "mesh data size changed");

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

    // TODO: pass Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST here
    return Mesh::MakeShared<Mesh>(
        vertexBuffer, GetLayoutForPlane(), indexBuffer, Diligent::VT_UINT16, 6);
}
// ------------------------------------ //
DLLEXPORT Mesh::pointer GeometryHelpers::CreateQuad(
    float left, float top, float width, float height)
{
    // Generate vertex data
    float uvBottom = 1.f;
    float uvTop = 0.f;

    const QuadVertex meshData[] = {// First vertex
        {Float2(left, top), Float2(0, uvTop)},

        // Second
        {Float2(left + width, top), Float2(1, uvTop)},

        // Third
        {Float2(left + width, top + height), Float2(1, uvBottom)},

        // Fourth
        {Float2(left, top + height), Float2(0, uvBottom)}};

    static_assert(sizeof(meshData) == 4 * 4 * sizeof(float), "mesh data size changed");

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

    // TODO: pass Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST here
    return Mesh::MakeShared<Mesh>(
        vertexBuffer, GetLayoutForQuad(), indexBuffer, Diligent::VT_UINT16, 6);
}
// ------------------------------------ //
DLLEXPORT Rendering::LayoutElements GeometryHelpers::GetLayoutForQuad()
{
    return Rendering::LayoutElements(
        QuadVertexElementDefinitions, std::size(QuadVertexElementDefinitions), false);
}
// ------------------------------------ //
DLLEXPORT Rendering::LayoutElements GeometryHelpers::GetLayoutForPlane()
{
    return Rendering::LayoutElements(
        PlaneVertexElementDefinitions, std::size(PlaneVertexElementDefinitions), false);
}

DLLEXPORT Rendering::LayoutElements GeometryHelpers::GetDefaultLayout()
{
    return GetLayoutForPlane();
}
// ------------------------------------ //
DLLEXPORT CountedPtr<Mesh> GeometryHelpers::CreateMesh(
    DefaultVertex* vertices, size_t vertexcount, uint16_t* indices, size_t indexcount)
{
    if(!vertices || vertexcount == 0 || !indices || indexcount == 0)
        return nullptr;

    if(vertexcount > std::numeric_limits<uint16_t>::max())
        LOG_ERROR("GeometryHelpers: CreateMesh: called with more vertices than the 16 bit "
                  "index type can handle");

    auto graphics = Engine::Get()->GetGraphics();

    const auto vertexDataInBytes = sizeof(DefaultVertex) * vertexcount;
    const auto indexDataInBytes = sizeof(uint16_t) * indexcount;

    Diligent::BufferDesc vertexBufferDesc;
    vertexBufferDesc.Name = "GHMesh vertex buffer";
    vertexBufferDesc.Usage = Diligent::USAGE_STATIC;
    vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    vertexBufferDesc.uiSizeInBytes = vertexDataInBytes;
    Diligent::BufferData vertexBufferData;
    vertexBufferData.pData = vertices;
    vertexBufferData.DataSize = vertexDataInBytes;

    auto vertexBuffer = graphics->CreateBuffer(vertexBufferDesc, &vertexBufferData);

    Diligent::BufferDesc indexBufferDesc;
    indexBufferDesc.Name = "GHMesh index buffer";
    indexBufferDesc.Usage = Diligent::USAGE_STATIC;
    indexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    indexBufferDesc.uiSizeInBytes = indexDataInBytes;
    Diligent::BufferData indexBufferData;
    indexBufferData.pData = indices;
    indexBufferData.DataSize = indexDataInBytes;

    auto indexBuffer = graphics->CreateBuffer(indexBufferDesc, &indexBufferData);

    // TODO: pass Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST here
    return Mesh::MakeShared<Mesh>(vertexBuffer, GetDefaultLayout(), indexBuffer,
        Diligent::VT_UINT16, static_cast<int>(indexcount));
}
