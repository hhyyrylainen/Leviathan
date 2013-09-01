#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATIONDEFINE
#include "AppDefine.h"
#endif
#include "ObjectFiles\ObjectFileProcessor.h"
#include "Application\Application.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::AppDef::AppDef(const bool &isdef /*= false*/) : ConfigurationValues(new NamedVars()), RWindow(NULL), HInstance(NULL){
	if(isdef){
		Defaultconf = this;
	}
}

AppDef::~AppDef(){
	// reset static access if this is it //
	if(Defaultconf == this)
		Defaultconf = NULL;
}

AppDef* Leviathan::AppDef::Defaultconf = NULL;
// ------------------------------------ //
NamedVars* Leviathan::AppDef::GetValues(){
	return ConfigurationValues.get();
}

DLLEXPORT AppDef* Leviathan::AppDef::GenerateAppdefine(){

	unique_ptr<AppDef> tmpptr(new AppDef(true));

	// load variables from configuration file //
	tmpptr->ConfigurationValues->LoadVarsFromFile(L".\\EngineConf.conf");

	return tmpptr.release();
}

DLLEXPORT void Leviathan::AppDef::StoreWindowDetails(const wstring &title, const bool &windowborder, HICON icon, WNDPROC wndproc, LeviathanApplication* appvirtualptr){
	// store the parameters to be used for window creation //

	int width;
	int height;
	bool window;

	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), L"Width", width, 800, true, L"Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), L"Height", height, 600, true, L"Create window: ");
	ObjectFileProcessor::LoadValueFromNamedVars(ConfigurationValues.get(), L"Windowed", window, true, true, L"Create window: ");

	this->SetWindowDetails(WindowDataDetails(title, width, height, window, windowborder, icon, wndproc, appvirtualptr));
}

// ------------------ WindowDataDetails ------------------ //
Leviathan::WindowDataDetails::WindowDataDetails(const wstring &title, const int &width, const int &height, const bool &windowed, 
	const bool &windowborder, HICON icon, WNDPROC wndproc, LeviathanApplication* appvirtualptr) : Title(title), Width(width), Height(height), Windowed(windowed)
{

}

Leviathan::WindowDataDetails::WindowDataDetails(){

}
