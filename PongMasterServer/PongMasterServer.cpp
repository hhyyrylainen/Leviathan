#include "PongMasterServerIncludes.h"
// ------------------------------------ //
#ifndef PONG_MASTERSERVER
#include "PongMasterServer.h"
#endif
#include "Common\DataStoring\NamedVars.h"
#include "Application\GameConfiguration.h"
using namespace Pong;
// ------------------------------------ //
Pong::PongMasterServer::PongMasterServer(){

}

Pong::PongMasterServer::~PongMasterServer(){

}

std::wstring Pong::PongMasterServer::GenerateWindowTitle(){
	return wstring(L"PongMasterServer for version " GAME_VERSIONS L" Leviathan " LEVIATHAN_VERSIONS);
}
// ------------------------------------ //
void Pong::PongMasterServer::Tick(int mspassed){

}
// ------------------------------------ //
void Pong::PongMasterServer::CustomizeEnginePostLoad(){

}

void Pong::PongMasterServer::EnginePreShutdown(){

}
// ------------------------------------ //
void Pong::PongMasterServer::InitLoadCustomScriptTypes(asIScriptEngine* engine){

}

void Pong::PongMasterServer::RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){

}
// ------------------------------------ //
void Pong::PongMasterServer::RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager){

}
// ------------------------------------ //
void Pong::PongMasterServer::CheckGameConfigurationVariables(GameConfiguration* configobj){
	// Check for various variables //

	ObjectLock lockit(*configobj);

	NamedVars* vars = configobj->AccessVariables(lockit);

	// Master server port //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<int>(L"MasterServerPort")){
		// Add new //
		vars->AddVar(L"MasterServerPort", new VariableBlock(53220));
		configobj->MarkModified();
	}
}

void Pong::PongMasterServer::CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}
