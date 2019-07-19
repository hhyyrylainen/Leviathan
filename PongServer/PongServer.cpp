#include "PongIncludes.h"
// ------------------------------------ //
#include "PongServer.h"

#include "Application/GameConfiguration.h"
#include "Common/DataStoring/NamedVars.h"
#include "Entities/Components.h"
#include "Networking/NetworkHandler.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/SyncedVariables.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "PongServerNetworking.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"
using namespace Pong;
// ------------------------------------ //
// Put this here, since nowhere else to put it //
BasePongParts* Pong::BasepongStaticAccess = NULL;

Pong::PongServer::PongServer() :
    ServerInputHandler(NULL), BallLastPos(0.f), DeadAxis(0.f), StuckThresshold(0)
{

    Staticaccess = this;
}

Pong::PongServer::~PongServer()
{
    Staticaccess = NULL;
}

std::string Pong::PongServer::GenerateWindowTitle()
{
    return string(
        "PongServer for version " GAME_VERSIONS " Leviathan " LEVIATHAN_VERSION_ANSIS);
}

PongServer* Pong::PongServer::Staticaccess = NULL;

PongServer* Pong::PongServer::Get()
{
    return Staticaccess;
}

Leviathan::NetworkInterface* PongServer::_GetApplicationPacketHandler()
{

    if(!ServerInterface) {
        ServerInterface = std::make_unique<PongServerNetworking>();
        SetInterface(ServerInterface.get());
    }
    return ServerInterface.get();
}

void PongServer::_ShutdownApplicationPacketHandler()
{

    ServerInterface.reset();
}
// ------------------------------------ //
void Pong::PongServer::Tick(int mspassed)
{

    using namespace Leviathan;

    // We will want to skip doing things if we are shutting down soon //
    if(Engine::Get()->HasPreReleaseBeenDone())
        return;

    // Let the AI think //
    if(GameArena && GameArena->GetBall() != 0 && !GamePaused) {

        // Find AI slots //
        for(size_t i = 0; i < _PlayerList.Size(); i++) {

            PlayerSlot* slotptr = _PlayerList[i];

            while(slotptr) {

                if(slotptr->GetControlType() == PLAYERCONTROLS_AI && slotptr->IsSlotActive()) {

                    // Set the slot ptr as the argument and call function based on difficulty
                    // //
                    if(GameAI) {
                        // The identifier defines the AI type and they are set in the database
                        // //
                        ScriptRunningSetup setup;
                        switch(slotptr->GetControlIdentifier()) {
                        case 1: setup.SetEntrypoint("BallTrackerAI"); break;
                        case 2: setup.SetEntrypoint("CombinedAI"); break;
                        case 0:
                        default: setup.SetEntrypoint("SimpleAI"); break;
                        }

                        GameAI->ExecuteOnModule<void>(setup, true, slotptr, mspassed);
                    }
                }

                slotptr = slotptr->GetSplit();
            }
        }

        // Check if ball is too far away (also check if it is vertically stuck or horizontally)
        // //

        ObjectID ball = GameArena->GetBall();

        try {
            DEBUG_BREAK;
            // auto& pos = WorldOfPong->GetComponent<Leviathan::Position>(ball).Members;

            // const auto& ballcurpos = pos._Position;

            // if(ballcurpos.HAddAbs() > 100 * BASE_ARENASCALE){

            //     _DisposeOldBall();

            //     // Serve new ball //
            //     GameArena->ServeBall();
            //     return;
            // }

        } catch(const NotFound&) {

            // Ball is invalid //
            Logger::Get()->Error("Invalid ball, resetting");

            _DisposeOldBall();

            // Serve new ball //
            GameArena->ServeBall();
            return;
        }

        bool ballstuck = false;

        try {

            // Check is the ball stuck on the dead axis (where no paddle can hit it) //
            auto& physics = WorldOfPong->GetComponent<Leviathan::Physics>(ball);

            Float3 ballspeed = physics.GetBody()->GetVelocity();
            ballspeed.X = abs(ballspeed.X);
            ballspeed.Y = 0;
            ballspeed.Z = abs(ballspeed.Z);
            ballspeed = ballspeed.Normalize();

            float ballvel = ballspeed.HAddAbs();

            if(ballvel < 0.04f) {

                if(!IsBallInGoalArea()) {

                    Logger::Get()->Info("Pong: ball going too slow outside goal area");
                    ballstuck = true;
                }

            } else if(DeadAxis.HAddAbs() != 0) {
                // Compare directions //

                float veldifference = (ballspeed - DeadAxis).HAddAbs();

                if(veldifference < BALLSTUCK_THRESHOLD) {

                    StuckThresshold++;

                    if(StuckThresshold >= BALLSTUCK_COUNT) {
                        // Check is ball in a goal area //
                        if(IsBallInGoalArea()) {
                            StuckThresshold = 0;
                        } else {

                            Logger::Get()->Info("Pong: ball stuck count over thresshold");
                            ballstuck = true;
                        }
                    }
                } else {
                    if(StuckThresshold >= 1)
                        StuckThresshold--;
                }
            }


        } catch(const NotFound&) {

            // Ball is invalid //
            Logger::Get()->Error("Invalid ball, resetting");

            _DisposeOldBall();

            // Serve new ball //
            GameArena->ServeBall();
            return;
        }

        if(ballstuck) {

            Logger::Get()->Info("Ball stuck!");

            _DisposeOldBall();
            // Serve new ball //
            GameArena->ServeBall();
        }
    }
}
// ------------------------------------ //
void Pong::PongServer::CheckGameConfigurationVariables(
    Lock& guard, GameConfiguration* configobj)
{
    // Check for various variables //
    NamedVars* vars = configobj->AccessVariables(guard);

    // Default server port //
    if(vars->ShouldAddValueIfNotFoundOrWrongType<int>("DefaultServerPort")) {
        // Add new //
        vars->AddVar("DefaultServerPort", new VariableBlock(int(53221)));
        configobj->MarkModified(guard);
    }

    // Game configuration database //
    if(vars->ShouldAddValueIfNotFoundOrWrongType<string>("GameDatabase")) {
        // Add new //
        vars->AddVar("GameDatabase", new VariableBlock(string("PongGameDatabase.txt")));
        configobj->MarkModified(guard);
    }
}

