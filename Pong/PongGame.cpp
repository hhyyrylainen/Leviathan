// ------------------------------------ //
#include "PongGame.h"

#include "Handlers/ObjectLoader.h"

#include "Networking/Connection.h"
#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"
#include "Networking/RemoteConsole.h"
#include "PongNetHandler.h"
#include "Utility/Random.h"
#include "Window.h"
#include "add_on/autowrapper/aswrappedcall.h"
using namespace Pong;
using namespace Leviathan;
// ------------------------------------ //
// Put this here, since nowhere else to put it //
BasePongParts* Pong::BasepongStaticAccess = NULL;

void TryToCrash(bool enable)
{

    string state = enable ? "On" : "Off";

    Engine::Get()->GetEventHandler()->CallEvent(new Leviathan::GenericEvent(
        "LobbyScreenState", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                                new NamedVariableList("State", new VariableBlock(state))))));

    Engine::Get()->GetEventHandler()->CallEvent(
        new Leviathan::GenericEvent("PrematchScreenState",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                new NamedVariableList("State", new VariableBlock(state))))));

    Engine::Get()->GetEventHandler()->CallEvent(new Leviathan::GenericEvent(
        "MatchScreenState", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                                new NamedVariableList("State", new VariableBlock(state))))));

    Engine::Get()->GetEventHandler()->CallEvent(
        new Leviathan::GenericEvent("ConnectStatusMessage",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                new NamedVariableList("Message", new VariableBlock(string("test")))))));

    DEBUG_BREAK;
    // Engine::Get()->GetWindowEntity()->GetGui()->SetCollectionState("ConnectionScreen",
    // enable);
    // Engine::Get()->GetWindowEntity()->GetGui()->SetCollectionState("DirectConnectScreen",
    // enable); Engine::Get()->GetWindowEntity()->GetGui()->SetCollectionState("TopLevelMenu",
    // enable);
}

int Pong::PongGame::OnEvent(Event* event)
{

    // TryToCrash(Toggle);
    return 0;
}

Pong::PongGame::PongGame() :
    GuiManagerAccess(NULL), GameInputHandler(NULL)
#ifdef _WIN32
    ,
    ServerProcessHandle(NULL)
#endif // _WIN32

{
    StaticGame = this;
}

Pong::PongGame::~PongGame()
{
    GUARD_LOCK();
    // delete memory //

    // Wait for the child process to die and close the handle //
#ifdef _WIN32
    // Wait for the server to close for 5 seconds //
#ifndef _DEBUG
    WaitForSingleObject(ServerProcessHandle, 5000);

    TerminateProcess(ServerProcessHandle, -1);
#endif

    CloseHandle(ServerProcessHandle);
    ServerProcessHandle = NULL;
#else


#endif // _WIN32
}

PongGame* Pong::PongGame::Get()
{
    return StaticGame;
}

PongGame* Pong::PongGame::StaticGame = NULL;

Leviathan::NetworkInterface* PongGame::_GetApplicationPacketHandler()
{

    if(!ClientInterface) {
        ClientInterface = std::make_unique<PongNetHandler>();
        SetInterface(ClientInterface.get());
    }
    return ClientInterface.get();
}

void PongGame::_ShutdownApplicationPacketHandler()
{

    ClientInterface.reset();
}

