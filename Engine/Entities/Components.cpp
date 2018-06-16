// ------------------------------------ //
#include "Components.h"

#include "CommonStateObjects.h"
#include "GameWorld.h"
#include "Handlers/IDFactory.h"
#include "Networking/Connection.h"
#include "Networking/SentNetworkThing.h"
#include "Newton/NewtonConversions.h"
#include "Utility/Convert.h"

#include "OgreBillboardChain.h"
#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreRibbonTrail.h"
#include "OgreSceneManager.h"

#include <limits>
using namespace Leviathan;
// ------------------------------------ //

// ------------------ RenderNode ------------------ //
DLLEXPORT RenderNode::RenderNode(Ogre::SceneManager* scene) : Component(TYPE)
{
    Marked = false;

    // TODO: allow for static render nodes
    Node = scene->getRootSceneNode(Ogre::SCENE_DYNAMIC)
               ->createChildSceneNode(Ogre::SCENE_DYNAMIC);

    // Node = scene->createSceneNode();
}

DLLEXPORT void RenderNode::Release(Ogre::SceneManager* worldsscene)
{

    worldsscene->destroySceneNode(Node);
    Node = nullptr;
}
// ------------------------------------ //
DLLEXPORT RenderNode::RenderNode(const Test::TestComponentCreation& test) : Component(TYPE)
{
    Marked = false;
    Node = nullptr;
}

