// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h"

#include <memory>

namespace Diligent { namespace GLTF {
struct Model;
}} // namespace Diligent::GLTF

namespace Leviathan {
class Graphics;
namespace Rendering {

//! \brief A higher abstraction level from Mesh, encapsulating a gltf mesh with a bunch of
//! properties
class Model : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Model(std::unique_ptr<Diligent::GLTF::Model>&& model, Graphics& owner);

public:
    DLLEXPORT ~Model();

    DLLEXPORT Diligent::GLTF::Model& GetInternal()
    {
        return *_Model;
    }

    REFERENCE_COUNTED_PTR_TYPE(Model);

private:
    //! Used to release the GLTF resources when destroyed
    Graphics& Owner;

    std::unique_ptr<Diligent::GLTF::Model> _Model;
};

} // namespace Rendering
} // namespace Leviathan