// ------------------------------------ //
std::string Pong::PongGame::GenerateWindowTitle()
{
    return string("Pong version " GAME_VERSIONS " Leviathan " LEVIATHAN_VERSION_ANSIS);
}
// ------------------------------------ //
int Pong::PongGame::StartServer()
{
    // Start the server process //
    GUARD_LOCK();

#ifdef _DEBUG
    string serverstartname = "PongServerD";
#else
    string serverstartname = "PongServer";
#endif // _DEBUG

#ifdef _WIN32
    serverstartname += ".exe";
#else
    // Linux type start //
    serverstartname = "./" + serverstartname;
#endif // _WIN32

    int Tokennmbr = Random::Get()->GetNumber();

    // Create proper arguments for the program //
    string args = "--nogui -RemoteConsole:CloseIfNone -RemoteConsole:OpenTo:\"localhost:" +
                  Convert::ToString(ClientInterface->GetOwner()->GetOurPort()) +
                  "\":Token:" + Convert::ToString(Tokennmbr);

    // We need to expect this connection //
    Engine::Get()->GetRemoteConsole()->ExpectNewConnection(Tokennmbr, "ServerConsole", true);

    // Start the server //
    DEBUG_BREAK;
    // StaticGame->ServerProcessHandle = LeviathanApplication::StartServerProcess(
    //     serverstartname, args);

    // Queue a task for this //

    _Engine->GetThreadingManager()->QueueTask(
        shared_ptr<Leviathan::QueuedTask>(new Leviathan::ConditionalTask(
            std::bind<void>(
                [](Leviathan::RemoteConsole* justforperformance) -> void {
                    // No more waiting connections, see what happened //
                    if(justforperformance->GetActiveConnectionCount() == 0) {
                        // Failed //
                        Logger::Get()->Error(
                            "Failed to receive a remote connection from the server");
                        Engine::Get()->GetEventHandler()->CallEvent(
                            new Leviathan::GenericEvent("ConnectStatusMessage",
                                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                                    new NamedVariableList("Message",
                                        new VariableBlock(string(
                                            "The server failed to start properly")))))));
                        StaticGame->Disconnect("Server failed to start properly");

            // Kill the server //
#ifdef _WIN32
                        TerminateProcess(StaticGame->ServerProcessHandle, -1);

#endif // _WIN32

                        GUARD_LOCK_OTHER_NAME(StaticGame, guardStaticGame);

#ifdef WIN32
                        CloseHandle(StaticGame->ServerProcessHandle);
                        StaticGame->ServerProcessHandle = NULL;
#endif // WIN32


                        return;
                    }

                    auto tmpconnection =
                        Engine::Get()
                            ->GetRemoteConsole()
                            ->GetConnectionForRemoteConsoleSession("ServerConsole");

                    Logger::Get()->Info("Server started successfully");
                    Engine::Get()->GetEventHandler()->CallEvent(
                        new Leviathan::GenericEvent("ConnectStatusMessage",
                            Leviathan::NamedVars(
                                shared_ptr<NamedVariableList>(new NamedVariableList("Message",
                                    new VariableBlock(string(
                                        "Server started, awaiting proper startup")))))));

                    DEBUG_BREAK;
                    //    // Add repeating timed task that checks if the server is up and
                    //    properly running //
                    //    Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<Leviathan::QueuedTask>(new
                    //            Leviathan::RepeatingDelayedTask(std::bind<void>([](
                    //                std::shared_ptr<Connection> safeptr,
                    //                std::shared_ptr<SentNetworkThing> taskdata) -> void
                    //    {
                    //        if(!safeptr->IsOpen()){
                    //            // Destroy the attempt //
                    //            auto threadspecific =
                    //            TaskThread::GetThreadSpecificThreadObject(); auto taskobject
                    //            = threadspecific->QuickTaskAccess; auto tmpptr =
                    //            dynamic_cast<RepeatingDelayedTask*>(taskobject.get());
                    //            assert(tmpptr != NULL && "this is not what I wanted, passed
                    //            wrong task object to task");

                    //            tmpptr->SetRepeatStatus(false);

                    //            // Queue disconnect //
                    //            Engine::Get()->GetEventHandler()->CallEvent(new
                    //            Leviathan::GenericEvent("ConnectStatusMessage",
                    //                    Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    //                            new NamedVariableList("Message", new
                    //                            VariableBlock(
                    //                                    string("Server connection closed
                    //                                    unexpectedly")))))));

                    //            return;
                    //        }

                    //        // Send state request //
                    //        if(!taskdata){
                    //            // Send a new request //

                    //            shared_ptr<Leviathan::NetworkRequest> tmprequest(new
                    //                NetworkRequest(NETWORKREQUESTTYPE_SERVERSTATUS));

                    //            taskdata = safeptr->SendPacketToConnection(tmprequest, 2);

                    //            return;
                    //        }

                    //        // Check if the request is ready //
                    //        if(taskdata->IsFinalized()){

                    //            auto response = taskdata->GotResponse;

                    //            if(response){
                    //                // Check what we got //
                    //                if(response->GetTypeOfResponse() ==
                    //                NETWORKRESPONSETYPE_SERVERSTATUS){

                    //                    auto responsedata =
                    //                    response->GetResponseDataForServerStatus();

                    //                    if(responsedata){
                    //                        // Check is it joinable //
                    //                        if(responsedata->Joinable){
                    //                            // Stop repeating the task //
                    //                            auto threadspecific =
                    //                            TaskThread::GetThreadSpecificThreadObject();
                    //                            auto taskobject =
                    //                            threadspecific->QuickTaskAccess; auto tmpptr
                    //                            =
                    //                            dynamic_cast<RepeatingDelayedTask*>(taskobject.get());
                    //                            assert(tmpptr != NULL && "this is not what I
                    //                            wanted, passed wrong task object to task");

                    //                            tmpptr->SetRepeatStatus(false);

                    //                            Engine::Get()->GetEventHandler()->CallEvent(new
                    //                            Leviathan::GenericEvent("ConnectStatusMessage",
                    //                                    Leviathan::NamedVars(shared_ptr<NamedVariableList>(new
                    //                                    NamedVariableList(
                    //                                                "Message", new
                    //                                                VariableBlock(string(
                    //                                                        "Attempting to
                    //                                                        connect in 1
                    //                                                        second...")))))));


                    //                            // Queue a connect to the server //
                    //                            Engine::Get()->GetThreadingManager()->QueueTask(
                    //                                shared_ptr<Leviathan::QueuedTask>(new
                    //                                    Leviathan::DelayedTask(std::bind(&PongGame::ConnectNoError,
                    //                                            PongGame::Get(),
                    //                                            safeptr->GenerateFormatedAddressString()),
                    //                                        MillisecondDuration(1000))));

                    //                        } else {
                    //                            Engine::Get()->GetEventHandler()->CallEvent(new
                    //                            Leviathan::GenericEvent("ConnectStatusMessage",
                    //                                    Leviathan::NamedVars(shared_ptr<NamedVariableList>(new
                    //                                    NamedVariableList(
                    //                                                "Message", new
                    //                                                VariableBlock(string("Server
                    //                                                still starting")))))));
                    //                        }

                    //                    } else {
                    //                        Engine::Get()->GetEventHandler()->CallEvent(new
                    //                        Leviathan::GenericEvent("ConnectStatusMessage",
                    //                                Leviathan::NamedVars(shared_ptr<NamedVariableList>(new
                    //                                NamedVariableList(
                    //                                            "Message", new
                    //                                            VariableBlock(string("Invalid
                    //                                            packet!")))))));
                    //                    }
                    //                }

                    //            } else {

                    //                Engine::Get()->GetEventHandler()->CallEvent(new
                    //                Leviathan::GenericEvent("ConnectStatusMessage",
                    //                        Leviathan::NamedVars(shared_ptr<NamedVariableList>(new
                    //                        NamedVariableList("Message",
                    //                                    new VariableBlock(string("Taskdata to
                    //                                    server timed out,
                    //                                    resending...")))))));
                    //            }

                    //            // Reset the sent taskdata to resend it //
                    //            taskdata.reset();

                    //            return;
                    //        }

                    //        // We are waiting for the request //
                    //        Engine::Get()->GetEventHandler()->CallEvent(new
                    //        Leviathan::GenericEvent("ConnectStatusMessage",
                    //                Leviathan::NamedVars(shared_ptr<NamedVariableList>(new
                    //                NamedVariableList("Message", new
                    //                            VariableBlock(string("Waiting for the server
                    //                            to respond to our status request")))))));


                    //    }, tmpconnection, shared_ptr<SentNetworkThing>()),
                    //    MillisecondDuration(50))));
                },
                Engine::Get()->GetRemoteConsole()),
            std::bind<bool>(
                [](Leviathan::RemoteConsole* justforperformance) -> bool {
                    // Check if all remote console connections are timed out / connected //
                    return !justforperformance->IsAwaitingConnections();
                },
                Engine::Get()->GetRemoteConsole()))));

    // succeeded //
    return 1;
}
// ------------------------------------ //
void Pong::PongGame::CustomEnginePreShutdown()
{

    GameInputHandler.reset();
}
// ------------------------------------ //
void Pong::PongGame::Tick(int mspassed) {}
// ------------------------------------ //
void Pong::PongGame::AllowPauseMenu()
{
    // Allow pause menu //
    DEBUG_BREAK;
    // GuiManagerAccess->SetCollectionAllowEnableState("PauseMenu", true);
}

