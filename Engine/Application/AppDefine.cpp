#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATIONDEFINE
#include "AppDefine.h"
#endif
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Application/Application.h"
#include "GameConfiguration.h"
#include "ForwardDeclarations.h"
#include "KeyConfiguration.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AppDef::AppDef(const bool &isdef /*= false*/) : ConfigurationValues(new NamedVars()), 
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

DLLEXPORT AppDef* Leviathan::AppDef::GenerateAppdefine(const wstring &logfile, const wstring &engineconfigfile,
    const wstring &gameconfig, const wstring &keyconfig, boost::function<void (GameConfiguration* configobj)>
    configchecker, boost::function<void (KeyConfiguration* keysobject)> keychecker)
{

	unique_ptr<AppDef> tmpptr(new AppDef(true));

	tmpptr->LogFile = logfile;

	// Create logger first if it doesn't exist //
	if(Logger::GetIfExists() != NULL){
		// already exists //
		tmpptr->Mainlog = Logger::Get();
		tmpptr->DeleteLog = false;

	} else {
		tmpptr->Mainlog = new Logger(logfile+L"Log.txt");
		// We created a new one //
		tmpptr->DeleteLog = true;
	}

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
DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(const wstring &title, const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr){

#else
DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(const wstring &title, const bool &windowborder, LeviathanApplication* appvirtualptr){
#endif

	// store the parameters to be used for window creation //
	int width;
	int height;
	bool window;

	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), L"Width", width, 800, true, L"Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), L"Height", height, 600, true, L"Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), L"Windowed", window, true, true, L"Create window: ");
#ifdef _WIN32
	this->SetWindowDetails(WindowDataDetails(title, width, height, window, windowborder, icon, appvirtualptr));
#else
	this->SetWindowDetails(WindowDataDetails(title, width, height, window, windowborder, appvirtualptr));
#endif
}

DLLEXPORT AppDef& Leviathan::AppDef::SetApplicationIdentification(const wstring &userreadable, const wstring &gamename, const wstring &gameversion){
	UserReadableGame = userreadable;
	Game = gamename;
	GameVersion = gameversion;
	LeviathanVersion = LEVIATHAN_VERSIONS;

	return *this;
}

DLLEXPORT void Leviathan::AppDef::GetGameIdentificationData(wstring &userreadable, wstring &gamename, wstring &gameversion){
	userreadable = UserReadableGame;
	gamename = Game;
	gameversion = GameVersion;
}

DLLEXPORT bool Leviathan::AppDef::GetVSync(){
	bool vsync;

	ObjectFileProcessor::LoadValueFromNamedVars<bool>(*ConfigurationValues, L"Vsync", vsync, false, false);
	return vsync;
}

// ------------------ WindowDataDetails ------------------ //
#ifdef _WIN32
Leviathan::WindowDataDetails::WindowDataDetails(const wstring &title, const int &width, const int &height, const bool &windowed,
	const bool &windowborder, HICON icon, LeviathanApplication* appvirtualptr) : Title(title), Width(width), Height(height),
	Windowed(windowed), Icon(icon)
{

}
#else
Leviathan::WindowDataDetails::WindowDataDetails(const wstring &title, const int &width, const int &height, const bool &windowed,
	const bool &windowborder, LeviathanApplication* appvirtualptr) : Title(title), Width(width), Height(height),
	Windowed(windowed)
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
