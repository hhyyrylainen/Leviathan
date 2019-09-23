// ------------------------------------ //
#include "GameWorld.h"

#include "Common/DataStoring/NamedVars.h"
#include "Components.h"
#include "Engine.h"
#include "Handlers/IDFactory.h"
#include "Networking/Connection.h"
#include "Networking/NetworkHandler.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkServerInterface.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Physics/PhysicalWorld.h"
#include "Physics/PhysicsMaterialManager.h"
#include "Rendering/Graphics.h"
#include "Script/ScriptConversionHelpers.h"
#include "Script/ScriptExecutor.h"
#include "ScriptComponentHolder.h"
#include "ScriptSystemWrapper.h"
#include "Sound/SoundDevice.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"

// Camera interpolation
#include "Generated/ComponentStates.h"
#include "StateInterpolator.h"

#include "Exceptions.h"

#include "bsfCore/Components/BsCCamera.h"
#include "bsfCore/Components/BsCLight.h"
#include "bsfCore/Components/BsCSkybox.h"
#include "bsfCore/Scene/BsSceneObject.h"

using namespace Leviathan;
// ------------------------------------ //

// // Ray callbacks //
// static dFloat RayCallbackDataCallbackClosest(const NewtonBody* const body,
//     const NewtonCollision* const shapeHit, const dFloat* const hitContact,
//     const dFloat* const hitNormal, dLong collisionID, void* const userData,
//     dFloat intersectParam);

// ------------------------------------ //
class GameWorld::Implementation {
public:
    ~Implementation()
    {
        // Check that there are no external references to these
        for(const auto& tuple : RegisteredScriptComponents) {

            if(tuple.second->GetRefCount() != 1) {

                LOG_FATAL("GameWorld: ImplRelease: RegisteredScriptComponent (holder): has "
                          "external refs, count: " +
                          std::to_string(tuple.second->GetRefCount()));
            }
        }
    }

    std::map<std::string, ScriptComponentHolder::pointer> RegisteredScriptComponents;
    std::map<std::string, std::unique_ptr<ScriptSystemWrapper>> RegisteredScriptSystems;

    //! Queued entity updates. Contains the time it was received in order to throw out old ones
    std::vector<std::tuple<WantedClockType::time_point, ResponseEntityUpdate>>
        QueuedEntityUpdates;

    // BSF rendering resources
    bs::HSceneObject WorldCameraSO;
    bs::HCamera WorldCamera;

    bs::HSceneObject SunlightSO;
    bs::HLight Sunlight;
    bs::HSceneObject SkyboxSO;
    bs::HSkybox Skybox;

    //! A temporary solution around no multiple scenes in BSF
    static int LayerNumber;
};

int GameWorld::Implementation::LayerNumber = 0;

// ------------------------------------ //
DLLEXPORT GameWorld::GameWorld(int32_t worldtype,
    const std::shared_ptr<PhysicsMaterialManager>& physicsMaterials, int worldid /*= -1*/) :
    pimpl(std::make_unique<Implementation>()),
    PhysicsMaterials(physicsMaterials), ID(worldid >= 0 ? worldid : IDFactory::GetID()),
    WorldType(worldtype)
{}

DLLEXPORT GameWorld::~GameWorld()
{
    // (*WorldDestroyed) = true;

    // Assert if all objects haven't been released already.
    // We can't call virtual methods here anymore
    // This can be hit in tests quite easily if something throws an exception
    LEVIATHAN_ASSERT(
        Entities.empty(), "GameWorld: Entities not empty in destructor. Was Release called?");
}
// ------------------------------------ //
DLLEXPORT bool GameWorld::Init(const WorldNetworkSettings& network, Graphics* graphics)
{
    NetworkSettings = network;

    // Detecting non-GUI mode //
    if(graphics) {

        GraphicalMode = true;
        // these are always required for worlds //
        _CreateRenderingResources(graphics);
    } else {

        GraphicalMode = false;
        InBackground = false;
    }

    // Acquire physics engine world only if we have been given physical materials indicating
    // that physics is wanted
    if(PhysicsMaterials) {

        _PhysicalWorld = std::make_shared<PhysicalWorld>(this, PhysicsMaterials.get());
    }

    _DoSystemsInit();
    return true;
}

DLLEXPORT void GameWorld::Release()
{
    _DoSystemsRelease();

    // (*WorldDestroyed) = true;

    ReceivingPlayers.clear();

    // release objects //
    // TODO: allow objects to know that they are about to get killed

    // As all objects are just pointers to components we can just dump the objects
    // and once the component pools are released
    ClearEntities();

    if(GraphicalMode) {
        // TODO: notify our window that it no longer has anything rendering on it
        LinkedToWindow = nullptr;

        _DestroyRenderingResources();
    }

    // This should be relatively cheap if the newton threads don't deadlock while waiting
    // for each other
    _PhysicalWorld.reset();

    // Let go of our these resources
    pimpl.reset();
}
// ------------------------------------ //
void GameWorld::_CreateRenderingResources(Graphics* graphics)
{
    // // create scene manager //
    // // Let's do what the Ogre samples do and use a bunch of threads for culling
    // const auto threads = std::max(2, static_cast<int>(std::thread::hardware_concurrency()));

    // // TODO: allow configuring scene type (the type was: Ogre::ST_EXTERIOR_FAR before)

    // Scene setup (TODO: redo once bsf has multiple scenes)
    BSFLayerHack = Implementation::LayerNumber++;

    // Camera
    pimpl->WorldCameraSO =
        bs::SceneObject::create(("Camera_World_" + std::to_string(ID)).c_str());
    pimpl->WorldCamera = pimpl->WorldCameraSO->addComponent<bs::CCamera>();
    pimpl->WorldCamera->setHorzFOV(bs::Degree(90));

    pimpl->WorldCamera->setLayers(1 << *GetScene());


    // TODO: allow changing and setting infinite
    pimpl->WorldCamera->setFarClipDistance(5000);
    // // enable infinite far clip distance if supported //
    // if(ogre->getRenderSystem()->getCapabilities()->hasCapability(
    //         Ogre::RSC_INFINITE_FAR_PLANE)) {

    //     WorldSceneCamera->setFarClipDistance(0); Maybe for bsf this needs to be float::max
    // }

    auto values = Engine::Get()->GetDefinition()->GetValues();


    // TODO: move to a better settings class and realtime updates for settings
    bool disableIndirectLighting, disableAmbientOcclusion, disableFXAA, disableShadows,
        disableLighting;

    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        values, "DisableIndirectLighting", disableIndirectLighting, false);
    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        values, "DisableAmbientOcclusion", disableAmbientOcclusion, false);
    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        values, "DisableFXAA", disableFXAA, false);
    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        values, "DisableShadows", disableShadows, false);
    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        values, "DisableLighting", disableLighting, false);

    int MSAACount;

    ObjectFileProcessor::LoadValueFromNamedVars<int>(values, "MSAACount", MSAACount, 1);


    pimpl->WorldCamera->setMSAACount(1);

    const auto& settings = pimpl->WorldCamera->getRenderSettings();

    // Needed. non default option
    if(!disableIndirectLighting) {
        settings->enableIndirectLighting = true;
    } else {
        settings->enableIndirectLighting = false;
    }

    if(disableAmbientOcclusion)
        settings->ambientOcclusion.enabled = false;

    // settings->autoExposure;
    // settings->bloom;
    // settings->bloom.enabled = false;
    // settings->colorGrading;
    // settings->depthOfField.enabled = false;
    // settings->screenSpaceLensFlare;
    // settings->screenSpaceReflections;
    // settings->shadowSettings;
    // settings->tonemapping;
    // settings->whiteBalance;
    // settings->enableAutoExposure = false;
    if(disableFXAA)
        settings->enableFXAA = false;
    // settings->enableHDR = false;
    if(disableLighting)
        settings->enableLighting = false;

    if(disableShadows)
        settings->enableShadows = false;
    // settings->enableSkybox = false;
    // settings->enableTonemapping = false;


    pimpl->WorldCamera->setRenderSettings(settings);

    // Default sun
    SetSunlight();
}