void Pong::PongGame::CustomizedGameEnd()
{
    GUARD_LOCK();

    // Disable pause menu //
    DEBUG_BREAK;
    // GuiManagerAccess->SetCollectionState("PauseMenu", false);
    // GuiManagerAccess->SetCollectionAllowEnableState("PauseMenu", false);
}
// ------------------------------------ //
void Pong::PongGame::CheckGameConfigurationVariables(Lock& guard, GameConfiguration* configobj)
{
    // Check for various variables //

    NamedVars* vars = configobj->AccessVariables(guard);

    // Master server force localhost //
    if(vars->ShouldAddValueIfNotFoundOrWrongType<bool>("MasterServerForceLocalhost")) {
        // Add new //
        vars->AddVar("MasterServerForceLocalhost", new VariableBlock(false));
        configobj->MarkModified(guard);
    }

    // Game configuration database //
    if(vars->ShouldAddValueIfNotFoundOrWrongType<string>("GameDatabase")) {
        // Add new //
        vars->AddVar("GameDatabase", new VariableBlock(string("PongGameDatabase.txt")));
        configobj->MarkModified(guard);
    }

    // Default server port //
    if(vars->ShouldAddValueIfNotFoundOrWrongType<int>("DefaultServerPort")) {
        // Add new //
        vars->AddVar("DefaultServerPort", new VariableBlock(int(53221)));
        configobj->MarkModified(guard);
    }
}

