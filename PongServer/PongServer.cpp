#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_SERVER
#include "PongServer.h"
#endif
#include "Common/DataStoring/NamedVars.h"
#include "Application/GameConfiguration.h"
#include "PongServerNetworking.h"
#include "Networking/NetworkResponse.h"
#include "Networking/SyncedVariables.h"
#include "Rendering/GraphicalInputEntity.h"
#include "OgreException.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/NetworkHandler.h"
#include "Threading/ThreadingManager.h"
#include "ObjectFiles/ObjectFileProcessor.h"
using namespace Pong;
// ------------------------------------ //
// Put this here, since nowhere else to put it //
BasePongParts* Pong::BasepongStaticAccess = NULL;

Pong::PongServer::PongServer() :
    ServerInputHandler(NULL), _PongServerNetworking(NULL), BallLastPos(0.f), DeadAxis(0.f),
    StuckThresshold(0)
{

	Staticaccess = this;
}

Pong::PongServer::~PongServer(){
	Staticaccess = NULL;
}

std::wstring Pong::PongServer::GenerateWindowTitle(){
	return wstring(L"PongServer for version " GAME_VERSIONS L" Leviathan " LEVIATHAN_VERSIONS);
}


PongServer* Pong::PongServer::Staticaccess = NULL;


PongServer* Pong::PongServer::Get(){
	return Staticaccess;
}
// ------------------------------------ //
void Pong::PongServer::Tick(int mspassed){

    using namespace Leviathan;

    // Let the AI think //
    if(GameArena && GameArena->GetBallPtr() && !GamePaused){

        // Find AI slots //
        for(size_t i = 0; i < _PlayerList.Size(); i++){

            PlayerSlot* slotptr = _PlayerList[i];

            while(slotptr){

                if(slotptr->GetControlType() == PLAYERCONTROLS_AI){

                    // Set the slot ptr as the argument and call function based on difficulty //
                    std::vector<shared_ptr<NamedVariableBlock>> scriptargs(2);
                    scriptargs[0] = shared_ptr<NamedVariableBlock>(new NamedVariableBlock(
                            new VoidPtrBlock(slotptr), L"PlayerSlot"));
                    scriptargs[1] = shared_ptr<NamedVariableBlock>(new NamedVariableBlock(
                            new IntBlock(mspassed), L"MSPassed"));

                    if(GameAI){
                        bool ran;

                        // The identifier defines the AI type and they are set in the database //
                        switch(slotptr->GetControlIdentifier()){
                            case 1: GameAI->ExecuteOnModule("BallTrackerAI", scriptargs, ran); break;
                            case 2: GameAI->ExecuteOnModule("CombinedAI", scriptargs, ran); break;
                            case 0: default:
                                GameAI->ExecuteOnModule("SimpleAI", scriptargs, ran);
                        }
                    }

                }

                slotptr = slotptr->GetSplit();
            }
        }

        // Check if ball is too far away (also check if it is vertically stuck or horizontally) //

        Leviathan::BasePhysicsObject* castedptr = dynamic_cast<Leviathan::BasePhysicsObject*>(
            GameArena->GetBallPtr().get());

        Float3 ballcurpos = castedptr->GetPos();

        if(ballcurpos.HAddAbs() > 100 * BASE_ARENASCALE){

            _DisposeOldBall();

            // Serve new ball //
            GameArena->ServeBall();
        }

        // Check is the ball stuck on the dead axis (where no paddle can hit it) //
        Float3 ballspeed = castedptr->GetBodyVelocity();
        ballspeed.X = abs(ballspeed.X);
        ballspeed.Y = 0;
        ballspeed.Z = abs(ballspeed.Z);
        ballspeed = ballspeed.Normalize();

        if(DeadAxis.HAddAbs() != 0 && ballspeed.HAddAbs() > BALLSTUCK_THRESHOLD){
            // Compare directions //

            float veldifference = (ballspeed - DeadAxis).HAddAbs();

            if(veldifference < BALLSTUCK_THRESHOLD){

                StuckThresshold++;

                if(StuckThresshold >= BALLSTUCK_COUNT){
                    // Check is ball in a goal area //
                    if(IsBallInGoalArea()){
                        StuckThresshold = 0;
                    } else {
                        Logger::Get()->Info(L"Ball stuck!");

                        _DisposeOldBall();
                        // Serve new ball //
                        GameArena->ServeBall();
                    }
                }
            } else {
                if(StuckThresshold >= 1)
                    StuckThresshold--;
            }
        }


        // Give the ball more speed //
        GameArena->GiveBallSpeed(1.00001f);
    }
}
// ------------------------------------ //
void Pong::PongServer::CheckGameConfigurationVariables(GameConfiguration* configobj){
	// Check for various variables //
	GUARD_LOCK_OTHER_OBJECT_NAME(configobj, lockit);

	NamedVars* vars = configobj->AccessVariables(lockit);

	// Default server port //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<int>(L"DefaultServerPort")){
		// Add new //
		vars->AddVar(L"DefaultServerPort", new VariableBlock(int(53221)));
		configobj->MarkModified();
	}

	// Game configuration database //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<wstring>(L"GameDatabase")){
		// Add new //
		vars->AddVar(L"GameDatabase", new VariableBlock(wstring(L"PongGameDatabase.txt")));
		configobj->MarkModified();
	}

}

