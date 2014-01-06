#include "PongServerIncludes.h"
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
void Pong::PongServer::CustomizeEnginePostLoad(){

}

void Pong::PongServer::EnginePreShutdown(){

}
// ------------------------------------ //
void Pong::PongServer::InitLoadCustomScriptTypes(asIScriptEngine* engine){

}

void Pong::PongServer::RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){

}
// ------------------------------------ //
void Pong::PongServer::RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager){

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
