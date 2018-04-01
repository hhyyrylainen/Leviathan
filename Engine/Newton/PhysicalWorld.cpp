// ------------------------------------ //
#include "PhysicalWorld.h"

#include "../TimeIncludes.h"
#include "Engine.h"
#include "Events/EventHandler.h"
#include "NewtonConversions.h"
#include "PhysicsMaterialManager.h"
#include <Newton.h>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::PhysicalWorld::PhysicalWorld(GameWorld* owner) : OwningWorld(owner)
{
    // create newton world //
    World = NewtonCreate();

    // set physics accuracy //
    // most accurate mode //

    // \todo figure out how to use this exact mode //
    // NewtonSetSolverModel(World, 0);

    // Accurate enough mode //
    NewtonSetSolverModel(World, 1);

    // Set us as the user data //
    NewtonWorldSetUserData(World, this);

    // Create materials for this world //
    PhysicsMaterialManager::Get()->CreateActualMaterialsForWorld(World);
}

DLLEXPORT Leviathan::PhysicalWorld::~PhysicalWorld()
{
    //  Destroy the newton world
    NewtonDestroy(World);

    // NewtonWorldSetUserData(World, NULL);

    auto physmanager = PhysicsMaterialManager::Get();

    if(physmanager)
        physmanager->DestroyActualMaterialsForWorld(World);

    World = NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::SimulateWorld(int maxruns /*= -1*/)
{
    if(maxruns <= 0) {

        // TODO: report error?
        maxruns = 1;
    }

    int runs = 0;

    auto curtime = Time::GetTimeMicro64();

    // Calculate passed time and reset //
    PassedTimeTotal += curtime - LastSimulatedTime;
    LastSimulatedTime = curtime;

    // Cap passed time, if over one second //
    if(PassedTimeTotal > MICROSECONDS_IN_SECOND)
        PassedTimeTotal = MICROSECONDS_IN_SECOND;

    Lock lock(WorldUpdateLock);

    while(PassedTimeTotal >= NEWTON_FPS_IN_MICROSECONDS) {

        // Call event //
        Engine::Get()->GetEventHandler()->CallEvent(new Event(EVENT_TYPE_PHYSICS_BEGIN,
            new PhysicsStartEventData(NEWTON_TIMESTEP, OwningWorld)));

        NewtonUpdate(World, NEWTON_TIMESTEP);
        PassedTimeTotal -= static_cast<int64_t>(NEWTON_FPS_IN_MICROSECONDS);
        runs++;

        if(runs == maxruns) {

            Logger::Get()->Warning("PhysicalWorld: bailing from update after " +
                                   Convert::ToString(runs) +
                                   " with time left: " + Convert::ToString(PassedTimeTotal));
            break;
        }
    }
}

DLLEXPORT void Leviathan::PhysicalWorld::SimulateWorldFixed(
    uint32_t mspassed, uint32_t stepcount /*= 1*/)
{
    float timestep = (mspassed / 1000.f) / stepcount;

    for(uint32_t i = 0; i < stepcount; ++i) {

        Engine::Get()->GetEventHandler()->CallEvent(new Event(
            EVENT_TYPE_PHYSICS_BEGIN, new PhysicsStartEventData(timestep, OwningWorld)));

        NewtonUpdate(World, timestep);
    }
}
// ------------------------------------ //
int Leviathan::SingleBodyUpdate(
    const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount)
{
    PhysicalWorld* pworld =
        reinterpret_cast<PhysicalWorld*>(NewtonWorldGetUserData(newtonWorld));

    for(int i = 0; i < bodyCount; i++) {

        if(NewtonIslandGetBody(islandHandle, i) == pworld->ResimulatedBody) {

            // Target body is part of this collision, simulate it //
            return 1;
        }
    }

    // Wasn't the target body, ignore //
    return 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::ClearTimers()
{
    LastSimulatedTime = Time::GetTimeMicro64();
    PassedTimeTotal = 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::PhysicalWorld::AdjustClock(int milliseconds)
{
    // Convert from milliseconds (10^-3) to micro seconds (10^-6) //
    LastSimulatedTime -= 1000 * milliseconds;
}
// ------------------------------------ //
DLLEXPORT void PhysicalWorld::DestroyCollision(NewtonCollision* collision)
{
    NewtonDestroyCollision(collision);
}

DLLEXPORT NewtonCollision* PhysicalWorld::CreateCompoundCollision()
{
    // 0 is shapeID
    return NewtonCreateCompoundCollision(World, UNUSED_SHAPE_ID);
}

DLLEXPORT NewtonCollision* PhysicalWorld::CreateSphere(
    float radius, const Ogre::Matrix4& offset /*= Ogre::Matrix4::IDENTITY*/)
{
#ifdef CHECK_FOR_NANS
    bool matrixNans = false;
    for(size_t i = 0; i < 16; ++i) {
        if(std::isnan(*(offset[0] + i))) {
            matrixNans = true;
            break;
        }
    }

    if(std::isnan(radius) || matrixNans) {

        DEBUG_BREAK;
        throw std::runtime_error("CreateSphere has NaNs in it!");
    }
#endif // CHECK_FOR_NANS
    const auto& prep = PrepareOgreMatrixForNewton(offset);
    return NewtonCreateSphere(World, radius, UNUSED_SHAPE_ID, prep[0]);
}

DLLEXPORT NewtonCollision* PhysicalWorld::CreateBox(float xdimension, float ydimension,
    float zdimension, const Ogre::Matrix4& offset /*= Ogre::Matrix4::IDENTITY*/)
{
    const auto& prep = PrepareOgreMatrixForNewton(offset);

    return NewtonCreateBox(
        World, xdimension, ydimension, zdimension, UNUSED_SHAPE_ID, prep[0]);
}
// ------------------------------------ //
DLLEXPORT NewtonBody* PhysicalWorld::CreateBodyFromCollision(NewtonCollision* collision)
{
    if(!collision)
        return nullptr;

    const auto& prep = PrepareOgreMatrixForNewton(Ogre::Matrix4::IDENTITY);
    return NewtonCreateDynamicBody(World, collision, prep[0]);
}

DLLEXPORT void PhysicalWorld::DestroyBody(NewtonBody* body)
{
    NewtonDestroyBody(body);
}

// ------------------------------------ //
DLLEXPORT void BaseCustomJoint::JointDestructorCallback(const NewtonJoint* joint)
{
    void* user = NewtonJointGetUserData(joint);

    if(user)
        delete static_cast<BaseCustomJoint*>(user);
}

class Joint2D : public BaseCustomJoint {
public:
    Joint2D(
        const Ogre::Vector3& planenormal, const Ogre::Vector3& planeorigin, NewtonBody* body) :
        Normal(planenormal),
        Origin(planeorigin), Body(body)
    {
    }

    //! \note This is continually called by newton to get what the object needs to do
    static void SubmitConstraints(
        const NewtonJoint* const joint, dFloat timestep, int threadIndex)
    {
        void* rawData = NewtonJointGetUserData(joint);
        Joint2D* self = static_cast<Joint2D*>(static_cast<BaseCustomJoint*>(rawData));

        dFloat matrix[16];
        NewtonBodyGetMatrix(self->Body, &matrix[0]);
        auto matrixPos = ExtractNewtonMatrixTranslation(matrix);

        // this line clamps the origin to the plane
        NewtonUserJointAddLinearRow(joint, &matrixPos.X, &self->Origin.x, &self->Normal.x);

        // we can prevent rotation that takes any points out of the plane by clamping a point
        // on the object that is on the line through the origin of the object with the same
        // vector as the plane normal.  The clamp is to another plane that is parallel to the
        // first, offset by the plane normal vector.  Rotations around either of the axes
        // orthogonal to the plane normal will be prevented because they take the object point
        // off that parallel plane.

        // TODO: find a faster way than transposing here
        auto bodyMatrix = NewtonMatrixToOgre(matrix);

        Ogre::Vector3 object_point = bodyMatrix * self->Normal;
        auto world_point = self->Origin + self->Normal;
        NewtonUserJointAddLinearRow(joint, &object_point.x, &world_point.x, &self->Normal.x);
    }

    const Ogre::Vector3 Normal;
    const Ogre::Vector3 Origin;
    const NewtonBody* Body;
};

// ------------------------------------ //
DLLEXPORT NewtonJoint* PhysicalWorld::Create2DJoint(
    NewtonBody* body, const Float3& planenormal)
{
    // Method from http://newtondynamics.com/wiki/index.php5?title=CJ_2D_Joint_planar_rotation
    dFloat matrix[16];
    NewtonBodyGetMatrix(body, &matrix[0]);
    const auto planeOrigin = ExtractNewtonMatrixTranslation(matrix);

    NewtonJoint* joint =
        NewtonConstraintCreateUserJoint(World, 6, Joint2D::SubmitConstraints, body, nullptr);

    Joint2D* jointData = new Joint2D(planenormal, planeOrigin, body);

    NewtonJointSetUserData(joint, static_cast<BaseCustomJoint*>(jointData));

    NewtonJointSetDestructor(joint, BaseCustomJoint::JointDestructorCallback);
    return joint;
}

// ------------------------------------ //
