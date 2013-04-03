#ifndef LEVIATHAN_KEYPRESSMANAGER
#define LEVIATHAN_KEYPRESSMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input.h"
#include "EventHandler.h"

namespace Leviathan{

	class KeyPressManager : public Object{
	public:
		DLLEXPORT KeyPressManager::KeyPressManager();
        DLLEXPORT KeyPressManager::~KeyPressManager();

		DLLEXPORT void ProcessInput(Input* input);
		DLLEXPORT void Clear();


	private:
	bool PreviousStates[256];
	bool Fetched;



	};

}
#endif