void GameWorld::_DestroyRenderingResources()
{
    if(!pimpl)
        return;

    RemoveSunlight();

    if(pimpl->WorldCameraSO) {
        pimpl->WorldCameraSO->destroy();
        pimpl->WorldCameraSO = nullptr;
        pimpl->WorldCamera = nullptr;
    }
}
// ------------------------------------ //
static bool SunCreated = false;

DLLEXPORT void GameWorld::SetSunlight()
{
    if(SunCreated) {
        LOG_WRITE("TODO: multi scene support in BSF needed for separate world lights");
        return;
    }

    SunCreated = true;

    // Create/update things if they are nullptr //
    if(!pimpl->SunlightSO) {

        pimpl->SunlightSO = bs::SceneObject::create("Sunlight");
        pimpl->Sunlight = pimpl->SunlightSO->addComponent<bs::CLight>();
        // Oh no! this method does not exist
        // pimpl->Sunlight->setLayer
    }

    // Default properties
    pimpl->Sunlight->setType(bs::LightType::Directional);

    SetLightProperties(Float3(1, 1, 1));
}

DLLEXPORT void GameWorld::RemoveSunlight()
{
    if(pimpl->SunlightSO) {
        pimpl->SunlightSO->destroy();
        pimpl->SunlightSO = nullptr;
        pimpl->Sunlight = nullptr;
        SunCreated = false;
    }
}

DLLEXPORT void GameWorld::SetSkybox(const std::string& skyboxname, float brightness /*= 1.f*/)
{
    if(!pimpl->SkyboxSO) {
        if(skyboxname.empty())
            return;

        pimpl->SkyboxSO = bs::SceneObject::create("Skybox");
        pimpl->Skybox = pimpl->SkyboxSO->addComponent<bs::CSkybox>();
        // Oh no! this method does not exist
        // pimpl->Skybox->setLayer
    }

    if(!skyboxname.empty()) {

        auto texture = Engine::Get()->GetGraphics()->LoadTextureByName(skyboxname);

        if(!texture) {

            LOG_ERROR("GameWorld: SetSkybox: could not load skybox texture with the name: " +
                      skyboxname);
            return;
        }

        pimpl->Skybox->setTexture(texture);
        pimpl->Skybox->setBrightness(brightness);

    } else {

        pimpl->Skybox->setTexture(nullptr);
        pimpl->Skybox->setBrightness(0);
    }
}

