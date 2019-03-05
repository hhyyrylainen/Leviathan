// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Common/ReferenceCounted.h"

namespace Leviathan {

//! Engine events that are triggered at certain times
enum EVENT_TYPE {
    EVENT_TYPE_ERROR = 0,
    EVENT_TYPE_WAKEUP,
    EVENT_TYPE_GENERAL,
    EVENT_TYPE_KEYPRESS,
    EVENT_TYPE_KEYDOWN,
    EVENT_TYPE_SHOW,
    EVENT_TYPE_HIDE,
    EVENT_TYPE_TICK,
    EVENT_TYPE_REMOVE,
    EVENT_TYPE_EVENT_SEQUENCE_BEGIN,
    EVENT_TYPE_EVENT_SEQUENCE_END,
    EVENT_TYPE_MOUSEMOVED,
    EVENT_TYPE_MOUSEPOSITION,
    EVENT_TYPE_GUIDISABLE,
    EVENT_TYPE_GUIENABLE,
    EVENT_TYPE_RESIZE,
    EVENT_TYPE_WINDOW_RESIZE,
    EVENT_TYPE_ONCLICK,
    EVENT_TYPE_LISTENERVALUEUPDATED,
    EVENT_TYPE_FRAME_BEGIN,
    EVENT_TYPE_FRAME_END,
    EVENT_TYPE_INIT,
    EVENT_TYPE_RELEASE,
    EVENT_TYPE_TEST,
    //! Only called on the client when a frame is about to be renderd and interpolation status
    //! needs to be determined
    EVENT_TYPE_CLIENT_INTERPOLATION,

    EVENT_TYPE_ALL
};

#define LISTENERNAME_ONSHOW "OnShow"
#define LISTENERNAME_ONHIDE "OnHide"
#define LISTENERNAME_ONLISTENUPDATE "OnListenUpdate"
#define LISTENERNAME_ONCLICK "OnClick"
#define LISTENERNAME_ONINIT "OnInit"
#define LISTENERNAME_ONRELEASE "OnRelease"
#define LISTENERNAME_ONVALUECHANGE "OnValueChange"
#define LISTENERNAME_ONSUBMIT "OnSubmit"
#define LISTENERNAME_ONTICK "OnTick"
#define LISTENERNAME_ONCLOSECLICKED "OnCloseClicked"
#define LISTENERNAME_LISTSELECTIONACCEPTED "OnListSelectionAccepted"

#define LISTENERVALUE_ONSHOW 100
#define LISTENERVALUE_ONHIDE 101
#define LISTENERVALUE_ONLISTENUPDATE 102
#define LISTENERVALUE_ONCLICK 103
#define LISTENERVALUE_ONINIT 104
#define LISTENERVALUE_ONRELEASE 105
#define LISTENERVALUE_ONVALUECHANGE 106
#define LISTENERVALUE_ONSUBMIT 107
#define LISTENERVALUE_ONTICK 108
#define LISTENERVALUE_ONCLOSECLICKED 109
#define LISTENERVALUE_LISTSELECTIONACCEPTED 110

//! Name of listener event type pairs, used by ResolveStringToType in CallableObject and
//! EventableScriptObject to register for global events
static const std::map<std::string, EVENT_TYPE> EventListenerNameToEventMap = {
    {LISTENERNAME_ONLISTENUPDATE, EVENT_TYPE_LISTENERVALUEUPDATED},
    {LISTENERNAME_ONTICK, EVENT_TYPE_TICK}};

//! Name of listener event type pairs. This contains also common event types that aren't global
//! but many objects use these
static const std::map<std::string, EVENT_TYPE> EventListenerCommonNameToEventMap = {
    {LISTENERNAME_ONINIT, EVENT_TYPE_INIT}, {LISTENERNAME_ONRELEASE, EVENT_TYPE_RELEASE},
    {LISTENERNAME_ONSHOW, EVENT_TYPE_SHOW}, {LISTENERNAME_ONHIDE, EVENT_TYPE_HIDE}};

//! \brief Base type for all event data types
//! \note Child classes constructors should contain one for creating from a packet
class BaseEventData {
public:
#ifdef LEVIATHAN_USING_SFML
    //! \brief Adds this to a packet for retrieving it later
    virtual void AddDataToPacket(sf::Packet& packet) = 0;
#endif

