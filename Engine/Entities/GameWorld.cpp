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
#include "Bases/BasePositionable.h"
#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkHandler.h"
#include "Threading/ThreadingManager.h"
#include "Handlers/EntitySerializerManager.h"
#include "Handlers/ConstraintSerializerManager.h"
#include "Entities/Objects/Constraints.h"
#include "Entities/Bases/BaseConstraintable.h"
#include "Entities/Bases/BaseSendableEntity.h"
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
        Pinging(PING_STATE_NONE), GameWorldCompromised(false), ObjectsReady(false), AllDone(false),
        Player(ply), World(world), Connection(connection), PingTime(0)
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

	// acquire physics engine world //
	_PhysicalWorld = NewtonManager::Get()->CreateWorld(this);

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
    // Objects should release their data in their destructors //
	for(size_t i = 0; i < Objects.size(); i++){

        // Check if the world will be destroyed //
		if(Objects[i]->GetRefCount() != 1){

            Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): entity("+
                Convert::ToString(Objects[i]->GetID())+
                ") has external references on world release");

            // Abandon the object //
            Objects[i]->Disown();
        }
	}

    // This will release our references and delete all objects that have no references
	Objects.clear();
    
    SendableObjects.clear();

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
    
    // Report waiting constraints //
    Lock lock(WaitingConstraintsMutex);
    if(!WaitingConstraints.empty()){

        Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): has "+Convert::ToString(
                WaitingConstraints.size())+" constraints waiting for entities");
        WaitingConstraints.clear();
    }
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
DLLEXPORT bool Leviathan::GameWorld::ShouldPlayerReceiveObject(BaseObject* obj, ConnectionInfo* connectionptr){

    return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SimulatePhysics(){
    GUARD_LOCK();

    if(IsOnServer){
    
        if(!WorldFrozen){

            // Let's set the maximum runs to 10000 to disable completely deadlocking
            _PhysicalWorld->SimulateWorld(10000);
        }
    } else {

        // Simulate direct control //
        
    }
    
}

DLLEXPORT void Leviathan::GameWorld::ClearTimers(){
    
    _PhysicalWorld->ClearTimers();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::Tick(int currenttick){

    GUARD_LOCK_NAME(lockit);

    TickNumber = currenttick;

    _HandleDelayedDelete(lockit);

    SimulatePhysics();

    // Sendable objects may need something to be done //
    auto nethandler = NetworkHandler::Get();

    // TODO: the vectors could be sorted to improve branch predictor performance
    if(nethandler && nethandler->GetNetworkType() == NETWORKED_TYPE_SERVER){

        // Skip if not tick that will be stored //
        if(TickNumber % WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL != 0)
            goto worldskiphandlingsendableobjectslabel;

        
        for(size_t i = 0; i < SendableObjects.size(); ++i){
            
            auto target = SendableObjects[i];

            if(target->IsAnyDataUpdated)
                target->SendUpdatesToAllClients(TickNumber);
            
        }
        
    } else if(nethandler && nethandler->GetNetworkType() == NETWORKED_TYPE_CLIENT){
#ifndef NETWORK_USE_SNAPSHOTS
        for(size_t i = 0; i < SendableObjects.size(); ++i){
            
            auto target = SendableObjects[i];

            if(target->IsAnyDataUpdated)
                target->StoreClientSideState(TickNumber);
            
        }
#else
        // TODO: put here to code to snap back objects that shouldn't have moved
        
        
#endif //NETWORK_USE_SNAPSHOTS
    }

worldskiphandlingsendableobjectslabel:

    return;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GameWorld::GetTickNumber() const{
    
    return TickNumber;
}
// ------------------ Object managing ------------------ //
DLLEXPORT void Leviathan::GameWorld::AddObject(BaseObject* obj){
	AddObject(BaseObject::MakeShared(obj));
}

DLLEXPORT void Leviathan::GameWorld::AddObject(ObjectPtr obj){

    if(!obj)
        return;

    {
        GUARD_LOCK();
        Objects.push_back(obj);

        // Check is it a sendable object //
        BaseSendableEntity* sendable = dynamic_cast<BaseSendableEntity*>(obj.get());

        if(sendable){

            SendableObjects.push_back(sendable);
        }
    }

    // Check for constraints //
    Lock lock(WaitingConstraintsMutex);
    if(WaitingConstraints.empty())
        return;

    int objid = obj->GetID();
    
    // Not sure if this is necessary //
    // TODO: move this to the tick function?
    size_t amount = WaitingConstraints.size();
    WaitingConstraint* first = &*WaitingConstraints.begin();
    for(size_t i = 0; i < amount; ){

        WaitingConstraint* current = (first+i);
        if(current->Entity1 == objid || current->Entity2 == objid){

            GUARD_LOCK();
            // Try to apply it //
            if(_TryApplyConstraint(guard, current->Packet->GetResponseDataForEntityConstraint())){
                
                WaitingConstraints.erase(WaitingConstraints.begin()+i);
                --amount;
                continue;
            }
        }

        i++;
    }
}

DLLEXPORT void Leviathan::GameWorld::CreateEntity(ObjectPtr obj){

    // Notify everybody that a new entity has been created //
    {
        GUARD_LOCK();

        // This is at least a decent place to send them, any constraints created later will get send
        // when they are created

        auto end = ReceivingPlayers.end();
        for(auto iter = ReceivingPlayers.begin(); iter != end; ++iter){

            auto unsafe = (*iter)->GetConnection();

            auto safe = NetworkHandler::Get()->GetSafePointerToConnection(unsafe);

            if(!safe){
                // Player has probably closed their connection //
                continue;
            }

            if(!SendObjectToConnection(obj, safe)){

                Logger::Get()->Warning("GameWorld: CreateEntity: failed to send object to player "
                    "("+(*iter)->GetNickname()+")");
                
                continue;
            }
        }
    }
    
    // And finally register it //
    AddObject(obj);
}
// ------------------------------------ //
DLLEXPORT ObjectPtr Leviathan::GameWorld::GetWorldObject(int ID){
	// ID shouldn't be under zero //
	if(ID == -1){

		Logger::Get()->Warning("GameWorld: GetWorldObject: trying to find object with ID == -1 "
            "(IDs shouldn't be negative)");
		return NULL;
	}

	GUARD_LOCK();

	auto end = Objects.end();
	for(auto iter = Objects.begin(); iter != end; ++iter){
		if((*iter)->GetID() == ID){
			return *iter;
		}
	}

	return NULL;
}
// ------------------------------------ //
DLLEXPORT ObjectPtr Leviathan::GameWorld::GetSmartPointerForObject(BaseObject* rawptr) const{
	GUARD_LOCK();

	auto end = Objects.end();
	for(auto iter = Objects.begin(); iter != end; ++iter){
		if(iter->get() == rawptr){
			return *iter;
		}
	}

	return NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::ClearObjects(Lock &guard){
	VerifyLock(guard);

    // release objects //
    // Objects should release their data in their destructors //
	for(size_t i = 0; i < Objects.size(); i++){

        // Check if the world will be destroyed //
		if(Objects[i]->GetRefCount() != 1){

            Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): entity("+
                Convert::ToString(Objects[i]->GetID())+
                ") has external references on world clear");

            // Abandon the object //
            Objects[i]->Disown();
        }
	}

    // This will release our references and delete all objects that have no references
	Objects.clear();
    
    SendableObjects.clear();

    guard.unlock();
    
    // Throw away the waiting constraints //
    {
        Lock lock(WaitingConstraintsMutex);
        WaitingConstraints.clear();
    }

    guard.lock();

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
void Leviathan::GameWorld::_EraseFromSendable(BaseSendableEntity* obj, Lock &guard){

    for(size_t i = 0; i < SendableObjects.size(); i++){

        if(SendableObjects[i] == obj){

            SendableObjects.erase(SendableObjects.begin()+i);
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::DestroyObject(int EntityID){
	GUARD_LOCK_NAME(lockit);

    Logger::Get()->Info("GameWorld destroying object "+Convert::ToString(EntityID));
    
    auto end = Objects.end();
	for(auto iter = Objects.begin(); iter != end; ++iter){
        
		if((*iter)->GetID() == EntityID){

            _ReportEntityDestruction(EntityID, lockit);

            // Also erase from sendable //
            _EraseFromSendable(dynamic_cast<BaseSendableEntity*>(iter->get()), lockit);
            
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

		ClearObjects();

		ClearAllObjects = false;
		// All are now cleared //
		return;
	}

    guard.unlock();

    Lock lock(DeleteMutex);
    
	// Return right away if no objects to delete //
	if(DelayedDeleteIDS.size() == 0)
		return;

	// Search all objects and find the ones that need to be deleted //
	for(auto iter = Objects.begin(); iter != Objects.end(); ){

		// Check does id match any //
		int curid = (*iter)->GetID();
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

            _ReportEntityDestruction(curid, guard);
            
            // Also erase from sendable //
            _EraseFromSendable(dynamic_cast<BaseSendableEntity*>((*iter).get()), guard);
            
            guard.lock();
            

			iter = Objects.erase(iter);
            
			// Check for end //
			if(DelayedDeleteIDS.size() == 0)
                return;

            guard.unlock();

		} else {
			++iter;
		}
	}
    
    guard.lock();
}

void Leviathan::GameWorld::_ReportEntityDestruction(int id, Lock &guard){

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
            NetworkResponseDataForEntityDestruction(ID, id));

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
DLLEXPORT void Leviathan::GameWorld::SetWorldPhysicsFrozenState(bool frozen){
	// Skip if set to the same //
	if(frozen == WorldFrozen)
		return;

	GUARD_LOCK();

	WorldFrozen = frozen;

    if(!WorldFrozen){

        // Reset timers //
        ClearTimers();
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
            if(num < 0 || num >= world->Objects.size()){

                goto taskstopprocessingobjectsforinitialsynclabel;
            }

            // Get the object //
            auto tosend = world->Objects[num];

            Logger::Get()->Info("Sending object, number: "+Convert::ToString(num));

            // Skip if shouldn't send //
            if(!world->ShouldPlayerReceiveObject(tosend.get(), connection.get())){

                goto exitcurrentiterationchecklabel;
            }


            // Send it //
            // TODO: could check for errors here
            world->SendObjectToConnection(tosend, connection);

            // Send all the constraints, TODO: this could be improved a lot //
            BaseConstraintable* constraintable = dynamic_cast<BaseConstraintable*>(tosend.get());

            if(constraintable){

                GUARD_LOCK_OTHER(constraintable);

                size_t count = constraintable->GetConstraintCount(guard);

                Logger::Get()->Info("Sending object's constraints, total: "+
                    Convert::ToString(count));

                for(size_t i = 0; i < count; i++){

                    // This should ignore NULL pointers so this should send all
                    // constraints just fine
                    world->SendConstraintToConnection(constraintable->GetConstraint(guard, i),
                        connection.get());
                }
            }


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
	BasePositionable* positionobj = ply->GetPositionInWorld(this, plylock);

	if(!positionobj){

		// Player is using a static position at (0, 0, 0) //
notusingapositionlabel:

		return;
	}

	// Store the pointer to the object //


	// To prevent us from deleting the pointer fetch a smart pointer that matches the object //
	BaseObject* bobject = dynamic_cast<BaseObject*>(positionobj);

	// Find the object //
	auto safeptr = GetSmartPointerForObject(bobject);

	if(!safeptr){

		// We were given an invalid world //
		Logger::Get()->Error("GameWorld: UpdatePlayersPositionData: could not find a matching "
            " object for player position in this world, wrong world?");
        
		goto notusingapositionlabel;
	}

	// Link the position to the thing //


}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::SendObjectToConnection(ObjectPtr obj,
    std::shared_ptr<ConnectionInfo> connection)
{
    // Fail if the obj is not a valid pointer //
    if(!obj)
        return false;
    
    // First create a packet which will be the object's data //
    GUARD_LOCK_OTHER(obj.get());

    auto objdata = EntitySerializerManager::Get()->CreateInitialEntityMessageFor(obj.get(), connection.get());

    if(!objdata)
        return false;

    // Then gather all sorts of other stuff to make an response //
    std::unique_ptr<NetworkResponseDataForInitialEntity> resdata(new NetworkResponseDataForInitialEntity(ID, objdata));
    
    

    // We return whatever the send function returns //
    std::shared_ptr<NetworkResponse> response = make_shared<NetworkResponse>(-1,
        PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 20);

    response->GenerateInitialEntityResponse(resdata.release());

    return connection->SendPacketToConnection(response, 5).get() ? true: false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SendConstraintToConnection(shared_ptr<Entity::BaseConstraint> constraint,
    ConnectionInfo* connectionptr)
{
    if(!constraint)
        return;

    GUARD_LOCK_OTHER(constraint);

    auto custompacketdata = ConstraintSerializerManager::Get()->SerializeConstraintData(constraint.get());

    if(!custompacketdata){

        Logger::Get()->Warning("GameWorld: failed to send constraint, type: "+Convert::ToString(
                static_cast<int>(constraint->GetType())));
        return;
    }
    
    // Gather all the other info //
    int obj1 = constraint->GetFirstEntity()->GetID();

    // The second object might be NULL so make sure not to segfault here //
    auto obj2ptr = constraint->GetSecondEntity();

    int obj2 = obj2ptr ? obj2ptr->GetID(): -1;

    auto packet = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000);
    
    // Wrap everything up and send //
    packet->GenerateEntityConstraintResponse(new NetworkResponseDataForEntityConstraint(ID,
            obj1, obj2, true, constraint->GetType(), custompacketdata));

    connectionptr->SendPacketToConnection(packet, 12);
    Logger::Get()->Info("Sent constraint");
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::HandleEntityInitialPacket(NetworkResponseDataForInitialEntity* data){

    bool somesucceeded = false;
    
    // Handle all the entities in the packet //
    auto end = data->EntityData.end();
    for(auto iter = data->EntityData.begin(); iter != end; ++iter){

        BaseObject* returnptr = NULL;
        
        EntitySerializerManager::Get()->CreateEntityFromInitialMessage(&returnptr, *(*iter).get(), this);

        if(!returnptr){

            Logger::Get()->Error("GameWorld: handle initial packet: failed to create entity");
            continue;
        }

        // Add the entity //
        AddObject(returnptr);

        somesucceeded = true;
    }

    return somesucceeded;
}

DLLEXPORT void Leviathan::GameWorld::HandleConstraintPacket(NetworkResponseDataForEntityConstraint* data,
    std::shared_ptr<NetworkResponse> packet)
{
    GUARD_LOCK();
    
    if(_TryApplyConstraint(guard, data)){

        // It is now applied //
        return;
    }

    guard.unlock();
    
    // Add it to the queue //
    Lock lock(WaitingConstraintsMutex);
    WaitingConstraints.push_back(WaitingConstraint(data->EntityID1, data->EntityID2, packet));
}

DLLEXPORT void Leviathan::GameWorld::HandleEntityUpdatePacket(NetworkResponseDataForEntityUpdate* data){

    // Get the target entity //
    auto target = GetWorldObject(data->EntityID);

    if(!target){

        Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): has no entity "+Convert::ToString(data->EntityID)+
            ", ignoring an update packet");
        return;
    }

    // Apply the update //
    // The object may not be locked as it might want to resimulate //
    
    if(!EntitySerializerManager::Get()->ApplyUpdateMessage(*data->UpdateData, data->TickNumber, data->ReferenceTick,
            target))
    {

        Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): applying update to entity "+
            Convert::ToString(data->EntityID)+" failed");
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleClockSyncPacket(RequestWorldClockSyncData* data){

    GUARD_LOCK_NAME(lockit);

    Logger::Get()->Info("GameWorld: adjusting our clock: Absolute: "+Convert::ToString(data->Absolute)+", tick: "+
        Convert::ToString(data->Ticks)+", enginems: "+Convert::ToString(data->EngineMSTweak));
    
    // Change our TickNumber to match //
    if(data->Absolute){

        TickNumber = data->Ticks;

        lockit.unlock();
        
        if(data->EngineMSTweak)
            Engine::Get()->_AdjustTickClock(data->EngineMSTweak, data->Absolute);
        
        lockit.lock();
        
    } else {

        TickNumber += data->Ticks;

        lockit.unlock();
        
        if(data->EngineMSTweak)
            Engine::Get()->_AdjustTickClock(data->EngineMSTweak, data->Absolute);

        lockit.lock();
    }

    // Only notify if the tick actually changed
    if(data->Ticks)
        Logger::Get()->Info("GameWorld("+Convert::ToString(ID)+"): world clock adjusted, tick is now: "+
            Convert::ToString(TickNumber));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleWorldFrozenPacket(NetworkResponseDataForWorldFrozen* data){

    GUARD_LOCK();

    Logger::Get()->Info("GameWorld("+Convert::ToString(ID)+"): frozen state updated, now: "+
        Convert::ToString<int>(data->Frozen)+", tick: "+Convert::ToString(data->TickNumber)+" (our tick:"+
        Convert::ToString(TickNumber)+")");

    if(data->TickNumber > TickNumber){

        Logger::Get()->Write("TODO: freeze the world in the future");
    }
    
    // Set the state //
    SetWorldPhysicsFrozenState(data->Frozen);

    // Simulate ticks if required //
    if(!data->Frozen){

        // Check how many ticks we are behind and simulate that number of physical updates //
        int tickstosimulate = TickNumber-data->TickNumber;

        if(tickstosimulate > 0){

            Logger::Get()->Info("GameWorld: unfreezing and simulating "+Convert::ToString(tickstosimulate*TICKSPEED)+
                " ms worth of more physical updates");

            _PhysicalWorld->AdjustClock(tickstosimulate*TICKSPEED);
        }
        
        
    } else {

        // Snap objects back //
        Logger::Get()->Info("TODO: world freeze snap things back a bit");
    }
}
// ------------------------------------ //
bool Leviathan::GameWorld::_TryApplyConstraint(Lock &guard,
    NetworkResponseDataForEntityConstraint* data)
{

    // Find the objects //
    BaseObject* first;
    BaseObject* second;
    first = second = NULL;

    bool found = false;
    
    // The constraint might only need the first entity //
    bool findsecond = data->EntityID2 >= 0 ? true: false;

    auto end = Objects.end();
    for(auto iter = Objects.begin(); iter != end; ++iter){

        int objid = (*iter)->GetID();
        if(!first && objid == data->EntityID1){
            
            first = (*iter).get();

            // If the second is found or only one entity is needed we are done //
            if(second || !findsecond){

                found = true;
                break;
            }
            
        } else if(findsecond && !second && objid == data->EntityID2){

            second = (*iter).get();

            // If the first is found we are done //
            if(first){

                found = true;
                break;
            }
        }
    }

    if(!found)
        return false;

    guard.unlock();
    
    // Apply the constraint //
    ConstraintSerializerManager::Get()->CreateConstraint(first, second, data->Type,
        *data->ConstraintData.get(), data->Create);

    guard.lock();

    return true;
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
