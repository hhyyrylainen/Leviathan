#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_GAME
#include "PongGame.h"
#endif
#include "Entities\Objects\ViewerCameraPos.h"
#include "Entities\GameWorld.h"
#include "Entities\Objects\Prop.h"
#include "..\Engine\Script\ScriptExecutor.h"
#include "Arena.h"
using namespace Pong;
using namespace Leviathan;
// ------------------------------------ //
Pong::PongGame::PongGame() : GameArena(nullptr), ErrorState("No error"), PlayerList(4), Tickcount(0), LastPlayerHitBallID(-1), ScoreLimit(20),
	BallLastPos(0.f), DeadAxis(0.f), StuckThresshold(0), GameConfigurationData("GameConfiguration")
{
	StaticAccess = this;

	GameInputHandler = new GameInputController();

	// fill the player list with the player 1 and empty slots //
	PlayerList[0] = new PlayerSlot(0, PLAYERTYPE_HUMAN, 1, PLAYERCONTROLS_WASD, 0, Float4(1.f, 0.f, 0.f, 1.f));
	PlayerList[1] = new PlayerSlot(1, true);
	PlayerList[2] = new PlayerSlot(2, PLAYERTYPE_HUMAN, 3, PLAYERCONTROLS_ARROWS, 0, Float4(0.f, 1.f, 0.f, 1.f));

	// other slots as empty //
	for(size_t i = 3; i < PlayerList.size(); i++){

		PlayerList[i] = new PlayerSlot(i, true);
	}

	// Setup match setup screen data //
	GameConfigurationData.AddValue(L"Colours", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Colour", shared_ptr<VariableBlock>(new VariableBlock(string("rgb(0,0,255)"))))
		(L"Name", shared_ptr<VariableBlock>(new VariableBlock(string("Blue")))))));
	GameConfigurationData.AddValue(L"Colours", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Colour", shared_ptr<VariableBlock>(new VariableBlock(string("rgb(255,0,0)"))))
		(L"Name", shared_ptr<VariableBlock>(new VariableBlock(string("Red")))))));
	GameConfigurationData.AddValue(L"Colours", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Colour", shared_ptr<VariableBlock>(new VariableBlock(string("rgb(0,255,0)"))))
		(L"Name", shared_ptr<VariableBlock>(new VariableBlock(string("Green")))))));

	// Add base controls //
	GameConfigurationData.AddValue(L"Controls", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Type", shared_ptr<VariableBlock>(new VariableBlock(string("ARROWS"))))
		(L"ID", shared_ptr<VariableBlock>(new VariableBlock(string("0")))))));
	GameConfigurationData.AddValue(L"Controls", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Type", shared_ptr<VariableBlock>(new VariableBlock(string("WASD"))))
		(L"ID", shared_ptr<VariableBlock>(new VariableBlock(string("0")))))));
	GameConfigurationData.AddValue(L"Controls", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Type", shared_ptr<VariableBlock>(new VariableBlock(string("AI"))))
		(L"ID", shared_ptr<VariableBlock>(new VariableBlock(string("0")))))));
	GameConfigurationData.AddValue(L"Controls", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Type", shared_ptr<VariableBlock>(new VariableBlock(string("IJKL"))))
		(L"ID", shared_ptr<VariableBlock>(new VariableBlock(string("0")))))));
	GameConfigurationData.AddValue(L"Controls", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Type", shared_ptr<VariableBlock>(new VariableBlock(string("NUMPAD"))))
		(L"ID", shared_ptr<VariableBlock>(new VariableBlock(string("0")))))));
	GameConfigurationData.AddValue(L"Controls", shared_ptr<Leviathan::SimpleDatabaseRowObject>(new Leviathan::SimpleDatabaseRowObject(
		boost::assign::map_list_of
		(L"Type", shared_ptr<VariableBlock>(new VariableBlock(string("CONTROLLER"))))
		(L"ID", shared_ptr<VariableBlock>(new VariableBlock(string("0")))))));
}

