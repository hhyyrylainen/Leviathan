// ------------------------------------ //
#include "GuiManager.h"

#include "Common/DataStoring/DataBlock.h"
#include "Engine.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GuiView.h"
#include "Handlers/IDFactory.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "Rendering/Graphics.h"
#include "Window.h"

#include <thread>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
GuiManager::GuiManager() : ID(IDFactory::GetID()) {}
GuiManager::~GuiManager() {}
// ------------------------------------ //
bool GuiManager::Init(Graphics* graph, Window* window, bool ismain)
{
    ThisWindow = window;
    MainGuiManager = ismain;

    // All rendering is now handled by individual Views and the
    // Window full screen compositor passes

    return true;
}

void GuiManager::Release()
{
    // Stop with the file updates //
    if(FileChangeID) {

        auto tmphandler = ResourceRefreshHandler::Get();

        if(tmphandler) {

            tmphandler->StopListeningForFileChanges(FileChangeID);
        }

        FileChangeID = 0;
    }

    // Default mouse back //
    // show default window cursor //
    ThisWindow->SetHideCursor(false);

    // Destroy the views //
    for(size_t i = 0; i < ThissViews.size(); i++) {

        ThissViews[i]->ReleaseResources();
        ThissViews[i]->Release();
    }

    Logger::Get()->Info("GuiManager: Gui successfully closed on window");
}
// ------------------------------------ //
void GuiManager::GuiTick(int mspassed)
{
    if(ReloadQueued) {

        ReloadQueued = false;

        // Reload //
        LOG_INFO("GuiManager: reloading file: " + MainGUIFile);
        DEBUG_BREAK;

        // Now load it //
        if(!LoadGUIFile(MainGUIFile, true)) {

            Logger::Get()->Error(
                "GuiManager: file changed: couldn't load updated file: " + MainGUIFile);
        }

        // Apply back the old states //
        // ApplyGuiStates(currentstate.get());
    }

    // check if we want mouse //
    if(GuiMouseUseUpdated) {

        GuiMouseUseUpdated = false;

        if(GuiDisallowMouseCapture) {
            // disable mouse capture //
            ThisWindow->SetMouseCapture(false);

            // Show the cursor
            LOG_WRITE("TODO: this needs some property when a custom cursor is used");
            ThisWindow->SetHideCursor(false);

        } else {

            // activate direct mouse capture //
            if(!ThisWindow->SetMouseCapture(true)) {
                // failed, GUI must be forced to stay on //
                OnForceGUIOn();
                GuiDisallowMouseCapture = true;
                GuiMouseUseUpdated = true;
            } else {
                // Success, hide GUI cursor
                ThisWindow->SetHideCursor(true);
                DEBUG_BREAK;
                // GuiContext->getCursor().hide();
            }
        }
    }
}

DLLEXPORT void GuiManager::OnForceGUIOn()
{
    DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT void GuiManager::SetDisableMouseCapture(bool newvalue)
{
    DisableGuiMouseCapture = newvalue;
    // This will cause the capture state to be checked next tick
    GuiMouseUseUpdated = true;
}

// ------------------------------------ //
void GuiManager::Render()
{
    // Update browser textures //
    for(size_t i = 0; i < ThissViews.size(); i++) {

        ThissViews[i]->CheckRender();
    }
}
// ------------------------------------ //
DLLEXPORT void GuiManager::OnResize()
{
    // Resize all CEF browsers on this window //
    for(size_t i = 0; i < ThissViews.size(); i++) {
        ThissViews[i]->NotifyWindowResized();
    }
}

DLLEXPORT void GuiManager::OnFocusChanged(bool focused)
{
    // Notify all CEF browsers on this window //
    for(size_t i = 0; i < ThissViews.size(); i++) {
        ThissViews[i]->NotifyFocusUpdate(focused);
    }
}
// ------------------------------------ //
DLLEXPORT bool GuiManager::LoadGUIFile(const std::string& urlorpath, bool nochangelistener)
{
    MainGUIFile = urlorpath;

    // Create the view //
    boost::intrusive_ptr<View> loadingView(new View(this, ThisWindow));

    loadingView->AddRef();

    // Create the final page //
    std::string finalpath;

    // If this is an internet page pass it unmodified //
    if(urlorpath.find("http") < 2 || urlorpath.find("file://") < 2) {

        finalpath = urlorpath;
    } else {
        // Local file, add to the end //
        finalpath = "file:///" + urlorpath;
    }

    LOG_INFO("GuiManager: loading GUI: " + finalpath);

    // TODO: extra configuration
    NamedVars varlist;

    // Initialize it //
    if(!loadingView->Init(finalpath, varlist)) {

        loadingView->ReleaseResources();
        loadingView->Release();

        Logger::Get()->Error(
            "GuiManager: LoadGUIFile: failed to initialize view for file: " + urlorpath);
        return false;
    }

    // Add the page //
    ThissViews.push_back(loadingView);

    // Set focus to the new View //
    ThissViews.back()->NotifyFocusUpdate(ThisWindow->IsWindowFocused());
    return true;
}

DLLEXPORT void GuiManager::UnLoadGUIFile()
{
    DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT CefBrowserHost* GuiManager::GetPrimaryInputReceiver()
{
    for(size_t i = 0; i < ThissViews.size(); i++) {

        return ThissViews[i]->GetBrowserHost().get();
    }

    return nullptr;
}
// ------------------------------------ //
void GuiManager::_FileChanged(const std::string& file, ResourceFolderListener& caller)
{
    // Any updated file will cause whole reload //
    LOG_WRITE("TODO: invoke file reload on main thread");
    //

    // ReloadQueued = true;

    // // Mark everything as non-updated //
    // caller.MarkAllAsNotUpdated();
}
