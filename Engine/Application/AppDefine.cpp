// ------------------------------------ //
#include "AppDefine.h"

#include "ObjectFiles/ObjectFileProcessor.h"
#include "Application/Application.h"
#include "GameConfiguration.h"
#include "ForwardDeclarations.h"
#include "KeyConfiguration.h"

#include <iostream>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AppDef::AppDef(const bool &isdef /*= false*/) :
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

AppDef::~AppDef(){
	// reset static access if this is it //
	if(Defaultconf == this)
		Defaultconf = nullptr;

	SAFE_RELEASEDEL(_GameConfiguration);
	SAFE_RELEASEDEL(_KeyConfiguration);
}

AppDef* Leviathan::AppDef::Defaultconf = nullptr;
// ------------------------------------ //
NamedVars* Leviathan::AppDef::GetValues(){
	return ConfigurationValues.get();
}

DLLEXPORT AppDef* Leviathan::AppDef::GenerateAppdefine(const std::string &engineconfigfile,
    const std::string &gameconfig, const std::string &keyconfig,
    std::function<void (Lock &guard, GameConfiguration* configobj)> configchecker,
    std::function<void (Lock &guard, KeyConfiguration* keysobject)> keychecker)
{
    auto tmpptr = std::make_unique<AppDef>(true);
    
    // We created a new one //
    tmpptr->DeleteLog = true;

    if(!Logger::Get()){

        std::cout << "Error: main log hasn't been created before AppDef: GenerateAppdefine" <<
            std::endl;
        return nullptr;
    }

	// load variables from configuration file //
	tmpptr->ConfigurationValues->LoadVarsFromFile(engineconfigfile, Logger::Get());

	// Load game configuration //
	tmpptr->_GameConfiguration = new GameConfiguration(gameconfig);

	if(!tmpptr->_GameConfiguration->Init(configchecker)){

		return nullptr;
	}

	// Load key configuration //
	tmpptr->_KeyConfiguration = new KeyConfiguration(keyconfig);

	if(!tmpptr->_KeyConfiguration->Init(keychecker)){

		return nullptr;
	}

	return tmpptr.release();
}


#ifdef _WIN32
DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(const std::string &title,
    const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr)
{

#else
DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(const std::string &title,
    const bool &windowborder, LeviathanApplication* appvirtualptr)
{
#endif

	// store the parameters to be used for window creation //
	int width;
	int height;
	bool window;
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), "Width", width,
        1280, Logger::Get(), "Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), "Height", height,
        720, Logger::Get(), "Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), "Windowed", window,
        true, Logger::Get(), "Create window: ");
    
#ifdef _WIN32
	this->SetWindowDetails(WindowDataDetails(title, width, height, window, windowborder, icon, appvirtualptr));
#else
	this->SetWindowDetails(WindowDataDetails(title, width, height, window, windowborder, appvirtualptr));
#endif
}

DLLEXPORT AppDef& Leviathan::AppDef::SetApplicationIdentification(const std::string &userreadable,
    const std::string &gamename, const std::string &gameversion)
{
	UserReadableGame = userreadable;
	Game = gamename;
	GameVersion = gameversion;
	LeviathanVersion = LEVIATHAN_VERSION_ANSIS;

	return *this;
}

DLLEXPORT void Leviathan::AppDef::GetGameIdentificationData(std::string &userreadable,
    std::string &gamename, std::string &gameversion)
{
	userreadable = UserReadableGame;
	gamename = Game;
	gameversion = GameVersion;
}

DLLEXPORT bool Leviathan::AppDef::GetVSync(){
	bool vsync;

	ObjectFileProcessor::LoadValueFromNamedVars<bool>(
        *ConfigurationValues, "Vsync", vsync, false);
	return vsync;
}

// ------------------ WindowDataDetails ------------------ //
#ifdef _WIN32
Leviathan::WindowDataDetails::WindowDataDetails(const std::string &title, const int &width,
    const int &height, const bool &windowed,
	const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr) :
    Title(title), Width(width), Height(height), Windowed(windowed), Icon(icon)
{

}
#else
Leviathan::WindowDataDetails::WindowDataDetails(const std::string &title, const int &width,
    const int &height, const bool &windowed,
	const bool &windowborder, LeviathanApplication* appvirtualptr) :
    Title(title), Height(height), Width(width), Windowed(windowed)
{

}
#endif

Leviathan::WindowDataDetails::WindowDataDetails(){

}

#ifdef _WIN32
void Leviathan::WindowDataDetails::ApplyIconToHandle(HWND hwnd) const{

	// send set icon message //
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Icon);
}
#else

#endif