void Pong::PongServer::CheckGameKeyConfigVariables(Lock& guard, KeyConfiguration* keyconfigobj)
{}
// ------------------------------------ //
void Pong::PongServer::CheckForGameEnd()
{
    // Look through all players and see if any team/player has reached score limit // //
    for(size_t i = 0; i < _PlayerList.Size(); i++) {

        PlayerSlot* slotptr = _PlayerList[i];

        int totalteamscore = 0;

        while(slotptr) {

            totalteamscore += slotptr->GetScore();
            slotptr = slotptr->GetSplit();
        }

        if(totalteamscore >= ScoreLimit) {
            // Team has won //
            Logger::Get()->Info("Team " + Convert::ToString(i) + " has won the match!");


            // Do various activities related to winning the game //

            // // Set the camera location //
            // auto cam = Engine::GetEngine()->GetWindowEntity()->GetLinkedCamera();

            DEBUG_BREAK;
            // switch(i){
            // case 0:
            //     {
            //         cam->SetPos(Float3(4.f*BASE_ARENASCALE, 2.f*BASE_ARENASCALE, 0.f));
            //         cam->SetRotation(Float3(-90.f, -30.f, 0.f));
            //     }
            //     break;
            // case 1:
            //     {
            //         cam->SetPos(Float3(0.f, 2.f*BASE_ARENASCALE, 4.f*BASE_ARENASCALE));
            //         cam->SetRotation(Float3(-180.f, -30.f, 0.f));
            //     }
            //     break;
            // case 2:
            //     {
            //         cam->SetPos(Float3(-4.f*BASE_ARENASCALE, 2.f*BASE_ARENASCALE, 0.f));
            //         cam->SetRotation(Float3(90.f, -30.f, 0.f));
            //     }
            //     break;
            // case 3:
            //     {
            //         cam->SetPos(Float3(0.f, 2.f*BASE_ARENASCALE, 4.f*BASE_ARENASCALE));
            //         cam->SetRotation(Float3(0.f, -30.f, 0.f));
            //     }
            //     break;
            // }

            Logger::Get()->Info("TODO: make clients move the camera around");

            // Send the game end event which should trigger proper menus //
            Leviathan::Engine::Get()->GetEventHandler()->CallEvent(
                new Leviathan::GenericEvent(new string("MatchEnded"),
                    new NamedVars(shared_ptr<NamedVariableList>(new NamedVariableList(
                        "WinningTeam", new Leviathan::VariableBlock((int)i))))));

            // And finally destroy the ball //
            GameArena->LetGoOfBall();

            // Send a message to all players that we are now in post match mode
            DEBUG_BREAK;

            // (Don't block input so players can wiggle around //


            return;
        }
    }
}

