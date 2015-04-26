// ------------------------------------ //
#include "GameConfiguration.h"

#include "FileSystem.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::GameConfiguration::GameConfiguration(const string &configfile) :
    GameConfigFile(configfile), Modified(false), GameVars(NULL)
{
	staticaccess = this;
}

DLLEXPORT Leviathan::GameConfiguration::~GameConfiguration(){
	staticaccess = NULL;
}

DLLEXPORT GameConfiguration* Leviathan::GameConfiguration::Get(){
	return staticaccess;
}

GameConfiguration* Leviathan::GameConfiguration::staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameConfiguration::Init(std::function<void (GameConfiguration*
        configobj)> sanitycheckcallback)
{
	GUARD_LOCK();

	GameVars = new NamedVars();

	

	if(!GameVars->LoadVarsFromFile(GameConfigFile)){
		// Unknown error //
		Logger::Get()->Error("GameConfiguration: Unknown error from LoadVarsFromFile");
		return false;
	}

	// First verify the global variables //
	VerifyGlobalVariables();

	// Call the checking function //
	sanitycheckcallback(this);
	return true;
}

DLLEXPORT void Leviathan::GameConfiguration::Release(){
	GUARD_LOCK();
	SaveCheck();

	// We can now delete our variables //
	SAFE_DELETE(GameVars);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameConfiguration::SaveCheck(){

	string newfilecontents = "";
	// Writing to file doesn't need locking //
	{
		GUARD_LOCK();
		// If not modified we don't need to save anything //
		if(!Modified)
			return;

		// Write the variables to the file //
		auto vec = GameVars->GetVec();

		for(size_t i = 0; i < vec->size(); i++){

			newfilecontents += vec->at(i)->ToText()+"\n";
		}
        
		// No longer needs to save modified values //
		Modified = false;
	}
    
	FileSystem::WriteToFile(newfilecontents, GameConfigFile);
}

DLLEXPORT void Leviathan::GameConfiguration::MarkModified(){
	GUARD_LOCK();

	Modified = true;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::GameConfiguration::AccessVariables(Lock &guard){
	VerifyLock(guard);

	return GameVars;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameConfiguration::VerifyGlobalVariables(){
	GUARD_LOCK();

	// Socket unbind control //
	if(GameVars->ShouldAddValueIfNotFoundOrWrongType<bool>("DisableSocketUnbind")){
		// Add new //
		GameVars->AddVar("DisableSocketUnbind", new VariableBlock(false));
		MarkModified();
	}


}

