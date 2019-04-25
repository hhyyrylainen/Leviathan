// ------------------------------------ //
#include "GuiManager.h"

#include "Common/DataStoring/DataBlock.h"
#include "Common/StringOperations.h"
#include "Engine.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "GuiLayer.h"
#include "GuiView.h"
#include "GuiWidgetLayer.h"
#include "Handlers/IDFactory.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "Rendering/Graphics.h"
#include "Widgets/ImageWidget.h"
#include "Widgets/VideoPlayerWidget.h"
#include "Window.h"

#include "Engine.h"

#include <boost/filesystem.hpp>

#include <thread>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
// CutscenePlayStatus
struct GuiManager::CutscenePlayStatus {

    inline CutscenePlayStatus(const boost::intrusive_ptr<VideoPlayerWidget>& player) :
        Player(player)
    {}

    boost::intrusive_ptr<VideoPlayerWidget> Player;
};
// ------------------------------------ //
GuiManager::GuiManager() : ID(IDFactory::GetID()) {}
GuiManager::~GuiManager() {}
// ------------------------------------ //
bool GuiManager::Init(Graphics* graph, Window* window)
{
    ThisWindow = window;

    // Setup the overlay layer that has cutscenes and the software cursor
    // Care needs to be taken to keep this as the last workspace
    OverlayLayer = WidgetLayer::MakeShared<WidgetLayer>(this, ThisWindow, -1);

    // The overlay ignores input events for now
    OverlayLayer->SetInputMode(INPUT_MODE::None);

    auto image = ImageWidget::MakeShared<ImageWidget>("oakfloor_basecolor.png");
    OverlayLayer->AddWidget(image);
    image->SetPosition(100, 100);
    image->SetZ(2);

    ManagedLayers.push_back(OverlayLayer);

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
    if(SoftwareCursorWidget)
        ThisWindow->SetHideCursor(false);
    SoftwareCursorWidget.reset();

    OverlayLayer.reset();
    CurrentlyPlayingCutscene.reset();

    // Destroy the views //
    for(size_t i = 0; i < ManagedLayers.size(); i++) {

        ManagedLayers[i]->ReleaseResources();
    }

    ManagedLayers.clear();

    Logger::Get()->Info("GuiManager: Gui successfully closed on window");
}
// ------------------------------------ //
void GuiManager::GuiTick(int mspassed)
{
    if(ReloadQueued) {

        ReloadQueued = false;

        // Reload //
        LOG_INFO("GuiManager: reloading file");
        DEBUG_BREAK;

        // // Now load it //
        // if(!LoadGUIFile(MainGUIFile, true)) {

        //     Logger::Get()->Error(
        //         "GuiManager: file changed: couldn't load updated file: " + MainGUIFile);
        // }

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
void GuiManager::OnRender(float passed)
{
    // Browser textures are updated in the main event loop

    // Update all widget layers (the browser layers ignore this)
    for(const auto& layer : ManagedLayers)
        layer->OnRender(passed);
}
// ------------------------------------ //
DLLEXPORT void GuiManager::OnResize()
{
    // Resize all CEF browsers on this window //
    for(size_t i = 0; i < ManagedLayers.size(); i++) {
        ManagedLayers[i]->NotifyWindowResized();
    }
}

DLLEXPORT void GuiManager::OnFocusChanged(bool focused)
{
    // Notify all CEF browsers on this window //
    for(size_t i = 0; i < ManagedLayers.size(); i++) {
        ManagedLayers[i]->NotifyFocusUpdate(focused);
    }
}
// ------------------------------------ //
DLLEXPORT bool GuiManager::LoadCEFLayer(const std::string& urlorpath, bool nochangelistener)
{
    // Create the view //
    auto loadingView = View::MakeShared<View>(this, ThisWindow, GuiViewCounter++);

    OverlayLayer->BringToFront();

    // Create the final page //
    std::string finalpath;

    // If this is an internet page pass it unmodified //
    if(urlorpath.find("http") < 2 || urlorpath.find("file://") < 2) {

        finalpath = urlorpath;
    } else {
        // Local file, add to the end //
        if(!boost::filesystem::exists(urlorpath)) {
            LOG_ERROR("GuiManager: LoadGUIFile: failed to get canonical path (is "
                      "the file missing?) for file: " +
                      urlorpath);
            return false;
        }

        // TODO: Probably needs to convert spaces here
        finalpath = StringOperations::CombineURL("http://leviathan-local/", urlorpath);
    }

    LOG_INFO("GuiManager: loading GUI: " + finalpath);

    // TODO: extra configuration
    NamedVars varlist;

    // Initialize it //
    if(!loadingView->Init(finalpath, varlist)) {

        loadingView->ReleaseResources();

        Logger::Get()->Error(
            "GuiManager: LoadGUIFile: failed to initialize view for file: " + urlorpath);
        return false;
    }

    // Add the page //
    ManagedLayers.push_back(loadingView);

    // Set focus to the new Layer //
    ManagedLayers.back()->NotifyFocusUpdate(ThisWindow->IsWindowFocused());
    return true;
}

DLLEXPORT Layer* GuiManager::LoadGUILayer(
    const std::string& layoutname, int renderorder /*= -1*/)
{
    DEBUG_BREAK;
    if(renderorder <= 0) {

        renderorder = GuiViewCounter++;
    }

    OverlayLayer->BringToFront();

    return nullptr;
}
// ------------------------------------ //
DLLEXPORT void GuiManager::PlayCutscene(const std::string& file,
    std::function<void()> onfinished, std::function<void(const std::string&)> onerror,
    bool allowskip /*= true*/)
{
    const bool showUnderCursor = true;

    if(CurrentlyPlayingCutscene) {
        LOG_ERROR("GuiManager: PlayCutscene: can't play multiple cutscenes at the same time");

        onerror("can't play multiple cutscenes at the same time");
        return;
    }

    if(!boost::filesystem::exists(file)) {
        onerror("file doesn't exist");
        return;
    }

    auto player = VideoPlayerWidget::MakeShared<VideoPlayerWidget>();
    OverlayLayer->AddWidget(player);

    player->SetEndCallback([=]() {
        // TODO: figure out if an error happened
        CurrentlyPlayingCutscene.reset();

        OverlayLayer->RemoveWidget(player.get());

        onfinished();

        // Due to release order we do clear the callback like this to destroy the player object
        // later
        Engine::Get()->Invoke([=]() { player->SetEndCallback(nullptr); });
    });

    player->Play(file);

    CurrentlyPlayingCutscene = std::make_unique<CutscenePlayStatus>(player);

    // TODO: focus setting and also focus fixing when popping
    // ManagedLayers.back()->NotifyFocusUpdate(ThisWindow->IsWindowFocused());
}

DLLEXPORT void GuiManager::CancelCutscene()
{
    if(CurrentlyPlayingCutscene) {

        // Just stop it and that will destroy it on next tick (immediately after this call a
        // new video can be played)
        CurrentlyPlayingCutscene->Player->Stop();
    }
}
// ------------------------------------ //
void GUI::GuiManager::SetSoftwareCursor(const std::string& cursor)
{
    if(cursor.empty()) {
        // Disable
        if(SoftwareCursorWidget) {

            OverlayLayer->RemoveWidget(SoftwareCursorWidget.get());
            SoftwareCursorWidget.reset();
        }

    } else {
        // Enable or change image
        if(!SoftwareCursorWidget) {
            LOG_INFO("GuiManager: enabling software cursor with image: " + cursor);

            SoftwareCursorWidget = ImageWidget::MakeShared<ImageWidget>(cursor);

            OverlayLayer->AddWidget(SoftwareCursorWidget);
            SoftwareCursorWidget->SetPosition(CursorX, CursorY);
            SoftwareCursorWidget->SetZ(20);

        } else {

            // Already enabled, change the image
            SoftwareCursorWidget->SetImage(cursor);
        }
    }
}

void GUI::GuiManager::SetCursorPosition(int x, int y)
{
    CursorX = x;
    CursorY = y;

    if(SoftwareCursorWidget) {

        int32_t width, height;
        ThisWindow->GetSize(width, height);

        SoftwareCursorWidget->SetPosition(CursorX, // height -
            CursorY);
    }
}

void GUI::GuiManager::SetSoftwareCursorVisible(bool visible)
{
    if(HideSoftwareCursor == !visible)
        return;

    HideSoftwareCursor = !visible;
}
// ------------------------------------ //
DLLEXPORT Layer* Leviathan::GUI::GuiManager::GetLayerByIndex(size_t index)
{
    if(index >= ManagedLayers.size())
        return nullptr;

    return ManagedLayers[index].get();
}
// ------------------------------------ //
DLLEXPORT Layer* Leviathan::GUI::GuiManager::GetTargetLayerForInput(
    INPUT_EVENT_TYPE type, int mousex, int mousey)
{
    Layer* bestFound = nullptr;

    for(size_t i = 0; i < ManagedLayers.size(); i++) {

        Layer* view = ManagedLayers[i].get();

        const auto mode = view->GetInputMode();
        if(mode == INPUT_MODE::None)
            continue;

        // TODO: coordinate check

        // TODO: proper Z-order and mouse position checking should be done
        if(mode == INPUT_MODE::Menu &&
            (!bestFound || bestFound->GetInputMode() != INPUT_MODE::Menu)) {

            bestFound = view;
            continue;
        }

        // The mode Gameplay is only best if nothing has been found so far
        if(bestFound)
            continue;

        LEVIATHAN_ASSERT(mode == INPUT_MODE::Gameplay, "Some input mode is not handled");

        // Allow mouse events except scroll but keyboard events are
        // only allowed if a text box is focused
        if(type == INPUT_EVENT_TYPE::Keypress) {

            if(view->HasFocusedInputElement()) {

                bestFound = view;
                continue;
            }
        } else if(type == INPUT_EVENT_TYPE::Scroll) {

            // Allow scroll events when over something scrollable
            if(view->HasScrollableElementUnderCursor()) {
                bestFound = view;
                continue;
            }
        } else {

            // Mouse movement
            bestFound = view;
        }
    }

    return bestFound;
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
