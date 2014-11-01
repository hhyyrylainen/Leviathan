#include "Include.h"
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
#include "Threading/ThreadingManager.h"
#include "Handlers/EntitySerializerManager.h"
using namespace Leviathan;
// ------------------------------------ //

// TODO: remember to delete these
#include "Entities/Objects/Brush.h"
#include "Utility/Random.h"

//! \brief Class used by _OnNotifiableConnected to hold temporary connection data
class Leviathan::PlayerConnectionPreparer{
public:

    //! Used to keep track of what's happening regards to the ping
    enum PING_STATE {PING_STATE_STARTED, PING_STATE_FAILED, PING_STATE_NONE, PING_STATE_COMPLETED};
    
    PlayerConnectionPreparer(ConnectedPlayer* ply) :
        Pinging(PING_STATE_NONE), GameWorldCompromised(false), ObjectsReady(false), AllDone(false),
        Player(ply)
    {


    }

    //! Called as an callback when pinging completes
    void OnPingCompleted(int msping, int lostpackets){

        Pinging = PING_STATE_COMPLETED;

        boost::unique_lock<boost::mutex> lock(_Mutex);
        if(ObjectsReady)
            AllDone = true;
    }

    //! Called when pinging fails
    void OnPingFailed(CONNECTION_PING_FAIL_REASON failreason, int lostpackets){

        DEBUG_BREAK;
    }

    //! Set by GameWorld if it isn't safe to use
    bool GameWorldCompromised;

    // This is needed to avoid rare cases where AllDone would never be set
    boost::mutex _Mutex;

    //! The player who is being handled, used only for deleting
    //! The pointer is unsafe to use
    ConnectedPlayer* Player;

    
    PING_STATE Pinging;
    bool ObjectsReady;
    bool AllDone;
};

// ------------------ GameWorld ------------------ //
DLLEXPORT Leviathan::GameWorld::GameWorld() :
    WorldSceneCamera(NULL), WorldsScene(NULL), Sunlight(NULL), SunLightNode(NULL), WorldFrozen(false),
    GraphicalMode(false), LinkedToWindow(NULL), WorldWorkspace(NULL), ClearAllObjects(false),
    ID(IDFactory::GetID())
{

}

