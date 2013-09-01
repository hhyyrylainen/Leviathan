#ifndef LEVIATHAN_GUI_BASEEVENTABLE
#define LEVIATHAN_GUI_BASEEVENTABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Events\Event.h"

namespace Leviathan{ namespace Gui{

	class GuiManager;

	class BaseGuiEventable{
	public:
		DLLEXPORT BaseGuiEventable::BaseGuiEventable(GuiManager* owner);
		DLLEXPORT virtual BaseGuiEventable::~BaseGuiEventable();

		DLLEXPORT virtual int OnEvent(Event** pEvent) = 0;

		// sort of private //
		DLLEXPORT virtual void RegisterForEvent(EVENT_TYPE toregister);
		DLLEXPORT virtual void UnRegister(EVENT_TYPE from, bool all = false);

	protected:

		GuiManager* InstanceWithRegisteredEvents;

	};

}}
#endif