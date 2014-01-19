#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATION
#include "Application.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"
#include "OGRE/OgreWindowEventUtilities.h"
#include <boost/date_time/posix_time/posix_time_duration.hpp>

DLLEXPORT Leviathan::LeviathanApplication::LeviathanApplication() : Quit(false), _Engine(new Engine(this)), ApplicationConfiguration(NULL), ShouldQuit(false)
{
	Curapp = this;
}

DLLEXPORT Leviathan::LeviathanApplication::~LeviathanApplication(){
	Curapp = NULL;
}

DLLEXPORT LeviathanApplication* Leviathan::LeviathanApplication::GetApp(){
	return Curapp;
}

LeviathanApplication* LeviathanApplication::Curapp = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::LeviathanApplication::Initialize(AppDef* configuration){
	ObjectLock guard(*this);
	// store configuration //
	ApplicationConfiguration = configuration;

	// init engine //
	if(!_Engine->Init(ApplicationConfiguration, NETWORKED_TYPE_CLIENT))
		return false;
	_InternalInit();
	return true;
}

DLLEXPORT void Leviathan::LeviathanApplication::Release(){
	ObjectLock guard(*this);
	// set as quitting //
	Quit = true;

	// let engine release itself and then delete it //
	SAFE_RELEASEDEL(_Engine);
	// configuration object needs to be destroyed by the program main function //
}

DLLEXPORT void Leviathan::LeviathanApplication::StartRelease(){
	ObjectLock guard(*this);
	ShouldQuit = true;
}

DLLEXPORT void Leviathan::LeviathanApplication::PassCommandLine(const wstring &params){
	_Engine->PassCommandLine(params);
}

DLLEXPORT void Leviathan::LeviathanApplication::FlushCommandLine(){
	_Engine->ExecuteCommandLine();
}

DLLEXPORT void Leviathan::LeviathanApplication::_InternalInit(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::Render(){
	_Engine->RenderFrame();
}

DLLEXPORT void Leviathan::LeviathanApplication::PreFirstTick(){

	_Engine->PreFirstTick();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::LeviathanApplication::RunMessageLoop(){
	// This is almost at tick so call this outside the loop for performance //
	PreFirstTick();

	while(_Engine->GetWindowOpenCount()){


		Ogre::WindowEventUtilities::messagePump();


		if(ShouldQuit || Quit){
			
			break;
		}

		// engine tick //
		_Engine->Tick();
		Render();
		// We could potentially wait here //
		try{
			boost::this_thread::sleep(boost::posix_time::microseconds(700));
		} catch(...){
			continue;
		}
	}
	ObjectLock guard(*this);
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

DLLEXPORT void Leviathan::LeviathanApplication::Tick(int mspassed){

}

DLLEXPORT void Leviathan::LeviathanApplication::EnginePreShutdown(){

}

DLLEXPORT void Leviathan::LeviathanApplication::DummyGameConfigurationVariables(GameConfiguration* configobj){

}

DLLEXPORT void Leviathan::LeviathanApplication::DummyGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}
// ------------------------------------ //