// ------------------------------------ //
// Plane
DLLEXPORT Plane::Plane(Ogre::SceneManager* scene, Ogre::SceneNode* parent,
    const std::string& material, const Ogre::Plane& plane, const Float2& size,
    const Ogre::Vector3& uvupvector /*= Ogre::Vector3::UNIT_Y*/) :
    Component(TYPE),
    GeneratedMeshName("Plane_Component_Mesh_" + std::to_string(IDFactory::GetID()))
{
    const auto mesh = Ogre::v1::MeshManager::getSingleton().createPlane(
        GeneratedMeshName + "_v1", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, size.X, size.Y, 1, 1,
        // Normals
        true, 1, 1.0f, 1.0f, uvupvector, Ogre::v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
        Ogre::v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false);

    const auto mesh2 = Ogre::MeshManager::getSingleton().createManual(
        GeneratedMeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    // Fourth true is qtangent encoding which is not needed if we don't do normal mapping
    mesh2->importV1(mesh.get(), true, true, true);

    Ogre::v1::MeshManager::getSingleton().remove(mesh);

    GraphicalObject = scene->createItem(mesh2);
    parent->attachObject(GraphicalObject);

    // This DOESN'T seem to throw if material is invalid
    GraphicalObject->setMaterialName(material);
}

DLLEXPORT void Plane::Release(Ogre::SceneManager* scene)
{

    scene->destroyItem(GraphicalObject);

    Ogre::MeshManager::getSingleton().remove(GeneratedMeshName);
}
// ------------------ Physics ------------------ //
DLLEXPORT Physics::~Physics()
{
    if(Body) {
        LOG_ERROR("Physics: Release not called before destructor!");
        Release();
    }
}

DLLEXPORT void Physics::Release()
{
    if(Body)
        NewtonDestroyBody(Body);
    if(Collision)
        NewtonDestroyCollision(Collision);

    Body = NULL;
    Collision = NULL;
}
// ------------------------------------ //
DLLEXPORT void Physics::JumpTo(Position& target)
{
    SetPosition(target.Members._Position, target.Members._Orientation);
}

DLLEXPORT bool Physics::SetPosition(const Float3& pos, const Float4& orientation)
{
    if(!Body)
        return false;

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(pos.HasInvalidValues() || orientation.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetPosition call had at least one value with non-finite value, pos: "
            << Convert::ToString(pos) << " orientation: " << Convert::ToString(orientation);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Ogre::Matrix4 matrix;

    Ogre::Vector3 ogrepos = pos;
    Ogre::Quaternion ogrerot = orientation;
    matrix.makeTransform(ogrepos, Float3(1, 1, 1), ogrerot);

    Ogre::Matrix4 tmatrix = PrepareOgreMatrixForNewton(matrix);

    // Update body //
    NewtonBodySetMatrix(Body, &tmatrix[0][0]);

    return true;
}

DLLEXPORT bool Physics::SetOnlyOrientation(const Float4& orientation)
{
    if(!Body)
        return false;

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(orientation.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetOnlyOrientation call had at least one value with non-finite value,  "
               "orientation: "
            << Convert::ToString(orientation);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    float newtonMatrix[16];
    NewtonBodyGetMatrix(Body, &newtonMatrix[0]);

    const auto pos = ExtractNewtonMatrixTranslation(newtonMatrix);

    Ogre::Matrix4 matrix;

    Ogre::Vector3 ogrepos = pos;
    Ogre::Quaternion ogrerot = orientation;
    matrix.makeTransform(ogrepos, Float3(1, 1, 1), ogrerot);

    Ogre::Matrix4 tmatrix = PrepareOgreMatrixForNewton(matrix);

    // ThrowIfMatrixIsNotOrthogonal(tmatrix);

    // Update body //
    NewtonBodySetMatrix(Body, &tmatrix[0][0]);

    return true;
}

DLLEXPORT Ogre::Matrix4 Physics::GetFullMatrix() const
{
    if(!Body)
        return Ogre::Matrix4::IDENTITY;

    float matrix[16];
    NewtonBodyGetMatrix(Body, &matrix[0]);

    return NewtonMatrixToOgre(matrix);
}
// ------------------------------------ //
void Physics::PhysicsMovedEvent(
    const NewtonBody* const body, const dFloat* const matrix, int threadIndex)
{
    // first create Ogre 4x4 matrix from the matrix //
    Ogre::Matrix4 mat = NewtonMatrixToOgre(matrix);

    Physics* tmp = static_cast<Physics*>(NewtonBodyGetUserData(body));

    const auto pos = mat.getTrans();

    // #ifdef CHECK_FOR_NANS
    if(!std::isfinite(pos.x)) {

        LOG_ERROR("Physics::PhysicsMovedEvent: physics body has NaN translation");
        // Would really like to find a fix instead of this hack
        if(std::isnan(tmp->_Position.Members._Position.X)) {
            LOG_ERROR("Even Position.X is NaN");
            tmp->SetPosition(Float3(0, 0, 0), Ogre::Quaternion::IDENTITY);
        } else {
            // Doesn't seem to work
            // tmp->JumpTo(tmp->_Position);
            // This doesn't work either
            tmp->SetPosition(Float3(0, 0, 0), Ogre::Quaternion::IDENTITY);
        }

        return;
    }
    // #endif // CHECK_FOR_NANS

    tmp->_Position.Members._Position = pos;
    tmp->_Position.Members._Orientation = mat.extractQuaternion();
    tmp->_Position.Marked = true;

    if(tmp->UpdateSendable) {

        tmp->UpdateSendable->Marked = true;
    }

    tmp->Marked = true;
}

void Physics::ApplyForceAndTorqueEvent(
    const NewtonBody* const body, dFloat timestep, int threadIndex)
{
    // Get object from body //
    Physics* tmp = static_cast<Physics*>(NewtonBodyGetUserData(body));

    // Check if physics can't apply //
    // Newton won't call this if the mass is 0

    Float3 Torque = tmp->SumQueuedTorque;

    // Get properties from newton //
    float mass;
    float Ixx;
    float Iyy;
    float Izz;

    NewtonBodyGetMass(body, &mass, &Ixx, &Iyy, &Izz);

    Float3 Force = tmp->SumQueuedForce;

    // get gravity force and apply mass to it //
    if(tmp->ApplyGravity)
        Force += tmp->World->GetGravityAtPosition(tmp->_Position.Members._Position) * mass;

    // add other forces //
    if(!tmp->ApplyForceList.empty()) {

        Force += tmp->_GatherApplyForces(mass);
    }

#ifdef CHECK_FOR_NANS
    if(Force.HasInvalidValues()) {

        LOG_ERROR("Physics::ApplyForceAndTorqueEvent: Force was calculated to have a "
                  "non-finite value in it. Skipping applying");

        return;
    }
#endif // CHECK_FOR_NANS

    NewtonBodyAddForce(body, &Force.X);

    tmp->SumQueuedForce = Float3(0, 0, 0);

    // Torque applying //
    Float3 torque;

    if(tmp->TorqueOverride) {

        torque = tmp->SumQueuedTorque;
        tmp->TorqueOverride = false;
        tmp->SumQueuedTorque = Float3(0, 0, 0);

    } else {

        torque = Torque;
    }


#ifdef CHECK_FOR_NANS
    if(Force.HasInvalidValues()) {

        LOG_ERROR("Physics::ApplyForceAndTorqueEvent: Torque was calculated to have a "
                  "non-finite value in it. Skipping applying");

        return;
    }
#endif // CHECK_FOR_NANS

    NewtonBodyAddTorque(body, &torque.X);
}

void Physics::DestroyBodyCallback(const NewtonBody* body)
{
    // This shouldn't be required as the newton world won't be cleared while running
    // Physics* tmp = reinterpret_cast<Physics*>(NewtonBodyGetUserData(body));
    // LOG_INFO("Destroy body callback");

    // GUARD_LOCK_OTHER(tmp);

    // tmp->Body = nullptr;
    // tmp->Collision = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Physics::GiveImpulse(const Float3& deltaspeed, const Float3& point
    /*= Float3(0)*/)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(deltaspeed.HasInvalidValues() || point.HasInvalidValues()) {

        std::stringstream msg;
        msg << "GiveImpulse call had at least one value with non-finite value, deltaspeed: "
            << Convert::ToString(deltaspeed) << " point: " << Convert::ToString(point);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    // No clue what the delta step should be
    NewtonBodyAddImpulse(Body, &deltaspeed.X, &point.X, 0.1f);
}

DLLEXPORT void Physics::AddForce(const Float3& force)
{
    // Safety check. Can be disabled in release builds if this is a performance issue
    if(force.HasInvalidValues()) {

        std::stringstream msg;
        msg << "AddForce call had at least one value with non-finite value, force: "
            << Convert::ToString(force);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    SumQueuedForce += force;
}

DLLEXPORT void Physics::SetVelocity(const Float3& velocities)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(velocities.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetVelocity call had at least one value with non-finite value, velocities: "
            << Convert::ToString(velocities);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    NewtonBodySetVelocity(Body, &velocities.X);
}

DLLEXPORT void Physics::ClearVelocity()
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    Float3 zeroVector(0, 0, 0);
    NewtonBodySetVelocity(Body, &zeroVector.X);
    // Forces are recalculated each physics update so we can't set them here
    SetTorque(Float3(0, 0, 0));
}

DLLEXPORT Float3 Physics::GetVelocity() const
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    Float3 vel(0);
    NewtonBodyGetVelocity(Body, &vel.X);
    return vel;
}
// ------------------------------------ //
DLLEXPORT Float3 Physics::GetOmega() const
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    Float3 vel(0);
    NewtonBodyGetOmega(Body, &vel.X);
    return vel;
}

DLLEXPORT void Physics::SetOmega(const Float3& velocities)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(velocities.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetOmega call had at least one value with non-finite value, velocities: "
            << Convert::ToString(velocities);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    NewtonBodySetOmega(Body, &velocities.X);
}

DLLEXPORT void Physics::AddOmega(const Float3& velocities)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(velocities.HasInvalidValues()) {

        std::stringstream msg;
        msg << "AddOmega call had at least one value with non-finite value, velocities: "
            << Convert::ToString(velocities);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Float3 temp;
    NewtonBodyGetOmega(Body, &temp.X);
    temp += velocities;
    NewtonBodySetOmega(Body, &temp.X);
}
// ------------------------------------ //
DLLEXPORT Float3 Physics::GetTorque() const
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    Float3 torq(0);
    NewtonBodyGetTorque(Body, &torq.X);
    return torq;
}

