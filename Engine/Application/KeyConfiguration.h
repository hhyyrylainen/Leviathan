#ifndef LEVIATHAN_KEYCONFIGURATION
#define LEVIATHAN_KEYCONFIGURATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input/Key.h"
#include <boost/function.hpp>
#include "Common/ThreadSafe.h"
#include <boost/bimap.hpp>


namespace Leviathan{

	class KeyConfiguration : public ThreadSafe{
	public:
		// The file is the file which is used for saving and loading keys //
		DLLEXPORT KeyConfiguration(const wstring &configfile);
		DLLEXPORT ~KeyConfiguration();

		// Loads the defined keys from a file, the function argument is called to verify that all required keys are defined
		// and it can add missing keys
		DLLEXPORT bool Init(boost::function<void (KeyConfiguration* checkfrom)> functocheck);
		// Saves all modified keys //
		DLLEXPORT void Release();

		// Saves current keys //
		DLLEXPORT void Save();

		// Resolves a control key string ("WalkForward") to a key (OIS::KC_W and modifiers SHIFT) //
		DLLEXPORT shared_ptr<std::vector<GKey>> ResolveControlNameToKey(const wstring &controlkey);

		// Checks which configuration key string matches input key
		// Warning this is quite costly function //
		DLLEXPORT wstring ResolveKeyToControlName(const GKey &key);


		DLLEXPORT static KeyConfiguration* Get();
	private:

		wstring KeyStorageFile;

		//// The keys are stored in a bidirectional map to allow searching either way through the map //
		// Actually we should support as many keys for a control as possible //
		std::map<wstring, shared_ptr<std::vector<GKey>>> KeyConfigurations;

		static KeyConfiguration* staticaccess;
	};

}
#endif