DLLEXPORT void GameWorld::SetLightProperties(const Float3& colour, float intensity,
    const Float3& direction, float sourceradius, bool castsshadows)
{
    if(!pimpl->SunlightSO) {

        LOG_ERROR("GameWorld: SetLightProperties: world doesn't have sun light set");
        return;
    }

    pimpl->Sunlight->setColor(bs::Color(colour.X, colour.Y, colour.Z));
    pimpl->Sunlight->setIntensity(intensity);
    pimpl->Sunlight->setSourceRadius(sourceradius);
    pimpl->Sunlight->setCastsShadow(castsshadows);

    // pimpl->SunlightSO->setPosition(bs::Vector3(1, 20, 1));
    // pimpl->SunlightSO->setPosition(bs::Vector3(20, 15, 20));
    pimpl->SunlightSO->setPosition(-direction);

    pimpl->SunlightSO->lookAt(bs::Vector3(0, 0, 0));
    // pimpl->SunlightSO->setWorldRotation(const Quaternion &rotation)

    // TODO: scene ambient colour

    // Set scene ambient colour //
    // TODO: Ogre samples also use this so maybe this works with PBR HLMS system
    // WorldsScene->setAmbientLight(Ogre::ColourValue(0.3f, 0.5f, 0.7f) * 0.1f * 0.75f,
    //     Ogre::ColourValue(0.6f, 0.45f, 0.3f) * 0.065f * 0.75f,
    //     -Sunlight->getDirection() + Ogre::Vector3::UNIT_Y * 0.2f);
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetAutoExposure(float mineyeadaptation, float maxeyeadaptation,
    float eyeadaptationspeeddown, float eyeadaptationspeedup, float histogramlog2max,
    float histogramlog2min, float histogrampcthigh, float histogrampctlow)
{
    const auto& settings = pimpl->WorldCamera->getRenderSettings();

    settings->autoExposure.eyeAdaptationSpeedDown = eyeadaptationspeeddown;
    settings->autoExposure.eyeAdaptationSpeedUp = eyeadaptationspeedup;
    settings->autoExposure.maxEyeAdaptation = maxeyeadaptation;
    settings->autoExposure.minEyeAdaptation = mineyeadaptation;

    settings->autoExposure.histogramLog2Max = histogramlog2max;
    settings->autoExposure.histogramLog2Min = histogramlog2min;
    settings->autoExposure.histogramPctHigh = histogrampcthigh;
    settings->autoExposure.histogramPctLow = histogrampctlow;

    settings->enableAutoExposure = false;

    pimpl->WorldCamera->setRenderSettings(settings);
}

// ------------------------------------ //
DLLEXPORT void GameWorld::Render(int mspassed, int tick, int timeintick)
{
    if(InBackground) {

        LOG_ERROR("GameWorld: Render: called while world is in the background (not attached"
                  "to a window)");
        return;
    }

    RunFrameRenderSystems(tick, timeintick);

    // Read camera entity and update position //

    // Skip if no camera //
    if(CameraEntity == 0)
        return;

    try {
        Camera& properties = GetComponent<Camera>(CameraEntity);

        Position& position = GetComponent<Position>(CameraEntity);

        auto& states = GetStatesFor<Position>();

        // set camera position //
        const auto interpolated =
            StateInterpolator::Interpolate(states, CameraEntity, &position, tick, timeintick);

        if(!std::get<0>(interpolated)) {

            // No interpolated pos //
            pimpl->WorldCameraSO->setPosition(position.Members._Position);
            pimpl->WorldCameraSO->setRotation(position.Members._Orientation);

        } else {

            const auto& interpolatedPos = std::get<1>(interpolated);
            pimpl->WorldCameraSO->setPosition(interpolatedPos._Position);
            pimpl->WorldCameraSO->setRotation(interpolatedPos._Orientation);
        }

        if(properties.SoundPerceiver) {

            Engine::Get()->GetSoundDevice()->SetSoundListenerPosition(
                position.Members._Position, position.Members._Orientation);
        }

        if(properties.Marked || AppliedCameraPropertiesPtr != &properties) {

            AppliedCameraPropertiesPtr = &properties;

            pimpl->WorldCamera->setHorzFOV(bs::Degree(properties.FOV));

            properties.Marked = false;
        }

    } catch(const Exception& e) {

        LOG_ERROR("GameWorld: Render: camera update failed. Was a component removed?, "
                  "exception:");
        e.PrintToLog();
        CameraEntity = 0;
        return;
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetCamera(ObjectID object)
{
    CameraEntity = object;

    AppliedCameraPropertiesPtr = nullptr;

    if(CameraEntity == NULL_OBJECT)
        return;

    // Check components //
    try {
        GetComponent<Camera>(object);
    } catch(const NotFound&) {

        throw InvalidArgument("SetCamera object is missing a needed component (Camera)");
    }

    try {
        GetComponent<Position>(object);
    } catch(const NotFound&) {

        throw InvalidArgument("SetCamera object is missing a needed component (Position)");
    }
}

DLLEXPORT bs::Ray GameWorld::CastRayFromCamera(int x, int y) const
{
    // Fail if there is no active camera //
    if(CameraEntity == NULL_OBJECT)
        throw InvalidState("This world has no active CameraEntity");

    if(!pimpl->WorldCamera)
        throw InvalidState("This world has no initialized camera resources");

    // Read the latest set data from the camera
    // TODO: could jump to the actual latest position here if wanted
    return pimpl->WorldCamera->screenPointToRay(bs::Vector2I(x, y));
}

DLLEXPORT bs::HSceneObject GameWorld::GetCameraSceneObject()
{
    return pimpl->WorldCameraSO;
}
// ------------------------------------ //
DLLEXPORT bool GameWorld::ShouldPlayerReceiveEntity(
    Position& atposition, Connection& connection)
{
    return true;
}

DLLEXPORT bool GameWorld::IsConnectionInWorld(Connection& connection) const
{
    for(auto& player : ReceivingPlayers) {

        if(player->GetConnection().get() == &connection) {

            return true;
        }
    }

    return false;
}

DLLEXPORT void GameWorld::SetPlayerReceiveWorld(std::shared_ptr<ConnectedPlayer> ply)
{
    // Skip if already added //
    for(auto& player : ReceivingPlayers) {

        if(player == ply) {

            return;
        }
    }

    LOG_INFO("GameWorld: player(\"" + ply->GetNickname() + "\") is now receiving world");

    // Add them to the list of receiving players //
    ReceivingPlayers.push_back(ply);

    if(!ply->GetConnection()->IsValidForSend()) {

        // The closing should be handled by somebody else
        Logger::Get()->Error("GameWorld: requested to sync with a player who has closed their "
                             "connection");

        return;
    }

    {
        // Start world receive information
        ply->GetConnection()->SendPacketToConnection(
            std::make_shared<ResponseStartWorldReceive>(0, ID, WorldType),
            RECEIVE_GUARANTEE::Critical);
    }

    // Update the position data //
    UpdatePlayersPositionData(*ply);

    // Start sending initial update //
    Logger::Get()->Info(
        "Starting to send " + Convert::ToString(Entities.size()) + " to player");

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

DLLEXPORT void GameWorld::SendToAllPlayers(
    const std::shared_ptr<NetworkResponse>& response, RECEIVE_GUARANTEE guarantee) const
{
    // Notify everybody that an entity has been destroyed //
    for(auto iter = ReceivingPlayers.begin(); iter != ReceivingPlayers.end(); ++iter) {

        auto safe = (*iter)->GetConnection();

        if(!safe->IsValidForSend()) {
            // Player has probably closed their connection //
            continue;
        }

        safe->SendPacketToConnection(response, guarantee);
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::CaptureEntityState(ObjectID id, EntityState& curstate) const {}

DLLEXPORT uint32_t GameWorld::CaptureEntityStaticState(ObjectID id, sf::Packet& receiver) const
{
    return 0;
}

DLLEXPORT void GameWorld::_CreateComponentsFromCreationMessage(
    ObjectID id, sf::Packet& data, int entriesleft, int decodedtype)
{
    if(entriesleft < 1)
        return;

    LOG_ERROR("GameWorld: entity static state decoding was not complete before calling base "
              "GameWorld implementation. Received entity won't be fully constructed");
}

DLLEXPORT void GameWorld::_CreateStatesFromUpdateMessage(
    ObjectID id, int32_t ticknumber, sf::Packet& data, int32_t referencetick, int decodedtype)
{
    LOG_ERROR(
        "GameWorld: entity component state decoding was not complete before calling base "
        "GameWorld implementation. Not all states have been decoded");
}

DLLEXPORT void GameWorld::_ApplyLocalControlUpdateMessage(
    ObjectID id, int32_t ticknumber, sf::Packet& data, int32_t referencetick, int decodedtype)
{
    LOG_ERROR("GameWorld: entity component state decoding for local control was not complete "
              "before calling base GameWorld implementation");
}
// ------------------------------------ //
DLLEXPORT void GameWorld::Tick(int currenttick)
{
    if(InBackground && !TickWhileInBackground && GraphicalMode)
        return;

    TickNumber = currenttick;

    // Apply queued packets //
    ApplyQueuedPackets();

    _HandleDelayedDelete();

    // All required nodes for entities are created //
    HandleAddedAndDeleted();
    ClearAddedAndRemoved();

    // Remove closed player connections //

    for(auto iter = ReceivingPlayers.begin(); iter != ReceivingPlayers.end();) {

        if(!(*iter)->GetConnection()->IsValidForSend()) {

            LOG_INFO("GameWorld: a player has diconnected, removing. TODO: release Sendable "
                     "memory");
            // DEBUG_BREAK;
            iter = ReceivingPlayers.erase(iter);
        } else {

            ++iter;
        }
    }

    // Set this to disallow deleting while running physics as well
    TickInProgress = true;

    // Simulate physics //
    if(!WorldFrozen) {

        // TODO: a game type that is a client and server at  the same time
        // if(IsOnServer) {

        // _ApplyEntityUpdatePackets();
        if(_PhysicalWorld)
            _PhysicalWorld->SimulateWorld(TICKSPEED / 1000.f);

        // } else {

        // Simulate direct control //
        // }
    }

    _RunTickSystems();

    TickInProgress = false;

    // Sendable objects may need something to be done //

    if(NetworkSettings.IsAuthoritative) {

        // Notify new entities //
        // DEBUG_BREAK;

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
DLLEXPORT void GameWorld::HandleAddedAndDeleted()
{
    // We are responsible for script systems //
    for(auto iter = pimpl->RegisteredScriptSystems.begin();
        iter != pimpl->RegisteredScriptSystems.end(); ++iter) {

        iter->second->CreateAndDestroyNodes();
    }
}

DLLEXPORT void GameWorld::ClearAddedAndRemoved()
{
    // We are responsible for script components //
    for(auto iter = pimpl->RegisteredScriptComponents.begin();
        iter != pimpl->RegisteredScriptComponents.end(); ++iter) {

        iter->second->ClearAdded();
        iter->second->ClearRemoved();
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::_ResetSystems()
{
    // Skip double Release
    if(!pimpl)
        return;

    // We are responsible for script systems //
    for(auto iter = pimpl->RegisteredScriptSystems.begin();
        iter != pimpl->RegisteredScriptSystems.end(); ++iter) {

        iter->second->Clear();
    }
}

DLLEXPORT void GameWorld::_ResetOrReleaseComponents()
{
    // Skip double Release
    if(!pimpl)
        return;

    // We are responsible for script components //
    for(auto iter = pimpl->RegisteredScriptComponents.begin();
        iter != pimpl->RegisteredScriptComponents.end(); ++iter) {

        iter->second->ReleaseAllComponents();
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::RunFrameRenderSystems(int tick, int timeintick)
{
    // Don't have any systems, but these updates may be important for interpolation //
    // _ApplyEntityUpdatePackets();

    // TODO: if there are any impactful simulation done here it needs to be also inside a block
    // where TickInProgress is set to true
}

DLLEXPORT void GameWorld::_RunTickSystems()
{
    // We are responsible for script systems //
    for(auto iter = pimpl->RegisteredScriptSystems.begin();
        iter != pimpl->RegisteredScriptSystems.end(); ++iter) {

        iter->second->Run();
    }
}
// ------------------------------------ //
DLLEXPORT float GameWorld::GetTickProgress() const
{
    float progress = Engine::Get()->GetTimeSinceLastTick() / (float)TICKSPEED;

    if(progress < 0.f)
        return 0.f;

    return progress < 1.f ? progress : 1.f;
}

DLLEXPORT std::tuple<int, int> GameWorld::GetTickAndTime() const
{
    int tick = TickNumber;
    int timeSince = static_cast<int>(Engine::Get()->GetTimeSinceLastTick());

    while(timeSince >= TICKSPEED) {

        ++tick;
        timeSince -= TICKSPEED;
    }

    return std::make_tuple(tick, timeSince);
}

// ------------------ Object managing ------------------ //
DLLEXPORT ObjectID GameWorld::CreateEntity()
{
    if(!GetNetworkSettings().IsAuthoritative) {
        // Clients create high number entities. This is not optimal but good enough for now
        auto id = (1 << 31) | static_cast<ObjectID>(IDFactory::GetID());

        Entities.push_back(id);

        return id;

    } else {
        auto id = static_cast<ObjectID>(IDFactory::GetID());

        Entities.push_back(id);

        if(NetworkSettings.IsAuthoritative) {
            // NewlyCreatedEntities.push_back(id);

            if(NetworkSettings.AutoCreateNetworkComponents) {
                _CreateSendableComponentForEntity(id);
            }
        }

        return id;
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::ClearEntities()
{
    // Release objects //
    Entities.clear();
    Parents.clear();
    // This shouldn't be used all that much so release the memory
    Parents.shrink_to_fit();

    // Clear all nodes //
    _ResetSystems();

    // Clears all components
    // Runs Release on components that need it
    _ResetOrReleaseComponents();

    // Notify everybody that all entities are discarded //
    for(auto iter = ReceivingPlayers.begin(); iter != ReceivingPlayers.end(); ++iter) {

        auto safe = (*iter)->GetConnection();

        if(!safe->IsValidForSend()) {
            // Player has probably closed their connection //
            continue;
        }

        Logger::Get()->Write("TODO: send world clear message");
        DEBUG_BREAK;
    }
}
// ------------------------------------ //
// DLLEXPORT Float3 GameWorld::GetGravityAtPosition(const Float3& pos)
// {
//     // \todo take position into account //
//     // create force without mass applied //
//     return Float3(0.f, PHYSICS_BASE_GRAVITY, 0.f);
// }
// ------------------------------------ //
DLLEXPORT int GameWorld::GetPhysicalMaterial(const std::string& name)
{
    if(!PhysicsMaterials)
        return -1;

    return PhysicsMaterials->GetMaterialID(name);
}
// ------------------------------------ //
DLLEXPORT void GameWorld::DestroyEntity(ObjectID id)
{
    // Fail if trying to delete NULL_OBJECT
    if(id == NULL_OBJECT)
        throw InvalidArgument("Cannot destroy NULL_OBJECT");

    // Fail if ticking currently //
    if(TickInProgress)
        throw InvalidState(
            "Cannot DestroyEntity while ticking. Use QueueDestroyEntity instead");

    for(auto iter = Entities.begin(); iter != Entities.end(); ++iter) {

        if(*iter == id) {

            Entities.erase(iter);
            _DoDestroy(id);
            return;
        }
    }

    LOG_ERROR("GameWorld: DestroyEntity: unknown entity id: " + std::to_string(id));
}

DLLEXPORT void GameWorld::QueueDestroyEntity(ObjectID id)
{
    // Fail if trying to delete NULL_OBJECT
    if(id == NULL_OBJECT)
        throw InvalidArgument("Cannot destroy NULL_OBJECT");

    // This is a sanity check, can be disabled (or made cheaper when the world no longer uses
    // IDFactory) when crashing stops
    bool exists = false;

    for(auto existingId : Entities) {
        if(existingId == id) {
            exists = true;
            break;
        }
    }

    if(!exists) {
        LOG_ERROR("GameWorld: QueueDestroyEntity: unknown entity id: " + std::to_string(id));
        return;
    }

    Lock lock(DeleteMutex);

    // Skip duplicates
    for(auto alreadyQueued : DelayedDeleteIDS)
        if(id == alreadyQueued)
            return;

    DelayedDeleteIDS.push_back(id);
}

void GameWorld::_HandleDelayedDelete()
{
    // We might want to delete everything //
    if(ClearAllEntities) {

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
    for(auto iter = Entities.begin(); iter != Entities.end();) {

        // Check does id match any //
        auto curid = *iter;
        bool delthis = false;

        for(auto iterids = DelayedDeleteIDS.begin(); iterids != DelayedDeleteIDS.end();) {

            if(*iterids == curid) {
                // Remove this as it will get deleted //
                delthis = true;
                DelayedDeleteIDS.erase(iterids);
                break;

            } else {
                ++iterids;
            }
        }

        if(delthis) {

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

void GameWorld::_DoDestroy(ObjectID id)
{
    // LOG_INFO("GameWorld destroying object " + Convert::ToString(id));

    if(NetworkSettings.IsAuthoritative)
        _ReportEntityDestruction(id);

    // TODO: find a better way to do this
    DestroyAllIn(id);

    // Parent destroy children //
    // We need to support recursively parented entities
    for(size_t i = 0; i < Parents.size();) {

        if(std::get<0>(Parents[i]) == id) {

            const auto childId = std::get<1>(Parents[i]);

            // Remove it //
            std::swap(Parents[i], Parents[Parents.size() - 1]);
            Parents.pop_back();

            // And then destroy //
            // LOG_WRITE("Destroying child ID: " + std::to_string(childId));
            DestroyEntity(childId);

            // To support recursively parented we go back to the start to scan again
            i = 0;

        } else if(std::get<1>(Parents[i]) == id) {
            // Child has been destroyed //
            // Remove it //
            std::swap(Parents[i], Parents[Parents.size() - 1]);
            Parents.pop_back();

        } else {
            ++i;
        }
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetEntitysParent(ObjectID child, ObjectID parent)
{
    Parents.push_back(std::make_tuple(parent, child));
}
// ------------------------------------ //
DLLEXPORT std::tuple<void*, bool> GameWorld::GetComponent(ObjectID id, COMPONENT_TYPE type)
{
    return std::make_tuple(nullptr, false);
}

DLLEXPORT std::tuple<void*, ComponentTypeInfo, bool> GameWorld::GetComponentWithType(
    ObjectID id, COMPONENT_TYPE type)
{
    return std::make_tuple(nullptr, ComponentTypeInfo(-1, -1), false);
}

DLLEXPORT std::tuple<void*, bool> GameWorld::GetStatesFor(COMPONENT_TYPE type)
{
    return std::make_tuple(nullptr, false);
}

DLLEXPORT bool GameWorld::GetRemovedFor(
    COMPONENT_TYPE type, std::vector<std::tuple<void*, ObjectID>>& result)
{
    return false;
}

DLLEXPORT bool GameWorld::GetRemovedForScriptDefined(
    const std::string& name, std::vector<std::tuple<asIScriptObject*, ObjectID>>& result)
{
    auto iter = pimpl->RegisteredScriptComponents.find(name);

    if(iter == pimpl->RegisteredScriptComponents.end())
        return false;

    auto& removed = iter->second->GetRemoved();

    result.insert(std::end(result), std::begin(removed), std::end(removed));
    return true;
}

DLLEXPORT bool GameWorld::GetAddedFor(
    COMPONENT_TYPE type, std::vector<std::tuple<void*, ObjectID, ComponentTypeInfo>>& result)
{
    return false;
}

DLLEXPORT bool GameWorld::GetAddedForScriptDefined(const std::string& name,
    std::vector<std::tuple<asIScriptObject*, ObjectID, ScriptComponentHolder*>>& result)
{
    auto iter = pimpl->RegisteredScriptComponents.find(name);

    if(iter == pimpl->RegisteredScriptComponents.end())
        return false;

    auto& added = iter->second->GetAdded();

    result.reserve(result.size() + added.size());

    for(const auto& tuple : added) {

        result.push_back(
            std::make_tuple(std::get<0>(tuple), std::get<1>(tuple), iter->second.get()));
    }

    return true;
}

// ------------------------------------ //
DLLEXPORT void GameWorld::_DoSystemsInit()
{
    // Script systems are initialized as they are created
}

DLLEXPORT void GameWorld::_DoSystemsRelease()
{
    // This can be called after Releasing once already
    if(!pimpl)
        return;

    // We are responsible for script systems //
    for(auto iter = pimpl->RegisteredScriptSystems.begin();
        iter != pimpl->RegisteredScriptSystems.end(); ++iter) {

        iter->second->Release();
    }

    pimpl->RegisteredScriptSystems.clear();
}

DLLEXPORT void GameWorld::_DoSuspendSystems()
{
    // We are responsible for script systems //
    for(auto iter = pimpl->RegisteredScriptSystems.begin();
        iter != pimpl->RegisteredScriptSystems.end(); ++iter) {

        iter->second->Suspend();
    }
}

DLLEXPORT void GameWorld::_DoResumeSystems()
{
    // We are responsible for script systems //
    for(auto iter = pimpl->RegisteredScriptSystems.begin();
        iter != pimpl->RegisteredScriptSystems.end(); ++iter) {

        iter->second->Resume();
    }
}

DLLEXPORT void GameWorld::_CreateSendableComponentForEntity(ObjectID id)
{
    LOG_ERROR("GameWorld: base version of _CreateSendableComponentForEntity, this shouldn't "
              "happen with correct configuration");
}

DLLEXPORT void GameWorld::_CreateReceivedComponentForEntity(ObjectID id)
{
    LOG_ERROR("GameWorld: base version of _CreateReceivedComponentForEntity, this shouldn't "
              "happen with correct configuration");
}

// ------------------------------------ //
DLLEXPORT void GameWorld::DestroyAllIn(ObjectID id)
{
    // This can be called after Releasing once already
    if(!pimpl)
        return;

    // Handle script types
    for(auto iter = pimpl->RegisteredScriptComponents.begin();
        iter != pimpl->RegisteredScriptComponents.end(); ++iter) {

        // Just try to remove like the normal c++ components until there is a better way
        iter->second->ReleaseComponent(id);
    }
}
// ------------------------------------ //
void GameWorld::_ReportEntityDestruction(ObjectID id)
{
    SendToAllPlayers(std::make_shared<ResponseEntityDestruction>(0, this->ID, id),
        RECEIVE_GUARANTEE::Critical);
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetWorldPhysicsFrozenState(bool frozen)
{
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

// DLLEXPORT RayCastHitEntity* GameWorld::CastRayGetFirstHit(const Float3& from, const Float3&
// to)
// {
//     // Create a data object for the ray cast //
//     RayCastData data(1, from, to);

//     // Call the actual ray firing function //
//     NewtonWorldRayCast(_PhysicalWorld->GetNewtonWorld(), &from.X, &to.X,
//         RayCallbackDataCallbackClosest, &data, nullptr, 0);

//     // Check the result //
//     if(data.HitEntities.size() == 0) {
//         // Nothing hit //
//         return new RayCastHitEntity();
//     }

//     // We need to increase reference count to not to accidentally delete the result while
//     // caller is using it
//     data.HitEntities[0]->AddRef();

//     // Return the only hit //
//     return data.HitEntities[0];
// }


// //! \todo improve this performance //
// dFloat RayCallbackDataCallbackClosest(const NewtonBody* const body,
//     const NewtonCollision* const shapeHit, const dFloat* const hitContact,
//     const dFloat* const hitNormal, dLong collisionID, void* const userData,
//     dFloat intersectParam)
// {
//     // Let's just store it as a NewtonBody pointer //
//     RayCastData* data = reinterpret_cast<RayCastData*>(userData);

//     if(data->HitEntities.size() == 0)
//         data->HitEntities.push_back(new RayCastHitEntity(body, intersectParam, data));
//     else
//         *data->HitEntities[0] = RayCastHitEntity(body, intersectParam, data);

//     // Continue //
//     return intersectParam;
// }

DLLEXPORT void GameWorld::MarkForClear()
{
    ClearAllEntities = true;
}
// ------------------------------------ //
void GameWorld::UpdatePlayersPositionData(ConnectedPlayer& ply)
{
    // Get the position for this player in this world //
    ObjectID id = ply.GetPositionInWorld(this);

    // Player is using a static position at (0, 0, 0) //
    if(id == 0)
        return;

    try {

        auto& position = GetComponent<Position>(id);

        (void)position.Members._Position;

    } catch(const NotFound&) {

        // Player has invalid position //
        Logger::Get()->Warning("Player position entity has no Position component");
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::HandleEntityPacket(
    ResponseEntityUpdate&& message, Connection& connection)
{
    if(NetworkSettings.IsAuthoritative) {

        // Find matching local control before allowing
        auto found = ActiveLocalControl.find(message.EntityID);

        if(found != ActiveLocalControl.end()) {

            if(found->second != &connection) {

                // It's unsafe to dereference found->second here
                LOG_WARNING("GameWorld: wrong player sent local control message, entity: " +
                            std::to_string(message.EntityID));
                return;
            }

            _ApplyLocalControlUpdateMessage(message.EntityID, message.TickNumber,
                message.UpdateData, message.ReferenceTick, -1);
            _OnLocalControlUpdatedEntity(message.EntityID, message.TickNumber);
            return;
        }

        LOG_WARNING(
            "GameWorld: didn't find local control entity for ResponseEntityUpdate, entity: " +
            std::to_string(message.EntityID));
        return;
    }

    // Don't apply if we don't have the entity
    bool found = false;

    for(auto entity : Entities) {
        if(entity == message.EntityID) {

            found = true;
            break;
        }
    }

    if(!found) {

        LOG_WARNING(
            "GameWorld: HandleEntityPacket: received update for non-existing entity, id: " +
            std::to_string(message.EntityID) +
            " TODO: queue for later applying in case packets are out of order");
        return;
    }

    // If this is controlled by us this is handled differently
    for(auto entity : OurActiveLocalControl) {

        if(entity == message.EntityID) {

            // TODO: apply corrections if our simulation was incorrect / not allowed
            return;
        }
    }

    try {
        _CreateStatesFromUpdateMessage(message.EntityID, message.TickNumber,
            message.UpdateData, message.ReferenceTick, -1);
    } catch(const InvalidArgument& e) {
        LOG_ERROR("GameWorld: HandleEntityPacket: trying to load update packet data caused an "
                  "exception: ");
        e.PrintToLog();
        LOG_INFO("GameWorld: note: entity may have partially updated states, id: " +
                 std::to_string(message.EntityID));
    }
}

DLLEXPORT ObjectID GameWorld::HandleEntityPacket(ResponseEntityCreation& message)
{
    if(NetworkSettings.IsAuthoritative) {

        LOG_WARNING("GameWorld: authoritative world is ignoring ResponseEntityCreation");
        return NULL_OBJECT;
    }

    if(message.ComponentCount > 1000) {
        LOG_ERROR("GameWorld: HandleEntityPacket: entity has more than 1000 components. "
                  "Packet is likely corrupted / forged, ignoring");
        return NULL_OBJECT;
    }

    // TODO: somehow detect if the ID collides with local entities (once those are allowed)
    Entities.push_back(message.EntityID);

    if(!NetworkSettings.IsAuthoritative) {

        if(NetworkSettings.AutoCreateNetworkComponents) {
            _CreateReceivedComponentForEntity(message.EntityID);
        }
    }

    try {
        _CreateComponentsFromCreationMessage(
            message.EntityID, message.InitialComponentData, message.ComponentCount, -1);

        return message.EntityID;
    } catch(const InvalidArgument& e) {
        LOG_ERROR(
            "GameWorld: HandleEntityPacket: trying to load packet data caused an exception: ");
        e.PrintToLog();
        LOG_INFO("GameWorld: destroying invalid received entity: " +
                 std::to_string(message.EntityID));
        DestroyEntity(message.EntityID);

        return NULL_OBJECT;
    }
}

DLLEXPORT void GameWorld::HandleEntityPacket(ResponseEntityDestruction& message)
{
    if(NetworkSettings.IsAuthoritative) {

        LOG_WARNING("GameWorld: authoritative world is ignoring ResponseEntityCreation");
        return;
    }

    for(auto entity : Entities) {
        if(entity == message.EntityID) {

            DestroyEntity(message.EntityID);
            return;
        }
    }

    // TODO: queue if we don't have an entity with the ID
    LOG_WARNING("GameWorld: HandleEntityPacket: received destruction message for unknown "
                "entity, TODO: queue");
}

DLLEXPORT void GameWorld::HandleEntityPacket(ResponseEntityLocalControlStatus& message)
{
    if(!message.Enabled) {

        for(auto iter = OurActiveLocalControl.begin(); iter != OurActiveLocalControl.end();
            ++iter) {

            if(*iter == message.EntityID) {
                OurActiveLocalControl.erase(iter);
                return;
            }
        }

        LOG_WARNING("GameWorld: received disable local control message for entity that wasn't "
                    "controlled by us");
    } else {

        // Ignore duplicates
        for(auto controlled : OurActiveLocalControl)
            if(controlled == message.EntityID)
                return;

        OurActiveLocalControl.push_back(message.EntityID);

        if(NetworkSettings.AutoCreateNetworkComponents) {
            try {
                _CreateSendableComponentForEntity(message.EntityID);
                LOG_INFO("GameWorld: created Sendable for locally controlled entity: " +
                         std::to_string(message.EntityID));
            } catch(const InvalidState&) {
                LOG_WARNING(
                    "GameWorld: couldn't create Sendable for now locally controlled entity:" +
                    std::to_string(message.EntityID));
            }
        }
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::SetLocalControl(
    ObjectID id, bool enabled, const std::shared_ptr<Connection>& allowedconnection)
{
    // Apply the change
    if(enabled) {

        // TODO: detect changing owner
        ActiveLocalControl[id] = allowedconnection.get();
    } else {

        auto found = ActiveLocalControl.find(id);

        if(found != ActiveLocalControl.end()) {
            ActiveLocalControl.erase(found);
        } else {
            LOG_ERROR("GameWorld: SetLocalControl: disable called on entity that wasn't being "
                      "controlled");
        }
    }

    // Notify
    auto response = std::make_shared<ResponseEntityLocalControlStatus>(0, ID, id, enabled);

    allowedconnection->SendPacketToConnection(response, RECEIVE_GUARANTEE::Critical);
}

DLLEXPORT void GameWorld::_OnLocalControlUpdatedEntity(ObjectID id, int32_t ticknumber)
{
    Position* position = GetComponentPtr<Position>(id);
    Physics* physics = GetComponentPtr<Physics>(id);

    if(position && physics) {

        if(position->Marked) {

            physics->JumpTo(*position);
        }
    }
}
// ------------------------------------ //
DLLEXPORT void GameWorld::ApplyQueuedPackets()
{
    if(!pimpl->QueuedEntityUpdates.empty()) {
    }

    // Applies packets that were received out of order. And throws out any too old packets

    // if(!InitialEntityPackets.empty())
    //     _ApplyInitialEntityPackets();

    // if(!EntityUpdatePackets.empty())
    //     _ApplyEntityUpdatePackets();
}
// ------------------------------------ //
// DLLEXPORT void GameWorld::HandleClockSyncPacket(RequestWorldClockSync* data)
// {
//     Logger::Get()->Info(
//         "GameWorld: adjusting our clock: Absolute: " + Convert::ToString(data->Absolute) +
//         ", tick: " + Convert::ToString(data->Ticks) +
//         ", engine ms: " + Convert::ToString(data->EngineMSTweak));

//     // Change our TickNumber to match //
//     Engine::Get()->_AdjustTickNumber(data->Ticks, data->Absolute);

//     if(data->EngineMSTweak)
//         Engine::Get()->_AdjustTickClock(data->EngineMSTweak, data->Absolute);
// }
// // ------------------------------------ //
// DLLEXPORT void GameWorld::HandleWorldFrozenPacket(ResponseWorldFrozen* data)
// {
//     Logger::Get()->Info(
//         "GameWorld(" + Convert::ToString(ID) +
//         "): frozen state updated, now: " + Convert::ToString<int>(data->Frozen) +
//         ", tick: " + Convert::ToString(data->TickNumber) +
//         " (our tick:" + Convert::ToString(TickNumber) + ")");

//     if(data->TickNumber > TickNumber) {

//         Logger::Get()->Write("TODO: freeze the world in the future");
//     }

//     // Set the state //
//     SetWorldPhysicsFrozenState(data->Frozen);

//     // Simulate ticks if required //
//     if(!data->Frozen) {

//         // Check how many ticks we are behind and simulate that number of physical updates
//         // int tickstosimulate = TickNumber - data->TickNumber;

//         if(tickstosimulate > 0) {

//             Logger::Get()->Info("GameWorld: unfreezing and simulating " +
//                                 Convert::ToString(tickstosimulate * TICKSPEED) +
//                                 " ms worth of more physical updates");

//             DEBUG_BREAK;
//             // _PhysicalWorld->AdjustClock(tickstosimulate * TICKSPEED);
//         }


//     } else {

//         // Snap objects back //
//         Logger::Get()->Info("TODO: world freeze snap things back a bit");
//     }
// }
// ------------------------------------ //
DLLEXPORT CScriptArray* GameWorld::GetRemovedIDsForComponents(CScriptArray* componenttypes)
{
    if(!componenttypes)
        return nullptr;

    if(componenttypes->GetElementTypeId() !=
        AngelScriptTypeIDResolver<uint16_t>::Get(ScriptExecutor::Get())) {

        LOG_ERROR("GameWorld: GetRemovedIDsForComponents: given an array of wrong type");
        componenttypes->Release();
        return nullptr;
    }

    std::vector<std::tuple<void*, ObjectID>> result;

    for(asUINT i = 0; i < componenttypes->GetSize(); ++i) {

        uint16_t* type = static_cast<uint16_t*>(componenttypes->At(i));

        if(!GetRemovedFor(static_cast<COMPONENT_TYPE>(*type), result)) {

            LOG_WARNING("GameWorld: GetRemovedIDsForComponents: unknown component type: " +
                        std::to_string(*type));
        }
    }

    componenttypes->Release();

    asITypeInfo* typeInfo =
        ScriptExecutor::Get()->GetASEngine()->GetTypeInfoByDecl("array<ObjectID>");

    CScriptArray* array = CScriptArray::Create(typeInfo, static_cast<asUINT>(result.size()));

    if(!array)
        return nullptr;

    for(asUINT i = 0; i < static_cast<asUINT>(result.size()); ++i) {

        array->SetValue(i, &std::get<1>(result[i]));
    }

    return array;
}

DLLEXPORT CScriptArray* GameWorld::GetRemovedIDsForScriptComponents(CScriptArray* typenames)
{
    if(!typenames)
        return nullptr;

    if(typenames->GetElementTypeId() !=
        AngelScriptTypeIDResolver<std::string>::Get(ScriptExecutor::Get())) {

        LOG_ERROR("GameWorld: GetRemovedIDsForScriptComponents: given an array of wrong type "
                  "(expected array<string>)");
        typenames->Release();
        return nullptr;
    }

    std::vector<std::tuple<asIScriptObject*, ObjectID>> result;

    for(asUINT i = 0; i < typenames->GetSize(); ++i) {

        std::string* type = static_cast<std::string*>(typenames->At(i));

        if(!GetRemovedForScriptDefined(*type, result)) {

            LOG_WARNING(
                "GameWorld: GetRemovedIDsForScriptComponents: unknown component type: " +
                *type);
        }
    }

    typenames->Release();

    asITypeInfo* typeInfo =
        ScriptExecutor::Get()->GetASEngine()->GetTypeInfoByDecl("array<ObjectID>");

    CScriptArray* array = CScriptArray::Create(typeInfo, static_cast<asUINT>(result.size()));

    if(!array)
        return nullptr;

    for(asUINT i = 0; i < static_cast<asUINT>(result.size()); ++i) {

        array->SetValue(i, &std::get<1>(result[i]));
    }

    return array;
}

DLLEXPORT bool GameWorld::RegisterScriptComponentType(
    const std::string& name, asIScriptFunction* factory)
{
    // Skip if already registered //
    if(pimpl->RegisteredScriptComponents.find(name) !=
        pimpl->RegisteredScriptComponents.end()) {
        factory->Release();
        return false;
    }

    // ScriptComponentHolder takes care of releasing the reference
    pimpl->RegisteredScriptComponents[name] =
        ScriptComponentHolder::MakeShared<ScriptComponentHolder>(name, factory, this);

    return true;
}

DLLEXPORT ScriptComponentHolder* GameWorld::GetScriptComponentHolder(const std::string& name)
{
    // if called after release
    if(!pimpl)
        return nullptr;

    auto iter = pimpl->RegisteredScriptComponents.find(name);

    if(iter == pimpl->RegisteredScriptComponents.end())
        return nullptr;

    iter->second->AddRef();
    return iter->second.get();
}

DLLEXPORT bool GameWorld::RegisterScriptSystem(
    const std::string& name, asIScriptObject* system)
{
    if(!system) {
        LOG_ERROR("GameWorld: RegisterScriptSystem: passed null object as new system");
        return false;
    }

    // if called after release
    if(!pimpl) {
        system->Release();
        return false;
    }

    // Skip if already registered //
    if(pimpl->RegisteredScriptSystems.find(name) != pimpl->RegisteredScriptSystems.end()) {
        system->Release();
        return false;
    }

    // Create a wrapper for it //
    // The wrapper handles unreferencing the handle
    pimpl->RegisteredScriptSystems[name] = std::make_unique<ScriptSystemWrapper>(name, system);

    // Might as well call Init now as other systems are almost certainly initialized as well //
    pimpl->RegisteredScriptSystems[name]->Init(this);
    return true;
}

DLLEXPORT bool GameWorld::UnregisterScriptSystem(const std::string& name)
{
    // if called after release
    if(!pimpl) {
        return false;
    }

    const auto iter = pimpl->RegisteredScriptSystems.find(name);
    if(iter == pimpl->RegisteredScriptSystems.end()) {
        return false;
    }

    iter->second->Release();
    pimpl->RegisteredScriptSystems.erase(iter);

    return true;
}

DLLEXPORT asIScriptObject* GameWorld::GetScriptSystem(const std::string& name)
{
    // if called after release
    if(!pimpl)
        return nullptr;

    auto iter = pimpl->RegisteredScriptSystems.find(name);

    if(iter == pimpl->RegisteredScriptSystems.end()) {

        LOG_ERROR("GameWorld: GetScriptSystemDirect: world has no system called: " + name);
        return nullptr;
    }

    return iter->second->GetASImplementationObject();
}
// ------------------------------------ //
DLLEXPORT void GameWorld::OnUnLinkedFromWindow(Window* window, Graphics* graphics)
{
    if(!LinkedToWindow) {
        LOG_WARNING(
            "GameWorld: unlink from window called while this wasn't linked to a window");
        return;
    }

    if(window != LinkedToWindow) {

        throw InvalidArgument("GameWorld attempted to be unlinked from window that wasn't the "
                              "one it was linked to");
    }

    pimpl->WorldCamera->getViewport()->setTarget(nullptr);

    // ogre->getCompositorManager2()->removeWorkspace(WorldWorkspace);
    // WorldWorkspace = nullptr;
    LinkedToWindow = nullptr;

    if(!TickWhileInBackground) {
        _DoSuspendSystems();
    }

    InBackground = true;
}

DLLEXPORT void GameWorld::OnLinkToWindow(Window* window, Graphics* graphics)
{
    // LEVIATHAN_ASSERT(WorldsScene, "World is not initialized");

    if(!window)
        throw InvalidArgument("GameWorld attempted to be linked to a nullptr window");

    if(LinkedToWindow // || WorldWorkspace
    ) {

        throw InvalidArgument(
            "GameWorld attempted to be linked to a window while it is already linked");
    }

    LinkedToWindow = window;

    // // Create the workspace for this scene //
    // // Which will be rendered before the overlay workspace but after potential
    // // clearing workspace
    // WorldWorkspace = ogre->getCompositorManager2()->addWorkspace(WorldsScene,
    //     LinkedToWindow->GetOgreWindow(), WorldSceneCamera, "WorldsWorkspace", true, 0);
    pimpl->WorldCamera->getViewport()->setTarget(window->GetBSFWindow());

    // TODO: this needs to be reapplied every time the window is resized
    // TODO: ask for BSF auto aspect ratio (setAutoAspectRatio?)
    int32_t width, height;
    window->GetSize(width, height);
    pimpl->WorldCamera->setAspectRatio(width / static_cast<float>(height));

    if(!TickWhileInBackground) {
        _DoResumeSystems();
    }

    InBackground = false;
}

DLLEXPORT void GameWorld::SetRunInBackground(bool tickinbackground)
{
    TickWhileInBackground = tickinbackground;
}

// // ------------------ RayCastHitEntity ------------------ //
// DLLEXPORT Leviathan::RayCastHitEntity::RayCastHitEntity(
//     const NewtonBody* ptr /*= nullptr*/, const float& tvar, RayCastData* ownerptr) :
//     HitEntity(ptr),
//     HitVariable(tvar)
// {
//     if(ownerptr) {
//         HitLocation = ownerptr->BaseHitLocationCalcVar * HitVariable;
//     } else {
//         HitLocation = (Float3)0;
//     }
// }

// DLLEXPORT bool Leviathan::RayCastHitEntity::HasHit()
// {
//     return HitEntity != nullptr;
// }

// DLLEXPORT bool Leviathan::RayCastHitEntity::DoesBodyMatchThisHit(NewtonBody* other)
// {
//     return HitEntity == other;
// }

// DLLEXPORT Float3 Leviathan::RayCastHitEntity::GetPosition()
// {
//     return HitLocation;
// }

// DLLEXPORT RayCastHitEntity& Leviathan::RayCastHitEntity::operator=(
//     const RayCastHitEntity& other)
// {
//     HitEntity = other.HitEntity;
//     HitVariable = other.HitVariable;
//     HitLocation = other.HitLocation;

//     return *this;
// }
// // ------------------ RayCastData ------------------ //
// DLLEXPORT Leviathan::RayCastData::RayCastData(
//     int maxcount, const Float3& from, const Float3& to) :
//     MaxCount(maxcount),
//     // Formula based on helpful guy on Newton forums
//     BaseHitLocationCalcVar(from + (to - from))
// {
//     // Reserve memory for maximum number of objects //
//     HitEntities.reserve(maxcount);
// }

// DLLEXPORT Leviathan::RayCastData::~RayCastData()
// {
//     // We want to release all hit data //
//     SAFE_RELEASE_VECTOR(HitEntities);
// }