DLLEXPORT void Physics::AddTorque(const Float3& torque)
{
    // Safety check. Can be disabled in release builds if this is a performance issue
    if(torque.HasInvalidValues()) {

        std::stringstream msg;
        msg << "AddTorque call had at least one value with non-finite value, torque: "
            << Convert::ToString(torque);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    if(TorqueOverride) {

        SumQueuedTorque = Float3(0, 0, 0);
        TorqueOverride = false;
    }

    SumQueuedTorque += torque;
}

DLLEXPORT void Physics::SetTorque(const Float3& torque)
{
    // Safety check. Can be disabled in release builds if this is a performance issue
    if(torque.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetTorque call had at least one value with non-finite value, torque: "
            << Convert::ToString(torque);
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    SumQueuedTorque = torque;
    TorqueOverride = true;
}

DLLEXPORT void Physics::SetLinearDamping(float factor /*= 0.1f*/)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    NewtonBodySetLinearDamping(Body, factor);
}

DLLEXPORT void Physics::SetAngularDamping(const Float3& factor /*= Float3(0.1f)*/)
{
    if(!Body)
        throw InvalidState("Physics object doesn't have a body");

    NewtonBodySetAngularDamping(Body, &factor.X);
}
// ------------------------------------ //
Float3 Physics::_GatherApplyForces(const float& mass)
{
    // Return if just an empty list //
    if(ApplyForceList.empty())
        return Float3(0);

    Float3 total(0);

    for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter) {
        // Add to total, and multiply by mass if wanted //
        const Float3 force = (*iter)->Callback((*iter).get(), *this);

        total += (*iter)->MultiplyByMass ? force * mass : force;

        // We might assert/crash here if the force was removed //
    }

    return total;
}
// ------------------------------------ //
DLLEXPORT void Physics::ApplyForce(ApplyForceInfo* pointertohandle)
{
    // Overwrite old if found //
    for(auto iter = ApplyForceList.begin(); iter != ApplyForceList.end(); ++iter) {

        // Check do the names match //
        if((bool)(*iter)->OptionalName == (bool)pointertohandle->OptionalName) {

            if(!pointertohandle->OptionalName ||
                *pointertohandle->OptionalName == *(*iter)->OptionalName) {
                // it's the default, overwrite //
                **iter = *pointertohandle;
                SAFE_DELETE(pointertohandle);
                return;
            }
        }
    }
    // got here, so add a new one //
    ApplyForceList.push_back(std::shared_ptr<ApplyForceInfo>(pointertohandle));
}

