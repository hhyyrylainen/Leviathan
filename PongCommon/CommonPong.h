#ifndef PONG_COMMON
#define PONG_COMMON
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Arena.h"
#include "PlayerSlot.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Utility/DataHandling/SimpleDatabase.h"
#include "Entities/Objects/ViewerCameraPos.h"
#include "Entities/GameWorld.h"
#include "Entities/Objects/Prop.h"
#include "Script/ScriptExecutor.h"
#include "Arena.h"
#include "Addons/GameModule.h"
#include "Threading/QueuedTask.h"
#include "add_on/autowrapper/aswrappedcall.h"
#include "Application/GameConfiguration.h"
#include "Application/Application.h"
#include "PongPackets.h"
#include "Newton/PhysicalMaterial.h"
#include "Newton/PhysicalMaterialManager.h"
#include "Networking/SyncedResource.h"

#define SCRIPT_REGISTERFAIL	Logger::Get()->Error(L"PongGame: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return;

#define BALLSTUCK_THRESHOLD		0.045f
#define BALLSTUCK_COUNT			8
#define SCOREPOINT_AMOUNT		1

namespace Pong{

	class BasePongParts;
	//! \brief Should be in BasePongParts, used for static access
	//!
	//! Why is gcc so stupid on linux that it does not allow __declspec(selectany)
	extern BasePongParts* BasepongStaticAccess;

	//! \brief A parent class for the CommonPongParts class to allow non-template use
	//!
	//! Mainly required for passing CommonPongParts to non-template functions
	class BasePongParts{
		friend Arena;
	public:

		static void StatUpdater(PlayerList* list){
			Get()->OnPlayerStatsUpdated(list);
		}


		BasePongParts(bool isserver) : GameArena(nullptr), ErrorState("No error"), Tickcount(0), LastPlayerHitBallID(-1), 
			ScoreLimit(L"ScoreLimit", 20), BallLastPos(0.f), DeadAxis(0.f), StuckThresshold(0), 
			GameConfigurationData(new Leviathan::SimpleDatabase(L"GameConfiguration")),
			GamePaused(L"GamePaused", false), GameAI(NULL), _PlayerList(boost::function<void (PlayerList*)>(&StatUpdater), 4)
		{
			BasepongStaticAccess = this;
		}

		~BasePongParts(){

			SAFE_DELETE(GameAI);
			BasepongStaticAccess = NULL;
		}



		void GameMatchEnded(){
			// This can be called from script so ensure that these are set //
			GameArena->LetGoOfBall();

			CustomizedGameEnd();
		}


		//! \brief Updates the ball trail based on the player colour
		void SetBallLastHitColour(){
			// Find the player with the last hit identifier and apply that player's colour //
			auto tmplist = _PlayerList.GetVec();

			for(size_t i = 0; i < tmplist.size(); i++){

				PlayerSlot* slotptr = tmplist[i];

				while(slotptr){

					if(LastPlayerHitBallID == slotptr->GetPlayerIdentifier()){
						// Set colour //
						GameArena->ColourTheBallTrail(slotptr->GetColour());
						return;
					}

					slotptr = slotptr->GetSplit();
				}
			}
			
			// No other colour is applied so set the default colour //
			GameArena->ColourTheBallTrail(Float4(1.f, 1.f, 1.f, 1.f));
		}


		//! \brief posts a quit message to quit after script has returned
		void ScriptCloseGame(){

			Leviathan::LeviathanApplication::GetApp()->MarkAsClosing();
		}


