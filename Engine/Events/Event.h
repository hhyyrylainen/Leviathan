#ifndef LEVIATHAN_EVENT
#define LEVIATHAN_EVENT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif

// ------------------------------------ //
// ---- includes ---- //
#include "Common/ReferenceCounted.h"
#include "Common/DataStoring/NamedVars.h"
#include <boost/assign/list_of.hpp>
#include "Script/ScriptModule.h"



namespace Leviathan{

	enum EVENT_TYPE{ EVENT_TYPE_ERROR = 0, EVENT_TYPE_WAKEUP, EVENT_TYPE_GENERAL , EVENT_TYPE_KEYPRESS, EVENT_TYPE_KEYDOWN, 
		EVENT_TYPE_SHOW, EVENT_TYPE_HIDE, EVENT_TYPE_ONCLICK, EVENT_TYPE_BECOMEFOREGROUND, EVENT_TYPE_BECOMEBACKGROUND, EVENT_TYPE_TICK, 
		EVENT_TYPE_ANIMATION_FINISH, EVENT_TYPE_REMOVE, EVENT_TYPE_EVENT_SEQUENCE_BEGIN, EVENT_TYPE_EVENT_SEQUENCE_END,
		EVENT_TYPE_MOUSEMOVED, EVENT_TYPE_MOUSEPOSITION,
		EVENT_TYPE_GUIDISABLE, EVENT_TYPE_GUIENABLE, EVENT_TYPE_WINDOW_RESIZE, EVENT_TYPE_RESIZE, EVENT_TYPE_TEST,
		EVENT_TYPE_LISTENERVALUEUPDATED,
		EVENT_TYPE_FRAME_BEGIN, EVENT_TYPE_FRAME_END, EVENT_TYPE_ENGINE_TICK, EVENT_TYPE_INIT, EVENT_TYPE_PHYSICS_BEGIN, EVENT_TYPE_RELEASE, 
		EVENT_TYPE_ALL};

	static const std::map<wstring, EVENT_TYPE> EventListenerNameToEventMap =  boost::assign::map_list_of
		(LISTENERNAME_ONSHOW, EVENT_TYPE_SHOW)
		(LISTENERNAME_ONHIDE, EVENT_TYPE_HIDE)
		(LISTENERNAME_ONCLICK, EVENT_TYPE_ONCLICK)
		(LISTENERNAME_ONLISTENUPDATE, EVENT_TYPE_LISTENERVALUEUPDATED)
		(LISTENERNAME_ONINIT, EVENT_TYPE_INIT)
		(LISTENERNAME_ONRELEASE, EVENT_TYPE_RELEASE);

	// Different event value types //
	struct PhysicsStartEventData{
		PhysicsStartEventData(const float &time, void* worldptr) : TimeStep(time), GameWorldPtr(worldptr){
		}

		float TimeStep;
		void* GameWorldPtr;
	};


	class Event : public ReferenceCounted{
	public:
		DLLEXPORT Event::Event();
		DLLEXPORT Event::Event(EVENT_TYPE type, void* data);
		DLLEXPORT Event::Event(EVENT_TYPE type, void* data, bool allowdelete);
		DLLEXPORT Event::~Event();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(Event);

		EVENT_TYPE Type;
		void* Data;
		bool DeleteIt : 1;

		DLLEXPORT EVENT_TYPE GetType();
	};

	class GenericEvent : public ReferenceCounted{
	public:
		DLLEXPORT GenericEvent(const wstring &type, const NamedVars &copyvals);
		DLLEXPORT GenericEvent(wstring* takeownershipstr, NamedVars* takeownershipvars);
		DLLEXPORT ~GenericEvent();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GenericEvent);

		// Warning this function returns the pointer with reference count increased //
		DLLEXPORT NamedVars* GetNamedVarsRefCounted();

		wstring* TypeStr;
		NamedVars* Variables;
	};

}
#endif