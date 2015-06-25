// ------------------------------------ //
#ifndef LEVIATHAN_GAMEWORLD
#include "GameWorld.h"
#endif
#include "Rendering/GraphicalInputEntity.h"
#include "Newton/NewtonManager.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreLight.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/NetworkResponse.h"
#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkHandler.h"
#include "Threading/ThreadingManager.h"
#include "Handlers/EntitySerializerManager.h"
#include "Handlers/ConstraintSerializerManager.h"
#include "Entities/Objects/Constraints.h"
#include "Engine.h"
#include "Newton/PhysicsMaterialManager.h"
#include "../Handlers/IDFactory.h"
#include "../Window.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
//! \brief Class used by _OnNotifiableConnected to hold temporary connection data
class Leviathan::PlayerConnectionPreparer{

public:

    //! Used to keep track of what's happening regards to the ping
    enum PING_STATE {PING_STATE_STARTED, PING_STATE_FAILED, PING_STATE_NONE, PING_STATE_COMPLETED,
                     PING_STATE_FINAL_STEP};
    
    PlayerConnectionPreparer(ConnectedPlayer* ply, GameWorld* world,
        std::shared_ptr<ConnectionInfo> connection) :
        GameWorldCompromised(false), World(world),
        Player(ply), Connection(connection), TimingFailed(false),
        Pinging(PING_STATE_NONE), PingTime(0), ObjectsReady(false), AllDone(false)
    {


    }
    
    ~PlayerConnectionPreparer(){

        // Cancel all tasks //
        ThreadingManager::Get()->RemoveTasksFromQueue(OurQueued);
    }

    //! Called as an callback when pinging completes
    void OnPingCompleted(int msping, int lostpackets){


        PingTime = msping;
        
        SendClockSyncRequest(msping);
    }

    //! Sends the clock sync request and starts waiting
    void SendClockSyncRequest(int msping){

        // Send a tick sync thing //
        if(GameWorldCompromised)
            return;

        int targettick;
        {
            // The world tick will be the same as the engine tick
            GUARD_LOCK_OTHER(World);

            targettick = World->TickNumber-INTERPOLATION_TIME/TICKSPEED;
        }

        // Check how long until we tick again //
        int timeintick = Engine::Get()->GetTimeSinceLastTick();

        // Take the ping into account //
        float sendtime = (msping) / (float)TICKSPEED;

        int wholeticks = floor(sendtime);

        targettick -= wholeticks;

        sendtime -= wholeticks;

        // For maximum accuray we are also going to adjust the receiver's engine tick //
        int enginemscorrect = timeintick - (sendtime*(float)TICKSPEED);

        Logger::Get()->Info("GameWorld: adjusting client to "+Convert::ToString(targettick)+" ticks and engine "
            "clock by "+Convert::ToString(enginemscorrect)+" ms");
        
        std::shared_ptr<NetworkRequest> clocksync = make_shared<NetworkRequest>(new RequestWorldClockSyncData(
                World->GetID(), targettick, enginemscorrect, true));

        auto sentthing = Connection->SendPacketToConnection(clocksync, 1);
        sentthing->SetAsTimed();

        // Start waiting for it //
        auto waitthing = make_shared<ConditionalTask>(std::bind<void>([](PlayerConnectionPreparer* plyprepare,
                    std::shared_ptr<SentNetworkThing> sentthing, int msping, int enginems) -> void
            {
                bool succeeded = sentthing->GetStatus();

                // Resend once //
                if(!succeeded){

                    if(plyprepare->TimingFailed){

                        // TODO: kick player
                        DEBUG_BREAK;
                        return;
                    }

                    plyprepare->TimingFailed = true;
                    plyprepare->SendClockSyncRequest(msping);
                    return;
                }
                
                // Send a correction packet //
                int64_t elapsedtime = sentthing->ConfirmReceiveTime-sentthing->RequestStartTime;
                
                // Here we calculate how much our initial estimate of the time taken is off by
                float correctingamount = elapsedtime-msping;

                Logger::Get()->Info("GameWorld: adjust clock expected to take "+Convert::ToString(msping)+
                    " and it took "+Convert::ToString(elapsedtime)+" correcting by "+
                    Convert::ToString(correctingamount));

                correctingamount /= (float)TICKSPEED;

                int wholecorrect = floor(correctingamount);

                correctingamount -= wholecorrect;

                int enginemscorrect = correctingamount*TICKSPEED;

                // Send correction if not 0 //
                if(wholecorrect != 0 || enginemscorrect != 0){

                    if(plyprepare->GameWorldCompromised)
                        return;
                    
                    Logger::Get()->Info("GameWorld: clock sync: sending a follow up correction of: "+Convert::ToString(
                            wholecorrect)+" ticks and "+Convert::ToString(enginemscorrect)+" ms");

                    std::shared_ptr<NetworkRequest> clocksync = make_shared<NetworkRequest>(new RequestWorldClockSyncData(
                            plyprepare->World->GetID(), wholecorrect, enginemscorrect, false), 500);

                    auto sentthing = plyprepare->Connection->SendPacketToConnection(clocksync, 20);
                    
                }

                // Pinging is now done //
                plyprepare->Pinging = PING_STATE_COMPLETED;
                
                std::unique_lock<std::mutex> lock(plyprepare->_Mutex);
                if(plyprepare->ObjectsReady)
                    plyprepare->AllDone = true;

                plyprepare->OurQueued.clear();


                }, this, sentthing, msping, timeintick), std::bind<bool>([](shared_ptr<SentNetworkThing> sentthing)
                    -> bool
                {
                    return sentthing->IsFinalized();
                }, sentthing));
        
        
        OurQueued.push_back(waitthing);

        ThreadingManager::Get()->QueueTask(waitthing);
    }

    //! Called when pinging fails
    void OnPingFailed(CONNECTION_PING_FAIL_REASON failreason, int lostpackets){

        // TODO: kick the player
        DEBUG_BREAK;
    }

    //! Set by GameWorld if it isn't safe to use
    bool GameWorldCompromised;

    //! The world
    GameWorld* World;

    // This is needed to avoid rare cases where AllDone would never be set
    std::mutex _Mutex;

    //! The player who is being handled, used only for deleting
    //! The pointer is unsafe to use
    ConnectedPlayer* Player;

    std::shared_ptr<ConnectionInfo> Connection;

    bool TimingFailed;
    
    PING_STATE Pinging;
    int PingTime;
    
    bool ObjectsReady;
    bool AllDone;

    std::vector<shared_ptr<QueuedTask>> OurQueued;
};

