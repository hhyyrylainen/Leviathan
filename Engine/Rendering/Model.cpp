// ------------------------------------ //
#include "Model.h"

#include "Graphics.h"

#include "DiligentTools/AssetLoader/interface/GLTFLoader.hpp"

using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //
DLLEXPORT Rendering::Model::Model(
    std::unique_ptr<Diligent::GLTF::Model>&& model, Graphics& owner) :
    Owner(owner),
    _Model(std::move(model))
{
    // In the future the resource creation could be deferred if wanted
    Owner.CreateModelResources(*this);
}

DLLEXPORT Rendering::Model::~Model()
{
    Owner.ReleaseModelResources(*this);
}
