#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/LeviathanJavaScriptAsync.h"


namespace {

class CustomJSInterface : public Leviathan::GUI::JSAsyncCustom {
public:
    CustomJSInterface();
    ~CustomJSInterface();

    //! Query processing function
    bool ProcessQuery(Leviathan::GUI::LeviathanJavaScriptAsync* caller,
        const CefString& request, int64 queryid, bool persists,
        CefRefPtr<Callback>& callback) override;

    //! Previously made ProcessQuery is canceled
    void CancelQuery(Leviathan::GUI::LeviathanJavaScriptAsync* caller, int64 queryid) override;

    //! Called when a Gui::View is closed, should CancelQuery all matching ones
    void CancelAllMine(Leviathan::GUI::LeviathanJavaScriptAsync* me) override;

protected:
    //! Store queries that need to be handled async
};

} // namespace
