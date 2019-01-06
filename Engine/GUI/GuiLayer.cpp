// ------------------------------------ //
#include "GuiLayer.h"

#include "Handlers/IDFactory.h"
#include "Window.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "OgreCamera.h"
#include "OgreRenderWindow.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

DLLEXPORT Layer::Layer(GuiManager* owner, Window* window, int renderorder) :
    ID(IDFactory::GetID()), Wind(window), Owner(owner)
{
    LEVIATHAN_ASSERT(Owner, "Layer has no owner");
    LEVIATHAN_ASSERT(Wind, "Layer has no window");
    LEVIATHAN_ASSERT(renderorder >= GUI_WORKSPACE_BEGIN_ORDER, "invalid render order passed");

    // Setup the scene
    Ogre::Root& ogre = Ogre::Root::getSingleton();

    // Create the scene manager //
    Scene = ogre.createSceneManager(Ogre::ST_GENERIC, 1, Ogre::INSTANCING_CULLING_SINGLETHREAD,
        "GUI_Layer_" + std::to_string(ID));

    // Create an orthographic camera //
    Camera = Scene->createCamera("layer camera");
    Camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
    // Setup permanent direction
    Camera->setPosition(0, 0, -10);
    Camera->lookAt(0, 0, 0);

    // Update properties for the window size
    AdjustCameraProperties();

    // Create the workspace for this layer that will render us after the normal scene
    Workspace = ogre.getCompositorManager2()->addWorkspace(
        Scene, window->GetOgreWindow(), Camera, "OverlayWorkspace", true, renderorder);
}

DLLEXPORT Layer::~Layer()
{
    LEVIATHAN_ASSERT(!Workspace, "ReleaseResources wasn't called on Layer");
}
// ------------------------------------ //
DLLEXPORT void Layer::ReleaseResources()
{
    // Release derived type resources first
    _DoReleaseResources();

    // Destroy the compositor //
    Ogre::Root& ogre = Ogre::Root::getSingleton();

    // Allow releasing twice
    if(Workspace) {
        ogre.getCompositorManager2()->removeWorkspace(Workspace);
        Workspace = nullptr;
    }

    if(Scene) {
        ogre.destroySceneManager(Scene);
        Scene = nullptr;
        Camera = nullptr;
    }
}
// ------------------------------------ //
DLLEXPORT void Layer::NotifyWindowResized()
{
    // Adjust camera

    // And let derived classes do their thing
    _OnWindowResized();
}

DLLEXPORT void Layer::NotifyFocusUpdate(bool focused)
{
    if(OurFocus == focused)
        return;

    OurFocus = focused;
    _OnFocusChanged();
}
// ------------------------------------ //
DLLEXPORT void Layer::AdjustCameraProperties()
{
    if(Camera) {
        int32_t width;
        int32_t height;
        Wind->GetSize(width, height);

        // Camera->setPosition(width / 2, height / 2, -10);
        Camera->setOrthoWindow(width, height);
    }
}
