// ------------------------------------ //
#include "GUIOverlayRenderer.h"

#include "bsfCore/Material/BsGpuParamsSet.h"
#include "bsfCore/Mesh/BsMesh.h"
#include "bsfCore/RenderAPI/BsRenderAPI.h"
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
    auto& rapi = bs::ct::RenderAPI::instance();

    bs::ct::gRendererUtility().setPass(QuadMaterial);

    bs::ct::gRendererUtility().setPassParams(QuadParamsSet);

    for(const auto& texture : FullScreenOverlays) {

        QuadMaterial->setTexture("image", texture);
        QuadParamsSet->update(QuadMaterial->_getInternalParams());

        // LOG_WRITE("drawing");
        bs::ct::gRendererUtility().draw(ScreenQuad);
    }
}
