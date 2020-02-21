// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <memory>

namespace Leviathan {

class Graphics;
class Window;

class Mesh;
class Texture;

namespace GUI {

class GuiManager;


//! \brief Provides rendering operations for the GUI to use
//!
//! There is one renderer per GuiManager (so one per window)
class GuiRenderer {
    struct Implementation;

    friend GuiManager;

public:
    GuiRenderer(GuiManager& owner);
    ~GuiRenderer();

    // ------------------------------------ //
    // Rendering operations
    //! \brief Draws a mesh with orthographic projection with transparent supported material
    //! and an explicit alpha multiply
    //! \note Currently only quad compatible meshes (triangle list, right vertex attributes)
    //! work. Other meshes will just break rendering
    DLLEXPORT void DrawTransparentWithAlpha(Mesh& mesh, Texture& texture, float xoffset = 0.f,
        float yoffset = 0.f, float alpha = 1.f);

protected:
    void Init(Graphics* graphics, Window* window);

    //! \brief Makes sure all rendering resources are setup
    void OnBeginRendering();
    void OnEndRendering();

private:
    GuiManager& Owner;
    Graphics* Graph = nullptr;
    Window* Wind = nullptr;
    std::unique_ptr<Implementation> Pimpl;
};

} // namespace GUI
} // namespace Leviathan