void Pong::PongGame::CheckGameKeyConfigVariables(Lock& guard, KeyConfiguration* keyconfigobj)
{}
// ------------------------------------ //
void Pong::PongGame::MoveBackToLobby()
{
    GUARD_LOCK();
    DEBUG_BREAK;
}

void Pong::PongGame::Disconnect(const string& reasonstring)
{
    GUARD_LOCK();

    // Disconnect from active servers //
    ClientInterface->DisconnectFromServer(reasonstring);

    // Disable lobby screen //
    Engine::Get()->GetEventHandler()->CallEvent(new Leviathan::GenericEvent("LobbyScreenState",
        Leviathan::NamedVars(shared_ptr<NamedVariableList>(
            new NamedVariableList("State", new VariableBlock(string("Off")))))));

    // Disable prematch screen //
    Engine::Get()->GetEventHandler()->CallEvent(
        new Leviathan::GenericEvent("PrematchScreenState",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                new NamedVariableList("State", new VariableBlock(string("Off")))))));

    // Disable match screen //
    Engine::Get()->GetEventHandler()->CallEvent(new Leviathan::GenericEvent("MatchScreenState",
        Leviathan::NamedVars(shared_ptr<NamedVariableList>(
            new NamedVariableList("State", new VariableBlock(string("Off")))))));


    // Disable during game screen //


    // Disable scores screen //
}
// ------------------------------------ //
void Pong::PongGame::DoSpecialPostLoad()
{

    GameInputHandler = shared_ptr<GameInputController>(new GameInputController());

    // Load GUI documents (but only if graphics are enabled) //
    if(Engine::Get()->GetNoGui()) {

        // Skip the graphical objects when not in graphical mode //
        return;
    }

    Window* window1 = Engine::GetEngine()->GetWindowEntity();

    GuiManagerAccess = window1->GetGui();

    if(!GuiManagerAccess->LoadGUIFile("./Data/Scripts/GUI/PongMenus.txt")) {

        Logger::Get()->Error("Pong: failed to load the GuiFile, quitting");
        LeviathanApplication::Get()->StartRelease();
    }

    // set skybox to have some sort of visuals //
    // Doesn't work needs fixing
    // WorldOfPong->SetSkyBox("NiceDaySky");
    // window1->SetAutoClearing("Stone");

    GameArena->VerifyTrail();

    // link world to a window //
    window1->LinkObjects(WorldOfPong);

    // Create camera in the world //
    const auto camera = Leviathan::ObjectLoader::LoadCamera(*WorldOfPong,
        Float3(0.f, 22.f * BASE_ARENASCALE, 0.f),
        // Camera should always point down towards the play field //
        bs::Quaternion(bs::Vector3(0, -1, 0), bs::Radian(0)));

    WorldOfPong->SetCamera(camera);

    // link window input to game logic //
    window1->SetCustomInputController(GameInputHandler);

    // This is how to do something every frame //
    Leviathan::Engine::Get()->GetEventHandler()->RegisterForEvent(this, EVENT_TYPE_FRAME_END);

    ClearTimers();
}
// ------------------------------------ //
string GetPongVersionProxy()
{

    return GAME_VERSIONS;
}
// ------------------------------------ //
int Pong::PongGame::GetOurPlayerID()
{

    return ClientInterface->GetOurID();
}
// ------------------------------------ //
bool Pong::PongGame::MoreCustomScriptTypes(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("PongGame", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        SCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterGlobalFunction(
           "PongGame@ GetPongGame()", WRAP_FN(PongGame::Get), asCALL_GENERIC) < 0) {
        SCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PongGame", "int StartServer()",
           WRAP_MFN(PongGame, StartServer), asCALL_GENERIC) < 0) {
        SCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PongGame", "void MoveBackToLobby()",
           WRAP_MFN(PongGame, MoveBackToLobby), asCALL_GENERIC) < 0) {
        SCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectMethod("PongGame",
           "void Disconnect(const string &in statusmessage)", WRAP_MFN(PongGame, Disconnect),
           asCALL_GENERIC) < 0) {
        SCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectMethod("PongGame",
           "bool Connect(const string &in address, string &out errormessage)",
           asMETHOD(PongGame, Connect), asCALL_THISCALL) < 0) {
        SCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectMethod("PongGame",
           "bool SendServerCommand(const string &in command)",
           asMETHOD(PongGame, SendServerCommand), asCALL_THISCALL) < 0) {
        SCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectMethod("PongGame", "int GetOurPlayerID()",
           asMETHOD(PongGame, GetOurPlayerID), asCALL_THISCALL) < 0) {
        SCRIPT_REGISTERFAIL;
    }




    // Version getting function //
    if(engine->RegisterGlobalFunction(
           "string GetPongVersion()", asFUNCTION(GetPongVersionProxy), asCALL_CDECL) < 0) {
        SCRIPT_REGISTERFAIL;
    }


    return true;
}