void Pong::PongServer::ServerCheckEnd()
{
    CheckForGameEnd();
}
// ------------------------------------ //
void Pong::PongServer::DoSpecialPostLoad()
{

    // Setup receiving networked controls from players //
    ServerInputHandler = shared_ptr<GameInputController>(new GameInputController());
    // ServerInterface.RegisterNetworkedInput(ServerInputHandler);

    // Create all the server variables //
    Leviathan::SyncedVariables* tmpvars = ServerInterface->GetOwner()->GetSyncedVariables();

    tmpvars->AddNewVariable(std::make_shared<SyncedValue>(
        new NamedVariableList("TheAnswer", new VariableBlock(42))));

    GameArena->VerifyTrail();

    ClearTimers();
}

void Pong::PongServer::CustomizedGameEnd()
{
    // Tell all clients to go to score screen //
}
// ------------------------------------ //
void Pong::PongServer::RunAITestMatch()
{

    Logger::Get()->Info("Running AI test");

    _PlayerList[0]->AddServerAI(1020 + 1, 2);
    _PlayerList[1]->AddServerAI(1020 + 2, 1);
    _PlayerList[2]->AddServerAI(1020 + 3, 0);

    Logger::Get()->Info("Starting a match");

    OnStartPreMatch();
}
// ------------------------------------ //
bool Pong::PongServer::MoreCustomScriptTypes(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("PongServer", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        SCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterGlobalFunction(
           "PongServer@ GetPongServer()", asFUNCTION(PongServer::Get), asCALL_CDECL) < 0) {
        SCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectMethod("PongServer", "void GameMatchEnded()",
           asMETHOD(PongServer, GameMatchEnded), asCALL_THISCALL) < 0) {
        SCRIPT_REGISTERFAIL;
    }

    // Testing functions //
    if(engine->RegisterObjectMethod("PongServer", "void RunAITestMatch()",
           asMETHOD(PongServer, RunAITestMatch), asCALL_THISCALL) < 0) {
        SCRIPT_REGISTERFAIL;
    }


    return true;
}

void Pong::PongServer::PreFirstTick()
{

    ServerInterface->SetServerAllowPlayers(true);
    ServerInterface->SetServerStatus(Leviathan::SERVER_STATUS::Running);
}
// ------------------------------------ //
void Pong::PongServer::OnStartPreMatch()
{

    // Setup the world first as that can fail //
    WorldOfPong->ClearEntities();
    WorldOfPong->SetWorldPhysicsFrozenState(true);

    // Setup the objects in the world //
    if(!GameArena->GenerateArena(this, _PlayerList)) {

        Logger::Get()->Warning("PongServer: failed to generate arena");
        return;
    }

    // Notify the clients //
    ServerInterface->SetStatus(PONG_JOINGAMERESPONSE_TYPE_PREMATCH);


    // Make sure that everyone is receiving our world //
    // This will send many objects at once to all the players (or rather should send them in
    // bulk)
    ServerInterface->VerifyWorldIsSyncedWithPlayers(WorldOfPong);


    // Queue a readyness checking task //
    ThreadingManager::Get()->QueueTask(new ConditionalTask(
        std::bind<void>(
            [](PongServer* server) -> void {
                Logger::Get()->Info("All players are synced, the match is ready to begin");

                // Start the match //
                server->WorldOfPong->SetWorldPhysicsFrozenState(false);
                server->ServerInterface->SetStatus(PONG_JOINGAMERESPONSE_TYPE_MATCH);

                // Spawn a ball //
                server->GameArena->ServeBall();

                // TODO: add a start timer here
            },
            this),
        std::bind<bool>(
            [](shared_ptr<GameWorld> world) -> bool {
                // We are ready to start once all clients are reported to be up to date by the
                // world //
                DEBUG_BREAK;
                // return world->AreAllPlayersSynced();
                return true;
            },
            WorldOfPong)));

    // Clear all other sorts of data like scores etc. //



    auto split0 = _PlayerList[0]->GetSplit();
    auto split1 = _PlayerList[1]->GetSplit();
    auto split2 = _PlayerList[2]->GetSplit();
    auto split3 = _PlayerList[3]->GetSplit();
    // Setup dead angle //
    DeadAxis = Float3(0.f);

    if(!_PlayerList[0]->IsSlotActive() && !_PlayerList[2]->IsSlotActive() &&
        (split0 ? !split0->IsSlotActive() : true) &&
        (split2 ? !split2->IsSlotActive() : true)) {

        DeadAxis = Float3(1.f, 0.f, 0.f);

    } else if(!_PlayerList[1]->IsSlotActive() && !_PlayerList[3]->IsSlotActive() &&
              (split1 ? !split1->IsSlotActive() : true) &&
              (split3 ? !split3->IsSlotActive() : true)) {

        DeadAxis = Float3(0.f, 0.f, 1.f);
    }
}
// ------------------------------------ //
void PongServer::SetScoreLimit(int scorelimit)
{
    ScoreLimit = scorelimit;
}

