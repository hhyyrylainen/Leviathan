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
		EVENT_TYPE_FRAME_BEGIN, EVENT_TYPE_FRAME_END, EVENT_TYPE_INIT, EVENT_TYPE_PHYSICS_BEGIN, EVENT_TYPE_RELEASE,
		EVENT_TYPE_ALL};

	//! \brief A map of name of listener event type pairs, used by GUI to hook to events
	static const std::map<wstring, EVENT_TYPE> EventListenerNameToEventMap =  boost::assign::map_list_of
		(LISTENERNAME_ONSHOW, EVENT_TYPE_SHOW)
		(LISTENERNAME_ONHIDE, EVENT_TYPE_HIDE)
		(LISTENERNAME_ONCLICK, EVENT_TYPE_ONCLICK)
		(LISTENERNAME_ONLISTENUPDATE, EVENT_TYPE_LISTENERVALUEUPDATED)
		(LISTENERNAME_ONINIT, EVENT_TYPE_INIT)
		(LISTENERNAME_ONRELEASE, EVENT_TYPE_RELEASE)
		(LISTENERNAME_ONTICK, EVENT_TYPE_TICK);

	//! \brief Base type for all event data types
	//! \note Child classes constructors should contain one for creating from a packet
	class BaseEventData{
	public:

		//! \brief Adds this to a packet for retrieving it later
		virtual void AddDataToPacket(sf::Packet &packet) = 0;

		virtual ~BaseEventData();


	};

	//! \brief Data for EVENT_TYPE_PHYSICS_BEGIN
	class PhysicsStartEventData : public BaseEventData{
	public:
		//! \brief Loads from a packet
		DLLEXPORT PhysicsStartEventData(sf::Packet &packet);
		//! \brief Creates a new PhysicsStartEventData
		DLLEXPORT PhysicsStartEventData(const float &time, void* worldptr);

		virtual void AddDataToPacket(sf::Packet &packet);


		float TimeStep;
		//! Pointer to the world
		//! \warning This is NULL if this event is passed through a packet
		void* GameWorldPtr;
	};

	//! \brief Data for EVENT_TYPE_SHOW
	class ShowEventData : public BaseEventData{
	public:
		//! \brief Loads from a packet
		DLLEXPORT ShowEventData(sf::Packet &packet);
		//! \brief Creates a new PhysicsStartEventData
		DLLEXPORT ShowEventData(bool shown);

		virtual void AddDataToPacket(sf::Packet &packet);


		bool IsShown;
	};


	//! \brief Data for EVENT_TYPE_ENGINE_TICK and all others that have only int data
	class IntegerEventData : public BaseEventData{
	public:
		//! \brief Loads from a packet
		DLLEXPORT IntegerEventData(sf::Packet &packet);
		//! \brief Creates a new PhysicsStartEventData
		DLLEXPORT IntegerEventData(int ticknumber);

		virtual void AddDataToPacket(sf::Packet &packet);

		//! Current engine tick count
		int IntegerDataValue;
	};

	//! \brief Class that represents a statically defined event
	class Event : public ReferenceCounted{
	public:
		//! \brief Loads this event from a packet
		DLLEXPORT Event(sf::Packet &packet);
		//! \brief Creates a new event
		//! \warning Funky things can happen if the type doesn't match the type of data
		DLLEXPORT Event(EVENT_TYPE type, BaseEventData* data);
		DLLEXPORT ~Event();

		//! \brief Gets the Type of the event
		DLLEXPORT EVENT_TYPE GetType() const;


		//! \brief Saves this event to a packet
		DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

		// Data getting functions //
		DLLEXPORT PhysicsStartEventData* GetDataForPhysicsStartEvent() const;
		DLLEXPORT ShowEventData* GetDataForShowEvent() const;
		//! \brief Gets the data if this is an event that has only one integer data member
		DLLEXPORT IntegerEventData* GetIntegerDataForEvent() const;

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(Event);
	protected:

		//! Events type
		EVENT_TYPE Type;
		//! Direct pointer to the data
		BaseEventData* Data;
	};

	//! \brief Class that represents a dynamically defined event
	class GenericEvent : public ReferenceCounted{
	public:
		//! \brief Constructs this object from a packet
		DLLEXPORT GenericEvent(sf::Packet &packet);

		//! \brief Constructs a generic event
		DLLEXPORT GenericEvent(const wstring &type, const NamedVars &copyvals);
		//! \brief Constructor that takes the pointers as it's own
		DLLEXPORT GenericEvent(wstring* takeownershipstr, NamedVars* takeownershipvars);
		DLLEXPORT ~GenericEvent();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(GenericEvent);

		//! \brief Serializes this event to a packet
		DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

		//! \brief Gets this event's variables
		DLLEXPORT const NamedVars GetVariablesConst() const;

		//! \brief Returns a direct pointer to this objects variables
		DLLEXPORT NamedVars* GetVariables();

		//! \brief Proxy for script to get Variables
		//! \warning this function returns the pointer with reference count increased //
		DLLEXPORT NamedVars* GetNamedVarsRefCounted();

		//! Returns the TypeStr ptr
		DLLEXPORT wstring* GetTypePtr();
		//! \brief Returns the name of the event
		//! \see GetTypePtr
		DLLEXPORT wstring GetType() const;

	protected:

		//! String that defines this event's type
		wstring* TypeStr;
		//! Pointer to this event's variables
		NamedVars* Variables;
	};

}
#endif
