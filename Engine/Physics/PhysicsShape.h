// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Common/Types.h"

#include <memory>

class btCollisionShape;
class btCompoundShape;

namespace Leviathan {

//! \brief A physical shape that can be used to create PhysicsBody objects
class PhysicsShape : public ReferenceCounted {
    friend class PhysicalWorld;

protected:
    friend ReferenceCounted;
    DLLEXPORT PhysicsShape(std::unique_ptr<btCollisionShape>&& shape);
    DLLEXPORT PhysicsShape(std::unique_ptr<btCompoundShape>&& shape);

public:
    DLLEXPORT inline btCollisionShape* GetShape()
    {
        return Shape1.get();
    }

    REFERENCE_COUNTED_PTR_TYPE(PhysicsShape);

    DLLEXPORT bool AddChildShape(const PhysicsShape::pointer& child,
        const Float3& offset = Float3(0, 0, 0),
        const Float4& orientation = Float4::IdentityQuaternion());
    DLLEXPORT bool RemoveChildShape(PhysicsShape* child);

    //
    // Script wrappers
    //
    DLLEXPORT inline bool AddChildShapeWrapper(PhysicsShape* child,
        const Float3& offset = Float3(0, 0, 0),
        const Float4& orientation = Float4::IdentityQuaternion())
    {
        return AddChildShape(ReferenceCounted::WrapPtr(child), offset, orientation);
    }

private:
    std::unique_ptr<btCollisionShape> Shape1;

    //! \todo Maybe it would be cleaner to have a child class
    bool Compound = false;
    std::vector<PhysicsShape::pointer> ChildShapes;
};

} // namespace Leviathan
