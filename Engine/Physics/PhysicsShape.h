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

    //! \note This assumes that AddChildShape and RemoveChildShape properly update ChildShapes
    //! indexes to match what bullet stores internally
    DLLEXPORT PhysicsShape::pointer GetChildByIndex(size_t index);

    //! \brief Calls GetCustomTag on the child at index. If invalid index returns -1
    //! \see GetChildByIndex
    DLLEXPORT int GetChildCustomTag(size_t index) const;

    //! \brief Finds the index that the child shape occupies
    //!
    //! This is useful for compound shape contact callbacks to determine which child shape
    //! collided
    //! \returns -1 if not found
    DLLEXPORT int FindChildShapeIndex(PhysicsShape* child);


    //! \brief Custom tags allow storing one int of application specific info in this class
    DLLEXPORT inline int GetCustomTag() const
    {
        return CustomData;
    }

    DLLEXPORT inline void SetCustomTag(int value)
    {
        CustomData = value;
    }

    //
    // Script wrappers
    //
    DLLEXPORT inline bool AddChildShapeWrapper(PhysicsShape* child,
        const Float3& offset = Float3(0, 0, 0),
        const Float4& orientation = Float4::IdentityQuaternion())
    {
        return AddChildShape(ReferenceCounted::WrapPtr(child), offset, orientation);
    }

    DLLEXPORT inline int FindChildShapeIndexWrapper(PhysicsShape* child)
    {
        const auto result = FindChildShapeIndex(child);

        if(child)
            child->Release();

        return result;
    }

private:
    std::unique_ptr<btCollisionShape> Shape1;

    //! \todo Maybe it would be cleaner to have a child class
    bool Compound = false;
    std::vector<PhysicsShape::pointer> ChildShapes;

    //! Data for application to use. The engine doesn't use this
    int CustomData = -1;
};

} // namespace Leviathan
