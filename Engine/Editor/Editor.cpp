// ------------------------------------ //
#include "Editor.h"

#include "Entities/GameWorldFactory.h"
#include "Exceptions.h"
#include "GUI/GuiManager.h"
#include "Window.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //

// Maybe it wasn't the best idea to name the class the same thing as the namespace...
Editor::Editor::Editor(Window* targetwindow, Engine* engine) : _Engine(engine)
{
    _SetupOnWindow(targetwindow);

    World = _Engine->CreateWorld(targetwindow, static_cast<int>(INBUILT_WORLD_TYPE::Standard));

    if(!World)
        throw InvalidState("Failed to create needed world of type Standard for editor");
}

Editor::Editor::~Editor()
{
    if(ShownOnWindow)
        _CloseEditor();

    _Engine->DestroyWorld(World);
    World.reset();
}
// ------------------------------------ //
void Editor::Editor::_SetupOnWindow(Window* targetwindow)
{
    if(ShownOnWindow)
        _CloseEditor();

    ShownOnWindow = targetwindow;

    GuiManager* guiManager = ShownOnWindow->GetGui();

    if(!guiManager->LoadGUIFile("Data/Scripts/GUI/EditorGUI.html")) {

        Logger::Get()->Error("Editor: failed to load the gui");
        return;
    }
}

void Editor::Editor::_CloseEditor()
{
    if(!Engine::Get()->CloseWindow(ShownOnWindow)) {
        LOG_WARNING("Editor: failed to close window");
    }

    // TODO: release editor resources, once there are some

    ShownOnWindow = nullptr;
}
