#ifndef LEVIATHAN_GAMEMODULE
#define LEVIATHAN_GAMEMODULE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Script/ScriptModule.h"
#include "Common/ThreadSafe.h"
#include "Events/EventableScriptObject.h"

namespace Leviathan{

	//! \brief Represents a scriptable part of a program
	class GameModule : public Object, public ReferenceCounted, public EventableScriptObject{
	public:

		//! \warning Quite expensive constructor since it loads the definition file
		//! \todo Make load all source files, instead of loading just the first
		DLLEXPORT GameModule(const wstring &modulename, const wstring &ownername, const wstring &extension = L"txt|levgm");
		DLLEXPORT ~GameModule();

		//! \brief Makes the scripts usable
		DLLEXPORT bool Init();

		//! \brief Releases the script
		//! 
		//! Use to release script before releasing any other objects
		DLLEXPORT void ReleaseScript();

		//! \brief Returns a string describing this module
		DLLEXPORT wstring GetDescriptionForError(bool full = false);

		// Used to actually use the module //

		//! Executes something on the module and returns the result. Adds the module as first parameter and existed is set to true if something was executed
		DLLEXPORT shared_ptr<VariableBlock> ExecuteOnModule(const string &entrypoint, std::vector<shared_ptr<NamedVariableBlock>> &otherparams, 
			bool &existed, bool fulldeclaration = false);


		// Script proxies //
		DLLEXPORT string GetDescriptionProxy(bool full);


		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GameModule);

	private:

		void _CallScriptListener(Event** pEvent, GenericEvent** event2);
		// ------------------------------------ //

		wstring OwnerName;
		wstring LoadedFromFile;

		shared_ptr<ScriptModule> ScriptMain;

		// Properties loaded from the file //
		wstring Name;
		wstring Version;
		wstring SourceFile;

	};

}
#endif