// ------------------------------------ //
#include "AppDefine.h"

#include "ObjectFiles/ObjectFileProcessor.h"
#include "Application/Application.h"
#include "GameConfiguration.h"
#include "ForwardDeclarations.h"
#include "KeyConfiguration.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::AppDef::AppDef(const bool &isdef /*= false*/) :
    ConfigurationValues(new NamedVars()), 
#ifdef _WIN32
	HInstance(NULL), 
#else
	HInstance(0),
#endif //_WIN32
	_GameConfiguration(NULL),
	_KeyConfiguration(NULL), DeleteLog(false), _NetworkInterface(NULL), Mainlog(NULL)
{
	// If this is the default configuration set as the static access one //
	if(isdef)
		Defaultconf = this;
}

AppDef::~AppDef(){
	// reset static access if this is it //
	if(Defaultconf == this)
		Defaultconf = NULL;

	SAFE_RELEASEDEL(_GameConfiguration);
	SAFE_RELEASEDEL(_KeyConfiguration);

	if(DeleteLog)
		SAFE_DELETE(Mainlog);
}

AppDef* Leviathan::AppDef::Defaultconf = NULL;
// ------------------------------------ //
NamedVars* Leviathan::AppDef::GetValues(){
	return ConfigurationValues.get();
}

DLLEXPORT AppDef* Leviathan::AppDef::GenerateAppdefine(const std::string &logfile,
    const std::string &engineconfigfile, const std::string &gameconfig,
    const std::string &keyconfig, std::function<void (Lock &guard, GameConfiguration* configobj)>
    configchecker, std::function<void (Lock &guard, KeyConfiguration* keysobject)> keychecker)
{

	unique_ptr<AppDef> tmpptr(new AppDef(true));

	tmpptr->LogFile = logfile;

    // Always create the logger //
    tmpptr->Mainlog = new Logger(logfile+"Log.txt");
    
    // We created a new one //
    tmpptr->DeleteLog = true;

	// load variables from configuration file //
	tmpptr->ConfigurationValues->LoadVarsFromFile(engineconfigfile);

	// Load game configuration //
	tmpptr->_GameConfiguration = new GameConfiguration(gameconfig);

	if(!tmpptr->_GameConfiguration->Init(configchecker)){

		return NULL;
	}

	// Load key configuration //
	tmpptr->_KeyConfiguration = new KeyConfiguration(keyconfig);

	if(!tmpptr->_KeyConfiguration->Init(keychecker)){

		return NULL;
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

	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), "Width", width, 800,
        true, "Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), "Height", height, 600,
        true, "Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), "Windowed", window,
        true, true, "Create window: ");
    
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

	ObjectFileProcessor::LoadValueFromNamedVars<bool>(*ConfigurationValues, "Vsync", vsync, false, false);
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
    Title(title), Width(width), Height(height), Windowed(windowed)
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
