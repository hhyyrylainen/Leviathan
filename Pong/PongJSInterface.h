#pragma once
#ifndef PONG_JSINTERFACE
#define PONG_JSINTERFACE
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GUI/LeviathanJavaScriptAsync.h"


namespace Pong{

	class CustomJSInterface : public Leviathan::Gui::JSAsyncCustom{
	public:
		CustomJSInterface();
		~CustomJSInterface();
		
		//! Query processing function
		virtual bool ProcessQuery(Leviathan::Gui::LeviathanJavaScriptAsync* caller, const CefString &request, 
			int64 queryid, bool persists, CefRefPtr<Callback> &callback);
		
		//! Previously made ProcessQuery is canceled
		virtual void CancelQuery(Leviathan::Gui::LeviathanJavaScriptAsync* caller, int64 queryid);

		//! Called when a Gui::View is closed, should CancelQuery all matching ones
		virtual void CancelAllMine(Leviathan::Gui::LeviathanJavaScriptAsync* me);

	protected:
		
		//! Store queries that need to be handled async
	};

}
#endif