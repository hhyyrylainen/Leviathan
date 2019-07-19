// ------------------------------------ //
#include "GuiLayer.h"

#include "Handlers/IDFactory.h"
#include "Window.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT Layer::Layer(GuiManager* owner, Window* window, int renderorder) :
    ID(IDFactory::GetID()), Wind(window), Owner(owner), RenderOrder(renderorder)
{
    LEVIATHAN_ASSERT(Owner, "Layer has no owner");
    LEVIATHAN_ASSERT(Wind, "Layer has no window");

    // Setup the scene
    // BSFLayerHack = 1 << (++LayerNumber);

    // CameraSO = bs::SceneObject::create(("LayerCamera_" + std::to_string(ID)).c_str());

    // // Create an orthographic camera //
    // Camera = CameraSO->addComponent<bs::CCamera>();
    // Camera->setProjectionType(bs::PT_ORTHOGRAPHIC);
    // // BSF has no multiple scenes so we do this
    // Camera->setLayers(BSFLayerHack);

    // // Setup permanent direction
    // CameraSO->setPosition(bs::Vector3(0, 0, -10));
    // CameraSO->lookAt(bs::Vector3(0, 0, 0));

    // // Update properties for the window size
    // AdjustCameraProperties();

    // // Attach to window
    // const auto& viewport = Camera->getViewport();
    // viewport->setTarget(window->GetBSFWindow());
    // viewport->setClearFlags(bs::ClearFlagBits::Depth | bs::ClearFlagBits::Stencil);
    // viewport->setClearColorValue(bs::Color::White);
    // auto& settings = Camera->getRenderSettings();
    // settings->autoExposure.minEyeAdaptation = 1;
    // settings->autoExposure.maxEyeAdaptation = 1;
    // settings->enableAutoExposure = false;
    // settings->enableHDR = false;
    // settings->enableLighting = false;
    // settings->enableShadows = false;
    // settings->enableTonemapping = false;
    // Camera->setRenderSettings(settings);

    // // Setup the order this is rendered in
    // // Higher priority is first so negating the order must make it be later
    // Camera->setPriority(-renderorder);
}

DLLEXPORT Layer::~Layer()
{
    // LEVIATHAN_ASSERT(!CameraSO, "ReleaseResources wasn't called on Layer");
}
// ------------------------------------ //
DLLEXPORT void Layer::ReleaseResources()
{
    // Release derived type resources first
    _DoReleaseResources();

    // Camera = nullptr;
    // CameraSO->destroy();
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
    // if(Camera) {
    //     int32_t width;
    //     int32_t height;
    //     Wind->GetSize(width, height);

    //     Camera->setPosition(width / 2, height / 2, -10);
    //     Camera->setOrthoWindow(width, height);
    // }
}