DLLEXPORT bool Physics::RemoveApplyForce(const std::string& name)
{
    // Search for a matching name //
    auto end = ApplyForceList.end();
    for(auto iter = ApplyForceList.begin(); iter != end; ++iter) {

        // Check do the names match //
        if((!((*iter)->OptionalName) && name.size() == 0) ||
            ((*iter)->OptionalName && *(*iter)->OptionalName == name)) {

            ApplyForceList.erase(iter);
            return true;
        }
    }

    return false;
}
// ------------------------------------ //
DLLEXPORT void Physics::SetPhysicalMaterialID(int ID)
{
    if(!Body) {
        throw InvalidState("Calling set material ID without having physical Body");
    }

    NewtonBodySetMaterialGroupID(Body, ID);
}
// ------------------------------------ //
DLLEXPORT NewtonBody* Physics::CreatePhysicsBody(
    PhysicalWorld* world, int physicsmaterialid /*= -1*/)
{
    if(!world || !Collision)
        return nullptr;

    // Destroy old if there is one //
    if(Body)
        NewtonDestroyBody(Body);

    Body = world->CreateBodyFromCollision(Collision);

    if(!Body)
        return nullptr;

    // Add this as user data //
    NewtonBodySetUserData(Body, this);

    // Callbacks //
    NewtonBodySetTransformCallback(Body, Physics::PhysicsMovedEvent);

    NewtonBodySetForceAndTorqueCallback(Body, Physics::ApplyForceAndTorqueEvent);

    NewtonBodySetDestructorCallback(Body, Physics::DestroyBodyCallback);

    // And material //
    if(physicsmaterialid != -1)
        NewtonBodySetMaterialGroupID(Body, physicsmaterialid);

    return Body;
}

