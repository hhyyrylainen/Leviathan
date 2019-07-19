// ------------------------------------ //
#include "Components.h"

#include "FileSystem.h"
#include "GameWorld.h"
#include "Generated/ComponentStates.h"
#include "Handlers/IDFactory.h"
#include "Networking/Connection.h"
#include "Networking/SentNetworkThing.h"
#include "Physics/PhysicalWorld.h"
#include "Utility/Convert.h"

#include "Engine.h"
#include "Rendering/Graphics.h"

#include "bsfCore/Components/BsCAnimation.h"
#include "bsfCore/Components/BsCRenderable.h"
// #include "bsfCore/Importer/BsImporter.h"
#include "bsfCore/Renderer/BsRenderable.h"
#include "bsfCore/Scene/BsSceneObject.h"

#include <limits>
using namespace Leviathan;
// ------------------------------------ //
// Position
DLLEXPORT void Position::ApplyState(const PositionState& state)
{
    Members._Orientation = state._Orientation;
    Members._Position = state._Position;
    Marked = true;
}

// ------------------ RenderNode ------------------ //
DLLEXPORT RenderNode::RenderNode(bs::Scene* scene) : Component(TYPE), Scene(*scene)
{
    Marked = false;

    // Skip if no graphics
    if(!Engine::Get()->IsInGraphicalMode())
        return;

    // TODO: allow for static render nodes
    Node = bs::SceneObject::create("");
}

DLLEXPORT void RenderNode::Release(bs::Scene* worldsscene)
{
    if(Node)
        Node->destroy();
    Node = nullptr;
}
// ------------------------------------ //
DLLEXPORT RenderNode::RenderNode(const Test::TestComponentCreation& test) : Component(TYPE)
{
    Marked = false;
    Node = nullptr;
}

// // ------------------------------------ //
// // Plane
// DLLEXPORT Plane::Plane(bs::Scene* scene, Ogre::SceneNode* parent, const std::string&
// material,
//     const Ogre::Plane& plane, const Float2& size,
//     const Ogre::Vector3& uvupvector /*= Ogre::Vector3::UNIT_Y*/) :
//     Component(TYPE),
//     GeneratedMeshName("Plane_Component_Mesh_" + std::to_string(IDFactory::GetID())),
//     Material(material), PlaneDefinition(plane), Size(size), UpVector(uvupvector)
// {
//     const auto mesh = Ogre::v1::MeshManager::getSingleton().createPlane(
//         GeneratedMeshName + "_v1", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
//         plane, size.X, size.Y, 1, 1,
//         // Normals
//         true, 1, 1.0f, 1.0f, uvupvector, Ogre::v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY,
//         Ogre::v1::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false, false);

//     const auto mesh2 = Ogre::MeshManager::getSingleton().createManual(
//         GeneratedMeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

//     // Fourth true is qtangent encoding which is not needed if we don't do normal mapping
//     mesh2->importV1(mesh.get(), true, true, true);

//     Ogre::v1::MeshManager::getSingleton().remove(mesh);

//     GraphicalObject = scene->createItem(mesh2);
//     parent->attachObject(GraphicalObject);

//     // This DOESN'T seem to throw if material is invalid
//     GraphicalObject->setDatablockOrMaterialName(material);

//     GraphicalObject->setRenderQueueGroup(DEFAULT_RENDER_QUEUE);
// }

// DLLEXPORT void Plane::Release(bs::Scene* scene)
// {
//     scene->destroyItem(GraphicalObject);

//     Ogre::MeshManager::getSingleton().remove(GeneratedMeshName);
// }
// ------------------ Physics ------------------ //
DLLEXPORT Physics::~Physics()
{
    if(Body) {
        LOG_FATAL("Physics: Release not called before destructor!");
    }
}

DLLEXPORT void Physics::Release(PhysicalWorld* world)
{
    if(Body) {
        Body->SetOwningEntity(NULL_OBJECT);
        world->DestroyBody(Body.get());
    }

    Body.reset();
}
// ------------------------------------ //
DLLEXPORT void Physics::JumpTo(Position& target)
{
    if(Body)
        Body->SetPosition(target.Members._Position, target.Members._Orientation);
}
// ------------------------------------ //
DLLEXPORT PhysicsBody::pointer Physics::CreatePhysicsBody(PhysicalWorld* world,
    const PhysicsShape::pointer& shape, float mass, int physicsmaterialid /*= -1*/)
{
    if(!world || !shape)
        return nullptr;

    // Destroy old if there is one //
    if(Body)
        world->DestroyBody(Body.get());

    Body = world->CreateBodyFromCollision(shape, mass, &_Position, physicsmaterialid);

    if(!Body)
        return nullptr;

    Body->SetOwningEntity(ThisEntity);

    return Body;
}

