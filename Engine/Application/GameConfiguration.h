#ifndef LEVIATHAN_GAMECONFIGURATION
#define LEVIATHAN_GAMECONFIGURATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/ThreadSafe.h"
#include "Common/DataStoring/NamedVars.h"
#include <boost/function.hpp>


namespace Leviathan{

#define GAMECONFIGURATION_GET_VARIABLEACCESS(x) 	GUARD_LOCK_OTHER_OBJECT_NAME(GameConfiguration::Get(), lockitc); NamedVars* x = GameConfiguration::Get()->AccessVariables(lockitc);


	class GameConfiguration : public ThreadSafe{
	public:
		DLLEXPORT GameConfiguration(const wstring &configfile);
		DLLEXPORT ~GameConfiguration();


		// This function loads the files from the defined file and calls the argument function afterwards
		// to verify that all requires values are set //
		DLLEXPORT bool Init(boost::function<void (GameConfiguration* configobj)> sanitycheckcallback);
		// Tries to save the changes //
		DLLEXPORT void Release();

		// Saves current values (if marked as unsaved) //
		DLLEXPORT void SaveCheck();

		// Gets the values. Note: you need to have locked this object while and after calling this (add ObjectLock guard(*config) to your code)
		DLLEXPORT NamedVars* AccessVariables(ObjectLock &guard);

		// Call this when you have changed variables //
		DLLEXPORT void MarkModified();


		DLLEXPORT static GameConfiguration* Get();
	private:

		wstring GameConfigFile;


		// This stores all the values and is passed around from this object to reduce bloat //
		NamedVars* GameVars;
		// Controls when to save changes //
		bool Modified;

		static GameConfiguration* staticaccess;
	};

}
#endif