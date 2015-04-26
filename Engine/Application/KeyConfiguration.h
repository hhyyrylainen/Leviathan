#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Input/Key.h"
#include "Common/ThreadSafe.h"
#include <functional>

namespace Leviathan{

	class KeyConfiguration : public ThreadSafe{
	public:
		// The file is the file which is used for saving and loading keys //
		DLLEXPORT KeyConfiguration(const std::string &configfile);
		DLLEXPORT ~KeyConfiguration();

		// Loads the defined keys from a file, the function argument is called to
        // verify that all required keys are defined and it can add missing keys
		DLLEXPORT bool Init(std::function<void (KeyConfiguration* checkfrom)> functocheck);
		// Saves all modified keys //
		DLLEXPORT void Release();

		// Saves current keys //
		DLLEXPORT void Save();

		// Resolves a control key string ("WalkForward") to a key (OIS::KC_W and modifiers SHIFT) //
		DLLEXPORT std::shared_ptr<std::vector<GKey>> ResolveControlNameToKey(
            const std::string &controlkey);

		// Checks which configuration key string matches input key
		// Warning this is quite costly function //
		DLLEXPORT std::string ResolveKeyToControlName(const GKey &key);

		DLLEXPORT static KeyConfiguration* Get();
	private:

        std::string KeyStorageFile;

        //! Not a bidirectional map to allow multiple keys to resolve to the same things
		std::map<std::string, std::shared_ptr<std::vector<GKey>>> KeyConfigurations;

		static KeyConfiguration* staticaccess;
	};

}