Pong::PongGame::~PongGame(){
	// delete memory //
	SAFE_DELETE(GameInputHandler);
	SAFE_DELETE_VECTOR(PlayerList);
}
// ------------------------------------ //
void Pong::PongGame::CustomizeEnginePostLoad(){
	// load GUI documents //

	Gui::GuiManager* manager = Engine::GetEngine()->GetWindowEntity()->GetGUI();

	manager->LoadGUIFile(FileSystem::GetScriptsFolder()+L"PongMenus.txt");

//#ifdef _DEBUG
	// load debug panel, too //

	manager->LoadGUIFile(FileSystem::GetScriptsFolder()+L"DebugPanel.txt");
//#endif // _DEBUG

	manager->SetMouseFile(FileSystem::GetScriptsFolder()+L"cursor.rml");

	// setup world //
	shared_ptr<GameWorld> world1 = Engine::GetEngine()->CreateWorld();

	// set skybox to have some sort of visuals //
	world1->SetSkyBox("NiceDaySky");

	// create playing field manager with the world //
	GameArena = unique_ptr<Arena>(new Arena(world1));

	ObjectLoader* loader = Engine::GetEngine()->GetObjectLoader();


	// camera //
	shared_ptr<ViewerCameraPos> MainCamera(new ViewerCameraPos());
	MainCamera->SetPos(Float3(0.f, 22.f*BASE_ARENASCALE, 0.f));

	// camera should always point down towards the play field //
	MainCamera->SetRotation(Float3(0.f, -90.f, 0.f));



	// link world and camera to a window //
	GraphicalInputEntity* window1 = Engine::GetEngine()->GetWindowEntity();

	window1->LinkObjects(MainCamera, world1);
	// sound listening camera //
	MainCamera->BecomeSoundPerceiver();

	// link window input to game logic //
	window1->GetInputController()->LinkReceiver(GameInputHandler);

	// I like the debugger //
#ifdef _DEBUG
	window1->GetGUI()->SetDebuggerOnThisContext();
	window1->GetGUI()->SetDebuggerVisibility(true);
#endif // _DEBUG

	
	// after loading reset time sensitive timers //
	Engine::GetEngine()->ResetPhysicsTime();
}