		//! \brief Called when scored, will handle everything
		int PlayerScored(Leviathan::BasePhysicsObject* goalptr){
			// Don't count if the player whose goal the ball is in is the last one to touch it or if none have touched it //
			if(PlayerIDMatchesGoalAreaID(LastPlayerHitBallID, goalptr) || LastPlayerHitBallID == -1){

				return 1;
			}

			// Add point to player who scored //

			// Look through all players and compare PlayerIDs //
			for(size_t i = 0; i < _PlayerList.Size(); i++){

				PlayerSlot* slotptr = _PlayerList[i];

				while(slotptr){


					if(LastPlayerHitBallID == slotptr->GetPlayerIdentifier()){
						// Found right player //
						slotptr->SetScore(slotptr->GetScore()+SCOREPOINT_AMOUNT);
						goto playrscorelistupdateendlabel;
					}

					slotptr = slotptr->GetSplit();
				}
			}
			// No players got points! //

playrscorelistupdateendlabel:


			// Send ScoreUpdated event //
			Leviathan::EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(new wstring(L"ScoreUpdated"), new NamedVars(shared_ptr<NamedVariableList>(new
				NamedVariableList(L"ScoredPlayer", new Leviathan::VariableBlock(LastPlayerHitBallID))))));

			_DisposeOldBall();

			// Serve new ball //
			GameArena->ServeBall();

			// Check for game end //
			ServerCheckEnd();

			return 0;
		}

		//! \brief Will determine if a paddle could theoretically hit the ball
		bool IsBallInGoalArea(){
			// Tell arena to handle this //
			return GameArena->IsBallInPaddleArea();
		}



		bool PlayerIDMatchesGoalAreaID(int plyid, Leviathan::BasePhysicsObject* goalptr){
			// Look through all players and compare find the right PlayerID and compare goal area ptr //
			for(size_t i = 0; i < _PlayerList.Size(); i++){

				PlayerSlot* slotptr = _PlayerList[i];

				while(slotptr){

					if(plyid == slotptr->GetPlayerIdentifier()){
						// Check if goal area matches //
						Leviathan::BasePhysicsObject* tmpptr = dynamic_cast<Leviathan::BasePhysicsObject*>(slotptr->GetGoalArea().get());
						if(tmpptr == goalptr){
							// Found matching goal area //
							return true;
						}
					}

					slotptr = slotptr->GetSplit();
				}
			}
			// Not found //
			return false;
		}


		//! \warning increases reference count
		Leviathan::Entity::Prop* GetBall(){
			auto tmp = GameArena->GetBallPtr();
			tmp->AddRef();
			return dynamic_cast<Leviathan::Entity::Prop*>(tmp.get());
		}

		Leviathan::SimpleDatabase* GetGameDatabase(){
			return GameConfigurationData.get();
		}
		Leviathan::GameWorld* GetGameWorld(){
			return WorldOfPong.get();
		}
		int GetLastHitPlayer(){
			return LastPlayerHitBallID;
		}

		// Variable set/get //
		PlayerSlot* GetPlayerSlot(int id){
			return _PlayerList[id];
		}

		void inline SetError(const string &error){
			ErrorState = error;
		}
		string GetErrorString(){
			return ErrorState;
		}

		int GetScoreLimit(){
			return ScoreLimit;
		}
		void SetScoreLimit(int scorelimit){
			ScoreLimit = scorelimit;
		}

		BaseNotifiableAll* GetPlayersAsNotifiable(){

			return static_cast<BaseNotifiableAll*>(&_PlayerList);
		}

		static BasePongParts* Get(){
			return BasepongStaticAccess;
		}

		//! This function sets the player ID who should get points for scoring //
		void _SetLastPaddleHit(Leviathan::BasePhysicsObject* objptr, Leviathan::BasePhysicsObject* objptr2){
			// Note: the object pointers can be in any order they want //

			Leviathan::BasePhysicsObject* realballptr = dynamic_cast<Leviathan::BasePhysicsObject*>(GameArena->GetBallPtr().get());

			// Look through all players and compare paddle ptrs //
			for(size_t i = 0; i < _PlayerList.Size(); i++){

				PlayerSlot* slotptr = _PlayerList[i];

				while(slotptr){

					Leviathan::BasePhysicsObject* castedptr = dynamic_cast<Leviathan::BasePhysicsObject*>(slotptr->GetPaddle().get());

					if((objptr == castedptr && objptr2 == realballptr) || (objptr2 == castedptr && objptr == realballptr)){
						// Found right player //
						LastPlayerHitBallID = slotptr->GetPlayerIdentifier();
						SetBallLastHitColour();
						return;
					}

					slotptr = slotptr->GetSplit();
				}
			}
		}
		//! Handles score increase from scoring and destruction of ball. The second parameter is used to ensuring it is the right ball //
		int _BallEnterGoalArea(Leviathan::BasePhysicsObject* goal, Leviathan::BasePhysicsObject* ballobject){
			// Note: the object pointers can be in any order they want //

			Leviathan::BasePhysicsObject* castedptr = dynamic_cast<Leviathan::BasePhysicsObject*>(GameArena->GetBallPtr().get());

			if(ballobject == castedptr){
				// goal is actually the goal area //
				return PlayerScored(goal);
			} else if(goal == castedptr){
				// ballobject is actually the goal area //
				return PlayerScored(ballobject);
			}
			return 0;
		}


