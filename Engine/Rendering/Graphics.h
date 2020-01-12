// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"

#include "bsfCore/RenderAPI/BsRenderWindow.h"

namespace Leviathan {

enum class GRAPHICS_API { D3D11, D3D12, Vulkan, OpenGL, OpenGLES, Metal };

class WindowRenderingResources;

class Graphics {
    friend Window;

    struct Implementation;

public:
    DLLEXPORT Graphics();
    DLLEXPORT ~Graphics();

    DLLEXPORT bool Init(AppDef* appdef);
    DLLEXPORT void Release();

    DLLEXPORT bool Frame();

    //! \todo remove
    DLLEXPORT void UpdateShownOverlays(
        bs::RenderTarget& target, const std::vector<bs::SPtr<bs::Texture>>& overlays);

    DLLEXPORT bool IsVerticalUVFlipped() const;

    inline auto GetUsedAPI() const
    {
        return SelectedAPI;
    }

    DLLEXPORT std::string GetUsedAPIName() const;

    // ------------------------------------ //
    // Resource loading helpers

    //! \brief Finds and loads a shader with the name
    //!
    //! If a full path or a valid relative path is specified a full search is not done. Unless
    //! a variant of the name with ".asset" is found, which is preferred to skip expensive
    //! importing.
    DLLEXPORT bs::HShader LoadShaderByName(const std::string& name);

    //! Works the same as LoadShaderByName
    DLLEXPORT bs::HTexture LoadTextureByName(const std::string& name);

    //! Works the same as LoadShaderByName
    DLLEXPORT bs::HMesh LoadMeshByName(const std::string& name);

    //! Works the same as LoadShaderByName
    DLLEXPORT bs::HAnimationClip LoadAnimationClipByName(const std::string& name);

#ifdef __linux
    //! \brief Returns true if our X11 error handler has been called. Remember to check this
    //! after every X11 call
    //!
    //! The value is reset to false after this call
    //! \note This is not thread safe. X11 is also not thread safe so only call on the main
    //! thread
    DLLEXPORT static bool HasX11ErrorOccured();
#endif

protected:
    //! \brief Called when Window objects are created to create the rendering resources for
    //! them
    std::unique_ptr<WindowRenderingResources> RegisterCreatedWindow(Window& window);

    //! \brief Called just before a window is destroyed. This needs to stop rendering to it
    //! \returns True if the window was primary and should only be hidden instead of destroyed
    bool UnRegisterWindow(Window& window);

private:
    void PrintDetectedSystemInformation();

    bool InitializeBSF(AppDef* appdef);
    void ShutdownBSF();

    bool InitializeDiligent(AppDef* appdef);
    void ShutdownDiligent();

    bool SelectPreferredGraphicsAPI(AppDef* appdef);
    //! Check that API selection is good and perform some initialization for some APIs
    bool CheckAndInitializeSelectedAPI();

private:
    bool Initialized = false;
    bool FirstWindowCreated = false;

    GRAPHICS_API SelectedAPI;

    std::unique_ptr<Implementation> Pimpl;
};
} // namespace Leviathan