bool Pong::PongGame::Connect(const string& address, string& errorstr)
{
    Logger::Get()->Info("About to connect to address " + address);

    // Send an event about the server name //
    Engine::Get()->GetEventHandler()->CallEvent(new Leviathan::GenericEvent(
        "ServerInfoUpdate", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                                new NamedVariableList("Name", new VariableBlock(address))))));


    // Get a connection to use //
    auto tmpconnection = ClientInterface->GetOwner()->OpenConnectionTo(address);

    if(!tmpconnection) {

        errorstr = "Tried to connect to an invalid address, " + address;

        Engine::Get()->GetEventHandler()->CallEvent(
            new Leviathan::GenericEvent("ConnectStatusMessage",
                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("Message", new VariableBlock(errorstr))))));
        return false;
    }

    Engine::Get()->GetEventHandler()->CallEvent(
        new Leviathan::GenericEvent("ConnectStatusMessage",
            Leviathan::NamedVars(shared_ptr<NamedVariableList>(new NamedVariableList("Message",
                new VariableBlock("Opening connection to server at " + address))))));

    // We are a client and we can use our interface to handle the server connection functions
    // //

    ClientInterface->JoinServer(tmpconnection);
    // The function automatically reports any errors //


    // Now it should be fine, waiting for messages //
    return true;
}

