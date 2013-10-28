#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATION
#include "Application.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"
#include "OGRE/OgreWindowEventUtilities.h"

DLLEXPORT Leviathan::LeviathanApplication::LeviathanApplication() : Quit(false), _Engine(NULL), ApplicationConfiguration(NULL), ShouldQuit(false){
	Curapp = this;
}

DLLEXPORT Leviathan::LeviathanApplication::~LeviathanApplication(){
	Curapp = NULL;
}

LeviathanApplication* LeviathanApplication::Curapp = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::LeviathanApplication::Initialize(AppDef* configuration){
	// store configuration //
	ApplicationConfiguration = configuration;


	// init engine //
	_Engine = new Engine(this);
	if(!_Engine->Init(ApplicationConfiguration))
		return false;
	_InternalInit();
	return true;
}

DLLEXPORT void Leviathan::LeviathanApplication::Release(){
	// set as quitting //
	Quit = true;

	// let engine release itself and then delete it //
	SAFE_RELEASEDEL(_Engine);
	// configuration object needs to be destroyed by the program main function //
}

DLLEXPORT void Leviathan::LeviathanApplication::StartRelease(){
	ShouldQuit = true;
}

DLLEXPORT void Leviathan::LeviathanApplication::PassCommandLine(const wstring &params){
	_Engine->ExecuteCommandLine(params);
}

DLLEXPORT void Leviathan::LeviathanApplication::_InternalInit(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::Render(){
	_Engine->RenderFrame();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::LeviathanApplication::RunMessageLoop(){

	while(_Engine->GetWindowOpenCount()){


		Ogre::WindowEventUtilities::messagePump();


		if(ShouldQuit || Quit){
			
			break;
		}

		// engine tick //
		_Engine->Tick();
		Render();
	}
	// always release before quitting to avoid tons of memory leaks //
	Release();

	return 0; 
}

// ------------------ Default callbacks that do nothing ------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::InitLoadCustomScriptTypes(asIScriptEngine* engine){

}

DLLEXPORT void Leviathan::LeviathanApplication::RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){

}

DLLEXPORT void Leviathan::LeviathanApplication::RegisterApplicationPhysicalMaterials(PhysicsMaterialManager* manager){

}

// ------------------------------------ //