DLLEXPORT bool Physics::ChangeShape(PhysicalWorld* world, const PhysicsShape::pointer& shape)
{
    if(!world || !shape)
        return false;

    return world->ChangeBodyShape(Body, shape);
}

// ------------------ Received ------------------ //
// DLLEXPORT void Received::GetServerSentStates(
//     StoredState const** first, StoredState const** second, int tick, float& progress) const
// {
//     // Used to find the first tick before or on tick //
//     int firstinpast = std::numeric_limits<int>::max();
//     int secondfound = 0;

//     for(auto& obj : ClientStateBuffer) {

//         if(tick - obj.Tick < firstinpast && tick - obj.Tick >= 0) {

//             // This is (potentially) the first state //
//             firstinpast = tick - obj.Tick;

//             *first = &obj;
//         }

//         // For this to be found the client should be around 50-100 milliseconds in the past
//         if(obj.Tick > tick && (secondfound == 0 || obj.Tick - tick < secondfound)) {

//             // The second state //
//             *second = &obj;

//             secondfound = obj.Tick - tick;
//             continue;
//         }
//     }

//     if(firstinpast == std::numeric_limits<int>::max() || secondfound == 0) {

//         throw InvalidState("No stored server states around tick");
//     }

//     // If the range is not 1, meaning firstinpast != 0 || secondfound > 1 we need to adjust
//     // progress
//     int range = firstinpast + secondfound;

//     if(range == 1)
//         return;

//     progress = ((tick + progress) - (*first)->Tick) / range;
// }
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
// DLLEXPORT void Trail::Release(bs::Scene* scene){

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

        auto& tuple = SentPackets[i];

        if(std::get<2>(tuple)->IsFinalized()) {

            if(std::get<2>(tuple)->GetStatus()) {

                // Succeeded //
                if(std::get<0>(tuple) > LastConfirmedTickNumber) {

                    LastConfirmedTickNumber = std::get<0>(tuple);

                    // This is always erased (a few lines later) so it's fine to swap
                    LastConfirmedData = std::move(std::get<1>(tuple));
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

        LOG_WARNING("Sendable::ActiveConnection: SentPackets has space for over 10 "
                    "sent packets");
        SentPackets.shrink_to_fit();
    }
}
// ------------------------------------ //
DLLEXPORT Model::Model(bs::Scene* scene, RenderNode& parent, const std::string& meshname,
    const bs::HMaterial& material) :
    Component(TYPE),
    MeshName(meshname), Material(material)
{
    Marked = false;

    // Skip if no graphics
    if(!parent.Node)
        return;

    GraphicalObject = parent.Node->addComponent<bs::CRenderable>();

    // GraphicalObject->setLayer(1 << *parent.Scene);
    GraphicalObject->setLayer(1 << *scene);

    ApplyMeshName();

    GraphicalObject->setMaterial(Material);
}

DLLEXPORT void Model::Release()
{
    if(GraphicalObject)
        GraphicalObject->destroy();
}

DLLEXPORT void Model::ApplyMeshName()
{
    if(MeshName.empty()) {
        GraphicalObject->setMesh(nullptr);
        return;
    }

    // Find the mesh
    auto mesh = Engine::Get()->GetGraphics()->LoadMeshByName(MeshName);

    GraphicalObject->setMesh(mesh);
}

// // ------------------ ManualObject ------------------ //
// DLLEXPORT ManualObject::ManualObject(bs::Scene* scene) : Component(TYPE)
// {
//     Object = scene->createManualObject();
//     Object->setRenderQueueGroup(DEFAULT_RENDER_QUEUE);
// }

// DLLEXPORT void ManualObject::Release(bs::Scene* scene)
// {
//     if(Object) {

//         scene->destroyManualObject(Object);
//         Object = nullptr;
//     }

//     CreatedMesh.clear();
// }

// ------------------------------------ //
// Animated
DLLEXPORT Animated::Animated(RenderNode& node) : Component(TYPE)
{
    Animation = node.Node->addComponent<bs::CAnimation>();
}

DLLEXPORT void Animated::Release()
{
    if(Animation)
        Animation->destroy();
    Animation = nullptr;
}
