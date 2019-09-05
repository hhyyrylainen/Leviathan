// ------------------------------------ //
#include "GUIOverlayRenderer.h"

#include "Utility/Convert.h"

#include "bsfCore/Material/BsGpuParamsSet.h"
#include "bsfCore/Mesh/BsMesh.h"
#include "bsfCore/RenderAPI/BsRenderAPI.h"
#include "bsfCore/Renderer/BsCamera.h"
#include "bsfEngine/Renderer/BsRendererUtility.h"
using namespace Leviathan;
// ------------------------------------ //
GUIOverlayRenderer::GUIOverlayRenderer() :
    bs::RendererExtension(bs::RenderLocation::Overlay, 1)
{}
// ------------------------------------ //
void GUIOverlayRenderer::initialize(const bs::Any& data)
{
    const auto& initData = bs::any_cast_ref<GUIOverlayInitializationData>(data);

    ScreenQuad = initData.ScreenQuad;
    QuadMaterial = initData.QuadMaterial;

    if(QuadMaterial->getNumTechniques() < 1) {
        LOG_FATAL("No techniques in overlay material");
        return;
    }

    QuadParamsSet = QuadMaterial->createParamsSet(0);
    LOG_INFO("Initialized GUIOverlayRenderer");
}

void GUIOverlayRenderer::destroy()
{
    QuadParamsSet.reset();
    ScreenQuad.reset();
    QuadMaterial.reset();
    FullScreenOverlays.clear();
}
// ------------------------------------ //
void GUIOverlayRenderer::render(const bs::ct::Camera& camera)
{
    const auto* overlays = GetOverlaysOnCamera(camera);

    if(!overlays)
        return;

    auto& rapi = bs::ct::RenderAPI::instance();

    bs::ct::gRendererUtility().setPass(QuadMaterial);

    bs::ct::gRendererUtility().setPassParams(QuadParamsSet);

    for(const auto& texture : *overlays) {

        QuadMaterial->setTexture("image", texture);
        QuadParamsSet->update(QuadMaterial->_getInternalParams());

        bs::ct::gRendererUtility().draw(ScreenQuad);
    }
}
// ------------------------------------ //
std::vector<bs::SPtr<bs::ct::Texture>>* GUIOverlayRenderer::GetOverlaysOnCamera(
    const bs::ct::Camera& camera)
{
    const auto currentRenderTarget =
        reinterpret_cast<uint64_t>(camera.getViewport()->getTarget().get());

    const auto found = FullScreenOverlays.find(currentRenderTarget);

    if(found != FullScreenOverlays.end())
        return &found->second;

    return nullptr;
}
