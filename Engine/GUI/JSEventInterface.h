#pragma once
#ifndef LEVIATHAN_JSEVENTINTERFACE
#define LEVIATHAN_JSEVENTINTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "include/cef_app.h"
#include "Events/Event.h"


namespace Leviathan{ namespace Gui{

	//! \brief Handles javascript functions that have native extensions //
	class JSNativeCoreAPI : public CefV8Handler, public ThreadSafe{
		friend CefApplication;
		//! \brief Class that holds everything related to a listen callback
		//! \todo Add support for passing event values
		class JSListener{
			friend CefApplication;
		public:
			//! Creates a new listener with a predefined event type
			JSListener(EVENT_TYPE etype, CefRefPtr<CefV8Value> callbackfunc, CefRefPtr<CefV8Context> currentcontext);
			//! Creates a new listener from a GenericEvent name
			JSListener(const wstring &eventname, CefRefPtr<CefV8Value> callbackfunc, CefRefPtr<CefV8Context> currentcontext);

			//! \brief Executes this if this is a predefined type
			bool ExecutePredefined(const Event &eventdata);

			//! \brief Executes this if this is a generic type
			bool ExecuteGenericEvent(const GenericEvent &eventdata);

		protected:


			//! Marks whether EventsType or EventName is filled
			bool IsGeneric;
			//! Event's type when it is a predefined event
			EVENT_TYPE EventsType;
			//! Stores name of the generic event
			wstring EventName;

			//! V8 pointers
			CefRefPtr<CefV8Value> FunctionValueObject;
			CefRefPtr<CefV8Context> FunctionsContext;
		};


	public:

		JSNativeCoreAPI(CefApplication* owner);;
		~JSNativeCoreAPI();;

		//! \brief Handles calls from javascript
		virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, 
			CefString& exception) OVERRIDE;

		//! \brief Called when context is released, causes everything to be cleared
		void ClearContextValues();


		//! \brief Handles a packet received by this process
		void HandlePacket(const Event &eventdata);
		//! \brief Handles a generic packet
		void HandlePacket(const GenericEvent &eventdata);

		IMPLEMENT_REFCOUNTING(JSNativeCoreAPI);
	protected:

		//! Owner stored to be able to use it to bridge our requests to Gui::View
		CefApplication* Owner;

		//! Stores all registered functions
		std::vector<unique_ptr<JSListener>> RegisteredListeners;
	};


}}

#endif