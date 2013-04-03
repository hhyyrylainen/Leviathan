#ifndef LEVIATHAN_GUI_BASE_EVENTABLE
#define LEVIATHAN_GUI_BASE_EVENTABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "RenderableGuiObject.h"
#include "Event.h"

namespace Leviathan{ namespace Gui{

	class BaseEventable : public RenderableGuiObject{
	public:
		DLLEXPORT BaseEventable::BaseEventable();
		DLLEXPORT virtual BaseEventable::~BaseEventable();

		DLLEXPORT virtual int OnEvent(Event** pEvent) = 0;


		// sort of private //
		DLLEXPORT virtual void RegisterForEvent(EVENT_TYPE toregister);
		DLLEXPORT virtual void UnRegister(EVENT_TYPE from, bool all = false);
	};

}}
#endif