// ------------------ GameWorld ------------------ //
DLLEXPORT Leviathan::GameWorld::GameWorld() :
    WorldSceneCamera(NULL), WorldsScene(NULL), Sunlight(NULL), SunLightNode(NULL), WorldFrozen(false),
    GraphicalMode(false), LinkedToWindow(NULL), WorldWorkspace(NULL), ClearAllObjects(false),
    ID(IDFactory::GetID()), TickNumber(0)
{
    IsOnServer = NetworkHandler::Get()->GetNetworkType() == NETWORKED_TYPE_SERVER;
}

DLLEXPORT Leviathan::GameWorld::~GameWorld(){

    // Entities need to be disposed before physical world is destroyed //
    // This in incase the world is not properly destroyed //
    Objects.clear();
    
    
    // This should be relatively cheap if the newton threads don't deadlock while waiting
    // for each other
    _PhysicalWorld.reset();
    //Logger::Get()->Info("GameWorld("+Convert::ToString(ID)+"): has been destroyed");
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::Init(GraphicalInputEntity* renderto, Ogre::Root* ogre){

	LinkedToWindow = renderto;

	// Detecting non-GUI mode //
	if(ogre){
        
		if(!renderto)
			return false;
        
		GraphicalMode = true;
		// these are always required for worlds //
		_CreateOgreResources(ogre, renderto->GetWindow());
	}

	// Acquire physics engine world //
    // This should not be required if it isn't available
    if(NewtonManager::Get()){
        
        _PhysicalWorld = NewtonManager::Get()->CreateWorld(this);
    }

	return true;
}

DLLEXPORT void Leviathan::GameWorld::Release(){
	GUARD_LOCK();
    
    // Tell initially syncing players that we are no longer valid //
    auto end = InitiallySyncingPlayers.end();
    for(auto iter = InitiallySyncingPlayers.begin(); iter != end; ++iter){

        (*iter)->GameWorldCompromised = true;
    }

    InitiallySyncingPlayers.clear();

    ReceivingPlayers.clear();
    
	// release objects //
    // TODO: allow objects to know that they are about to get killed

    // As all objects are just pointers to components we can just dump the objects
    // and once the component pools are released
    Objects.clear();

    // Clear all nodes //
    NodeRenderingPosition.Clear();
    NodeSendableNode.Clear();
    NodeReceivedPosition.Clear();
    NodeRenderNodeHiderNode.Clear();

    // Clear all componenst //
    ComponentPosition.Clear();
    ComponentRenderNode.Clear();
    ComponentSendable.Clear();
    ComponentModel.Clear();
    ComponentPhysics.Clear();
    ComponentConstraintable.Clear();
    ComponentBoxGeometry.Clear();
    ComponentManualObject.Clear();
    ComponentPositionMarkerOwner.Clear();
    ComponentParent.Clear();
    ComponentTrail.Clear();
    ComponentTrackController.Clear();
    ComponentReceived.Clear();
    ComponentParentable.Clear();

    
	if(GraphicalMode){
		// TODO: notify our window that it no longer has a world workspace
		LinkedToWindow = NULL;

		// release Ogre resources //

		// Destroy the compositor //
		Ogre::Root& ogre = Ogre::Root::getSingleton();
		ogre.getCompositorManager2()->removeWorkspace(WorldWorkspace);
		WorldWorkspace = NULL;
		ogre.destroySceneManager(WorldsScene);
		WorldsScene = NULL;
	}

    // Unhook from other objects //
    ReleaseChildHooks(guard);

    guard.unlock();
}
// ------------------------------------ //
void Leviathan::GameWorld::_CreateOgreResources(Ogre::Root* ogre, Window* rendertarget){
	// create scene manager //
	WorldsScene = ogre->createSceneManager(Ogre::ST_EXTERIOR_FAR, 2, Ogre::INSTANCING_CULLING_THREADED,
        "MainSceneManager_"+Convert::ToString(ID));

	WorldsScene->setShadowFarDistance(1000.f);
	WorldsScene->setShadowDirectionalLightExtrusionDistance(10000.f);

	// create camera //
	WorldSceneCamera = WorldsScene->createCamera("Camera01");

	// near and far clipping planes //
	WorldSceneCamera->setFOVy(Ogre::Radian(60.f*DEGREES_TO_RADIANS));
	WorldSceneCamera->setNearClipDistance(0.1f);
	WorldSceneCamera->setFarClipDistance(50000.f);

	// enable infinite far clip distance if supported //
	if(ogre->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE)){
		
		WorldSceneCamera->setFarClipDistance(0);
	}

	// set scene ambient colour //
	WorldsScene->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

	// default sun //
	SetSunlight();
	
	// Create the workspace for this scene //
	// Which will be rendered before the overlay workspace //
	WorldWorkspace = ogre->getCompositorManager2()->addWorkspace(WorldsScene,
        rendertarget->GetOgreWindow(), 
		WorldSceneCamera, "WorldsWorkspace", true, 0);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SetSkyBox(const string &materialname){
	try{
		WorldsScene->setSkyBox(true, materialname);
	}
	catch(const Ogre::InvalidParametersException &e){

		Logger::Get()->Error("[EXCEPTION] "+e.getFullDescription());
	}
}

DLLEXPORT void Leviathan::GameWorld::SetFog(){
	WorldsScene->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue(0.7f, 0.7f, 0.8f), 0, 4000, 10000);
	WorldsScene->setFog(Ogre::FOG_NONE);
}

DLLEXPORT void Leviathan::GameWorld::SetSunlight(){
	// create/update things if they are NULL //
	if(!Sunlight){
		Sunlight = WorldsScene->createLight();
		Sunlight->setName("sunlight");
	}

	Sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
	Sunlight->setDiffuseColour(0.98f, 1.f, 0.95f);
	Sunlight->setSpecularColour(1.f, 1.f, 1.f);

	if(!SunLightNode){

		SunLightNode = WorldsScene->getRootSceneNode()->createChildSceneNode();
		SunLightNode->setName("sunlight node");

		SunLightNode->attachObject(Sunlight);
	}

	Ogre::Quaternion quat;
	quat.FromAngleAxis(Ogre::Radian(1.f), Float3(0.55f, -0.3f, 0.75f));
	SunLightNode->setOrientation(quat);
}

DLLEXPORT void Leviathan::GameWorld::RemoveSunlight(){
	if(SunLightNode){
		SunLightNode->detachAllObjects();
		// might be safe to delete
		OGRE_DELETE SunLightNode;
		SunLightNode = NULL;
	}
}

