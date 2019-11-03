// ------------------------------------ //
#include "PhysicsDebugDrawer.h"

#include "Common/Types.h"

#include "bsfEngine/Debug/BsDebugDraw.h"

using namespace Leviathan;
// ------------------------------------ //
PhysicsDebugDrawer::PhysicsDebugDrawer(Window& drawtarget, GameWorld& cameraworld) :
    Target(drawtarget), World(cameraworld)
{}
// ------------------------------------ //
void PhysicsDebugDrawer::drawLine(
    const btVector3& from, const btVector3& to, const btVector3& color)
{
    bs::DebugDraw::instance().setColor(Float4(color, 1.f));
    bs::DebugDraw::instance().drawLine(Float3(from), Float3(to));
}

void PhysicsDebugDrawer::drawSphere(
    const btVector3& p, btScalar radius, const btVector3& color)
{
    bs::DebugDraw::instance().setColor(Float4(color, 1.f));
    bs::DebugDraw::instance().drawWireSphere(Float3(p), radius);
}

void PhysicsDebugDrawer::drawContactPoint(const btVector3& pointOnB,
    const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    const auto to = pointOnB + normalOnB * distance;

    drawLine(pointOnB, to, color);

    // According to one source lifeTime should be drawn as a string near pointOnB
}

void PhysicsDebugDrawer::reportErrorWarning(const char* warningString)
{
    LOG_WARNING("[PHYSICS DEBUG] " + std::string(warningString));
}
// ------------------------------------ //
DLLEXPORT void PhysicsDebugDrawer::OnBeginDraw()
{
    bs::DebugDraw::instance().clear();
}