DLLEXPORT Leviathan::GameWorld::~GameWorld(){

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
	GUARD_LOCK_THIS_OBJECT();

    // Tell initially syncing players that we are no longer valid //
    auto end = InitiallySyncingPlayers.end();
    for(auto iter = InitiallySyncingPlayers.begin(); iter != end; ++iter){

        (*iter)->GameWorldCompromised = true;
    }

    InitiallySyncingPlayers.clear();
    
	// release objects //
	for(size_t i = 0; i < Objects.size(); i++){

		Objects[i]->ReleaseData();
	}

	Objects.clear();

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

	// some smart ptrs need releasing //
	_PhysicalWorld.reset();

}
// ------------------------------------ //
void Leviathan::GameWorld::_CreateOgreResources(Ogre::Root* ogre, Window* rendertarget){
	// create scene manager //
	WorldsScene = ogre->createSceneManager(Ogre::ST_EXTERIOR_FAR, 2, Ogre::INSTANCING_CULLING_THREADED,
        "MainSceneManager");

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
	WorldWorkspace = ogre->getCompositorManager2()->addWorkspace(WorldsScene, rendertarget->GetOgreWindow(), 
		WorldSceneCamera, "WorldsWorkspace", true, 0);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SetSkyBox(const string &materialname){
	try{
		WorldsScene->setSkyBox(true, materialname);
	}
	catch(const Ogre::InvalidParametersException &e){

		Logger::Get()->Error(L"[EXCEPTION] "+Convert::StringToWstring(e.getFullDescription()));
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

DLLEXPORT void Leviathan::GameWorld::UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos, ObjectLock &guard){
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
// ------------------ Object managing ------------------ //
DLLEXPORT void Leviathan::GameWorld::AddObject(BaseObject* obj){
	AddObject(shared_ptr<BaseObject>(obj, SharedPtrReleaseDeleter<BaseObject>::DoRelease));
}

DLLEXPORT void Leviathan::GameWorld::AddObject(shared_ptr<BaseObject> obj){
	GUARD_LOCK_THIS_OBJECT();
	Objects.push_back(obj);
}

DLLEXPORT shared_ptr<BaseObject> Leviathan::GameWorld::GetWorldObject(int ID){
	// ID shouldn't be under zero //
	if(ID == -1){

		Logger::Get()->Warning(L"GameWorld: GetWorldObject: trying to find object with ID == -1 "
            L"(IDs shouldn't be negative)");
		return NULL;
	}

	GUARD_LOCK_THIS_OBJECT();

	auto end = Objects.end();
	for(auto iter = Objects.begin(); iter != end; ++iter){
		if((*iter)->GetID() == ID){
			return *iter;
		}
	}

	return NULL;
}
// ------------------------------------ //
DLLEXPORT shared_ptr<BaseObject> Leviathan::GameWorld::GetSmartPointerForObject(BaseObject* rawptr) const{
	GUARD_LOCK_THIS_OBJECT();

	auto end = Objects.end();
	for(auto iter = Objects.begin(); iter != end; ++iter){
		if(iter->get() == rawptr){
			return *iter;
		}
	}

	return NULL;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::ClearObjects(ObjectLock &guard){
	VerifyLock(guard);

	for(std::vector<shared_ptr<BaseObject>>::iterator iter = Objects.begin(); iter != Objects.end(); ++iter){
		// Release the object //
		(*iter)->ReleaseData();
	}
	// Release our reference //
	Objects.clear();
}
// ------------------------------------ //
DLLEXPORT Float3 Leviathan::GameWorld::GetGravityAtPosition(const Float3 &pos){
	// \todo take position into account //
	// create force without mass applied //
	return Float3(0.f, PHYSICS_BASE_GRAVITY, 0.f);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SimulateWorld(){
	GUARD_LOCK_THIS_OBJECT();
	// This is probably the best place to remove dead objects //
	_HandleDelayedDelete(guard);

	// Only if not frozen run physics //
	if(!WorldFrozen)
		_PhysicalWorld->SimulateWorld();
}

DLLEXPORT void Leviathan::GameWorld::ClearSimulatePassedTime(){
	_PhysicalWorld->ClearTimers();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::DestroyObject(int ID){
	GUARD_LOCK_THIS_OBJECT();
	for(std::vector<shared_ptr<BaseObject>>::iterator iter = Objects.begin(); iter != Objects.end(); ++iter){
		if((*iter)->GetID() == ID){
			// release the object and then erase our reference //
			(*iter)->ReleaseData();
			Objects.erase(iter);
			return;
		}
	}
}

DLLEXPORT void Leviathan::GameWorld::QueueDestroyObject(int ID){
	GUARD_LOCK_THIS_OBJECT();
	DelayedDeleteIDS.push_back(ID);
}

void Leviathan::GameWorld::_HandleDelayedDelete(ObjectLock &guard){

	VerifyLock(guard);

	// We might want to delete everything //
	if(ClearAllObjects){


		ClearObjects();

		ClearAllObjects = false;
		// All are now cleared //
		return;
	}

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

			(*iter)->ReleaseData();
			iter = Objects.erase(iter);
			// Check for end //
			if(DelayedDeleteIDS.size() == 0)
				return;

		} else {
			++iter;
		}
	}

}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SetWorldPhysicsFrozenState(bool frozen){
	// Skip if set to the same //
	if(frozen == WorldFrozen)
		return;

	GUARD_LOCK_THIS_OBJECT();

	WorldFrozen = frozen;
	// If unfrozen reset physics time //
	if(!WorldFrozen){
		if(_PhysicalWorld)
			_PhysicalWorld->ClearTimers();
	}
}

DLLEXPORT RayCastHitEntity* Leviathan::GameWorld::CastRayGetFirstHit(const Float3 &from, const Float3 &to,
    ObjectLock &guard)
{
	VerifyLock(guard);
	// Create a data object for the ray cast //
	RayCastData data(1, from, to);

	// Call the actual ray firing function //
	NewtonWorldRayCast(_PhysicalWorld->GetNewtonWorld(), &from.X, &to.X, RayCallbackDataCallbackClosest, &data, NULL, 0);

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

DLLEXPORT RayCastHitEntity* Leviathan::GameWorld::CastRayGetFirstHitProxy(Float3 from, Float3 to){
	return CastRayGetFirstHit(from, to);
}

DLLEXPORT void Leviathan::GameWorld::MarkForClear(){
	ClearAllObjects = true;
}

bool Leviathan::GameWorld::AreAllPlayersSynced() const{
    // The vector of sending players is empty if not sending //
    GUARD_LOCK_THIS_OBJECT();
    return InitiallySyncingPlayers.size() == 0;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::_OnNotifiableConnected(BaseNotifiableAll* parentadded){

	// The connected object will always have to be a ConnectedPlayer
	auto plyptr = static_cast<ConnectedPlayer*>(parentadded);

    Logger::Get()->Info("GameWorld: player(\""+plyptr->GetNickname()+"\") is now receiving world");

	// Create an entry for this player //
    shared_ptr<PlayerConnectionPreparer> connectobject(new PlayerConnectionPreparer(plyptr));


    // We need a safe pointer to the connection //
    ConnectionInfo* unsafeconnection = plyptr->GetConnection();


    auto safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(unsafeconnection);

    if(!safeconnection){

        // The closing should be handled by somebody else
        Logger::Get()->Error(L"GameWorld: requested to sync with a player who has closed their connection");
        return;
    }


    // Start pinging //
    connectobject->Pinging = PlayerConnectionPreparer::PING_STATE_STARTED;

    safeconnection->CalculateNetworkPing(WORLD_CLOCK_SYNC_PACKETS, WORLD_CLOCK_SYNC_ALLOW_FAILS,
        boost::bind(&PlayerConnectionPreparer::OnPingCompleted, connectobject, _1, _2),
        boost::bind(&PlayerConnectionPreparer::OnPingFailed, connectobject, _1, _2));


	// This lock is only required for this one call (we are always locked before this call) //
	{
		// Update the position data //
		GUARD_LOCK_THIS_OBJECT();
		UpdatePlayersPositionData(plyptr, guard);

        // Add it to the list //
        InitiallySyncingPlayers.push_back(connectobject);
	}


	// Start sending initial update //

    Logger::Get()->Info("Starting to send "+Convert::ToString(Objects.size())+" to player");
    
    // Now we can queue all objects for sending //
    // TODO: make sure that all objects are sent
    ThreadingManager::Get()->QueueTask(new RepeatCountedTask(boost::bind<void>([](shared_ptr<ConnectionInfo>
                    connection, shared_ptr<PlayerConnectionPreparer> processingobject, GameWorld* world) -> void
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
                boost::unique_lock<boost::mutex> lock(processingobject->_Mutex);
                
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
            
            GUARD_LOCK_OTHER_OBJECT(world);

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


            // Sent, check the exit things //
            goto exitcurrentiterationchecklabel;
            
        }, safeconnection, connectobject, this), Objects.size()));

    // Task that will remove the added InitiallySyncingPlayers entry once done
    ThreadingManager::Get()->QueueTask(new ConditionalTask(boost::bind<void>([](shared_ptr<PlayerConnectionPreparer>
                    processingobject, GameWorld* world) -> void
        {
            // It is done //
            Logger::Get()->Info("GameWorld: initial sync with player completed");

            if(processingobject->GameWorldCompromised)
                return;
            
            // Remove it //
            GUARD_LOCK_OTHER_OBJECT(world);

            auto end = world->InitiallySyncingPlayers.end();
            for(auto iter = world->InitiallySyncingPlayers.begin(); iter != end; ++iter){

                if((*iter).get() == processingobject.get()){
                    world->InitiallySyncingPlayers.erase(iter);
                    return;
                }

            }

            // Might want to report this as an error //
            

        }, connectobject, this), boost::bind<bool>([](shared_ptr<PlayerConnectionPreparer> processingobject) -> bool
            {
                return processingobject->AllDone;

            }, connectobject)));
}

DLLEXPORT void Leviathan::GameWorld::_OnNotifiableDisconnected(BaseNotifiableAll* parenttoremove){

	auto plyptr = static_cast<ConnectedPlayer*>(parenttoremove);

	// Destroy the update object containing this player and cancel all current packets //
    GUARD_LOCK_THIS_OBJECT();

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

void Leviathan::GameWorld::UpdatePlayersPositionData(ConnectedPlayer* ply, ObjectLock &guard){
	VerifyLock(guard);
	// Get the position for this player in this world //

	GUARD_LOCK_OTHER_OBJECT_NAME(ply, guard2);
	BasePositionable* positionobj = ply->GetPositionInWorld(this, guard2);

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
		Logger::Get()->Error(L"GameWorld: UpdatePlayersPositionData: could not find a matching object for "
            L"player position in this world, wrong world?");
		goto notusingapositionlabel;
	}

	// Link the position to the thing //


}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::SendObjectToConnection(shared_ptr<BaseObject> obj,
    shared_ptr<ConnectionInfo> connection)
{
    // Fail if the obj is not a valid pointer //
    if(!obj)
        return false;
    
    // First create a packet which will be the object's data //
    GUARD_LOCK_OTHER_OBJECT(obj.get());

    auto objdata = EntitySerializerManager::Get()->CreateInitialEntityMessageFor(obj.get(), connection.get());

    if(!objdata)
        return false;

    // Then gather all sorts of other stuff to make an response //
    unique_ptr<NetworkResponseDataForInitialEntity> resdata(new NetworkResponseDataForInitialEntity(ID, objdata));
    
    

    // We return whatever the send function returns //
    shared_ptr<NetworkResponse> response = make_shared<NetworkResponse>(-1,
        PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 20);

    response->GenerateInitialEntityResponse(resdata.release());

    return connection->SendPacketToConnection(response, 5).get() ? true: false;
}


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
