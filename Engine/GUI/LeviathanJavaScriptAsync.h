// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GuiView.h"

#include "wrapper/cef_message_router.h"

namespace Leviathan { namespace GUI {

//! \brief Provides client applications and modules (not done) support for defining custom
//! request responses \note This object should be directly passed to
//! GlobalCEFHandler::RegisterCustomJavaScriptQueryHandler
class JSAsyncCustom {
public:
    typedef CefMessageRouterBrowserSide::Callback Callback;

    DLLEXPORT JSAsyncCustom();
    DLLEXPORT virtual ~JSAsyncCustom();

    //! \brief Used to allow custom handling on JavaScript query
    //! \return Return true when this object has handled this query
    //! \param queryid Unique identifier which defines this is caller's context
    //! \param caller Is the LeviathanJavaScriptAsync object associated with a View that has
    //! called this function (these should not be mixed) \warning If caller is different the ID
    //! of different queries can be the same, so be sure to store both the caller and the id to
    //! uniquely identify queries
    DLLEXPORT virtual bool ProcessQuery(LeviathanJavaScriptAsync* caller,
        const CefString& request, int64 queryid, bool persists,
        CefRefPtr<Callback>& callback) = 0;

    //! \brief Called if a query is canceled, nothing should be called after this
    //! \note This should work so that the waiting matching query is canceled and not responded
    //! to
    DLLEXPORT virtual void CancelQuery(LeviathanJavaScriptAsync* caller, int64 queryid) = 0;


    //! \brief Called when JSAsyncCustom is disconnected from a LeviathanJavaScriptAsync and
    //! this should CancelQuery all queries associated with that object
    DLLEXPORT virtual void CancelAllMine(LeviathanJavaScriptAsync* me) = 0;
};



//! \brief Handles asynchronous javascript requests
class LeviathanJavaScriptAsync : public CefMessageRouterBrowserSide::Handler,
                                 public ThreadSafe {
public:
    DLLEXPORT LeviathanJavaScriptAsync(View* owner);
    DLLEXPORT ~LeviathanJavaScriptAsync();

    DLLEXPORT void BeforeRelease();

    //! \brief Creates a new request
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        int64 query_id, const CefString& request, bool persistent,
        CefRefPtr<Callback> callback);

    //! \brief Destroys a previously made request without receiving anything in the callback
    virtual void OnQueryCanceled(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id);

    //! \brief Registers a new JSAsyncCustom to use for handling
    DLLEXPORT void RegisterNewCustom(Lock& guard, std::shared_ptr<JSAsyncCustom> newhandler);
    //! \brief Removes an entry from RegisteredCustomHandlers
    DLLEXPORT void UnregisterCustom(JSAsyncCustom* handler);


    //! \brief Checks if this window has the minimumlevel or higher VIEW_SECURITYLEVEL
    //! \return True when access is blocked and the calling function should return
    DLLEXPORT inline bool _VerifyJSAccess(
        VIEW_SECURITYLEVEL minimumlevel, CefRefPtr<Callback>& callback)
    {
        // Check is the access level the same or higher //
        if(Owner->ViewSecurity >= minimumlevel) {
            // Allowed //
            return false;
        }

        // Block with an error message //
        callback->Failure(
            403, "View doesn't have a high enough security level to access this feature");
        return true;
    }

protected:
    //! Holds a list of custom handlers, updated by GlobalCEFHandler
    std::vector<std::shared_ptr<JSAsyncCustom>> RegisteredCustomHandlers;
    View* Owner;
};

}} // namespace Leviathan::GUI
