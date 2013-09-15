#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATION
#include "Application.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"
#include "OGRE/OgreWindowEventUtilities.h"

DLLEXPORT Leviathan::LeviathanApplication::LeviathanApplication() : Quit(false), engine(NULL), ApplicationConfiguration(NULL), ShouldQuit(false){
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
	engine = new Engine();
	if(!engine->Init(ApplicationConfiguration))
		return false;
	_InternalInit();
	return true;
}

DLLEXPORT void Leviathan::LeviathanApplication::Release(){
	// set as quitting //
	Quit = true;

	// let engine release itself and then delete it //
	SAFE_RELEASEDEL(engine);
	// configuration object needs to be destroyed by the program main function //
}

DLLEXPORT void Leviathan::LeviathanApplication::StartRelease(){
	ShouldQuit = true;
}

DLLEXPORT void Leviathan::LeviathanApplication::PassCommandLine(const wstring &params){
	engine->ExecuteCommandLine(params);
}

DLLEXPORT void Leviathan::LeviathanApplication::_InternalInit(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::Render(){
	engine->RenderFrame();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::DoResizeWindow(const int &newwidth, const int &newheight){
	// tell engine to resize window //
	engine->DoWindowResize(newwidth, newheight);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::LeviathanApplication::RunMessageLoop(){

	//MSG Msg;

	//for(int GameLoopCount = 0; ; GameLoopCount++){
	while(ApplicationConfiguration->GetWindow()->IsOpen()){


		Ogre::WindowEventUtilities::messagePump();


		if(ShouldQuit || Quit){
			
			break;
		}

		// engine tick //
		engine->Tick();
		Render();
	}
	// always release before quitting to avoid tons of memory leaks //
	Release();

	return 0; 
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::OnLoseFocus(){

}

DLLEXPORT void Leviathan::LeviathanApplication::OnGainFocus(){

}

DLLEXPORT void Leviathan::LeviathanApplication::OnResize(const int &width, const int &height){

}

// ------------------------------------ //
