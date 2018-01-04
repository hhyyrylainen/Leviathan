// ------------------------------------ //
#include "GameWorld.h"

#include "Common/DataStoring/NamedVars.h"
#include "Rendering/GraphicalInputEntity.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"
#include "Networking/Connection.h"
#include "Networking/NetworkHandler.h"
#include "Threading/ThreadingManager.h"
#include "Serializers/EntitySerializer.h"
#include "Entities/Objects/Constraints.h"
#include "Engine.h"
#include "Newton/PhysicsMaterialManager.h"
#include "../Handlers/IDFactory.h"
#include "../Window.h"

// Camera interpolation
#include "StateInterpolator.h"
#include "Generated/ComponentStates.h"

#include "Sound/SoundDevice.h"

#include "Newton/NewtonManager.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreLight.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "OgreRenderWindow.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorManager2.h"

#include "Exceptions.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameWorld::GameWorld() :
    ID(IDFactory::GetID())
{

}

DLLEXPORT Leviathan::GameWorld::~GameWorld(){

    (*WorldDestroyed) = true;

    // Assert if all objects haven't been released already.
    // We can't call virtual methods here anymore
    LEVIATHAN_ASSERT(Entities.empty(),
        "GameWorld: Entities not empty in destructor. Was Release called?");
    
    // This should be relatively cheap if the newton threads don't deadlock while waiting
    // for each other
    _PhysicalWorld.reset();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::Init(NETWORKED_TYPE type, GraphicalInputEntity* renderto,
    Ogre::Root* ogre)
{
    IsOnServer = (type == NETWORKED_TYPE::Server);
    
    LinkedToWindow = renderto;

    // Detecting non-GUI mode //
    if(ogre){
        
        if(!renderto)
            return false;
        
        GraphicalMode = true;
        // these are always required for worlds //
        _CreateOgreResources(ogre, renderto);
    }

    // Acquire physics engine world //
    // This should not be required if it isn't available
    if(NewtonManager::Get()){
        
        _PhysicalWorld = NewtonManager::Get()->CreateWorld(this);
    }

    _DoSystemsInit();
    return true;
}

DLLEXPORT void Leviathan::GameWorld::Release(){

    _DoSystemsRelease();

    (*WorldDestroyed) = true;
    
    ReceivingPlayers.clear();
    
    // release objects //
    // TODO: allow objects to know that they are about to get killed

    // As all objects are just pointers to components we can just dump the objects
    // and once the component pools are released
    ClearEntities();
    
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
    
}
// ------------------------------------ //
void Leviathan::GameWorld::_CreateOgreResources(Ogre::Root* ogre,
    GraphicalInputEntity* rendertarget)
{
    // create scene manager //
    WorldsScene = ogre->createSceneManager(Ogre::ST_EXTERIOR_FAR, 2,
        Ogre::INSTANCING_CULLING_THREADED,
        "MainSceneManager_"+Convert::ToString(ID));

    // WorldsScene->setShadowFarDistance(1000.f);
    // WorldsScene->setShadowDirectionalLightExtrusionDistance(10000.f);

    // create camera //
    WorldSceneCamera = WorldsScene->createCamera("Camera01");

    // WorldSceneCamera->lookAt( Ogre::Vector3( 0, 0, 0 ) );
    
    // near and far clipping planes //
    WorldSceneCamera->setFOVy(Ogre::Degree(60));
    WorldSceneCamera->setNearClipDistance(0.1f);
    WorldSceneCamera->setFarClipDistance(50000.f);

    // enable infinite far clip distance if supported //
    if(ogre->getRenderSystem()->getCapabilities()->hasCapability(
            Ogre::RSC_INFINITE_FAR_PLANE))
    {
        
        WorldSceneCamera->setFarClipDistance(0);
    }

    // set scene ambient colour //
    // TODO: update this to the PBR HLMS system
    // WorldsScene->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f),
    //     Ogre::ColourValue(0.2f, 0.2f, 0.2f), Ogre::Vector3(0.1f, 1.f, 0.085f));

    // // default sun //
    SetSunlight();
    
    // Create the workspace for this scene //
    // Which will be rendered before the overlay workspace but after potential
    // clearing workspace
    WorldWorkspace = ogre->getCompositorManager2()->addWorkspace(WorldsScene,
        rendertarget->GetOgreWindow(), 
        WorldSceneCamera, "WorldsWorkspace", true, 0);

}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SetSkyBox(const std::string &materialname){
    try{
        WorldsScene->setSkyBox(true, materialname);
    }
    catch(const Ogre::InvalidParametersException &e){

        Logger::Get()->Error("[EXCEPTION] "+e.getFullDescription());
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetSkyPlane(const std::string &material, const Ogre::Plane &plane
    /*= Ogre::Plane(1, 1, 1, 1)*/)
{
    WorldsScene->setSkyPlane(true, plane, material);
}

DLLEXPORT void GameWorld::DisableSkyPlane(){

    WorldsScene->setSkyPlaneEnabled(false);
}
// ------------------------------------ //
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

DLLEXPORT void Leviathan::GameWorld::Render(int mspassed, int tick, int timeintick){
    
    RunFrameRenderSystems(tick, timeintick);
    
    // Read camera entity and update position //

    // Skip if no camera //
    if(CameraEntity == 0)
        return;

    try{
        Camera& properties = GetComponent<Camera>(CameraEntity);

        Position& position = GetComponent<Position>(CameraEntity);

        auto& states = GetStatesFor<Position>();
        
        // set camera position //
        const auto interpolated =
            StateInterpolator::Interpolate(states, CameraEntity, &position, tick, timeintick);

        if(!std::get<0>(interpolated)){

            // No interpolated pos //
            WorldSceneCamera->setPosition(position.Members._Position); 
            WorldSceneCamera->setOrientation(position.Members._Orientation); 
            
        } else {

            const auto& interpolatedPos = std::get<1>(interpolated);
            WorldSceneCamera->setPosition(interpolatedPos._Position); 
            WorldSceneCamera->setOrientation(interpolatedPos._Orientation); 
        }

        if(properties.SoundPerceiver){
            
            SoundDevice::Get()->SetSoundListenerPosition(position.Members._Position,
                position.Members._Orientation);   
        }

        if(properties.Marked || AppliedCameraPropertiesPtr != &properties){

            AppliedCameraPropertiesPtr = &properties;

            WorldSceneCamera->setFOVy(Ogre::Degree(properties.FOVY));
            
            properties.Marked = false;
        }

    } catch(const Exception &e){

        LOG_ERROR("GameWorld: Render: camera update failed. Was a component removed?, "
            "exception:");
        e.PrintToLog();
        CameraEntity = 0;
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetCamera(ObjectID object){

    CameraEntity = object;
    
    AppliedCameraPropertiesPtr = nullptr;

    if(CameraEntity == 0)
        return;

    // Check components //
    try{
        GetComponent<Camera>(object);
    } catch(const NotFound&){

        throw InvalidArgument("SetCamera object is missing a needed component (Camera)");
    }

    try{
        GetComponent<Position>(object);
    } catch(const NotFound&){

        throw InvalidArgument("SetCamera object is missing a needed component (Position)");
    }    
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::ShouldPlayerReceiveEntity(Position &atposition,
    Connection &connection)
{

    return true;
}

DLLEXPORT bool GameWorld::IsConnectionInWorld(Connection &connection) const {

    for(auto& player : ReceivingPlayers){

        if(player->GetConnection().get() == &connection){

            return true;
        }
    }
    
    return false;
}

DLLEXPORT void GameWorld::SetPlayerReceiveWorld(std::shared_ptr<ConnectedPlayer> ply){

    // Skip if already added //
    for(auto& player : ReceivingPlayers){

        if(player == ply){

            return;
        }
    }

    Logger::Get()->Info("GameWorld: player(\"" + ply->GetNickname() +
        "\") is now receiving world");

    // Add them to the list of receiving players //
    ReceivingPlayers.push_back(ply);
    
    if(!ply->GetConnection()->IsValidForSend()){

        // The closing should be handled by somebody else
        Logger::Get()->Error("GameWorld: requested to sync with a player who has closed their "
            "connection");
        
        return;
    }

    // Update the position data //
    UpdatePlayersPositionData(*ply);

    // Start sending initial update //
    Logger::Get()->Info("Starting to send "+Convert::ToString(Entities.size())+" to player");
    
    // Now we can queue all objects for sending //
    // TODO: make sure that all objects are sent
    // TODO: redo this inside the world tick
    // ThreadingManager::Get()->QueueTask(
    //     new RepeatCountedTask(std::bind<void>([](
    //                 std::shared_ptr<Connection> connection,
    //                 std::shared_ptr<ConnectedPlayer> processingobject, GameWorld* world,
    //                 std::shared_ptr<bool> WorldInvalid)
    //             -> void
    //     {
    //         // Get the next object //
    //         RepeatCountedTask* task =
    //             dynamic_cast<RepeatCountedTask*>(TaskThread::GetThreadSpecificThreadObject()->
    //             QuickTaskAccess.get());

    //         LEVIATHAN_ASSERT(task, "wrong type passed to our task");

    //         size_t num = task->GetRepeatCount();

    //         if(*WorldInvalid){

    //         taskstopprocessingobjectsforinitialsynclabel:
                
    //             // Stop processing //
    //             task->StopRepeating();
    //             return;
    //         }
            
    //         // Stop if out of bounds //
    //         if(num >= world->Entities.size()){

    //             goto taskstopprocessingobjectsforinitialsynclabel;
    //         }

    //         // Get the object //
    //         auto tosend = world->Entities[num];

    //         // Skip if shouldn't send //
    //         try{

    //             auto& position = world->GetComponent<Position>(tosend);

    //             if(!world->ShouldPlayerReceiveObject(position, *connection)){

    //                 return;
    //             }
                
    //         } catch(const NotFound&){

    //             // No position, should probably always send //
    //         }


    //         // Send it //
    //         world->SendObjectToConnection(guard, tosend, connection);

    //         return;
            
    //     }, ply->GetConnection(), ply, this, WorldDestroyed), Entities.size()));
}

DLLEXPORT void GameWorld::SendToAllPlayers(const std::shared_ptr<NetworkResponse> &response,
    RECEIVE_GUARANTEE guarantee) const
{
    // Notify everybody that an entity has been destroyed //
    for(auto iter = ReceivingPlayers.begin(); iter != ReceivingPlayers.end(); ++iter){

        auto safe = (*iter)->GetConnection();

        if(!safe->IsValidForSend()){
            // Player has probably closed their connection //
            continue;
        }
        
        safe->SendPacketToConnection(response, guarantee);
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::Tick(int currenttick){

    TickNumber = currenttick;

    // Apply queued packets //
    ApplyQueuedPackets();

    _HandleDelayedDelete();

    // All required nodes for entities are created //
    HandleAdded();
    ClearAdded();

    // Remove closed player connections //

    for(auto iter = ReceivingPlayers.begin(); iter != ReceivingPlayers.end(); ++iter){

        if(!(*iter)->GetConnection()->IsValidForSend()){

            DEBUG_BREAK;
            
        } else {

            ++iter;
        }
    }

    // Simulate physics //
    if(!WorldFrozen){
        
        if(IsOnServer){
    
            _ApplyEntityUpdatePackets();
            _PhysicalWorld->SimulateWorldFixed(TICKSPEED, 2);
 
        } else {

            // Simulate direct control //
        
        }
    }

    _RunTickSystems();
    
    // Sendable objects may need something to be done //

    if(IsOnServer){

        // Notify new entities //
        //DEBUG_BREAK;

        // Skip if not tick that will be stored //
        // if(TickNumber % WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL == 0){

        //     _SendableSystem.Run(ComponentSendable.GetIndex(), *this);
        // }
        
    } else {

        // TODO: direct control objects
        // _ReceivedSystem.Run(ComponentReceived.GetIndex(), *this);
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::HandleAdded(){

}

DLLEXPORT void GameWorld::ClearAdded(){
    
}
// ------------------------------------ //
DLLEXPORT void GameWorld::RunFrameRenderSystems(int tick, int timeintick){

    _ApplyEntityUpdatePackets();
}

DLLEXPORT void GameWorld::_RunTickSystems(){

    
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GameWorld::GetTickNumber() const{
    
    return TickNumber;
}

DLLEXPORT float Leviathan::GameWorld::GetTickProgress() const {

    float progress = Engine::Get()->GetTimeSinceLastTick() / (float)TICKSPEED;

    if (progress < 0.f)
        return 0.f;

    return progress < 1.f ? progress : 1.f;
}

DLLEXPORT std::tuple<int, int> GameWorld::GetTickAndTime() const{

    int tick = TickNumber;
    int timeSince = static_cast<int>(Engine::Get()->GetTimeSinceLastTick());

    while(timeSince >= TICKSPEED){

        ++tick;
        timeSince -= TICKSPEED;
    }

    return std::make_tuple(tick, timeSince);
}

// ------------------ Object managing ------------------ //
DLLEXPORT ObjectID GameWorld::CreateEntity(){

    auto id = static_cast<ObjectID>(IDFactory::GetID());

    Entities.push_back(id);

    return id;
}

DLLEXPORT void GameWorld::NotifyEntityCreate(ObjectID id){

    if(IsOnServer){

        // This is at least a decent place to send them,
        Sendable* issendable = nullptr;
        
        try{
            issendable = &GetComponent<Sendable>(id);

        } catch(const NotFound&){

            // Not sendable no point in continuing //
            return;
        }

        LEVIATHAN_ASSERT(issendable, "GetComponent didn't throw");

        auto end = ReceivingPlayers.end();
        for(auto iter = ReceivingPlayers.begin(); iter != end; ++iter){

            auto safe = (*iter)->GetConnection();

            if(!safe->IsValidForSend()){
                // Player has probably closed their connection //
                continue;
            }

            // TODO: pass issendable here to avoid an extra lookup
            if(!SendEntityToConnection(id, safe)){

                Logger::Get()->Warning("GameWorld: CreateEntity: failed to send "
                    "object to player (" + (*iter)->GetNickname() + ")");
                
                continue;
            }
        }
        
    } else {

        // Clients register received objects here //
        Entities.push_back(id);
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::ClearEntities(){

    // release objects //
    // TODO: allow objects to do something
    Entities.clear();
    
    // Clear all nodes //
    _ResetSystems();

    // Runs Release on components that need it
    _ReleaseAllComponents();

    // Clear all components //
    _ResetComponents();

    // Notify everybody that all entities are discarded //
    for(auto iter = ReceivingPlayers.begin(); iter != ReceivingPlayers.end(); ++iter){

        auto safe = (*iter)->GetConnection();

        if(!safe->IsValidForSend()){
            // Player has probably closed their connection //
            continue;
        }

        Logger::Get()->Write("TODO: send world clear message");
        DEBUG_BREAK;
    }
}
// ------------------------------------ //
DLLEXPORT Float3 Leviathan::GameWorld::GetGravityAtPosition(const Float3 &pos){
    // \todo take position into account //
    // create force without mass applied //
    return Float3(0.f, PHYSICS_BASE_GRAVITY, 0.f);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::GameWorld::GetPhysicalMaterial(const std::string &name){

    return PhysicsMaterialManager::Get()->GetMaterialIDForWorld(name,
        _PhysicalWorld->GetNewtonWorld());
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::DestroyEntity(ObjectID id){

    auto end = Entities.end();
    for(auto iter = Entities.begin(); iter != end; ++iter){
        
        if(*iter == id){

            _DoDestroy(id);
            Entities.erase(iter);
            
            return;
        }
    }
}

DLLEXPORT void Leviathan::GameWorld::QueueDestroyEntity(ObjectID id) {

    Lock lock(DeleteMutex);
    DelayedDeleteIDS.push_back(id);
}

void Leviathan::GameWorld::_HandleDelayedDelete(){

    // We might want to delete everything //
    if(ClearAllEntities){

        ClearEntities();

        ClearAllEntities = false;

        Lock lock(DeleteMutex);
        DelayedDeleteIDS.clear();
        
        // All are now cleared //
        return;
    }

    Lock lock(DeleteMutex);
    
    // Return right away if no objects to delete //
    if(DelayedDeleteIDS.empty())
        return;

    // Search all objects and find the ones that need to be deleted //
    for(auto iter = Entities.begin(); iter != Entities.end(); ){

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

            _DoDestroy(curid);
            iter = Entities.erase(iter);
            
            // Check for end //
            if(DelayedDeleteIDS.empty())
                return;

        } else {
            ++iter;
        }
    }
}

void GameWorld::_DoDestroy(ObjectID id){

    Logger::Get()->Info("GameWorld destroying object " + Convert::ToString(id));

    if(IsOnServer)
        _ReportEntityDestruction(id);

    // TODO: find a better way to do this
    DestroyAllIn(id);
}

DLLEXPORT void GameWorld::DestroyAllIn(ObjectID id){
    
}

DLLEXPORT std::tuple<void*, bool> GameWorld::GetComponent(ObjectID id, COMPONENT_TYPE type){

    return std::make_tuple(nullptr, false);
}

DLLEXPORT std::tuple<void*, bool> GameWorld::GetStatesFor(COMPONENT_TYPE type){

    return std::make_tuple(nullptr, false);
}

DLLEXPORT void GameWorld::_ReleaseAllComponents(){

    
}

DLLEXPORT void GameWorld::_DoSystemsInit(){

}

DLLEXPORT void GameWorld::_DoSystemsRelease(){

}
// ------------------------------------ //
void Leviathan::GameWorld::_ReportEntityDestruction(ObjectID id){

    SendToAllPlayers(std::make_shared<ResponseEntityDestruction>(0, this->ID, id),
        RECEIVE_GUARANTEE::Critical);
}

// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::SetWorldPhysicsFrozenState(bool frozen){
    // Skip if set to the same //
    if(frozen == WorldFrozen)
        return;

    WorldFrozen = frozen;

    // Send it to receiving players (if we are a server) //
    if(ReceivingPlayers.empty())
        return;

    // Should be safe to create the packet now and send it to all the connections //
    SendToAllPlayers(std::make_shared<ResponseWorldFrozen>(0, ID, WorldFrozen, TickNumber),
        RECEIVE_GUARANTEE::Critical);
}

DLLEXPORT RayCastHitEntity* Leviathan::GameWorld::CastRayGetFirstHit(const Float3 &from,
    const Float3 &to)
{
    // Create a data object for the ray cast //
    RayCastData data(1, from, to);

    // Call the actual ray firing function //
    NewtonWorldRayCast(_PhysicalWorld->GetNewtonWorld(), &from.X, &to.X,
        RayCallbackDataCallbackClosest, &data, NULL, 0);

    // Check the result //
    if(data.HitEntities.size() == 0){
        // Nothing hit //
        return new RayCastHitEntity();
    }
    
    // We need to increase reference count to not to accidentally delete the result while
    // caller is using it
    data.HitEntities[0]->AddRef();
    
    // Return the only hit //
    return data.HitEntities[0];
}

// \todo improve this performance //
dFloat Leviathan::GameWorld::RayCallbackDataCallbackClosest(const NewtonBody* const body,
    const NewtonCollision* const shapeHit, const dFloat* const hitContact,
    const dFloat* const hitNormal, dLong collisionID, void* const userData,
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
    ClearAllEntities = true;
}
// ------------------------------------ //
void Leviathan::GameWorld::UpdatePlayersPositionData(ConnectedPlayer &ply)
{
    // Get the position for this player in this world //
    ObjectID id = ply.GetPositionInWorld(this);

    // Player is using a static position at (0, 0, 0) //
    if(id == 0)
        return;
    
    try{

        auto& position = GetComponent<Position>(id);

        (void)position.Members._Position;
        
    } catch(const NotFound&){

        // Player has invalid position //
        Logger::Get()->Warning("Player position entity has no Position component");
    }
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameWorld::SendEntityToConnection(ObjectID id,
    std::shared_ptr<Connection> connection)
{
    // First create a packet which will be the object's data //

    sf::Packet packet;

    DEBUG_BREAK;
    // try {
    //     if (!Engine::Get()->GetEntitySerializer()->CreatePacketForConnection(this, id,
    //         GetComponent<Sendable>(id), packet, *connection))
    //     {
    //         return false;
    //     }
    // }
    // catch (const NotFound&) {
    //     return false;
    // }

    // Then gather all sorts of other stuff to make an response //
    return connection->SendPacketToConnection(std::make_shared<ResponseEntityCreation>(
            0, id, std::move(packet)), RECEIVE_GUARANTEE::Critical).get() ? true: false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleEntityInitialPacket(
    std::shared_ptr<NetworkResponse> message, ResponseEntityCreation* data)
{
    if(!data)
        return;

    InitialEntityPackets.push_back(message);
}

void GameWorld::_ApplyInitialEntityPackets(){

    auto serializer = Engine::Get()->GetEntitySerializer();
    
    for(auto& response : InitialEntityPackets){

        LEVIATHAN_ASSERT(response->GetType() == NETWORK_RESPONSE_TYPE::EntityCreation,
            "invalid type in InitialEntityPackets");
        
        ObjectID id = 0;

        DEBUG_BREAK;
        // serializer->DeserializeWholeEntityFromPacket(this, id,
        //     static_cast<ResponseEntityCreation*>(response.get())->InitialEntity);

        if(id < 1){

            Logger::Get()->Error("GameWorld: handle initial packet: failed to create entity");
        }
    }
    
    InitialEntityPackets.clear();
}

DLLEXPORT void Leviathan::GameWorld::HandleEntityUpdatePacket(
    std::shared_ptr<NetworkResponse> message)
{
    if(message->GetType() == NETWORK_RESPONSE_TYPE::EntityUpdate)
        return;
    
    EntityUpdatePackets.push_back(message);
}

void GameWorld::_ApplyEntityUpdatePackets(){

    auto serializer = Engine::Get()->GetEntitySerializer();
    
    for(auto& response : EntityUpdatePackets){

        // Data cannot be NULL here //
        ResponseEntityUpdate* data = static_cast<ResponseEntityUpdate*>(response.get());

        bool found = false;
        
        // Just check if the entity is created/exists //
        for(auto iter = Entities.begin(); iter != Entities.end(); ++iter){

            if((*iter) == data->EntityID){

                found = true;

                // Apply the update //
                DEBUG_BREAK;
                // if(!serializer->ApplyUpdateFromPacket(this, guard, data->EntityID,
                //         data->TickNumber, data->ReferenceTick, data->UpdateData))
                // {
                //     Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): "
                //         "applying update to entity " + 
                //         Convert::ToString(data->EntityID) + " failed");
                // }
            
                break;
            }
        }

        if(!found){
            // It hasn't been created yet //
            Logger::Get()->Warning("GameWorld("+Convert::ToString(ID)+"): has no entity "+
                Convert::ToString(data->EntityID)+", ignoring an update packet");
        }
    }
    
    EntityUpdatePackets.clear();
}
// ------------------------------------ //
DLLEXPORT void GameWorld::ApplyQueuedPackets(){

    if(!InitialEntityPackets.empty())
        _ApplyInitialEntityPackets();

    if(!EntityUpdatePackets.empty())
        _ApplyEntityUpdatePackets();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleClockSyncPacket(RequestWorldClockSync* data){

    Logger::Get()->Info("GameWorld: adjusting our clock: Absolute: " +
        Convert::ToString(data->Absolute)+", tick: " + Convert::ToString(data->Ticks) +
        ", engine ms: "+Convert::ToString(data->EngineMSTweak));
    
    // Change our TickNumber to match //
    Engine::Get()->_AdjustTickNumber(data->Ticks, data->Absolute);
        
    if(data->EngineMSTweak)
        Engine::Get()->_AdjustTickClock(data->EngineMSTweak, data->Absolute);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameWorld::HandleWorldFrozenPacket(ResponseWorldFrozen* data){

    Logger::Get()->Info("GameWorld("+Convert::ToString(ID)+"): frozen state updated, now: "+
        Convert::ToString<int>(data->Frozen)+", tick: "+Convert::ToString(data->TickNumber)+
        " (our tick:"+Convert::ToString(TickNumber)+")");

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

DLLEXPORT void Leviathan::GameWorld::_OnComponentDestroyed(ObjectID id, COMPONENT_TYPE type) {

    if (type > COMPONENT_TYPE::Custom) {

        // Handle custom components //
        _OnCustomComponentDestroyed(id, type);
        return;
    }

    switch (type)
    {
    case Leviathan::COMPONENT_TYPE::Position:
        break;
    case Leviathan::COMPONENT_TYPE::RenderNode:
        break;
    case Leviathan::COMPONENT_TYPE::Sendable:
        break;
    case Leviathan::COMPONENT_TYPE::Received:
        break;
    case Leviathan::COMPONENT_TYPE::Physics:
        break;
    case Leviathan::COMPONENT_TYPE::BoxGeometry:
        break;
    case Leviathan::COMPONENT_TYPE::Model:
        break;
    case Leviathan::COMPONENT_TYPE::ManualObject:
        break;
    case Leviathan::COMPONENT_TYPE::Custom:
    {
        // This is not allowed
        DEBUG_BREAK;
    }
    break;
    default:
        DEBUG_BREAK;
        break;
    }
}

DLLEXPORT void Leviathan::GameWorld::_OnCustomComponentDestroyed(ObjectID id, 
    COMPONENT_TYPE type) 
{
    DEBUG_BREAK;
}
// ------------------ RayCastHitEntity ------------------ //
DLLEXPORT Leviathan::RayCastHitEntity::RayCastHitEntity(const NewtonBody* ptr /*= NULL*/,
    const float &tvar, RayCastData* ownerptr) : HitEntity(ptr), HitVariable(tvar)
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

DLLEXPORT RayCastHitEntity& Leviathan::RayCastHitEntity::operator=(
    const RayCastHitEntity& other)
{
    HitEntity = other.HitEntity;
    HitVariable = other.HitVariable;
    HitLocation = other.HitLocation;

    return *this;
}
// ------------------ RayCastData ------------------ //
DLLEXPORT Leviathan::RayCastData::RayCastData(int maxcount, const Float3 &from,
    const Float3 &to) :
    MaxCount(maxcount), 
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

