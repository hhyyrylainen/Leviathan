// ------------------------------------ //
#include "AppDefine.h"

#include "Application/Application.h"
#include "FileSystem.h"
#include "ForwardDeclarations.h"
#include "GameConfiguration.h"
#include "KeyConfiguration.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Sound/SoundDevice.h"

#include <iostream>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AppDef::AppDef(const bool& isdef /*= false*/) :
    ConfigurationValues(new NamedVars()),
#ifdef _WIN32
    HInstance(NULL)
#else
    HInstance(0)
#endif //_WIN32
{
    // If this is the default configuration set as the static access one //
    if(isdef)
        Defaultconf = this;
}

AppDef::~AppDef()
{
    // reset static access if this is it //
    if(Defaultconf == this)
        Defaultconf = nullptr;

    SAFE_RELEASEDEL(_GameConfiguration);
    SAFE_RELEASEDEL(_KeyConfiguration);
}

AppDef* Leviathan::AppDef::Defaultconf = nullptr;
// ------------------------------------ //
NamedVars* Leviathan::AppDef::GetValues()
{
    return ConfigurationValues.get();
}

DLLEXPORT AppDef* Leviathan::AppDef::GenerateAppdefine(const std::string& engineconfigfile,
    const std::string& gameconfig, const std::string& keyconfig,
    std::function<void(Lock& guard, GameConfiguration* configobj)> configchecker,
    std::function<void(Lock& guard, KeyConfiguration* keysobject)> keychecker)
{
    auto tmpptr = std::make_unique<AppDef>(true);

    // We created a new one //
    tmpptr->DeleteLog = true;

    if(!Logger::Get()) {

        std::cout << "Error: main log hasn't been created before AppDef: GenerateAppdefine"
                  << std::endl;
        return nullptr;
    }

    // load variables from configuration file //
    // TODO: refactor this duplicated code
    if(!tmpptr->ConfigurationValues->LoadVarsFromFile(engineconfigfile, Logger::Get())) {

        // Set default values //
        tmpptr->FillDefaultEngineConf(*tmpptr->ConfigurationValues);

        // Write it to the file for next time //
        if(!FileSystem::WriteToFile(
               tmpptr->ConfigurationValues->Serialize(), engineconfigfile)) {

            LOG_ERROR(
                "AppDef: failed to write default engine config to file: " + engineconfigfile);
        }
    } else {
        // Fill missing values //
        if(tmpptr->FillDefaultEngineConf(*tmpptr->ConfigurationValues)) {

            // Save changes
            if(!FileSystem::WriteToFile(
                   tmpptr->ConfigurationValues->Serialize(), engineconfigfile)) {

                LOG_ERROR("AppDef: failed to write amended engine config to file: " +
                          engineconfigfile);
            }
        }
    }

    // Load game configuration //
    tmpptr->_GameConfiguration = new GameConfiguration(gameconfig);

    if(!tmpptr->_GameConfiguration->Init(configchecker)) {

        return nullptr;
    }

    // Load key configuration //
    tmpptr->_KeyConfiguration = new KeyConfiguration(keyconfig);

    if(!tmpptr->_KeyConfiguration->Init(keychecker)) {

        return nullptr;
    }

    return tmpptr.release();
}

DLLEXPORT void AppDef::ReplaceGameAndKeyConfigInMemory(
    std::function<void(Lock& guard, GameConfiguration* configobj)> configchecker /*= nullptr*/,
    std::function<void(Lock& guard, KeyConfiguration* keysobject)> keychecker /*= nullptr*/)
{
    if(_GameConfiguration)
        delete _GameConfiguration;

    _GameConfiguration = new GameConfiguration();

    if(!_GameConfiguration->Init(configchecker)) {

        LOG_ERROR("AppDef: failed to init new in-memory game configuration");
    }

    // Load key configuration //
    _KeyConfiguration = new KeyConfiguration();

    if(!_KeyConfiguration->Init(keychecker)) {

        LOG_ERROR("AppDef: failed to init new in-memory key configuration");
    }
}