void Pong::PongServer::CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}
// ------------------------------------ //
void Pong::PongServer::TryStartMatch(){
	// The world is already setup and all the players are ready at this point, just setup some cheap final values //

	auto split0 = _PlayerList[0]->GetSplit();
	auto split1 = _PlayerList[1]->GetSplit();
	auto split2 = _PlayerList[2]->GetSplit();
	auto split3 = _PlayerList[3]->GetSplit();
	// Setup dead angle //
	DeadAxis = Float3(0.f);

	if(!_PlayerList[0]->IsSlotActive() && !_PlayerList[2]->IsSlotActive() && (split0 ? !split0->IsSlotActive() : true) && (split2 ? 
		!split2->IsSlotActive() : true))
	{

		DeadAxis = Float3(1.f, 0.f, 0.f);

	} else if(!_PlayerList[1]->IsSlotActive() && !_PlayerList[3]->IsSlotActive() && (split1 ? !split1->IsSlotActive() : true) && (split3 ? 
		!split3->IsSlotActive() : true))
	{

		DeadAxis = Float3(0.f, 0.f, 1.f);
	}

	// send start event //
	Leviathan::EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"GameStart"));

	// Set the camera location //
	auto cam = Engine::GetEngine()->GetWindowEntity()->GetLinkedCamera();
	cam->SetPos(Float3(0.f, 22.f*BASE_ARENASCALE, 0.f));
	cam->SetRotation(Float3(0.f, -90.f, 0.f));

	// now that we are ready to start let's serve the ball //
	GameArena->ServeBall();
}

void Pong::PongServer::CheckForGameEnd(){
	// Look through all players and see if any team/player has reached score limit // //
	for(size_t i = 0; i < _PlayerList.Size(); i++){

		PlayerSlot* slotptr = _PlayerList[i];

		int totalteamscore = 0;

		while(slotptr){

			totalteamscore += slotptr->GetScore();
			slotptr = slotptr->GetSplit();
		}

		if(totalteamscore >= ScoreLimit){
			// Team has won //
			Logger::Get()->Info(L"Team "+Convert::ToWstring(i)+L" has won the match!");


			// Do various activities related to winning the game //

			// Set the camera location //
			auto cam = Engine::GetEngine()->GetWindowEntity()->GetLinkedCamera();

			switch(i){
			case 0:
				{
					cam->SetPos(Float3(4.f*BASE_ARENASCALE, 2.f*BASE_ARENASCALE, 0.f));
					cam->SetRotation(Float3(-90.f, -30.f, 0.f));
				}
				break;
			case 1:
				{
					cam->SetPos(Float3(0.f, 2.f*BASE_ARENASCALE, 4.f*BASE_ARENASCALE));
					cam->SetRotation(Float3(-180.f, -30.f, 0.f));
				}
				break;
			case 2:
				{
					cam->SetPos(Float3(-4.f*BASE_ARENASCALE, 2.f*BASE_ARENASCALE, 0.f));
					cam->SetRotation(Float3(90.f, -30.f, 0.f));
				}
				break;
			case 3:
				{
					cam->SetPos(Float3(0.f, 2.f*BASE_ARENASCALE, 4.f*BASE_ARENASCALE));
					cam->SetRotation(Float3(0.f, -30.f, 0.f));
				}
				break;
			}

			// Send the game end event which should trigger proper menus //
			Leviathan::EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(new wstring(L"MatchEnded"),
                    new NamedVars(shared_ptr<NamedVariableList>(new NamedVariableList(L"WinningTeam",
                                new Leviathan::VariableBlock((int)i))))));

			// And finally destroy the ball //
			GameArena->LetGoOfBall();

			// (Don't block input so players can wiggle around //


			return;
		}
	}
}