	protected:


		void _DisposeOldBall(){

			// Tell arena to let go of old ball //
			GameArena->LetGoOfBall();

			// Reset variables //
			LastPlayerHitBallID = -1;
			StuckThresshold = 0;
			// This should reset the ball trail colour //
			SetBallLastHitColour();
		}

		// These should be overridden by the child class //
		virtual void DoSpecialPostLoad() = 0;
		virtual void CustomizedGameEnd() = 0;
		virtual void OnPlayerStatsUpdated(PlayerList* list) = 0;

		virtual void ServerCheckEnd(){

		}

		// ------------------------------------ //

		// game objects //
		unique_ptr<Arena> GameArena;
		shared_ptr<GameWorld> WorldOfPong;

		// AI module //
		GameModule* GameAI;

		int LastPlayerHitBallID;

		SyncedPrimitive<bool> GamePaused;
		SyncedPrimitive<int> ScoreLimit;

		//! Used to count ticks to not have to call set apply force every tick
		int Tickcount;
		//! Ball's position during last tick. This is used to see if the ball is "stuck"
		Float3 BallLastPos;
		//! Direction in which the ball can get stuck
		Float3 DeadAxis;
		int StuckThresshold;

		// Configuration data //
		shared_ptr<Leviathan::SimpleDatabase> GameConfigurationData;

		PlayerList _PlayerList;

		//! stores last error string for easy access from scripts
		string ErrorState;

	};
#ifdef _MSC_VER
//	__declspec(selectany) BasePongParts* BasePongParts::StaticAccess = NULL;
#else
	// Apparently the above thing only works on Windows targets //