int PongServer::PlayerScored(ObjectID goal)
{
    // Don't count if the player whose goal the ball is in is the last one to touch
    // it or if none have touched it
    if(PlayerIDMatchesGoalAreaID(LastPlayerHitBallID, goal) || LastPlayerHitBallID == -1) {

        return 1;
    }

    // Add point to the player who scored //

    // Look through all players and compare PlayerIDs //
    for(size_t i = 0; i < _PlayerList.Size(); i++) {

        PlayerSlot* slotptr = _PlayerList[i];

        while(slotptr) {


            if(LastPlayerHitBallID == slotptr->GetPlayerNumber()) {
                // Found right player //
                slotptr->SetScore(slotptr->GetScore() + SCOREPOINT_AMOUNT);
                _PlayerList.NotifyUpdatedValue();

                goto playrscorelistupdateendlabel;
            }

            slotptr = slotptr->GetSplit();
        }
    }
    // No players got points! //

playrscorelistupdateendlabel:

    GameArena->LetGoOfBall();

    Leviathan::ThreadingManager::Get()->QueueTask(new QueuedTask(std::bind<void>(
        [](int LastPlayerHitBallID, PongServer* instance) -> void {
            // Serve new ball //
            instance->GameArena->ServeBall();

            // Check for game end //
            instance->ServerCheckEnd();
        },
        LastPlayerHitBallID.GetValue(), this)));

    return 0;
}

void PongServer::_SetLastPaddleHit(ObjectID objptr, ObjectID objptr2)
{
    // Note: the object pointers can be in any order they want //

    auto realballptr = GameArena->GetBall();

    // Look through all players and compare paddle ptrs //
    for(size_t i = 0; i < _PlayerList.Size(); i++) {

        PlayerSlot* slotptr = _PlayerList[i];

        while(slotptr) {

            auto paddle = slotptr->GetPaddle();

            if((objptr == paddle && objptr2 == realballptr) ||
                (objptr2 == paddle && objptr == realballptr)) {
                // Found right player //
                if(LastPlayerHitBallID != slotptr->GetPlayerNumber()) {
                    LastPlayerHitBallID = slotptr->GetPlayerNumber();
                    SetBallLastHitColour();
                }

                return;
            }

            slotptr = slotptr->GetSplit();
        }
    }
}

int PongServer::_BallEnterGoalArea(ObjectID goal, ObjectID ballobject)
{
    // Note: the object pointers can be in any order they want //

    auto castedptr = GameArena->GetBall();

    if(ballobject == castedptr) {
        // goal is actually the goal area //
        return PlayerScored(goal);

    } else if(goal == castedptr) {

        // ballobject is actually the goal area //
        return PlayerScored(ballobject);
    }

    return 0;
}

void PongServer::_DisposeOldBall()
{

    // Tell arena to let go of old ball //
    GameArena->LetGoOfBall();

    // Reset variables //
    LastPlayerHitBallID = -1;
    StuckThresshold = 0;
    // This should reset the ball trail colour //
}

void PongServer::GameMatchEnded()
{
    // This can be called from script so ensure that these are set //
    GameArena->LetGoOfBall();

    CustomizedGameEnd();
}

// void PongServer::BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep,
//     int threadIndex)
// {

//     // Call the callback //
//     Staticaccess->_SetLastPaddleHit(reinterpret_cast<Physics*>(
//             NewtonBodyGetUserData(NewtonJointGetBody0(contact)))->ThisEntity,
//         reinterpret_cast<Physics*>(NewtonBodyGetUserData(
//                 NewtonJointGetBody1(contact)))->ThisEntity);
// }

// void PongServer::BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep,
//     int threadIndex)
// {
//     // Call the function and set the collision state as the last one //
//     NewtonJointSetCollisionState(contact,
//         Staticaccess->_BallEnterGoalArea(reinterpret_cast<
//             Physics*>(NewtonBodyGetUserData(NewtonJointGetBody0(contact)))->ThisEntity,
//             reinterpret_cast<Physics*>(NewtonBodyGetUserData(
//                     NewtonJointGetBody1(contact)))->ThisEntity));
// }

// PhysicsMaterialContactCallback PongServer::GetBallPaddleCallback(){

//     return BallContactCallbackPaddle;
// }

// PhysicsMaterialContactCallback PongServer::GetBallGoalAreaCallback(){

//     return BallContactCallbackGoalArea;
// }
