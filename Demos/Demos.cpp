// ------------------------------------ //
#include "Demos.h"

#include "Sample1.h"

#include "Application/GameConfiguration.h"
#include "Application/KeyConfiguration.h"
#include "Events/EventHandler.h"
#include "GUI/GuiManager.h"
#include "Handlers/ObjectLoader.h"
#include "Script/Bindings/BindHelpers.h"
#include "Window.h"

using namespace Demos;
using namespace Leviathan;
// ------------------------------------ //
DemosApplication::DemosApplication()
{
    StaticGame = this;
}

DemosApplication::~DemosApplication() {}

DemosApplication* DemosApplication::Get()
{
    return StaticGame;
}

DemosApplication* DemosApplication::StaticGame = NULL;

Leviathan::NetworkInterface* DemosApplication::_GetApplicationPacketHandler()
{
    if(!ClientInterface) {
        ClientInterface = std::make_unique<DemosNetHandler>();
    }
    return ClientInterface.get();
}

void DemosApplication::_ShutdownApplicationPacketHandler()
{
    ClientInterface.reset();
}

// ------------------------------------ //
std::string DemosApplication::GenerateWindowTitle()
{
    return std::string("Demos version " GAME_VERSIONS " Leviathan " LEVIATHAN_VERSION_ANSIS);
}
// ------------------------------------ //
void DemosApplication::Tick(int mspassed) {}
// ------------------------------------ //
void DemosApplication::CustomizeEnginePostLoad()
{
    auto* engine = Engine::GetEngine();
    Leviathan::Window* window1 = engine->GetWindowEntity();

    // Create game world //
    // TODO: physics materials
    World = std::dynamic_pointer_cast<Leviathan::StandardWorld>(engine->CreateWorld(
        window1, 0, nullptr, WorldNetworkSettings::GetSettingsForClient()));

    LEVIATHAN_ASSERT(World, "World creation failed");

    // Load GUI documents (but only if graphics are enabled) //
    if(Engine::Get()->GetNoGui()) {

        // Skip the graphical objects when not in graphical mode //
        return;
    }

    GuiManager* guiManager = window1->GetGui();

    if(!guiManager->LoadGUIFile("https://www.google.fi")) {

        Logger::Get()->Error("Demos: failed to load the GuiFile, quitting");
        LeviathanApplication::Get()->StartRelease();
        return;
    }

    // Link world to a window //
    window1->LinkObjects(World);

    // Create camera in the world //
    const auto camera = Leviathan::ObjectLoader::LoadCamera(
        *World, Float3(0.f, 1.8f, 0.f), Float4::IdentityQuaternion());

    World->SetCamera(camera);

    // // Create player input handler in the world

    // // link window input to game logic //
    // window1->SetCustomInputController(GameInputHandler);
}

void DemosApplication::EnginePreShutdown()
{
    // Release current sample
    CurrentSample.reset();
}
// ------------------------------------ //
void DemosApplication::PlaySample1()
{
    LOG_INFO("Playing sample 1");

    auto* event = new GenericEvent("SampleChanged");
    event->GetVariables()->Add(
        std::make_shared<NamedVariableList>("Sample", new StringBlock("Sample1")));

    GetEngine()->GetEventHandler()->CallEvent(event);

    CurrentSample = std::make_unique<Sample1>();

    CurrentSample->Start(*World);
}
// ------------------------------------ //
std::string GetDemosVersionProxy()
{
    return Demos_VERSIONS;
}

bool DemosApplication::InitLoadCustomScriptTypes(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("DemosApplication", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("DemosApplication", "void PlaySample1()",
           asMETHOD(DemosApplication, PlaySample1), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("DemosApplication@ GetDemosApplication()",
           asFUNCTION(DemosApplication::Get), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Version getting function //
    if(engine->RegisterGlobalFunction(
           "string GetDemosVersion()", asFUNCTION(GetDemosVersionProxy), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
void DemosApplication::CheckGameConfigurationVariables(
    Lock& guard, GameConfiguration* configobj)
{
    // Check for various variables //
    NamedVars* vars = configobj->AccessVariables(guard);

    if(vars->ShouldAddValueIfNotFoundOrWrongType<std::string>("SomeDemoVariable")) {
        // Add new //
        vars->AddVar("SomeDemoVariable", new VariableBlock(new StringBlock("has a value")));
        configobj->MarkModified(guard);
    }
}

void DemosApplication::CheckGameKeyConfigVariables(Lock& guard, KeyConfiguration* keyconfigobj)
{
    keyconfigobj->AddKeyIfMissing(guard, "MoveForward", {"W"});
    keyconfigobj->AddKeyIfMissing(guard, "MoveBackwards", {"S"});
    keyconfigobj->AddKeyIfMissing(guard, "MoveLeft", {"A"});
    keyconfigobj->AddKeyIfMissing(guard, "MoveRight", {"D"});
    keyconfigobj->AddKeyIfMissing(guard, "Jump", {"P"});
    keyconfigobj->AddKeyIfMissing(guard, "Crouch", {"CTRL"});
    keyconfigobj->AddKeyIfMissing(guard, "Prone", {"ALT"});
    keyconfigobj->AddKeyIfMissing(guard, "MenuKey", {"Q"});
}
// ------------------------------------ //