//    BasePongParts* BasePongParts::StaticAccess = NULL;
#endif



	//! \brief Class that contains common functions required both by Pong and PongServer
	//!
	//! Set the program type to match the proper class LeviathanApplication or ServerApplication and the IsServer bool
	//! to match it (to make syncing with the server work)
	template<class ProgramType, bool IsServer>
	class CommonPongParts : public BasePongParts, public ProgramType{
	public:
		CommonPongParts() : BasePongParts(IsServer)
		{

		}
		~CommonPongParts(){

		}


		// These handle the common code between the server and client //
		virtual void CustomizeEnginePostLoad(){
			using namespace Leviathan;

			QUICKTIME_THISSCOPE;

			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](
				shared_ptr<Leviathan::SimpleDatabase> GameConfigurationData) -> void
			{

				wstring savefile;

				GAMECONFIGURATION_GET_VARIABLEACCESS(vars);

				assert(vars->GetValueAndConvertTo<wstring>(L"GameDatabase", savefile) && "invalid game variable configuration, no GameDatabase");

				GameConfigurationData->LoadFromFile(savefile);
				Logger::Get()->Info(L"Loaded game configuration database");

			}, GameConfigurationData))));

			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](CommonPongParts* game) -> void{
				// Load the game AI //
				game->GameAI = new GameModule(L"PongAIModule", L"PongGameCore");

				if(!game->GameAI->Init()){
					// No AI for the game //
					Logger::Get()->Error(L"Failed to load AI!");
					SAFE_DELETE(game->GameAI);
				}

			}, this))));


			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([]() -> void{
				// Load Pong specific packets //
				PongPackets::RegisterAllPongPacketTypes();
				Logger::Get()->Info(L"Pong specific packets loaded");

			}))));

			// setup world //
			WorldOfPong = Engine::GetEngine()->CreateWorld(Engine::Get()->GetWindowEntity(), NULL);

			// create playing field manager with the world //
			GameArena = unique_ptr<Arena>(new Arena(WorldOfPong));

			DoSpecialPostLoad();

			// Register the variables //
			ScoreLimit.StartSync();
			GamePaused.StartSync();
			_PlayerList.StartSync();

			// Wait for everything to finish //
			Engine::Get()->GetThreadingManager()->WaitForAllTasksToFinish();

			// after loading reset time sensitive timers //
			Engine::GetEngine()->ResetPhysicsTime();
		}

		void EnginePreShutdown(){
			// Only the AI needs this //
			if(GameAI)
				GameAI->ReleaseScript();
		}

		virtual void Tick(int mspassed){

			using namespace Leviathan;

			Tickcount++;
			// Let the AI think //
			if(GameArena->GetBallPtr() && !GamePaused){

				// Find AI slots //
				for(size_t i = 0; i < _PlayerList.Size(); i++){

					PlayerSlot* slotptr = _PlayerList[i];

					while(slotptr){

						if(slotptr->GetControlType() == PLAYERCONTROLS_AI){

							// Set the slot ptr as the argument and call function based on difficulty //
							std::vector<shared_ptr<NamedVariableBlock>> scriptargs(2);
							scriptargs[0] = shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new VoidPtrBlock(slotptr), L"PlayerSlot"));
							scriptargs[1] = shared_ptr<NamedVariableBlock>(new NamedVariableBlock(new IntBlock(mspassed), L"MSPassed"));

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

			}

			// Check if ball is too far away (also check if it is vertically stuck or horizontally) //

			if(GameArena->GetBallPtr()){
				Leviathan::BasePhysicsObject* castedptr = dynamic_cast<Leviathan::BasePhysicsObject*>(GameArena->GetBallPtr().get());

				Float3 ballcurpos = castedptr->GetPos();

				if(ballcurpos.HAddAbs() > 100*BASE_ARENASCALE){

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

					float veldifference = (ballspeed-DeadAxis).HAddAbs();

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
			}

			// Give the ball more speed //
			GameArena->GiveBallSpeed(1.00001f);
		}

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine){

			// register PongGame type //
			if(engine->RegisterObjectType("PongBase", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
				SCRIPT_REGISTERFAIL;
			}

			// get function //
			if(engine->RegisterGlobalFunction("PongBase@ GetPongBase()", WRAP_FN(&BasePongParts::Get), asCALL_GENERIC) < 0){
				SCRIPT_REGISTERFAIL;
			}

			// functions //
			if(engine->RegisterObjectMethod("PongBase", "void Quit()", WRAP_MFN(BasePongParts, ScriptCloseGame), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterObjectMethod("PongBase", "string GetErrorString()", WRAP_MFN(BasePongParts, GetErrorString), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterObjectMethod("PongBase", "void GameMatchEnded()", WRAP_MFN(BasePongParts, GameMatchEnded), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PongBase", "int GetLastHitPlayer()", WRAP_MFN(BasePongParts, GetLastHitPlayer), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			// For getting the game database //
			if(engine->RegisterObjectMethod("PongBase", "SimpleDatabase& GetGameDatabase()", WRAP_MFN(BasePongParts, GetGameDatabase), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PongBase", "GameWorld& GetGameWorld()", WRAP_MFN(BasePongParts, GetGameWorld), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PongBase", "Prop@ GetBall()", WRAP_MFN(BasePongParts, GetBall), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterObjectMethod("PongBase", "BaseNotifiableAll& GetPlayerChanges()", asMETHOD(BasePongParts, GetPlayersAsNotifiable), asCALL_THISCALL) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}



			// Type enums //
			if(engine->RegisterEnum("PLAYERTYPE") < 0){
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_CLOSED", PLAYERTYPE_CLOSED) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_COMPUTER", PLAYERTYPE_COMPUTER) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_EMPTY", PLAYERTYPE_EMPTY) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_HUMAN", PLAYERTYPE_HUMAN) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterEnum("PLAYERCONTROLS") < 0){
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_NONE", PLAYERCONTROLS_NONE) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_AI", PLAYERCONTROLS_AI) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_WASD", PLAYERCONTROLS_WASD) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_ARROWS", PLAYERCONTROLS_ARROWS) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_IJKL", PLAYERCONTROLS_IJKL) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_NUMPAD", PLAYERCONTROLS_NUMPAD) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("PLAYERCONTROLS", "PLAYERCONTROLS_CONTROLLER", PLAYERCONTROLS_CONTROLLER) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}


			if(engine->RegisterEnum("CONTROLKEYACTION") < 0){
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("CONTROLKEYACTION", "CONTROLKEYACTION_LEFT", CONTROLKEYACTION_LEFT) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("CONTROLKEYACTION", "CONTROLKEYACTION_RIGHT", CONTROLKEYACTION_RIGHT) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("CONTROLKEYACTION", "CONTROLKEYACTION_POWERUPDOWN", CONTROLKEYACTION_POWERUPDOWN) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterEnumValue("CONTROLKEYACTION", "CONTROLKEYACTION_POWERUPUP", CONTROLKEYACTION_POWERUPUP) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			// PlayerSlot //
			if(engine->RegisterObjectType("PlayerSlot", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
				SCRIPT_REGISTERFAIL;
			}

			// get function //
			if(engine->RegisterObjectMethod("PongBase", "PlayerSlot@ GetSlot(int number)", WRAP_MFN(BasePongParts, GetPlayerSlot), asCALL_GENERIC) < 0){
				SCRIPT_REGISTERFAIL;
			}

			// functions //
			if(engine->RegisterObjectMethod("PlayerSlot", "bool IsActive()", WRAP_MFN(PlayerSlot, IsSlotActive), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterObjectMethod("PlayerSlot", "PLAYERTYPE GetPlayerType()", asMETHOD(PlayerSlot, GetPlayerType), asCALL_THISCALL) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			

			if(engine->RegisterObjectMethod("PlayerSlot", "int GetPlayerNumber()", WRAP_MFN(PlayerSlot, GetPlayerIdentifier), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterObjectMethod("PlayerSlot", "int GetScore()", WRAP_MFN(PlayerSlot, GetScore), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}

			if(engine->RegisterObjectMethod("PlayerSlot", "PlayerSlot@ GetSplit()", WRAP_MFN(PlayerSlot, GetSplit), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "PLAYERCONTROLS GetControlType()", WRAP_MFN(PlayerSlot, GetControlType), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "void AddEmptySubSlot()", WRAP_MFN(PlayerSlot, AddEmptySubSlot), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "void SetControls(PLAYERCONTROLS type, int identifier)", WRAP_MFN(PlayerSlot, SetControls), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "void SetPlayerAutoID(PLAYERTYPE type)", WRAP_MFN(PlayerSlot, SetPlayerProxy), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "void PassInputAction(CONTROLKEYACTION actiontoperform, bool active)", WRAP_MFN(PlayerSlot, PassInputAction), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "bool IsVerticalSlot()", WRAP_MFN(PlayerSlot, IsVerticalSlot), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "float GetTrackProgress()", WRAP_MFN(PlayerSlot, GetTrackProgress), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "BaseObject@ GetPaddle()", WRAP_MFN(PlayerSlot, GetPaddleProxy), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "BaseObject@ GetGoalArea()", WRAP_MFN(PlayerSlot, GetGoalAreaProxy), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "TrackEntityController@ GetTrackController()", WRAP_MFN(PlayerSlot, GetTrackController), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}
			if(engine->RegisterObjectMethod("PlayerSlot", "bool DoesPlayerIDMatchThisOrParent(int id)", WRAP_MFN(PlayerSlot, DoesPlayerIDMatchThisOrParent), asCALL_GENERIC) < 0)
			{
				SCRIPT_REGISTERFAIL;
			}


			MoreCustomScriptTypes(engine);
		}

		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){
			// we have registered just a one type, add it //
			typeids.insert(make_pair(engine->GetTypeIdByDecl("PongBase"), L"PongBase"));
			typeids.insert(make_pair(engine->GetTypeIdByDecl("PlayerSlot"), L"PlayerSlot"));

			MoreCustomScriptRegister(engine, typeids);
		}

		virtual void RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager){
			// \todo implement loading from files //

			// load predefined materials //
			unique_ptr<Leviathan::PhysicalMaterial> PaddleMaterial(new Leviathan::PhysicalMaterial(L"PaddleMaterial"));
			unique_ptr<Leviathan::PhysicalMaterial> ArenaMaterial(new Leviathan::PhysicalMaterial(L"ArenaMaterial"));
			unique_ptr<Leviathan::PhysicalMaterial> ArenaBottomMaterial(new Leviathan::PhysicalMaterial(L"ArenaBottomMaterial"));
			unique_ptr<Leviathan::PhysicalMaterial> BallMaterial(new Leviathan::PhysicalMaterial(L"BallMaterial"));
			unique_ptr<Leviathan::PhysicalMaterial> GoalAreaMaterial(new Leviathan::PhysicalMaterial(L"GoalAreaMaterial"));

			// Set callbacks //
			BallMaterial->FormPairWith(*PaddleMaterial).SetSoftness(1.f).SetElasticity(1.0f).SetFriction(1.f, 1.f).
				SetCallbacks(NULL, BallContactCallbackPaddle);
			BallMaterial->FormPairWith(*GoalAreaMaterial).SetCallbacks(NULL, BallContactCallbackGoalArea);

			PaddleMaterial->FormPairWith(*GoalAreaMaterial).SetCollidable(false);
			PaddleMaterial->FormPairWith(*ArenaMaterial).SetCollidable(false).SetElasticity(0.f).SetSoftness(0.f);
			PaddleMaterial->FormPairWith(*ArenaBottomMaterial).SetCollidable(false).SetSoftness(0.f).SetFriction(0.f, 0.f).SetElasticity(0.f);
			PaddleMaterial->FormPairWith(*PaddleMaterial).SetCollidable(false);
			ArenaMaterial->FormPairWith(*GoalAreaMaterial).SetCollidable(false);
			ArenaMaterial->FormPairWith(*BallMaterial).SetFriction(0.f, 0.f).SetSoftness(1.f).SetElasticity(1.f);
			ArenaBottomMaterial->FormPairWith(*BallMaterial).SetElasticity(0.f).SetFriction(0.f, 0.f).SetSoftness(0.f);
			ArenaBottomMaterial->FormPairWith(*GoalAreaMaterial).SetCollidable(false);

			// Add the materials //
			Leviathan::PhysicsMaterialManager* tmp = Leviathan::PhysicsMaterialManager::Get();

			tmp->LoadedMaterialAdd(PaddleMaterial.release());
			tmp->LoadedMaterialAdd(ArenaMaterial.release());
			tmp->LoadedMaterialAdd(BallMaterial.release());
			tmp->LoadedMaterialAdd(GoalAreaMaterial.release());
			tmp->LoadedMaterialAdd(ArenaBottomMaterial.release());
		}
		// ------------------ Physics callbacks for game logic ------------------ //
		// Ball handling callback //
		static void BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep, int threadIndex){

			// Call the callback //
			BasepongStaticAccess->_SetLastPaddleHit(reinterpret_cast<Leviathan::BasePhysicsObject*>(
				NewtonBodyGetUserData(NewtonJointGetBody0(contact))), reinterpret_cast<Leviathan::BasePhysicsObject*>(
				NewtonBodyGetUserData(NewtonJointGetBody1(contact))));
		}
		static void BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep, int threadIndex){
			// Call the function and set the collision state as the last one //
			NewtonJointSetCollisionState(contact, BasepongStaticAccess->_BallEnterGoalArea(reinterpret_cast<Leviathan::BasePhysicsObject*>(
				NewtonBodyGetUserData(NewtonJointGetBody0(contact))), reinterpret_cast<Leviathan::BasePhysicsObject*>(
				NewtonBodyGetUserData(NewtonJointGetBody1(contact)))));
		}

	protected:

		virtual void MoreCustomScriptTypes(asIScriptEngine* engine) = 0;
		virtual void MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids) = 0;

		// ------------------------------------ //

	};

}
#endif
