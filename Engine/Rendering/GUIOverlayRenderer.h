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
        // TODO: check are some GUI resources used by this camera
        return true;
    }

    void render(const bs::ct::Camera& camera) override;

    void initialize(const bs::Any& data) override;
    void destroy() override;

    void UpdateShownOverlays(const std::vector<bs::SPtr<bs::ct::Texture>>& overlays)
    {
        LOG_INFO("shown overlay count: " + std::to_string(overlays.size()));
        FullScreenOverlays = overlays;
    }

private:
    bs::SPtr<bs::ct::Mesh> ScreenQuad;
    bs::SPtr<bs::ct::Material> QuadMaterial;
    bs::SPtr<bs::ct::GpuParamsSet> QuadParamsSet;

    std::vector<bs::SPtr<bs::ct::Texture>> FullScreenOverlays;
};

} // namespace Leviathan
