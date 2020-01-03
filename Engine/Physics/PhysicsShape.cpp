// ------------------------------------ //
#include "PhysicsShape.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT PhysicsShape::PhysicsShape(std::unique_ptr<btCollisionShape>&& shape) :
    Shape1(std::move(shape))
{}

DLLEXPORT PhysicsShape::PhysicsShape(std::unique_ptr<btCompoundShape>&& shape) :
    Shape1(std::move(shape)), Compound(true)
{}
// ------------------------------------ //
DLLEXPORT bool PhysicsShape::AddChildShape(const PhysicsShape::pointer& child,
    const Float3& offset /*= Float3(0, 0, 0)*/, const Quaternion& orientation)
{
    if(!Compound || !child)
        return false;

    // Let's stop some silliness
    if(child.get() == this) {
        LOG_ERROR("PhysicsShape: AddChildShape: can't add 'this' as child");
        return false;
    }

    btTransform transform;
    transform.setIdentity();

    transform.setRotation(orientation);
    transform.setOrigin(offset);

    static_cast<btCompoundShape*>(Shape1.get())->addChildShape(transform, child->GetShape());

    ChildShapes.emplace_back(child);
    return true;
}

DLLEXPORT PhysicsShape::pointer PhysicsShape::GetChildByIndex(size_t index)
{
    if(!Compound || ChildShapes.size() <= index)
        return nullptr;
    return ChildShapes[index];
}

DLLEXPORT bool PhysicsShape::RemoveChildShape(PhysicsShape* child)
{
    if(!Compound)
        return false;

    for(auto iter = ChildShapes.begin(); iter != ChildShapes.end(); ++iter) {

        if(iter->get() == child) {

            static_cast<btCompoundShape*>(Shape1.get())->removeChildShape((*iter)->GetShape());
            ChildShapes.erase(iter);
            return true;
        }
    }

    return false;
}

DLLEXPORT int PhysicsShape::FindChildShapeIndex(PhysicsShape* child)
{
    if(!Compound || !child)
        return -1;

    const btCollisionShape* childPtr = child->GetShape();

    auto* casted = static_cast<btCompoundShape*>(Shape1.get());

    auto children = casted->getChildList();

    const auto count = casted->getNumChildShapes();

    for(int i = 0; i < count; ++i) {

        if(children[i].m_childShape == childPtr) {
            return i;
        }
    }

    return -1;
}
// ------------------------------------ //
DLLEXPORT int PhysicsShape::GetChildCustomTag(size_t index) const
{
    if(!Compound || ChildShapes.size() <= index)
        return -1;
    return ChildShapes[index]->GetCustomTag();
}
