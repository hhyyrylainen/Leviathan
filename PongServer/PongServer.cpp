#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_SERVER
#include "PongServer.h"
#endif
#include "Common/DataStoring/NamedVars.h"
#include "Application/GameConfiguration.h"
using namespace Pong;
// ------------------------------------ //
Pong::PongServer::PongServer(){

}

Pong::PongServer::~PongServer(){

}

std::wstring Pong::PongServer::GenerateWindowTitle(){
	return wstring(L"PongServer for version " GAME_VERSIONS L" Leviathan " LEVIATHAN_VERSIONS);
}
// ------------------------------------ //
void Pong::PongServer::Tick(int mspassed){

}
// ------------------------------------ //
void Pong::PongServer::CheckGameConfigurationVariables(GameConfiguration* configobj){
	// Check for various variables //

	ObjectLock lockit(*configobj);

	NamedVars* vars = configobj->AccessVariables(lockit);

	// Master server port //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<int>(L"ServerPort")){
		// Add new //
		vars->AddVar(L"ServerPort", new VariableBlock(53221));
		configobj->MarkModified();
	}
}

void Pong::PongServer::CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}
// ------------------------------------ //
void Pong::PongServer::TryStartMatch(){
	// Destroy old game world //
	GameArena->GetWorld()->ClearObjects();
	GamePaused = false;

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
			//! \todo send error //
			return;
		}
	} catch(const Ogre::InvalidParametersException &e){
		//! \todo send error //
		return;
	}
	auto split0 = PlayerList[0]->GetSplit();
	auto split1 = PlayerList[1]->GetSplit();
	auto split2 = PlayerList[2]->GetSplit();
	auto split3 = PlayerList[3]->GetSplit();
	// Setup dead angle //
	DeadAxis = Float3(0.f);

	if(!PlayerList[0]->IsSlotActive() && !PlayerList[2]->IsSlotActive() && (split0 ? !split0->IsSlotActive(): true) && (split2 ? !split2->IsSlotActive(): true)){

		DeadAxis = Float3(1.f, 0.f, 0.f);

	} else if(!PlayerList[1]->IsSlotActive() && !PlayerList[3]->IsSlotActive() && (split1 ? !split1->IsSlotActive(): true) && (split3 ? !split3->IsSlotActive(): true)){

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
}

void Pong::PongServer::CheckForGameEnd(){
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

void Pong::PongServer::ServerCheckEnd(){
	CheckForGameEnd();
}
// ------------------------------------ //
void Pong::PongServer::DoSpecialPostLoad(){

}

void Pong::PongServer::CustomizedGameEnd(){
	// Tell all clients to go to score screen //

}
// ------------------------------------ //
void Pong::PongServer::MoreCustomScriptTypes(asIScriptEngine* engine){

}

void Pong::PongServer::MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids){

}
