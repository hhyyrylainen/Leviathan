// ------------------------------------ //
#include "Editor.h"

#include "Entities/GameWorldFactory.h"
#include "Exceptions.h"
#include "GUI/GuiManager.h"
#include "Generated/StandardWorld.h"
#include "Handlers/ObjectLoader.h"
#include "Rendering/Material.h"
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
void Editor::Editor::Tick(float elapsed)
{
    if(RotateModelSet) {
        CurrentRotationPassed += elapsed;

        RotateModel(
            Quaternion(Float3::UnitYAxis, Degree(RotateSpeed * CurrentRotationPassed)));
    }
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

    CameraID = ObjectLoader::LoadCamera(*World, Float3(0, 0, 5), Quaternion::IDENTITY);

    CameraPos = &World->GetComponent_Position(CameraID);
    CameraProps = &World->GetComponent_Camera(CameraID);
    CameraProps->FOV = 45;
    CameraProps->Marked = true;

    World->SetCamera(CameraID);


    // ------------------------------------ //
    // Test model
    // LoadModel("DamagedHelmet.gltf");
    // AutoRotateModel(true);
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

    // TODO: notify engine that this is closed
}
// ------------------------------------ //
DLLEXPORT void Editor::Editor::LoadModel(const std::string& file)
{
    if(LoadedModel != NULL_OBJECT)
        UnloadModel();

    LoadedModel = World->CreateEntity();
    World->Create_Position(LoadedModel, Float3(0, 0, 0), Quaternion::IDENTITY);
    auto& renderNode = World->Create_RenderNode(LoadedModel);

    World->Create_Model(LoadedModel, file, Material::MakeShared<Material>());
}

DLLEXPORT void Editor::Editor::UnloadModel()
{
    if(LoadedModel != NULL_OBJECT) {
        if(World)
            World->QueueDestroyEntity(LoadedModel);
        LoadedModel = NULL_OBJECT;
    }
}
// ------------------------------------ //
DLLEXPORT void Editor::Editor::PositionModel(const Float3& pos)
{
    if(!World)
        return;

    auto* component = World->GetComponentPtr_Position(LoadedModel);

    if(component) {
        component->Members._Position = pos;
        component->Marked = true;
    }
}

DLLEXPORT void Editor::Editor::RotateModel(const Quaternion& rotation)
{
    if(!World)
        return;

    auto* component = World->GetComponentPtr_Position(LoadedModel);

    if(component) {
        component->Members._Orientation = rotation;
        component->Marked = true;
    }
}

DLLEXPORT void Editor::Editor::ScaleModel(const Float3& scales)
{
    if(!World)
        return;

    auto* component = World->GetComponentPtr_RenderNode(LoadedModel);

    if(component) {
        component->Scale = scales;
        component->Marked = true;
    }
}
// ------------------------------------ //
DLLEXPORT void Editor::Editor::AutoRotateModel(bool autorotate)
{
    CurrentRotationPassed = 0.f;
    RotateModelSet = autorotate;
}
// ------------------------------------ //
DLLEXPORT void Editor::Editor::PositionCamera(const Float3& pos)
{
    CameraPos->Members._Position = pos;
    CameraPos->Marked = true;
}

DLLEXPORT void Editor::Editor::RotateCamera(const Quaternion& rotation)
{
    CameraPos->Members._Orientation = rotation;
    CameraPos->Marked = true;
}
