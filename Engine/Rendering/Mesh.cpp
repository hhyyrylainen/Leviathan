// ------------------------------------ //
#include "Mesh.h"


using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Mesh::Mesh(const std::shared_ptr<Rendering::Buffer>& vertexbuffer,
    Rendering::LayoutElements vertexlayout,
    const std::shared_ptr<Rendering::Buffer>& indexbuffer, Diligent::VALUE_TYPE indextype,
    int indexcount) :
    VertexBuffer(vertexbuffer),
    VertexLayout(vertexlayout), IndexBuffer(indexbuffer), IndexType(indextype),
    IndexCount(indexcount)
{}