#ifdef _WIN32
DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(const std::string& title,
    const bool& windowborder, HICON icon, LeviathanApplication* appvirtualptr)
{

#else
DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(
    const std::string& title, const bool& windowborder, LeviathanApplication* appvirtualptr)
{
#endif
    // store the parameters to be used for window creation //
    int width;
    int height;
    std::string fullscreen;
    int FSAA;
    int displayNumber;
    bool vsync;

    ObjectFileProcessor::LoadValueFromNamedVars(
        ConfigurationValues.get(), "Width", width, 1280, Logger::Get(), "Create window: ");
    ObjectFileProcessor::LoadValueFromNamedVars(
        ConfigurationValues.get(), "Height", height, 720, Logger::Get(), "Create window: ");
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(ConfigurationValues.get(),
        "FullScreen", fullscreen, "no", Logger::Get(), "Create window: ");
    ObjectFileProcessor::LoadValueFromNamedVars<int>(
        ConfigurationValues.get(), "WindowMultiSampleCount", FSAA, 1);
    ObjectFileProcessor::LoadValueFromNamedVars<int>(ConfigurationValues.get(),
        "DisplayNumber", displayNumber, 0, Logger::Get(), "Create window: ");
    ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        *ConfigurationValues, "Vsync", vsync, false, Logger::Get(), "Create window: ");

#ifdef _WIN32
    this->SetWindowDetails(WindowDataDetails(title, width, height, fullscreen, vsync,
        displayNumber, FSAA, windowborder, icon, appvirtualptr));
#else
    this->SetWindowDetails(WindowDataDetails(title, width, height, fullscreen, vsync,
        displayNumber, FSAA, windowborder, appvirtualptr));
#endif
}

DLLEXPORT AppDef& Leviathan::AppDef::SetApplicationIdentification(
    const std::string& userreadable, const std::string& gamename,
    const std::string& gameversion)
{
    UserReadableGame = userreadable;
    Game = gamename;
    GameVersion = gameversion;
    LeviathanVersion = LEVIATHAN_VERSION_ANSIS;

    return *this;
}

DLLEXPORT void Leviathan::AppDef::GetGameIdentificationData(
    std::string& userreadable, std::string& gamename, std::string& gameversion)
{
    userreadable = UserReadableGame;
    gamename = Game;
    gameversion = GameVersion;
}
// ------------------------------------ //
DLLEXPORT bool AppDef::FillDefaultEngineConf(NamedVars& variables)
{
    bool changed = false;

    if(variables.ShouldAddValueIfNotFoundOrWrongType<bool>("Vsync")) {
        changed = true;
        variables.Add(std::make_shared<NamedVariableList>("Vsync", false));
    }

    if(variables.ShouldAddValueIfNotFoundOrWrongType<std::string>("FullScreen")) {
        changed = true;
        variables.Add(
            std::make_shared<NamedVariableList>("FullScreen", new StringBlock("no")));
    }

    if(variables.ShouldAddValueIfNotFoundOrWrongType<int>("DisplayNumber")) {
        changed = true;
        variables.Add(std::make_shared<NamedVariableList>("DisplayNumber", 0));
    }

    if(variables.ShouldAddValueIfNotFoundOrWrongType<int>("Width")) {
        changed = true;
        variables.Add(std::make_shared<NamedVariableList>("Width", 1280));
    }

    if(variables.ShouldAddValueIfNotFoundOrWrongType<int>("Height")) {
        changed = true;
        variables.Add(std::make_shared<NamedVariableList>("Height", 720));
    }

    if(variables.ShouldAddValueIfNotFoundOrWrongType<int>("MaxFPS")) {
        changed = true;
        variables.Add(std::make_shared<NamedVariableList>("MaxFPS", 144));
    }

    if(variables.ShouldAddValueIfNotFoundOrWrongType<std::string>("AudioDevice")) {

        changed = true;
        variables.Add(
            std::make_shared<NamedVariableList>("AudioDevice", new StringBlock("default")));
    }

    return changed;
}

// ------------------ WindowDataDetails ------------------ //
#ifdef _WIN32
Leviathan::WindowDataDetails::WindowDataDetails(const std::string& title, const int& width,
    const int& height, const std::string& fullscreen, bool vsync, int displaynumber, int fsaa,
    const bool& windowborder, HICON icon, LeviathanApplication* appvirtualptr) :
    Title(title),
    Width(width), Height(height), FullScreen(fullscreen), VSync(vsync),
    DisplayNumber(displaynumber), FSAA(fsaa), Icon(icon)
{}
#else
Leviathan::WindowDataDetails::WindowDataDetails(const std::string& title, const int& width,
    const int& height, const std::string& fullscreen, bool vsync, int displaynumber, int fsaa,
    const bool& windowborder, LeviathanApplication* appvirtualptr) :
    Title(title),
    Height(height), Width(width), FullScreen(fullscreen), VSync(vsync),
    DisplayNumber(displaynumber), FSAA(fsaa)
{}
#endif

Leviathan::WindowDataDetails::WindowDataDetails() {}

#ifdef _WIN32
void Leviathan::WindowDataDetails::ApplyIconToHandle(HWND hwnd) const
{

    // send set icon message //
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Icon);
}
#else

#endif