std::wstring Pong::PongGame::GenerateWindowTitle(){
	return wstring(L"Pong version " GAME_VERSIONS L" Leviathan " LEVIATHAN_VERSIONS);
}
// ------------------------------------ //
void Pong::PongGame::InitLoadCustomScriptTypes(asIScriptEngine* engine){

	// register PongGame type //
	if(engine->RegisterObjectType("PongGame", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		SCRIPT_REGISTERFAIL;
	}

	// get function //
	if(engine->RegisterGlobalFunction("PongGame@ GetPongGame()", asFUNCTION(PongGame::Get), asCALL_CDECL) < 0){
		SCRIPT_REGISTERFAIL;
	}

	// functions //
	if(engine->RegisterObjectMethod("PongGame", "int StartGame()", asMETHOD(PongGame, TryStartGame), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PongGame", "void Quit()", asMETHOD(PongGame, ScriptCloseGame), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PongGame", "string GetErrorString()", asMETHOD(PongGame, GetErrorString), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PongGame", "void GameMatchEnded()", asMETHOD(PongGame, GameMatchEnded), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	// For getting the game database //
	if(engine->RegisterObjectMethod("PongGame", "SimpleDatabase& GetGameDatabase()", asMETHOD(PongGame, GetGameDatabase), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	
	// Type enums //
	if(engine->RegisterEnum("PLAYERTYPE") < 0){
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterEnumValue("EVENT_TYPE", "PLAYERTYPE_CLOSED", PLAYERTYPE_CLOSED) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterEnumValue("EVENT_TYPE", "PLAYERTYPE_COMPUTER", PLAYERTYPE_COMPUTER) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterEnumValue("EVENT_TYPE", "PLAYERTYPE_EMPTY", PLAYERTYPE_EMPTY) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterEnumValue("EVENT_TYPE", "PLAYERTYPE_HUMAN", PLAYERTYPE_HUMAN) < 0)
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

	// PlayerSlot //
	if(engine->RegisterObjectType("PlayerSlot", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		SCRIPT_REGISTERFAIL;
	}

	// get function //
	if(engine->RegisterObjectMethod("PongGame", "PlayerSlot@ GetSlot(int number)", asMETHOD(PongGame, GetPlayerSlot), asCALL_THISCALL) < 0){
		SCRIPT_REGISTERFAIL;
	}

	// functions //
	if(engine->RegisterObjectMethod("PlayerSlot", "bool IsActive()", asMETHOD(PlayerSlot, IsSlotActive), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PlayerSlot", "int GetPlayerNumber()", asMETHOD(PlayerSlot, GetPlayerIdentifier), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PlayerSlot", "int GetScore()", asMETHOD(PlayerSlot, GetScore), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PlayerSlot", "PlayerSlot@ GetSplit()", asMETHOD(PlayerSlot, GetSplit), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("PlayerSlot", "PLAYERCONTROLS GetControlType()", asMETHOD(PlayerSlot, GetControlType), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("PlayerSlot", "void SetColourFromRML(string rmlcolour)", asMETHOD(PlayerSlot, SetColourFromRML), asCALL_THISCALL) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	

	
	


	// static functions //

}

void Pong::PongGame::RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){
	// we have registered just a one type, add it //
	typeids.insert(make_pair(engine->GetTypeIdByDecl("PongGame"), L"PongGame"));
	typeids.insert(make_pair(engine->GetTypeIdByDecl("PlayerSlot"), L"PlayerSlot"));
}
// ------------------------------------ //
PongGame* Pong::PongGame::Get(){
	return StaticAccess;
}

int Pong::PongGame::TryStartGame(){
	// Destroy old game world //
	GameArena->GetWorld()->ClearObjects();

	int activeplycount = 0;
	int maxsplit = 0;
	for(size_t i = 0; i < PlayerList.size(); i++){
		PlayerList[i]->SetScore(0);
		if(PlayerList[i]->IsSlotActive())
			activeplycount++;
		int split = PlayerList[i]->GetSplitCount();
		if(PlayerList[i]->GetSplit())
			PlayerList[i]->GetSplit()->SetScore(0);
		if(split > maxsplit)
			maxsplit = split;
	}
	try{
		if(!GameArena->GenerateArena(this, PlayerList, activeplycount, maxsplit, true)){

			return -3;
		}
	} catch(const Ogre::InvalidParametersException &e){

		return -3;
	}

	GameInputHandler->StartReceivingInput(PlayerList);
	GameInputHandler->SetBlockState(false);

	// Setup dead angle //
	if(!PlayerList[0]->IsSlotActive() && !PlayerList[2]->IsSlotActive()){

		DeadAxis = Float3(1.f, 0.f, 0.f);

	} else if(!PlayerList[1]->IsSlotActive() && !PlayerList[3]->IsSlotActive()){

		DeadAxis = Float3(0.f, 0.f, 1.f);
	}

	// send start event //
	Leviathan::EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(new wstring(L"GameStart"), new NamedVars(shared_ptr<NamedVariableList>(new
		NamedVariableList(L"PlayerCount", new Leviathan::VariableBlock(activeplycount))))));

	// Set the camera location //
	auto cam = Engine::GetEngine()->GetWindowEntity()->GetLinkedCamera();
	cam->SetPos(Float3(0.f, 22.f*BASE_ARENASCALE, 0.f));
	cam->SetRotation(Float3(0.f, -90.f, 0.f));

	// now that we are ready to start let's serve the ball //
	GameArena->ServeBall();

	// succeeded //
	return 1;
}

void Pong::PongGame::GameMatchEnded(){
	GameInputHandler->UnlinkPlayers();
	GameInputHandler->SetBlockState(true);


}

void Pong::PongGame::ScriptCloseGame(){
	Engine::GetEngine()->GetWindowEntity()->GetWindow()->SendCloseMessage();
}

string Pong::PongGame::GetErrorString(){
	return ErrorState;
}

PlayerSlot* Pong::PongGame::GetPlayerSlot(int id){
	return PlayerList[id];
}

void Pong::PongGame::RegisterApplicationPhysicalMaterials(PhysicsMaterialManager* manager){
	// TODO: implement loading from files //

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

void Pong::PongGame::Tick(int mspassed){
	Tickcount++;
	// Let the AI think //


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
// ------------------ Physics callbacks for game logic ------------------ //
void Pong::PongGame::BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep, int threadIndex){

	// Call the callback //
	StaticAccess->_SetLastPaddleHit(reinterpret_cast<Leviathan::BasePhysicsObject*>(
		NewtonBodyGetUserData(NewtonJointGetBody0(contact))), reinterpret_cast<Leviathan::BasePhysicsObject*>(
		NewtonBodyGetUserData(NewtonJointGetBody1(contact))));
}

void Pong::PongGame::BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep, int threadIndex){
	// Call the function and set the collision state as the last one //
	NewtonJointSetCollisionState(contact, StaticAccess->_BallEnterGoalArea(reinterpret_cast<Leviathan::BasePhysicsObject*>(
		NewtonBodyGetUserData(NewtonJointGetBody0(contact))), reinterpret_cast<Leviathan::BasePhysicsObject*>(
		NewtonBodyGetUserData(NewtonJointGetBody1(contact)))));
}

void Pong::PongGame::_SetLastPaddleHit(Leviathan::BasePhysicsObject* objptr, Leviathan::BasePhysicsObject* objptr2){
	// Note: the object pointers can be in any order they want //

	Leviathan::BasePhysicsObject* realballptr = dynamic_cast<Leviathan::BasePhysicsObject*>(GameArena->GetBallPtr().get());

	// Look through all players and compare paddle ptrs //
	for(size_t i = 0; i < PlayerList.size(); i++){

		PlayerSlot* slotptr = PlayerList[i];

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

int Pong::PongGame::_BallEnterGoalArea(Leviathan::BasePhysicsObject* goal, Leviathan::BasePhysicsObject* ballobject){
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

int Pong::PongGame::PlayerScored(Leviathan::BasePhysicsObject* goalptr){
	// Don't count if the player whose goal the ball is in is the last one to touch it or if none have touched it //
	if(PlayerIDMatchesGoalAreaID(LastPlayerHitBallID, goalptr) || LastPlayerHitBallID == -1){

		return 1;
	}

	// Add point to player who scored //

	// Look through all players and compare PlayerIDs //
	for(size_t i = 0; i < PlayerList.size(); i++){

		PlayerSlot* slotptr = PlayerList[i];

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
	CheckForGameEnd();

	return 0;
}

bool Pong::PongGame::PlayerIDMatchesGoalAreaID(int plyid, Leviathan::BasePhysicsObject* goalptr){
	// Look through all players and compare find the right PlayerID and compare goal area ptr //
	for(size_t i = 0; i < PlayerList.size(); i++){

		PlayerSlot* slotptr = PlayerList[i];

		while(slotptr){

			if(plyid == slotptr->GetPlayerIdentifier()){
				// Check if goal area matches //
				Leviathan::BasePhysicsObject* tmpptr = dynamic_cast<Leviathan::BasePhysicsObject*>(PlayerList[i]->GetGoalArea().get());
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

int Pong::PongGame::GetScoreLimit(){
	return ScoreLimit;
}

void Pong::PongGame::SetScoreLimit(int scorelimit){
	ScoreLimit = scorelimit;
}

void Pong::PongGame::CheckForGameEnd(){
	// Look through all players and see if any team/player has reached score limit // //
	for(size_t i = 0; i < PlayerList.size(); i++){

		PlayerSlot* slotptr = PlayerList[i];

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
			Leviathan::EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(new wstring(L"MatchEnded"), new NamedVars(shared_ptr<NamedVariableList>(new
				NamedVariableList(L"WinningTeam", new Leviathan::VariableBlock((int)i))))));

			// And finally destroy the ball //
			GameArena->LetGoOfBall();

			// (Don't block input so players can wiggle around //


			return;
		}
	}
}
// ------------------------------------ //
void Pong::PongGame::SetBallLastHitColour(){
	// Find the player with the last hit identifier and apply that player's colour //
	for(size_t i = 0; i < PlayerList.size(); i++){

		PlayerSlot* slotptr = PlayerList[i];

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

void Pong::PongGame::_DisposeOldBall(){

	// Tell arena to let go of old ball //
	GameArena->LetGoOfBall();

	// Reset variables //
	LastPlayerHitBallID = -1;
	StuckThresshold = 0;
	// This should reset the ball trail colour //
	SetBallLastHitColour();
}

bool Pong::PongGame::IsBallInGoalArea(){
	// Tell arena to handle this //
	return GameArena->IsBallInPaddleArea();
}

Leviathan::SimpleDatabase* Pong::PongGame::GetGameDatabase(){
	return &GameConfigurationData;
}










PongGame* Pong::PongGame::StaticAccess = NULL;