void Pong::PongServer::ServerCheckEnd(){
	CheckForGameEnd();
}
// ------------------------------------ //
void Pong::PongServer::DoSpecialPostLoad(){

	_PongServerNetworking = dynamic_cast<PongServerNetworking*>(Leviathan::NetworkHandler::GetInterface());


	// Setup receiving networked controls from players //
	ServerInputHandler = shared_ptr<GameInputController>(new GameInputController());
	_PongServerNetworking->RegisterNetworkedInput(ServerInputHandler);

	// Create all the server variables //
	Leviathan::SyncedVariables* tmpvars = Leviathan::SyncedVariables::Get();

	tmpvars->AddNewVariable(shared_ptr<SyncedValue>(new SyncedValue(new NamedVariableList(L"TheAnswer",
                    new VariableBlock(42)))));

    GameArena->VerifyTrail();
}

void Pong::PongServer::CustomizedGameEnd(){
	// Tell all clients to go to score screen //

}
// ------------------------------------ //
bool Pong::PongServer::MoreCustomScriptTypes(asIScriptEngine* engine){

    if(engine->RegisterObjectType("PongServer", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		SCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterGlobalFunction("PongServer@ GetPongServer()", asFUNCTION(PongServer::Get), asCALL_CDECL) < 0){
		SCRIPT_REGISTERFAIL;
	}

    
    if(engine->RegisterObjectMethod("PongServer", "void GameMatchEnded()", asMETHOD(PongServer, GameMatchEnded),
            asCALL_THISCALL) < 0)
    {
        SCRIPT_REGISTERFAIL;
    }

    
    return true;
}

void Pong::PongServer::MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids){
    typeids.insert(make_pair(engine->GetTypeIdByDecl("PongServer"), L"PongServer"));
}

void Pong::PongServer::PreFirstTick(){
	auto casted = static_cast<PongServerNetworking*>(Leviathan::NetworkHandler::GetInterface());
	casted->SetServerAllowPlayers(true);
	casted->SetServerStatus(Leviathan::NETWORKRESPONSE_SERVERSTATUS_RUNNING);

	// Don't want to forget to call this //
	_Engine->PreFirstTick();
}

void Pong::PongServer::PassCommandLine(const wstring &params){
	// Add "--nogui" if not found //
	if(params.find(L"--nogui") == wstring::npos){

		_Engine->PassCommandLine(params+L" --nogui");
		return;
	}

	// Now pass it //
	_Engine->PassCommandLine(params);

}
// ------------------------------------ //
void Pong::PongServer::OnStartPreMatch(){

    // Setup the world first as that can fail //
	WorldOfPong->ClearObjects();
	WorldOfPong->SetWorldPhysicsFrozenState(true);

	// Setup the objects in the world //
	if(!GameArena->GenerateArena(this, _PlayerList)){

        Logger::Get()->Warning(L"PongServer: failed to generate arena");
        return;
    }

	// Notify the clients //
	_PongServerNetworking->SetStatus(PONG_JOINGAMERESPONSE_TYPE_PREMATCH);

    
	// Make sure that everyone is receiving our world //
    // This will send many objects at once to all the players (or rather should send them in bulk)
	_PongServerNetworking->VerifyWorldIsSyncedWithPlayers(WorldOfPong);


	// Queue a readyness checking task //
	ThreadingManager::Get()->QueueTask(new ConditionalTask(boost::bind<void>([](PongServer* server) -> void
        {

            Logger::Get()->Info(L"All players are synced, the match is ready to begin");
            
            // Start the match //
            server->WorldOfPong->SetWorldPhysicsFrozenState(false);
            server->_PongServerNetworking->SetStatus(PONG_JOINGAMERESPONSE_TYPE_MATCH);

            // Spawn a ball //
            server->GameArena->ServeBall();

            // TODO: add a start timer here


        }, this), boost::bind<bool>([](shared_ptr<GameWorld> world) -> bool
            {
                // We are ready to start once all clients are reported to be up to date by the world //
                return world->AreAllPlayersSynced();

            }, WorldOfPong)));

	// Clear all other sorts of data like scores etc. //


}