DLLEXPORT void Leviathan::GameWorld::UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos, Lock &guard){
	VerifyLock(guard);
	// Skip if no camera //
	if(camerapos == NULL)
		return;

	// update camera //
	camerapos->UpdatePos(mspassed);

	// set camera position //
	WorldSceneCamera->setPosition(camerapos->GetPosition());

	// convert rotation into a quaternion //
	const Float3& angles = camerapos->GetRotation();

	// create quaternion from quaternion rotations around each axis //
	Ogre::Quaternion rotq(Ogre::Degree(angles.Y), Ogre::Vector3::UNIT_X);
	Ogre::Quaternion rotyaw(Ogre::Degree(angles.X), Ogre::Vector3::UNIT_Y);
	Ogre::Quaternion rotroll(Ogre::Degree(angles.Z), Ogre::Vector3::UNIT_Z);

	rotq = rotyaw*rotq*rotroll;

	WorldSceneCamera->setOrientation(rotq);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::ShouldPlayerReceiveObject(Position &atposition,
    ConnectionInfo* connectionptr)
{

    return true;
}
// ------------------------------------ //
DLLEXPORT int GameWorld::GetObjectCount() const{

    return Objects.size();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SimulatePhysics(Lock &guard){

    if(IsOnServer){
    
        if(!WorldFrozen){

            _ApplyEntityUpdatePackets(guard);
            _ApplyConstraintPackets(guard);

            // Let's set the maximum runs to 10000 to disable completely deadlocking
            _PhysicalWorld->SimulateWorld(10000);
        }
    } else {

        // Simulate direct control //
        
    }
    
}

DLLEXPORT void Leviathan::GameWorld::ClearTimers(Lock &guard){
    
    _PhysicalWorld->ClearTimers();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::Tick(int currenttick){

    GUARD_LOCK();

    TickNumber = currenttick;

    // Apply queued packets //
    ApplyQueuedPackets(guard);

    // All required nodes for entities are created //
    HandleAdded(guard);

    _HandleDelayedDelete(guard);

    HandleDeleted(guard);

    // Sendable objects may need something to be done //
    auto nethandler = NetworkHandler::Get();

    // TODO: the vectors could be sorted to improve branch predictor performance
    if(nethandler && nethandler->GetNetworkType() == NETWORKED_TYPE_SERVER){

        // Skip if not tick that will be stored //
        if(TickNumber % WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL == 0){

            RunSendableSystem(this, guard, TickNumber);
        }
        
    } else if(nethandler && nethandler->GetNetworkType() == NETWORKED_TYPE_CLIENT){

        // TODO: direct control objects
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::RemoveInvalidNodes(Lock &guard){

    // This first gets the vector that contain the possibly deleted components and then deletes
    // them from node pools and finally properly clears or releases each pool that possibly got
    // updated

    // Position, RenderNode
    {
        GUARD_LOCK_OTHER_NAME((&ComponentPosition), positionlock);
        GUARD_LOCK_OTHER_NAME((&ComponentRenderNode), rendernodelock);

        auto& removedposition = ComponentPosition.GetRemoved(positionlock);
        auto& removedrendernode = ComponentRenderNode.GetRemoved(rendernodelock);

        if(!removedposition.empty()){
            
            NodeRenderingPosition.RemoveBasedOnKeyTupleList(removedposition, false);
            NodeReceivedPosition.RemoveBasedOnKeyTupleList(removedposition, false);
        }

        if(!removedrendernode.empty()){
            
            NodeRenderingPosition.RemoveBasedOnKeyTupleList(removedrendernode, false);
            NodeRenderNodeHiderNode.RemoveBasedOnKeyTupleList(removedrendernode, false);
        }
    }

    // Received
    {
        GUARD_LOCK_OTHER_NAME((&ComponentReceived), receivedlock);
        GUARD_LOCK_OTHER_NAME((&ComponentRenderNode), rendernodelock);

        auto& removedreceived = ComponentReceived.GetRemoved(receivedlock);

        if(!removedreceived.empty()){

            NodeReceivedPosition.RemoveBasedOnKeyTupleList(removedreceived, false);
        }
    }

    // Sendable
    {
        GUARD_LOCK_OTHER_NAME((&ComponentSendable), sendablelock);

        auto& removedsendable = ComponentSendable.GetRemoved(sendablelock);

        if(!removedsendable.empty()){
            
            NodeSendableNode.RemoveBasedOnKeyTupleList(removedsendable, false);
        }
    }
    
}

DLLEXPORT void GameWorld::HandleDeleted(Lock &guard){

    // Delete queued objects //
    if(ComponentRenderNode.HasElementsInQueued()){

        if(WorldsScene){

            // Scene still exists, delete scene nodes //
            ComponentRenderNode.ReleaseQueued(WorldsScene);
            
        } else {

            // Clear without deleting, Ogre has already released the memory //
            ComponentRenderNode.ClearQueued();
        }
    }
    
    if(ComponentTrail.HasElementsInRemoved()){

        if(WorldsScene){

            // Scene still exists, delete scene nodes //
            ComponentTrail.ReleaseQueued(WorldsScene);
            
        } else {

            // Clear without deleting, Ogre has already released the memory //
            ComponentTrail.ClearQueued();
        }
    }

    if(ComponentModel.HasElementsInQueued()){

        if(WorldsScene){

            // Scene still exists, delete scene nodes //
            ComponentModel.ReleaseQueued(WorldsScene);
            
        } else {

            // Clear without deleting, Ogre has already released the memory //
            ComponentModel.ClearQueued();
        }
    }

    if(ComponentManualObject.HasElementsInQueued()){

        if(WorldsScene){

            // Scene still exists, delete scene nodes //
            ComponentManualObject.ReleaseQueued(WorldsScene);
            
        } else {

            // Clear without deleting, Ogre has already released the memory //
            ComponentManualObject.ClearQueued();
        }
    }

    if(ComponentPhysics.HasElementsInQueued()){

        if(_PhysicalWorld){

            // Safe for the newton objects to be destroyed
            ComponentPhysics.ReleaseQueued();
            
        } else {

            ComponentPhysics.ClearQueued();
        }
    }

    ComponentPositionMarkerOwner.ReleaseQueued(this, guard);

    // Handle nodes with now missing components //
    // This uses Removed vectors inside the components
    RemoveInvalidNodes(guard);
    
    ComponentPosition.ClearRemoved();
    ComponentRenderNode.ClearRemoved();
    ComponentSendable.ClearRemoved();
    ComponentModel.ClearRemoved();
    ComponentPhysics.ClearRemoved();
    ComponentConstraintable.ClearRemoved();
    ComponentBoxGeometry.ClearRemoved();
    ComponentManualObject.ClearRemoved();
    ComponentPositionMarkerOwner.ClearRemoved();
    ComponentParent.ClearRemoved();
    ComponentTrail.ClearRemoved();
    ComponentTrackController.ClearRemoved();
    ComponentReceived.ClearRemoved();
    ComponentParentable.ClearRemoved();
}

DLLEXPORT void GameWorld::HandleAdded(Lock &guard){

    // Construct new nodes based on components values //
    // Almost the opposite of RemoveInvalidNodes
    // CreateNodes automatically removes the used onces from the Added in component pool

    // Position, RenderNode (Received)
    {
        GUARD_LOCK_OTHER_NAME((&ComponentPosition), positionlock);
        GUARD_LOCK_OTHER_NAME((&ComponentRenderNode), rendernodelock);
        GUARD_LOCK_OTHER_NAME((&ComponentReceived), receivedlock);

        auto& addedposition = ComponentPosition.GetAdded(positionlock);
        auto& addedrendernode = ComponentRenderNode.GetAdded(rendernodelock);

        auto& addedreceived = ComponentReceived.GetAdded(receivedlock);

        if(!addedposition.empty()){

            _RenderingPositionSystem.CreateNodes(NodeRenderingPosition, addedposition,
                addedrendernode, ComponentRenderNode, rendernodelock);

            _ReceivedPositionSystem.CreateNodes(NodeReceivedPosition, addedposition,
                addedreceived, ComponentReceived, receivedlock);
        }

        if(!addedreceived.empty()){

            _ReceivedPositionSystem.CreateNodes(NodeReceivedPosition, addedreceived,
                addedposition, ComponentPosition, positionlock);
        }
        
        if(!addedrendernode.empty()){

            _RenderingPositionSystem.CreateNodes(NodeRenderingPosition, addedrendernode,
                addedposition, ComponentPosition, positionlock);

            _RenderNodeHiderSystem.CreateNodes(NodeRenderNodeHiderNode, addedrendernode);
        }
    }

    // Sendable
    {
        GUARD_LOCK_OTHER_NAME((&ComponentSendable), sendablelock);

        auto& addedsendable = ComponentSendable.GetAdded(sendablelock);

        if(!addedsendable.empty()){
            
            _SendableSystem.CreateNodes(NodeSendableNode, addedsendable);
        }
    }

    // Constraintable
    // Attach to missing constraints here, run system
    

    // Clear added //
    ComponentPosition.ClearAdded();
    ComponentRenderNode.ClearAdded();
    ComponentSendable.ClearAdded();
    ComponentReceived.ClearAdded();
    ComponentModel.ClearAdded();
    ComponentPhysics.ClearAdded();
    ComponentConstraintable.ClearAdded();
    ComponentBoxGeometry.ClearAdded();
    ComponentManualObject.ClearAdded();
    ComponentPositionMarkerOwner.ClearAdded();
    ComponentParent.ClearAdded();
    ComponentTrail.ClearAdded();
    ComponentTrackController.ClearAdded();
    ComponentReceived.ClearAdded();
    ComponentParentable.ClearAdded();
}
// ------------------------------------ //
DLLEXPORT void GameWorld::RunFrameRenderSystems(int timeintick){

    GUARD_LOCK();

    HandleDeleted(guard);

    HandleAdded(guard);

    ApplyExistingEntityUpdates(guard);

    // Client interpolation //
    if(!IsOnServer){

        const float interpolatepercentage = std::max(0.f,
            std::min(timeintick / (float)TICKSPEED, 1.f));
        
        RunInterpolationSystem(TickNumber, interpolatepercentage);
        
        // TODO: run direct control system

        cout << "Interpolate: " << TickNumber << interpolatepercentage << "\n";
    }

    // Skip in non-gui mode //
    if(!GraphicalMode)
        return;
    
    RunRenderingPositionSystem();
    RunRenderNodeHiderSystem();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GameWorld::GetTickNumber() const{
    
    return TickNumber;
}
// ------------------------------------ //
DLLEXPORT void GameWorld::NotifyNewConstraint(std::shared_ptr<BaseConstraint> constraint){

    Lock lock(ConstraintListMutex);

    ConstraintList.push_back(constraint);

    if(IsOnServer){

        GUARD_LOCK();
        
        GUARD_LOCK_OTHER_NAME(constraint, constraintlock);

        auto serialized = ConstraintSerializerManager::Get()->SerializeConstraintData(
            constraint.get());

        auto packet = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000);
        
        packet->GenerateEntityConstraintResponse(new NetworkResponseDataForEntityConstraint(ID,
                constraint->GetID(), constraint->GetFirstEntity().PartOfEntity,
                constraint->GetSecondEntity().PartOfEntity, true, constraint->GetType(),
                serialized));

        constraintlock.unlock();
        
        for(auto&& player : ReceivingPlayers){

            auto safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(
                player->GetConnection());

            if(safeconnection)
                safeconnection->SendPacketToConnection(packet, 5);
        }
    }
}

DLLEXPORT void GameWorld::ConstraintDestroyed(BaseConstraint* constraint){

    if(IsOnServer){

        GUARD_LOCK();

        GUARD_LOCK_OTHER_NAME(constraint, constraintlock);

        auto packet = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1500);
        
        packet->GenerateEntityConstraintResponse(new NetworkResponseDataForEntityConstraint(ID,
                constraint->GetID(), 0, 0, false, constraint->GetType()));

        constraintlock.unlock();
        
        for(auto&& player : ReceivingPlayers){

            auto safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(
                player->GetConnection());

            if(safeconnection)
                safeconnection->SendPacketToConnection(packet, 10);
        }
    }

    Lock lock(ConstraintListMutex);

    for(auto iter = ConstraintList.begin(); iter != ConstraintList.end(); ++iter){

        if((*iter).get() == constraint){

            ConstraintList.erase(iter);
            return;
        }
    }
}
// ------------------ Object managing ------------------ //
DLLEXPORT ObjectID GameWorld::CreateEntity(Lock &guard){

    auto id = IDFactory::GetID();

    Objects.push_back(id);

    return static_cast<ObjectID>(id);
}

DLLEXPORT void GameWorld::NotifyEntityCreate(Lock &guard, ObjectID id){

    if(IsOnServer){

        // This is at least a decent place to send them,
        // any constraints created later will get send when they are created
        Sendable* issendable = nullptr;
        
        try{
            issendable = &GetComponent<Sendable>(id);

        } catch(const NotFound&){

            // Not sendable no point in continuing //
            return;
        }

        if(!issendable)
            return;

        auto end = ReceivingPlayers.end();
        for(auto iter = ReceivingPlayers.begin(); iter != end; ++iter){

            auto unsafe = (*iter)->GetConnection();

            auto safe = NetworkHandler::Get()->GetSafePointerToConnection(unsafe);

            if(!safe){
                // Player has probably closed their connection //
                continue;
            }

            // TODO: pass issendable here to avoid an extra lookup
            if(!SendObjectToConnection(guard, id, safe)){

                Logger::Get()->Warning("GameWorld: CreateEntity: failed to send object to player "
                    "("+(*iter)->GetNickname()+")");
                
                continue;
            }
        }

        
    } else {

        // Clients register received objects here //
        Objects.push_back(id);
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::ClearObjects(Lock &guard){
	VerifyLock(guard);

    // release objects //
    // TODO: allow objects to do something
    Objects.clear();

    ComponentPosition.Clear();
    ComponentRenderNode.Clear();
    ComponentSendable.Clear();
    ComponentModel.Clear();
    ComponentPhysics.Clear();
    ComponentConstraintable.Clear();
    ComponentBoxGeometry.Clear();
    ComponentManualObject.Clear();
    ComponentPositionMarkerOwner.Clear();
    ComponentParent.Clear();
    ComponentTrail.Clear();
    ComponentTrackController.Clear();
    ComponentReceived.Clear();
    ComponentParentable.Clear();

    
    NodeRenderingPosition.Clear();
    NodeSendableNode.Clear();
    NodeReceivedPosition.Clear();
    NodeRenderNodeHiderNode.Clear();

    // Notify everybody that all entities are discarded //
    auto end = ReceivingPlayers.end();
    for(auto iter = ReceivingPlayers.begin(); iter != end; ++iter){

        auto unsafe = (*iter)->GetConnection();

        auto safe = NetworkHandler::Get()->GetSafePointerToConnection(unsafe);

        if(!safe){
            // Player has probably closed their connection //
            continue;
        }

        Logger::Get()->Write("TODO: send world clear message");
    }
}
// ------------------------------------ //
DLLEXPORT Float3 Leviathan::GameWorld::GetGravityAtPosition(const Float3 &pos){
	// \todo take position into account //
	// create force without mass applied //
	return Float3(0.f, PHYSICS_BASE_GRAVITY, 0.f);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GameWorld::GetPhysicalMaterial(const string &name){

    return PhysicsMaterialManager::Get()->GetMaterialIDForWorld(name,
        _PhysicalWorld->GetNewtonWorld());
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::DestroyObject(ObjectID id){
    GUARD_LOCK();

    auto end = Objects.end();
	for(auto iter = Objects.begin(); iter != end; ++iter){
        
		if(*iter == id){

            _DoDestroy(guard, id);
			Objects.erase(iter);
            
			return;
		}
	}
}

DLLEXPORT void Leviathan::GameWorld::QueueDestroyObject(int EntityID){

	Lock lock(DeleteMutex);
	DelayedDeleteIDS.push_back(EntityID);
}

void Leviathan::GameWorld::_HandleDelayedDelete(Lock &guard){

	// We might want to delete everything //
	if(ClearAllObjects){

		ClearObjects(guard);

		ClearAllObjects = false;

        guard.unlock();
        
        Lock lock(DeleteMutex);
        DelayedDeleteIDS.clear();
        
        guard.lock();
        
		// All are now cleared //
		return;
	}

    guard.unlock();

    Lock lock(DeleteMutex);

    guard.lock();
    
	// Return right away if no objects to delete //
	if(DelayedDeleteIDS.empty())
		return;

	// Search all objects and find the ones that need to be deleted //
	for(auto iter = Objects.begin(); iter != Objects.end(); ){

		// Check does id match any //
		auto curid = *iter;
		bool delthis = false;

		for(auto iterids = DelayedDeleteIDS.begin(); iterids != DelayedDeleteIDS.end(); ){

			if(*iterids == curid){
				// Remove this as it will get deleted //
				delthis = true;
				DelayedDeleteIDS.erase(iterids);
				break;

			} else {
				++iterids;
			}
		}

		if(delthis){

            _DoDestroy(guard, curid);
			iter = Objects.erase(iter);
            
			// Check for end //
			if(DelayedDeleteIDS.empty())
                return;

		} else {
			++iter;
		}
	}
}

void GameWorld::_DoDestroy(Lock &guard, ObjectID id){

    Logger::Get()->Info("GameWorld destroying object "+Convert::ToString(id));

    if(IsOnServer)
        _ReportEntityDestruction(guard, id);

    // TODO: find a better way to do this
    RemoveComponent<Position>(id);
    RemoveComponent<RenderNode>(id);
    RemoveComponent<Sendable>(id);
    RemoveComponent<Model>(id);
    RemoveComponent<Physics>(id);
    RemoveComponent<Constraintable>(id);
    RemoveComponent<BoxGeometry>(id);
    RemoveComponent<ManualObject>(id);
    RemoveComponent<PositionMarkerOwner>(id);
    RemoveComponent<Parent>(id);
    RemoveComponent<Trail>(id);
    RemoveComponent<TrackController>(id);
    RemoveComponent<Received>(id);
    RemoveComponent<Parentable>(id);

    
    NodesToInvalidate.push_back(id);
}

void Leviathan::GameWorld::_ReportEntityDestruction(Lock &guard, ObjectID id){

    // Notify everybody that an entity has been destroyed //
    auto end = ReceivingPlayers.end();
    for(auto iter = ReceivingPlayers.begin(); iter != end; ++iter){

        auto unsafe = (*iter)->GetConnection();

        auto safe = NetworkHandler::Get()->GetSafePointerToConnection(unsafe);

        if(!safe){
            // Player has probably closed their connection //
            continue;
        }
        
        // Then gather all sorts of other stuff to make an response //
        std::unique_ptr<NetworkResponseDataForEntityDestruction> resdata(new
            NetworkResponseDataForEntityDestruction(this->ID, id));

        // We return whatever the send function returns //
        std::shared_ptr<NetworkResponse> response = make_shared<NetworkResponse>(-1,
            PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 20);

        response->GenerateEntityDestructionResponse(resdata.release());

        if(!safe->SendPacketToConnection(response, 5).get()){

            Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+
                "): failed to send entity destruction message to client");
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SetWorldPhysicsFrozenState(Lock &guard, bool frozen){
	// Skip if set to the same //
	if(frozen == WorldFrozen)
		return;

	WorldFrozen = frozen;

    if(!WorldFrozen){

        // Reset timers //
        ClearTimers(guard);
    }

    // Send it to receiving players (if we are a server) //
    if(ReceivingPlayers.empty())
        return;

    // Should be safe to create the packet now and send it to all the connections //
    auto packet = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

    packet->GenerateWorldFrozenResponse(new NetworkResponseDataForWorldFrozen(ID, WorldFrozen, TickNumber));

    auto end = ReceivingPlayers.end();
    for(auto iter = ReceivingPlayers.begin(); iter != end; ++iter){

        auto connection = NetworkHandler::Get()->GetSafePointerToConnection((*iter)->GetConnection());
        if(!connection)
            continue;

        connection->SendPacketToConnection(packet, 10);
    }
}

DLLEXPORT RayCastHitEntity* Leviathan::GameWorld::CastRayGetFirstHit(const Float3 &from, const Float3 &to,
    Lock &guard)
{
	VerifyLock(guard);
	// Create a data object for the ray cast //
	RayCastData data(1, from, to);

	// Call the actual ray firing function //
	NewtonWorldRayCast(_PhysicalWorld->GetNewtonWorld(), &from.X, &to.X, RayCallbackDataCallbackClosest, &data, NULL,
        0);

	// Check the result //
	if(data.HitEntities.size() == 0){
		// Nothing hit //
		return new RayCastHitEntity();
	}
	// We need to increase reference count to not to accidentally delete the result while caller is using it //
	data.HitEntities[0]->AddRef();
	// Return the only hit //
	return data.HitEntities[0];
}

// \todo improve this performance //
dFloat Leviathan::GameWorld::RayCallbackDataCallbackClosest(const NewtonBody* const body, const NewtonCollision* const
    shapeHit, const dFloat* const hitContact, const dFloat* const hitNormal, dLong collisionID, void* const userData,
    dFloat intersectParam)
{
	// Let's just store it as a NewtonBody pointer //
	RayCastData* data = reinterpret_cast<RayCastData*>(userData);

	if(data->HitEntities.size() == 0)
		data->HitEntities.push_back(new RayCastHitEntity(body, intersectParam, data));
	else
		*data->HitEntities[0] = RayCastHitEntity(body, intersectParam, data);

	// Continue //
	return intersectParam;
}

DLLEXPORT RayCastHitEntity* Leviathan::GameWorld::CastRayGetFirstHitProxy(const Float3 &from,
    const Float3 &to)
{
	return CastRayGetFirstHit(from, to);
}

DLLEXPORT void Leviathan::GameWorld::MarkForClear(){
	ClearAllObjects = true;
}

bool Leviathan::GameWorld::AreAllPlayersSynced() const{
    // The vector of sending players is empty if not sending //
    GUARD_LOCK();
    return InitiallySyncingPlayers.size() == 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::_OnNotifiableConnected(Lock &guard,
    BaseNotifiableAll* parentadded, Lock &parentlock)
{

	// The connected object will always have to be a ConnectedPlayer
	auto plyptr = static_cast<ConnectedPlayer*>(parentadded);

    Logger::Get()->Info("GameWorld: player(\""+plyptr->GetNickname()+
        "\") is now receiving world");

    // Add them to the list of receiving players //
    ReceivingPlayers.push_back(plyptr);
    
    // We need a safe pointer to the connection //
    ConnectionInfo* unsafeconnection = plyptr->GetConnection();


    auto safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(unsafeconnection);

	// Create an entry for this player //
    auto connectobject = make_shared<PlayerConnectionPreparer>(plyptr, this, safeconnection);

    if(!safeconnection){

        // The closing should be handled by somebody else
        Logger::Get()->Error("GameWorld: requested to sync with a player who has closed their "
            "connection");
        
        return;
    }

    // Start pinging //
    connectobject->Pinging = PlayerConnectionPreparer::PING_STATE_STARTED;

    safeconnection->CalculateNetworkPing(WORLD_CLOCK_SYNC_PACKETS, WORLD_CLOCK_SYNC_ALLOW_FAILS,
        std::bind(&PlayerConnectionPreparer::OnPingCompleted, connectobject, placeholders::_1,
            placeholders::_2),
        std::bind(&PlayerConnectionPreparer::OnPingFailed, connectobject, placeholders::_1,
            placeholders::_2));


    // Update the position data //
    UpdatePlayersPositionData(guard, plyptr, parentlock);

    // Add it to the list //
    InitiallySyncingPlayers.push_back(connectobject);

	// Start sending initial update //

    Logger::Get()->Info("Starting to send "+Convert::ToString(Objects.size())+" to player");
    
    // Now we can queue all objects for sending //
    // TODO: make sure that all objects are sent
    ThreadingManager::Get()->QueueTask(new RepeatCountedTask(std::bind<void>([](shared_ptr<ConnectionInfo>
                    connection, std::shared_ptr<PlayerConnectionPreparer> processingobject, GameWorld* world) -> void
        {
            // Get the next object //
            RepeatCountedTask* task = dynamic_cast<RepeatCountedTask*>(TaskThread::GetThreadSpecificThreadObject()->
                QuickTaskAccess.get());

            assert(task && "wrong type passed to our task");

            int num = task->GetRepeatCount();

            if(processingobject->GameWorldCompromised){

    taskstopprocessingobjectsforinitialsynclabel:
                
                // Stop processing //
                task->StopRepeating();
                processingobject->ObjectsReady = true;
                
    exitcurrentiterationchecklabel:
                
                
                // This will stop the otherwise infinitely waiting task //
                Lock lock(processingobject->_Mutex);
                
                if(task->IsThisLastRepeat()){

                    processingobject->ObjectsReady = true;
                }

                if(processingobject->ObjectsReady && processingobject->Pinging ==
                    PlayerConnectionPreparer::PING_STATE_COMPLETED)
                {
                    processingobject->AllDone = true;
                }
                return;
            }
            
            GUARD_LOCK_OTHER(world);

            // Stop if out of bounds //
            if(num < 0 || num >= static_cast<int>(world->Objects.size())){

                goto taskstopprocessingobjectsforinitialsynclabel;
            }

            // Get the object //
            auto tosend = world->Objects[num];

            // Skip if shouldn't send //
            try{

                auto& position = world->GetComponent<Position>(tosend);

                if(!world->ShouldPlayerReceiveObject(position, connection.get())){

                    goto exitcurrentiterationchecklabel;
                }
                
            } catch(const NotFound&){

                // No position, should probably always send //
            }


            // Send it //
            // TODO: could check for errors here
            // TODO: make sure that this will also send constraints
            world->SendObjectToConnection(guard, tosend, connection);

            // Sent, check the exit things //
            goto exitcurrentiterationchecklabel;
            
        }, safeconnection, connectobject, this), Objects.size()));

    // Task that will remove the added InitiallySyncingPlayers entry once done
    ThreadingManager::Get()->QueueTask(new ConditionalTask(std::bind<void>([](
                    std::shared_ptr<PlayerConnectionPreparer> processingobject, GameWorld* world)
                -> void
        {
            // It is done //
            Logger::Get()->Info("GameWorld: initial sync with player completed");

            if(processingobject->GameWorldCompromised)
                return;
            
            // Remove it //
            GUARD_LOCK_OTHER(world);

            auto end = world->InitiallySyncingPlayers.end();
            for(auto iter = world->InitiallySyncingPlayers.begin(); iter != end; ++iter){

                if((*iter).get() == processingobject.get()){
                    world->InitiallySyncingPlayers.erase(iter);
                    return;
                }

            }

            // Might want to report this as an error //
            

        }, connectobject, this), std::bind<bool>([](
                std::shared_ptr<PlayerConnectionPreparer> processingobject) -> bool
            {
                return processingobject->AllDone;

            }, connectobject)));
}

DLLEXPORT void Leviathan::GameWorld::_OnNotifiableDisconnected(Lock &guard,
    BaseNotifiableAll* parenttoremove, Lock &parentlock)
{

	auto plyptr = static_cast<ConnectedPlayer*>(parenttoremove);

	// Destroy the update object containing this player and cancel all current packets //
    for(size_t i = 0; i < ReceivingPlayers.size(); i++){

        if(ReceivingPlayers[i] == plyptr){

            ReceivingPlayers.erase(ReceivingPlayers.begin()+i);
            break;
        }
    }


    auto end = InitiallySyncingPlayers.end();
    for(auto iter = InitiallySyncingPlayers.begin(); iter != end; ++iter){

        if((*iter)->Player == plyptr){
            (*iter)->GameWorldCompromised = true;

            InitiallySyncingPlayers.erase(iter);
            return;
        }
    }

    // Didn't find any //
    Logger::Get()->Warning("GameWorld: disconnected plyptr not found in list");
}

void Leviathan::GameWorld::UpdatePlayersPositionData(Lock &guard, ConnectedPlayer* ply,
    Lock &plylock)
{
	// Get the position for this player in this world //
	auto id = ply->GetPositionInWorld(this, plylock);

    // Player is using a static position at (0, 0, 0) //
    if(id == 0)
        return;
    
    try{

        auto& position = GetComponent<Position>(id);

        (void)position._Position;
        
    } catch(const NotFound&){

        // Player has invalid position //
        Logger::Get()->Warning("Player position entity has no Position component");
    }
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::SendObjectToConnection(Lock &guard, ObjectID id,
    std::shared_ptr<ConnectionInfo> connection)
{
    // First create a packet which will be the object's data //

    auto objdata = EntitySerializerManager::Get()->CreateInitialEntityMessageFor(
        this, guard, id, connection.get());
    
    if(!objdata)
        return false;

    // Then gather all sorts of other stuff to make an response //
    std::unique_ptr<NetworkResponseDataForInitialEntity> resdata(
        new NetworkResponseDataForInitialEntity(ID, objdata));
    
    // We return whatever the send function returns //
    std::shared_ptr<NetworkResponse> response = make_shared<NetworkResponse>(-1,
        PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 20);

    response->GenerateInitialEntityResponse(resdata.release());

    return connection->SendPacketToConnection(response, 5).get() ? true: false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleEntityInitialPacket(
    shared_ptr<NetworkResponse> message, NetworkResponseDataForInitialEntity* data)
{
    if(!data)
        return;
    
    GUARD_LOCK();

    InitialEntityPackets.push_back(message);
}

void GameWorld::_ApplyInitialEntityPackets(Lock &guard){

    auto serializer = EntitySerializerManager::Get();
    
    for(auto&& response : InitialEntityPackets){

        // Data cannot be NULL here //
        NetworkResponseDataForInitialEntity* data = response->GetResponseDataForInitialEntity();
        
        // Handle all the entities in the packet //
        auto end = data->EntityData.end();
        for(auto iter = data->EntityData.begin(); iter != end; ++iter){

            ObjectID id = 0;
        
            serializer->CreateEntityFromInitialMessage(this, guard, id,
                *(*iter).get());

            if(id < 1){

                Logger::Get()->Error("GameWorld: handle initial packet: failed to create entity");
            }
        }
    }
    
    InitialEntityPackets.clear();
}

DLLEXPORT void Leviathan::GameWorld::HandleEntityUpdatePacket(shared_ptr<NetworkResponse> message,
    NetworkResponseDataForEntityUpdate* data)
{
    if(!data)
        return;
    
    GUARD_LOCK();

    EntityUpdatePackets.push_back(message);
}

void GameWorld::_ApplyEntityUpdatePackets(Lock &guard){

    auto serializer = EntitySerializerManager::Get();
    
    for(auto&& response : EntityUpdatePackets){

        // Data cannot be NULL here //
        NetworkResponseDataForEntityUpdate* data = response->GetResponseDataForEntityUpdate();
        
        // Just check if the entity is created/exists //
        for(auto iter = Objects.begin(); iter != Objects.end(); ++iter){

            if((*iter) == data->EntityID){

                // Apply the update //
                if(!serializer->ApplyUpdateMessage(this, guard, data->EntityID,
                        *data->UpdateData, data->TickNumber, data->ReferenceTick))
                {

                    Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): applying update "
                        "to entity "+Convert::ToString(data->EntityID)+" failed");
                }
            
                return;
            }
        }

        // It hasn't been created yet //
        Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): has no entity "+
            Convert::ToString(data->EntityID)+", ignoring an update packet");
    }
    
    EntityUpdatePackets.clear();
}

DLLEXPORT void GameWorld::ApplyExistingEntityUpdates(Lock &guard){

    if(!EntityUpdatePackets.empty())
        return;

    auto serializer = EntitySerializerManager::Get();
    
    for(auto iter = EntityUpdatePackets.begin(); iter != EntityUpdatePackets.end(); ){

        // Data cannot be NULL here //
        NetworkResponseDataForEntityUpdate* data = (*iter)->GetResponseDataForEntityUpdate();

        bool applied = false;
        
        // Just check if the entity is created/exists //
        for(auto iter = Objects.begin(); iter != Objects.end(); ++iter){

            if((*iter) == data->EntityID){

                // Apply the update //
                if(!serializer->ApplyUpdateMessage(this, guard, data->EntityID,
                        *data->UpdateData, data->TickNumber, data->ReferenceTick))
                {

                    Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): applying update "
                        "to entity "+Convert::ToString(data->EntityID)+" failed");
                }

                applied = true;
                break;
            }
        }

        if(!applied){
            
            ++iter;
            
        } else {

            iter = EntityUpdatePackets.erase(iter);
        }
    }
    
}

DLLEXPORT void GameWorld::HandleConstraintPacket(shared_ptr<NetworkResponse> message,
    NetworkResponseDataForEntityConstraint* data)
{
    if(!data)
        return;
    
    GUARD_LOCK();

    ConstraintPackets.push_back(message);
}

void GameWorld::_ApplyConstraintPackets(Lock &guard){

    for(auto&& response : ConstraintPackets){

        NetworkResponseDataForEntityConstraint* data =
            response->GetResponseDataForEntityConstraint();

        // Find the entities //
        try{
            
            auto& first = GetComponent<Constraintable>(data->EntityID1);
            auto& second = GetComponent<Constraintable>(data->EntityID2);

            if(!ConstraintSerializerManager::Get()->CreateConstraint(first, second,
                    data->Type, *data->ConstraintData, data->Create))
            {
                Logger::Get()->Error("GameWorld: apply constraint: failed to create constraint");
            }
            
        } catch(const NotFound&){

            // TODO: move to waiting constraints
            continue;
        }
    }

    ConstraintPackets.clear();
}
// ------------------------------------ //
DLLEXPORT void GameWorld::ApplyQueuedPackets(Lock &guard){

    if(!InitialEntityPackets.empty())
        _ApplyInitialEntityPackets(guard);

    if(!EntityUpdatePackets.empty())
        _ApplyEntityUpdatePackets(guard);
    
    if(!ConstraintPackets.empty())
        _ApplyConstraintPackets(guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleClockSyncPacket(RequestWorldClockSyncData* data){

    GUARD_LOCK_NAME(lockit);

    Logger::Get()->Info("GameWorld: adjusting our clock: Absolute: "+
        Convert::ToString(data->Absolute)+", tick: "+Convert::ToString(data->Ticks)+
        ", enginems: "+Convert::ToString(data->EngineMSTweak));
    
    // Change our TickNumber to match //
    lockit.unlock();

    Engine::Get()->_AdjustTickNumber(data->Ticks, data->Absolute);
        
    if(data->EngineMSTweak)
        Engine::Get()->_AdjustTickClock(data->EngineMSTweak, data->Absolute);
    
    lockit.lock();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleWorldFrozenPacket(NetworkResponseDataForWorldFrozen* data){

    GUARD_LOCK();

    Logger::Get()->Info("GameWorld("+Convert::ToString(ID)+"): frozen state updated, now: "+
        Convert::ToString<int>(data->Frozen)+", tick: "+Convert::ToString(data->TickNumber)+
        " (our tick:"+Convert::ToString(TickNumber)+")");

    if(data->TickNumber > TickNumber){

        Logger::Get()->Write("TODO: freeze the world in the future");
    }
    
    // Set the state //
    SetWorldPhysicsFrozenState(guard, data->Frozen);

    // Simulate ticks if required //
    if(!data->Frozen){

        // Check how many ticks we are behind and simulate that number of physical updates //
        int tickstosimulate = TickNumber-data->TickNumber;

        if(tickstosimulate > 0){

            Logger::Get()->Info("GameWorld: unfreezing and simulating "+
                Convert::ToString(tickstosimulate*TICKSPEED)+
                " ms worth of more physical updates");

            _PhysicalWorld->AdjustClock(tickstosimulate*TICKSPEED);
        }
        
        
    } else {

        // Snap objects back //
        Logger::Get()->Info("TODO: world freeze snap things back a bit");
    }
}
// ------------------ RayCastHitEntity ------------------ //
DLLEXPORT Leviathan::RayCastHitEntity::RayCastHitEntity(const NewtonBody* ptr /*= NULL*/, const float &tvar,
    RayCastData* ownerptr) : HitEntity(ptr), HitVariable(tvar)
{
	if(ownerptr){
		HitLocation = ownerptr->BaseHitLocationCalcVar*HitVariable;
	} else {
		HitLocation = (Float3)0;
	}
}

DLLEXPORT bool Leviathan::RayCastHitEntity::HasHit(){
	return HitEntity != NULL;
}

DLLEXPORT bool Leviathan::RayCastHitEntity::DoesBodyMatchThisHit(NewtonBody* other){
	return HitEntity == other;
}

DLLEXPORT Float3 Leviathan::RayCastHitEntity::GetPosition(){
	return HitLocation;
}

DLLEXPORT RayCastHitEntity& Leviathan::RayCastHitEntity::operator=(const RayCastHitEntity& other){
	HitEntity = other.HitEntity;
	HitVariable = other.HitVariable;
	HitLocation = other.HitLocation;

	return *this;
}
// ------------------ RayCastData ------------------ //
DLLEXPORT Leviathan::RayCastData::RayCastData(int maxcount, const Float3 &from, const Float3 &to) : MaxCount(maxcount), 
	// Formula based on helpful guy on Newton forums
	BaseHitLocationCalcVar(from+(to-from))
{
	// Reserve memory for maximum number of objects //
	HitEntities.reserve(maxcount);
}

DLLEXPORT Leviathan::RayCastData::~RayCastData(){
	// We want to release all hit data //
	SAFE_RELEASE_VECTOR(HitEntities);
}


#undef ADDCOMPONENTFUNCTIONSTOGAMEWORLD
#define ADDCOMPONENTFUNCTIONSTOGAMEWORLD(type, holder, destroyfunc) template<> type& \
    GameWorld::GetComponent<type>(ObjectID id){                         \
                                                                        \
        auto component = holder.Find(id);                               \
        if(!component)                                                  \
            throw NotFound("Component for entity with id was not found"); \
                                                                        \
        return *component;                                              \
    }                                                                   \
                                                                        \
    template<> bool GameWorld::RemoveComponent<type>(ObjectID id){ \
        try{                                                            \
            holder.destroyfunc(id);                                     \
            return true;                                                \
                                                                        \
        } catch(...){                                                   \
                                                                        \
            return false;                                               \
        }                                                               \
    }

ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Position, ComponentPosition, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(RenderNode, ComponentRenderNode, QueueDestroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Sendable, ComponentSendable, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Physics, ComponentPhysics, QueueDestroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(BoxGeometry, ComponentBoxGeometry, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Model, ComponentModel, QueueDestroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(TrackController, ComponentTrackController, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Parent, ComponentParent, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Parentable, ComponentParentable, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(PositionMarkerOwner, ComponentPositionMarkerOwner,
    QueueDestroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Received, ComponentReceived, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Constraintable, ComponentConstraintable, Destroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Trail, ComponentTrail, QueueDestroy);
ADDCOMPONENTFUNCTIONSTOGAMEWORLD(ManualObject, ComponentManualObject, QueueDestroy);
