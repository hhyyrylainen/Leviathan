// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "bsfCore/Renderer/BsRendererExtension.h"

namespace Leviathan {

struct GUIOverlayInitializationData {
    bs::SPtr<bs::ct::Mesh> ScreenQuad;
    bs::SPtr<bs::ct::Material> QuadMaterial;
};

class GUIOverlayRenderer final : public bs::RendererExtension {
public:
    GUIOverlayRenderer();

    bool check(const bs::ct::Camera& camera) override
    {
        return GetOverlaysOnCamera(camera) != nullptr;
    }

    void render(const bs::ct::Camera& camera) override;

    void initialize(const bs::Any& data) override;
    void destroy() override;

    //! \param target is the identifier for the bs::RenderTarget, currently the pointer to the
    //! core version
    void UpdateShownOverlays(
        uint64_t target, const std::vector<bs::SPtr<bs::ct::Texture>>& overlays)
    {
        LOG_INFO("shown overlay count (on: " + std::to_string(target) +
                 "): " + std::to_string(overlays.size()));
        FullScreenOverlays[target] = overlays;
    }

private:
    std::vector<bs::SPtr<bs::ct::Texture>>* GetOverlaysOnCamera(const bs::ct::Camera& camera);

private:
    bs::SPtr<bs::ct::Mesh> ScreenQuad;
    bs::SPtr<bs::ct::Material> QuadMaterial;
    bs::SPtr<bs::ct::GpuParamsSet> QuadParamsSet;

    std::unordered_map<uint64_t, std::vector<bs::SPtr<bs::ct::Texture>>> FullScreenOverlays;
};

} // namespace Leviathan
