// ------------------------------------ //
#include "Editor.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //

// Maybe it wasn't the best idea to name the class the same thing as the namespace...
Editor::Editor::Editor() {}
Editor::Editor::~Editor() {}
// ------------------------------------ //
void Editor::Editor::_SetupOnWindow(Window* targetwindow)
{
    if(ShownOnWindow)
        _CloseEditor();

    // GuiManager* guiManager = window1->GetGui();

    // if(!guiManager->LoadGUIFile("https://www.google.fi")) {

    //     Logger::Get()->Error("Editor: failed to load the GuiFile, quitting");
    //     LeviathanApplication::Get()->StartRelease();
    //     return;
    // }
}

void Editor::Editor::_CloseEditor()
{
    // if(!Engine::Get()->CloseWindow(ShownOnWindow)) {
    //     LOG_WARNING("Editor: failed to close window");
    // }

    // TODO: release editor resources

    ShownOnWindow = nullptr;
}
