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

    ReadWindowSize();

    if(Width <= 1 || Height <= 1)
        LEVIATHAN_ASSERT(false, "Gui Layer failed to get window size");

    // Setup the scene
    Ogre::Root& ogre = Ogre::Root::getSingleton();

    // Create the scene manager //
    Scene = ogre.createSceneManager(Ogre::ST_GENERIC, 1, Ogre::INSTANCING_CULLING_SINGLETHREAD,
        "GUI_Layer_" + std::to_string(ID));

    // Make all the layers work by setting the right render types on them
    auto* queue = Scene->getRenderQueue();
    for(int i = 0; i <= std::numeric_limits<uint8_t>::max(); ++i) {
        queue->setRenderQueueMode(static_cast<uint8_t>(i), Ogre::RenderQueue::FAST);
    }

    // Create an orthographic camera //
    Camera = Scene->createCamera("layer camera");
    // Camera->setFixedYawAxis(true, -Ogre::Vector3::UNIT_Y);
    Camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
    Camera->setPosition(0, 0, 10000);
    Camera->lookAt(0, 0, 0);
    // This is not an awesome idea
    // Camera->roll(Ogre::Degree(180));
    Camera->setNearClipDistance(1);

    if(!ogre.getRenderSystem()->getCapabilities()->hasCapability(
           Ogre::RSC_INFINITE_FAR_PLANE)) {

        LOG_FATAL("Ogre render system reports infinite far plane not being available");
    }

    Camera->setFarClipDistance(0);

    // Update properties for the window size
    // For some reason this doesn't work here
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
DLLEXPORT void Layer::BringToFront()
{
    Ogre::Root& ogre = Ogre::Root::getSingleton();

    auto target = Workspace->getFinalTarget();

    // Allow releasing twice
    ogre.getCompositorManager2()->removeWorkspace(Workspace);
    Workspace = nullptr;

    Workspace = ogre.getCompositorManager2()->addWorkspace(
        Scene, target, Camera, "OverlayWorkspace", true, -1);
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
    ReadWindowSize();

    // Adjust camera
    AdjustCameraProperties();

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
        Camera->setPosition(Width / 2, -Height / 2, 10000);
        Camera->setOrthoWindow(Width, Height);
    }
}
// ------------------------------------ //
DLLEXPORT void Layer::ReadWindowSize()
{
    Wind->GetSize(Width, Height);
}
