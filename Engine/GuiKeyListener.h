#ifndef LEVIATHAN_GUI_KEYLISTENER
#define LEVIATHAN_GUI_KEYLISTENER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "CallableObject.h"
#include "KeyPressManager.h"

namespace Leviathan{ 
	class GuiManager;
namespace Gui{

	

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