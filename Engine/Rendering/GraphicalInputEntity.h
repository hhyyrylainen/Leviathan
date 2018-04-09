// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include <atomic>
#include <memory>

namespace CEGUI {

class OgreRenderer;
}

namespace Ogre {

class RenderWindow;
class Camera;
} // namespace Ogre

namespace Leviathan {

class GEntityAutoClearResources;

//! \brief Represents a collection of objects that represents
//! everything related to a single game's Windows window \note
//! Even though this class is marked thread safe only one instance
//! maybe constructed at once
class GraphicalInputEntity : public ThreadSafe {
public:
    //! \warning You can only create one window at a time since this is not thread safe
    DLLEXPORT GraphicalInputEntity(Graphics* windowcreater, AppDef* windowproperties);
    DLLEXPORT ~GraphicalInputEntity();

    DLLEXPORT void Tick(int mspassed);

    // This function uses the LinkObjects function objects //
    DLLEXPORT bool Render(int mspassed, int tick, int timeintick);

    // object linking //
    // This function also updates the camera aspect ratio //
    DLLEXPORT void LinkObjects(std::shared_ptr<GameWorld> world);

    // returns true if succeeds, false if another window has input //
    DLLEXPORT bool SetMouseCapture(bool state);

    DLLEXPORT void UnlinkAll();

    //! Returns how many windows have been created
    //! \see GlobalWindowCount
    DLLEXPORT static int GetGlobalWindowCount();

    //! Returns this windows creation number
    //! \note This is quaranteed to be unique among all windows
    DLLEXPORT int GetWindowNumber() const;


    // Input function //

    //! \brief Called everytime input event handling ends even if we didn't receive
    //! any keypresses
    DLLEXPORT void InputEnd();

    DLLEXPORT void InjectMouseMove(int xpos, int ypos);
    DLLEXPORT void InjectMouseWheel(int xamount, int yamount);
    DLLEXPORT void InjectMouseButtonDown(int32_t whichbutton);
    DLLEXPORT void InjectMouseButtonUp(int32_t whichbutton);
    DLLEXPORT void InjectCodePoint(uint32_t utf32char);
    DLLEXPORT void InjectKeyDown(int32_t sdlkey);
    DLLEXPORT void InjectKeyUp(int32_t sdlkey);

    // graphics related //
    // DLLEXPORT float GetViewportAspectRatio();
    DLLEXPORT void SaveScreenShot(const std::string& filename);

    DLLEXPORT void OnResize(int width, int height);

    DLLEXPORT inline Window* GetWindow()
    {
        return DisplayWindow;
    }
    DLLEXPORT inline GUI::GuiManager* GetGui()
    {
        return WindowsGui;
    }
    DLLEXPORT inline InputController* GetInputController()
    {
        return TertiaryReceiver.get();
    }
    DLLEXPORT void OnFocusChange(bool focused);

    DLLEXPORT CEGUI::OgreRenderer* GetCEGUIRenderer() const
    {
        return CEGUIRenderer;
    }

    DLLEXPORT inline bool GetVsync() const;
    DLLEXPORT inline Ogre::RenderWindow* GetOgreWindow() const
    {
        return OWindow;
    };

    //! \warning This is broken!
    //! \brief Creates a workspace that clears this window to a
    //! specified colour \note A world cannot be attached to this
    //! object if this is used
    //! \param skyboxmaterial The material to use for a skybox. Empty if not wanted.
    //! \warning A sky box is required to have CEGUI not flicker while rendering
    //! on this window
    DLLEXPORT void SetAutoClearing(const std::string& skyboxmaterial);

    //! \brief Destroyes the workspace that is clearing this window each frame
    DLLEXPORT void StopAutoClearing();

    //! \brief Overwrites the default InputController with a
    //! custom one
    DLLEXPORT void SetCustomInputController(std::shared_ptr<InputController> controller);


    //! \brief Creates a workspace definition that can be used to clear a window
    DLLEXPORT static void CreateAutoClearWorkspaceDefIfNotAlready();

protected:
    //! \brief Creates an Ogre scene to display GUI on this window
    void _CreateOverlayScene();

protected:
    Window* DisplayWindow = nullptr;
    std::shared_ptr<InputController> TertiaryReceiver;
    GUI::GuiManager* WindowsGui = nullptr;
    CEGUI::OgreRenderer* CEGUIRenderer = nullptr;


    Ogre::RenderWindow* OWindow = nullptr;
    Ogre::SceneManager* OverlayScene = nullptr;
    Ogre::Camera* OverLayCamera = nullptr;

    //! Like entity ID
    //! Makes sure that created Ogre resources are unique
    int ID;

    //! Used to do input setup each time some input is received
    bool InputStarted = false;

    std::shared_ptr<GameWorld> LinkedWorld;

    //! this count variable is needed to parse resource groups after first window
    static int GlobalWindowCount;

    static Mutex GlobalCountMutex;

    //! Keeps track of how many windows in total have been created
    static std::atomic<int> TotalCreatedWindows;

    //! Pointer to the first CEGUI::OgreRenderer
    static CEGUI::OgreRenderer* FirstCEGUIRenderer;

    //! The number of this window (starts from 1)
    int WindowNumber;

    bool MouseCaptureState = false;
    static GraphicalInputEntity* InputCapturer;

    //! True when auto clear ogre workspace has been created
    static bool AutoClearResourcesCreated;
    static Mutex AutoClearResourcesMutex;

    //! Used when this window does'nt have a world and the background needs to be cleared
    std::unique_ptr<GEntityAutoClearResources> AutoClearResources;
};

} // namespace Leviathan
