// ------------------------------------ //
#include "Editor.h"

#include "Entities/GameWorldFactory.h"
#include "Exceptions.h"
#include "GUI/GuiManager.h"
#include "Generated/StandardWorld.h"
#include "Handlers/ObjectLoader.h"
#include "Window.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //

// Maybe it wasn't the best idea to name the class the same thing as the namespace...
Editor::Editor::Editor(Window* targetwindow, Engine* engine) : _Engine(engine)
{
    _SetupOnWindow(targetwindow);
}

Editor::Editor::~Editor()
{
    if(ShownOnWindow)
        _CloseEditor();
}
// ------------------------------------ //
void Editor::Editor::BringToFront()
{
    if(ShownOnWindow)
        ShownOnWindow->BringToFront();
}
// ------------------------------------ //
void Editor::Editor::_SetupOnWindow(Window* targetwindow)
{
    if(ShownOnWindow)
        _CloseEditor();

    ShownOnWindow = targetwindow;

    World = std::dynamic_pointer_cast<StandardWorld>(
        _Engine->CreateWorld(targetwindow, static_cast<int>(INBUILT_WORLD_TYPE::Standard),
            // no physics
            nullptr, WorldNetworkSettings::GetSettingsForClient()));

    if(!World) {
        LOG_ERROR("Editor: failed to create needed world of type Standard for editor");
    }

    GuiManager* guiManager = ShownOnWindow->GetGui();

    if(!guiManager->LoadGUIFile("Data/EditorResources/GUI/EditorGUI.html")) {

        LOG_ERROR("Editor: failed to load the gui");
        return;
    }

    ObjectID camera =
        ObjectLoader::LoadCamera(*World, Float3(0, 0, 5), Float4::IdentityQuaternion());

    World->SetCamera(camera);


    // ------------------------------------ //
    // Test cube
    // ObjectID box = World->CreateEntity();
    // World->Create_Position(box, Float3(0, 0, 0), Float4::IdentityQuaternion());
    // auto& renderNode = World->Create_RenderNode(box);
    // World->Create_Model(box, renderNode.Node, "UnitCube.mesh");
}

void Editor::Editor::_CloseEditor()
{
    if(!Engine::Get()->CloseWindow(ShownOnWindow)) {
        LOG_WARNING("Editor: failed to close window");
    }

    // Release editor resources

    _Engine->DestroyWorld(World);
    World.reset();

    ShownOnWindow = nullptr;
}
