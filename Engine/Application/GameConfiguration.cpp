#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GAMECONFIGURATION
#include "GameConfiguration.h"
#endif
#include "FileSystem.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameConfiguration::GameConfiguration(const wstring &configfile) : GameConfigFile(configfile), Modified(false){
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
DLLEXPORT bool Leviathan::GameConfiguration::Init(boost::function<void (GameConfiguration* configobj)> sanitycheckcallback){
	GUARD_LOCK_THIS_OBJECT();

	GameVars = new NamedVars();

	int res = GameVars->LoadVarsFromFile(GameConfigFile);

	if(res != 404 && res != 0){
		// Unknown error //
		Logger::Get()->Error(L"GameConfiguration: Unknown error from LoadVarsFromFile, result code: "+Convert::ToWstring(res));
		return false;
	}


	// Call the checking function //
	sanitycheckcallback(this);
	return true;
}

DLLEXPORT void Leviathan::GameConfiguration::Release(){
	GUARD_LOCK_THIS_OBJECT();
	SaveCheck();

	// We can now delete our variables //
	SAFE_DELETE(GameVars);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameConfiguration::SaveCheck(){

	wstring newfilecontents = L"";
	// Writing to file doesn't need locking //
	{
		GUARD_LOCK_THIS_OBJECT();
		// If not modified we don't need to save anything //
		if(!Modified)
			return;

		// Write the variables to the file //
		auto vec = GameVars->GetVec();

		for(size_t i = 0; i < vec->size(); i++){

			newfilecontents += vec->at(i)->ToText()+L"\n";
		}
		// No longer needs to save modified values //
		Modified = false;
	}
	FileSystem::WriteToFile(newfilecontents, GameConfigFile);
}

DLLEXPORT void Leviathan::GameConfiguration::MarkModified(){
	GUARD_LOCK_THIS_OBJECT();

	Modified = true;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::GameConfiguration::AccessVariables(ObjectLock &guard){
	VerifyLock(guard);

	return GameVars;
}
// ------------------------------------ //

