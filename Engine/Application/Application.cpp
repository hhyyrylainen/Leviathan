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


// ------------------ LeviathanApplication ------------------ //
DLLEXPORT Leviathan::LeviathanApplication::LeviathanApplication() :
    Quit(false), _Engine(new Engine(this)), ApplicationConfiguration(NULL), ShouldQuit(false), QuitSometime(false)
{
	Curapp = this;
}

DLLEXPORT Leviathan::LeviathanApplication::~LeviathanApplication(){
	Curapp = NULL;
}

DLLEXPORT LeviathanApplication* Leviathan::LeviathanApplication::GetApp(){
    
	return Curapp;
}

DLLEXPORT LeviathanApplication* Leviathan::LeviathanApplication::Get(){
    
	return Curapp;
}

LeviathanApplication* LeviathanApplication::Curapp = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::LeviathanApplication::Initialize(AppDef* configuration){
	GUARD_LOCK_THIS_OBJECT();
	// store configuration //
	ApplicationConfiguration = configuration;

	// init engine //
	if(!_Engine->Init(ApplicationConfiguration, NETWORKED_TYPE_CLIENT))
		return false;
	_InternalInit();
	return true;
}

DLLEXPORT void Leviathan::LeviathanApplication::Release(){
	{
		GUARD_LOCK_THIS_OBJECT();
		// set as quitting //
		Quit = true;

		// Nothing else to do if no engine //
		if(!_Engine)
			return;
	}

	// This avoids deadlocking //
	_Engine->Release();

	{
		GUARD_LOCK_THIS_OBJECT();
		// Delete the already released engine //
		delete _Engine;
		_Engine = NULL;
	}
}

DLLEXPORT void Leviathan::LeviathanApplication::StartRelease(){
	GUARD_LOCK_THIS_OBJECT();
	ShouldQuit = true;

	// Tell Engine to expect a Release soon //
	_Engine->PreRelease();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::ForceRelease(){
	GUARD_LOCK_THIS_OBJECT();
	ShouldQuit = true;
	Quit = true;

	if(_Engine){
		// The prelease does some stuff which is necessary and that requires tick to be called... //
		_Engine->PreRelease();
		_Engine->Tick();
		_Engine->Release(true);
	}

	SAFE_DELETE(_Engine);
}
// ------------------------------------ //
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

}
// ------------------------------------ //
DLLEXPORT int Leviathan::LeviathanApplication::RunMessageLoop(){
	// This is almost at tick so call this outside the loop for performance //
    _Engine->PreFirstTick();
	PreFirstTick();

	// For reporting wait failures //
	int FailCount = 0;

	while(!_Engine->HasPreReleaseBeenDone()){
		// Store this //
		bool canprocess = _Engine->GetWindowOpenCount() != 0;

		Ogre::WindowEventUtilities::messagePump();

		// Set as quitting //
		if((!canprocess || QuitSometime) && !ShouldQuit){
			Logger::Get()->Info(L"Application: starting real close");
			StartRelease();
		}

		// engine tick //
		_Engine->Tick();

		if(ShouldQuit || Quit){
			// We need to have done a proper run after calling StartRelease //
			continue;
		}


		Render();


		// We could potentially wait here //
		try{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		} catch(...){
			FailCount++;
		}
	}
	GUARD_LOCK_THIS_OBJECT();
	// Report problems //
	if(FailCount)
		DEBUG_OUTPUT_AUTO(string("Application main loop sleep fails: "+Convert::ToString(FailCount)));

	// always release before quitting to avoid tons of memory leaks //
	Release();

	return 0; 
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::ClearTimers(){

    _Engine->ClearTimers();
}
// ------------------ Default callbacks that do nothing ------------------ //
DLLEXPORT bool Leviathan::LeviathanApplication::InitLoadCustomScriptTypes(asIScriptEngine* engine){

    return true;
}

DLLEXPORT void Leviathan::LeviathanApplication::RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids){

}

DLLEXPORT void Leviathan::LeviathanApplication::RegisterApplicationPhysicalMaterials(PhysicsMaterialManager* manager){

}

DLLEXPORT void Leviathan::LeviathanApplication::Tick(int mspassed){

}

DLLEXPORT void Leviathan::LeviathanApplication::EnginePreShutdown(){

}

DLLEXPORT shared_ptr<GameWorld> Leviathan::LeviathanApplication::GetGameWorld(int id){

    return nullptr;
}


DLLEXPORT void Leviathan::LeviathanApplication::DummyGameConfigurationVariables(GameConfiguration* configobj){

}

DLLEXPORT void Leviathan::LeviathanApplication::DummyGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}

DLLEXPORT void Leviathan::LeviathanApplication::MarkAsClosing(){
	QuitSometime = true;
}
// ------------------------------------ //