void Pong::PongGame::OnPlayerStatsUpdated(PlayerList* list)
{
    // Fire an event to notify the GUI about this //
    Engine::Get()->GetEventHandler()->CallEvent(
        new GenericEvent("PlayerStatusUpdated", NamedVars(new NamedVariableList())));
}

bool Pong::PongGame::SendServerCommand(const string& command)
{

    if(!ClientInterface->IsConnected())
        return false;

    try {
        ClientInterface->SendCommandStringToServer(command);

    } catch(const Exception& e) {

        Logger::Get()->Warning("Failed to send command to the server: ");
        e.PrintToLog();
        return false;
    }

    // Nothing failed so it should have worked //
    return true;
}

void Pong::PongGame::VerifyCorrectState(PONG_JOINGAMERESPONSE_TYPE serverstatus)
{

    switch(serverstatus) {
    case PONG_JOINGAMERESPONSE_TYPE_LOBBY: {
        // Show the lobby //
        // Send event to enable the lobby screen //
        Engine::Get()->GetEventHandler()->CallEvent(
            new Leviathan::GenericEvent("LobbyScreenState",
                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("State", new VariableBlock(string("On")))))));

        return;
    }
    case PONG_JOINGAMERESPONSE_TYPE_PREMATCH: {
        // First hide the lobby screen //
        Engine::Get()->GetEventHandler()->CallEvent(
            new Leviathan::GenericEvent("LobbyScreenState",
                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("State", new VariableBlock(string("Off")))))));


        // Display the preparation screen //
        Engine::Get()->GetEventHandler()->CallEvent(
            new Leviathan::GenericEvent("PrematchScreenState",
                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("State", new VariableBlock(string("On")))))));

        // Set the camera to be in the game playing position //
        DEBUG_BREAK;
        // auto cam = Engine::GetEngine()->GetWindowEntity()->GetLinkedCamera();
        // cam->SetPos(Float3(0.f, 22.f*BASE_ARENASCALE, 0.f));
        // cam->SetRotation(Float3(0.f, -90.f, 0.f));

        return;
    }
    case PONG_JOINGAMERESPONSE_TYPE_MATCH: {
        // First hide the preparation screen  //
        Engine::Get()->GetEventHandler()->CallEvent(
            new Leviathan::GenericEvent("PrematchScreenState",
                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("State", new VariableBlock(string("Off")))))));


        // And display the match HUD //
        Engine::Get()->GetEventHandler()->CallEvent(
            new Leviathan::GenericEvent("MatchScreenState",
                Leviathan::NamedVars(shared_ptr<NamedVariableList>(
                    new NamedVariableList("State", new VariableBlock(string("On")))))));


        return;
    }
    default: {
        Logger::Get()->Error("Pong: unknown state!");
        DEBUG_BREAK;
    }
    }
}