DLLEXPORT void Physics::SetMass(float mass)
{
    if(!Body)
        return;

    // Safety check. Shouldn't be disabled
    if(!std::isfinite(mass)) {

        std::stringstream msg;
        msg << "SetMass mass is invalid value: " << Convert::ToString(mass)
            << " (use 0 for immovable)";
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Mass = mass;

    // First calculate inertia and center of mass points //
    Float3 inertia;
    Float3 centerofmass;

    // TODO: cache this per Collision object
    NewtonConvexCollisionCalculateInertialMatrix(Collision, &inertia.X, &centerofmass.X);

    // Apply mass to inertia
    inertia *= Mass;

#ifdef CHECK_FOR_NANS

    // Doesn't work. Apparently there is a chance to screw up the collision object permanently
    // if(std::isnan(inertia.X) || std::isnan(inertia.Y) || std::isnan(inertia.Z) ||
    //     std::isnan(centerofmass.X) || std::isnan(centerofmass.Y) ||
    //     std::isnan(centerofmass.Z)) {

    //     LOG_ERROR("Physics: SetMass: failed to calculate collision inertial matrix, "
    //               "trying again");
    //     SetMass(mass);
    //     return;
    // }

    inertia.CheckForNans();
    centerofmass.CheckForNans();

#endif // CHECK_FOR_NANS

    NewtonBodySetMassMatrix(Body, Mass, inertia.X, inertia.Y, inertia.Z);
    NewtonBodySetCentreOfMass(Body, &centerofmass.X);
}

DLLEXPORT bool Physics::SetCollision(NewtonCollision* collision)
{
    if(Body)
        return false;

    if(Collision)
        Release();

    Collision = collision;

    return true;
}
// ------------------------------------ //
DLLEXPORT bool Physics::CreatePlaneConstraint(
    PhysicalWorld* world, const Float3& planenormal /*= Float3(0, 1, 0)*/)
{
    if(!Body || !world)
        return false;

    world->Create2DJoint(Body, planenormal);
    return true;
}

// ------------------ Received ------------------ //
DLLEXPORT void Received::GetServerSentStates(
    StoredState const** first, StoredState const** second, int tick, float& progress) const
{
    // Used to find the first tick before or on tick //
    int firstinpast = std::numeric_limits<int>::max();
    int secondfound = 0;

    for(auto& obj : ClientStateBuffer) {

        if(tick - obj.Tick < firstinpast && tick - obj.Tick >= 0) {

            // This is (potentially) the first state //
            firstinpast = tick - obj.Tick;

            *first = &obj;
        }

        // For this to be found the client should be around 50-100 milliseconds in the past
        if(obj.Tick > tick && (secondfound == 0 || obj.Tick - tick < secondfound)) {

            // The second state //
            *second = &obj;

            secondfound = obj.Tick - tick;
            continue;
        }
    }

    if(firstinpast == std::numeric_limits<int>::max() || secondfound == 0) {

        throw InvalidState("No stored server states around tick");
    }

    // If the range is not 1, meaning firstinpast != 0 || secondfound > 1 we need to adjust
    // progress
    int range = firstinpast + secondfound;

    if(range == 1)
        return;

    progress = ((tick + progress) - (*first)->Tick) / range;
}
// // ------------------ Trail ------------------ //
// DLLEXPORT bool Trail::SetTrailProperties(const Properties &variables, bool force /*=
// false*/){

//

//     if(!TrailEntity || !_RenderNode)
//         return false;

//     // Set if we unconnected the node and we should reconnect it afterwards //
// 	bool ConnectAgain = false;

// 	// Determine if we need to unconnect the node //
// 	if(force || variables.MaxChainElements != CurrentSettings.MaxChainElements){

// 		// This to avoid Ogre bug //
// 		TrailEntity->removeNode(_RenderNode->Node);
// 		ConnectAgain = true;

// 		// Apply the properties //
// 		TrailEntity->setUseVertexColours(true);
// 		TrailEntity->setRenderingDistance(variables.MaxDistance);
// 		TrailEntity->setMaxChainElements(variables.MaxChainElements);
// 		TrailEntity->setCastShadows(variables.CastShadows);
// 		TrailEntity->setTrailLength(variables.TrailLenght);
// 	}

// 	// Update cached settings //
// 	CurrentSettings = variables;

// 	// Apply per element properties //
// 	for(size_t i = 0; i < variables.Elements.size(); i++){
// 		// Apply settings //
// 		const ElementProperties& tmp = variables.Elements[i];

//         TrailEntity->setInitialColour(i, tmp.InitialColour);
//         TrailEntity->setInitialWidth(i, tmp.InitialSize);
//         TrailEntity->setColourChange(i, tmp.ColourChange);
//         TrailEntity->setWidthChange(i, tmp.SizeChange);
// 	}

// 	// More bug avoiding //
// 	if(ConnectAgain)
// 		TrailEntity->addNode(_RenderNode->Node);

// 	return true;
// }
// // ------------------------------------ //
// DLLEXPORT void Trail::Release(Ogre::SceneManager* scene){

//     if(TrailEntity){

//         scene->destroyRibbonTrail(TrailEntity);
//         TrailEntity = nullptr;
//     }
// }
// ------------------ Sendable ------------------ //
DLLEXPORT void Sendable::ActiveConnection::CheckReceivedPackets()
{

    if(SentPackets.empty())
        return;

    // Looped in reverse to hopefully remove only last elements //
    for(int i = static_cast<int>(SentPackets.size() - 1); i >= 0;) {

        const auto& tuple = SentPackets[i];

        if(std::get<2>(tuple)->IsFinalized()) {

            if(std::get<2>(tuple)->GetStatus()) {

                // Succeeded //
                if(std::get<0>(tuple) > LastConfirmedTickNumber) {

                    LastConfirmedTickNumber = std::get<0>(tuple);
                    LastConfirmedData = std::get<1>(tuple);
                }
            }

            SentPackets.erase(SentPackets.begin() + i);

            if(SentPackets.empty())
                break;

        } else {

            i--;
        }
    }

    if(SentPackets.capacity() > 10) {

        Logger::Get()->Warning("Sendable::ActiveConnection: SentPackets has space for over 10 "
                               "sent packets");
        SentPackets.shrink_to_fit();
    }
}
// ------------------------------------ //
DLLEXPORT Model::Model(
    Ogre::SceneManager* scene, Ogre::SceneNode* parent, const std::string& meshname) :
    Component(TYPE)
{
    GraphicalObject = scene->createItem(meshname);
    GraphicalObject->setRenderQueueGroup(DEFAULT_RENDER_QUEUE);
    parent->attachObject(GraphicalObject);
}

DLLEXPORT void Model::Release(Ogre::SceneManager* scene)
{

    scene->destroyItem(GraphicalObject);
}

// ------------------ ManualObject ------------------ //
DLLEXPORT ManualObject::ManualObject(Ogre::SceneManager* scene) : Component(TYPE)
{
    Object = scene->createManualObject();
    Object->setRenderQueueGroup(DEFAULT_RENDER_QUEUE);
}

DLLEXPORT void ManualObject::Release(Ogre::SceneManager* scene)
{
    if(Object) {

        scene->destroyManualObject(Object);
        Object = nullptr;
    }

    CreatedMesh.clear();
}
