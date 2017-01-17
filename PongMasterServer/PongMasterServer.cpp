#include "PongMasterServerIncludes.h"
// ------------------------------------ //
#include "PongMasterServer.h"

#include "Common/DataStoring/NamedVars.h"
#include "Application/GameConfiguration.h"
using namespace Pong;
using namespace std;
// ------------------------------------ //
Pong::PongMasterServer::PongMasterServer(PongMasterNetworking &network) :
    MasterInterface(network)
{

}

Pong::PongMasterServer::~PongMasterServer(){

}

std::string Pong::PongMasterServer::GenerateWindowTitle(){
	return string("PongMasterServer for version " GAME_VERSIONS_ANSI " Leviathan "
        LEVIATHAN_VERSION_ANSIS);
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
bool Pong::PongMasterServer::InitLoadCustomScriptTypes(asIScriptEngine* engine){

	return true;
}

void Pong::PongMasterServer::RegisterCustomScriptTypes(asIScriptEngine* engine,
    std::map<int, string> &typeids)
{

}
// ------------------------------------ //
void Pong::PongMasterServer::RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager){

}
// ------------------------------------ //
void Pong::PongMasterServer::CheckGameConfigurationVariables(Lock &guard,
    GameConfiguration* configobj)
{
	// Check for various variables //
	NamedVars* vars = configobj->AccessVariables(guard);

	// Master server port //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<int>("MasterServerPort")){
		// Add new //
		vars->AddVar("MasterServerPort", new VariableBlock(53220));
		configobj->MarkModified(guard);
	}
}

void Pong::PongMasterServer::CheckGameKeyConfigVariables(Lock &guard,
    KeyConfiguration* keyconfigobj)
{

}
