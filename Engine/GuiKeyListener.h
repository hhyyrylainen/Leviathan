#ifndef LEVIATHAN_GUI_KEYLISTENER
#define LEVIATHAN_GUI_KEYLISTENER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "CallableObject.h"

namespace Leviathan{ namespace Gui{

	class KeyListener : public CallableObject{
	public:
		DLLEXPORT KeyListener::KeyListener();
		DLLEXPORT KeyListener::~KeyListener();

		DLLEXPORT void OnEvent(Event** pEvent);

	private:

	};

}}
#endif