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

#define GAMECONFIGURATION_GET_VARIABLEACCESS(x) 	NamedVars* x = NULL; unique_ptr<ObjectLock> varlockaccess; {GameConfiguration* configvar = GameConfiguration::Get(); \
	if(configvar != NULL){ varlockaccess = unique_ptr<ObjectLock>(new ObjectLock(configvar->ObjectsLock)); x = configvar->AccessVariables(*varlockaccess.get());}}


	class GameConfiguration : public ThreadSafe{
	public:
		DLLEXPORT GameConfiguration(const wstring &configfile);
		DLLEXPORT ~GameConfiguration();


		//! This function loads the files from the defined file and calls the argument function afterwards
		//! to verify that all requires values are set
		DLLEXPORT bool Init(boost::function<void (GameConfiguration* configobj)> sanitycheckcallback);

		//! Tries to save the changes
		DLLEXPORT void Release();

		//! Saves current values (if marked as unsaved)
		DLLEXPORT void SaveCheck();

		//! Gets the values
		//! \note you need to have locked this object while and after calling this
		//! \pre Call GAMECONFIGURATION_GET_VARIABLEACCESS(variables); to use this
		DLLEXPORT NamedVars* AccessVariables(ObjectLock &guard);

		//! Call this when you have changed variables
		DLLEXPORT void MarkModified();

		//! Verifies that the global default values are added properly
		//! \note This doesn't need to be called manually as it is called by Init
		DLLEXPORT void VerifyGlobalVariables();

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