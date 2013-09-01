#ifndef LEVIATHAN_GUI_KEYLISTENER
#define LEVIATHAN_GUI_KEYLISTENER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Events\CallableObject.h"
#include "Input\KeyPressManager.h"

namespace Leviathan{ namespace Gui{

	class GuiManager;

	class KeyListener : public InputReceiver{
	public:
		DLLEXPORT KeyListener::KeyListener(GuiManager* owner, KeyPressManager* eventsource);
		DLLEXPORT KeyListener::~KeyListener();

		DLLEXPORT virtual bool OnEvent(InputEvent** pEvent, InputReceiver* pending);

	private:
		GuiManager* Master;
		KeyPressManager* KeySource;

	};

}}
#endif