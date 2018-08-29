// ------------------------------------ //
#include "EditorApplication.h"


#include "Application/GameConfiguration.h"
#include "Application/KeyConfiguration.h"
#include "Events/EventHandler.h"
#include "GUI/GuiManager.h"
#include "Handlers/ObjectLoader.h"
#include "Script/Bindings/BindHelpers.h"
#include "Window.h"

using namespace Editor;
using namespace Leviathan;
// ------------------------------------ //
EditorApplication::EditorApplication()
{
    StaticGame = this;
}

EditorApplication::~EditorApplication() {}

EditorApplication* EditorApplication::Get()
{
    return StaticGame;
}

EditorApplication* EditorApplication::StaticGame = NULL;

Leviathan::NetworkInterface* EditorApplication::_GetApplicationPacketHandler()
{
    if(!ClientInterface) {
        ClientInterface = std::make_unique<EditorNetHandler>();
    }
    return ClientInterface.get();
}

void EditorApplication::_ShutdownApplicationPacketHandler()
{
    ClientInterface.reset();
}
// ------------------------------------ //
std::string EditorApplication::GenerateWindowTitle()
{
    return std::string("Standalone Editor for Leviathan " LEVIATHAN_VERSION_ANSIS);
}
// ------------------------------------ //
void EditorApplication::Tick(int mspassed) {}
// ------------------------------------ //
void EditorApplication::CustomizeEnginePostLoad()
{
    auto* engine = Engine::GetEngine();

    Leviathan::Window* window1 = engine->GetWindowEntity();

    engine->OpenEditorWindow(window1);
}

void EditorApplication::EnginePreShutdown() {}
// ------------------------------------ //
bool EditorApplication::InitLoadCustomScriptTypes(asIScriptEngine* engine)
{
    return true;
}
// ------------------------------------ //
void EditorApplication::CheckGameConfigurationVariables(
    Lock& guard, GameConfiguration* configobj)
{
    // Check for various variables //
    NamedVars* vars = configobj->AccessVariables(guard);

    if(vars->ShouldAddValueIfNotFoundOrWrongType<std::string>("SomeEditorVariable")) {
        // Add new //
        vars->AddVar("SomeEditorVariable", new VariableBlock(new StringBlock("has a value")));
        configobj->MarkModified(guard);
    }
}

void EditorApplication::CheckGameKeyConfigVariables(
    Lock& guard, KeyConfiguration* keyconfigobj)
{
    keyconfigobj->AddKeyIfMissing(guard, "MenuKey", {"Q"});
}
// ------------------------------------ //