    virtual ~BaseEventData();
};

//! \brief Data for EVENT_TYPE_CLIENT_INTERPOLATION
class ClientInterpolationEventData : public BaseEventData {
public:
    DLLEXPORT ClientInterpolationEventData(int tick, int mspassed);

#ifdef LEVIATHAN_USING_SFML
    DLLEXPORT ClientInterpolationEventData(sf::Packet& packet);

    DLLEXPORT void AddDataToPacket(sf::Packet& packet) override;
#endif

private:
    void CalculatePercentage();

public:
    //! The current tick to use for interpolation
    int TickNumber;

    //! Time passed since start of tick
    //! In milliseconds
    int TimeInTick;

    //! The calculated percentage the tick has advanced
    //!
    //! In case of extreme lag this is forced to be between 0.f-1.f to not break
    //! even more badly
    float Percentage;
};

//! \brief Data for EVENT_TYPE_ENGINE_TICK and all others that have only int data
class IntegerEventData : public BaseEventData {
public:
    //! \brief Loads from a packet
    DLLEXPORT IntegerEventData(int ticknumber);

#ifdef LEVIATHAN_USING_SFML
    DLLEXPORT IntegerEventData(sf::Packet& packet);

    virtual void AddDataToPacket(sf::Packet& packet);
#endif

    //! Current engine tick count
    int IntegerDataValue;
};

//! \brief Class that represents a statically defined event
class Event : public ReferenceCounted {
public:
#ifdef LEVIATHAN_USING_SFML
    //! \brief Loads this event from a packet
    DLLEXPORT Event(sf::Packet& packet);
#endif
    //! \brief Creates a new event
    //! \warning Funky things can happen if the type doesn't match the type of data
    DLLEXPORT Event(EVENT_TYPE type, BaseEventData* data);
    DLLEXPORT ~Event();

    //! \brief Gets the Type of the event
    DLLEXPORT EVENT_TYPE GetType() const;

#ifdef LEVIATHAN_USING_SFML
    //! \brief Saves this event to a packet
    DLLEXPORT void AddDataToPacket(sf::Packet& packet) const;
#endif

    // Data getting functions //
    DLLEXPORT ClientInterpolationEventData* GetDataForClientInterpolationEvent() const;
    //! \brief Gets the data if this is an event that has only one integer data member
    DLLEXPORT IntegerEventData* GetIntegerDataForEvent() const;

    REFERENCE_COUNTED_PTR_TYPE(Event);

protected:
    //! Events type
    EVENT_TYPE Type;

    //! Direct pointer to the data
    BaseEventData* Data;
};

//! \brief Class that represents a dynamically defined event
class GenericEvent : public ReferenceCounted {
public:
#ifdef LEVIATHAN_USING_SFML
    //! \brief Constructs this object from a packet
    DLLEXPORT GenericEvent(sf::Packet& packet);
#endif

    //! \brief Constructs a generic event
    DLLEXPORT GenericEvent(const std::string& type, const NamedVars& copyvals);

    //! \brief Constructs a generic event without any values
    DLLEXPORT GenericEvent(const std::string& type);

    //! \brief Constructor that takes the pointers as it's own
    DLLEXPORT GenericEvent(std::string* takeownershipstr, NamedVars* takeownershipvars);
    DLLEXPORT ~GenericEvent();

#ifdef LEVIATHAN_USING_SFML
    //! \brief Serializes this event to a packet
    DLLEXPORT void AddDataToPacket(sf::Packet& packet) const;
#endif

    //! \brief Gets this event's variables
    DLLEXPORT const NamedVars GetVariablesConst() const;

    //! \brief Returns a direct pointer to this objects variables
    DLLEXPORT NamedVars* GetVariables();

    //! \brief Proxy for script to get Variables
    //! \warning this function returns the pointer with reference count increased //
    DLLEXPORT NamedVars* GetNamedVarsRefCounted();

    //! Returns the TypeStr ptr
    DLLEXPORT std::string* GetTypePtr();
    //! \brief Returns the name of the event
    //! \see GetTypePtr
    DLLEXPORT std::string GetType() const;

    REFERENCE_COUNTED_PTR_TYPE(GenericEvent);

protected:
    //! String that defines this event's type
    std::string* TypeStr;

    //! Pointer to this event's variables
    NamedVars* Variables;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Event;
using Leviathan::GenericEvent;
#endif
