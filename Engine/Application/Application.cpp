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

DLLEXPORT Leviathan::LeviathanApplication::LeviathanApplication() : Quit(false), _Engine(new Engine(this)), ApplicationConfiguration(NULL), 
	ShouldQuit(false), QuitSometime(false)
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
	{
		ObjectLock guard(*this);
		// set as quitting //
		Quit = true;

		// Nothing else to do if no engine //
		if(!_Engine)
			return;
	}

	// This avoids deadlocking //
	_Engine->Release();

	{
		ObjectLock guard(*this);
		// Delete the already released engine //
		delete _Engine;
		_Engine = NULL;
	}
}

DLLEXPORT void Leviathan::LeviathanApplication::StartRelease(){
	ObjectLock guard(*this);
	ShouldQuit = true;

	// Tell Engine to expect a Release soon //
	_Engine->PreRelease();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeviathanApplication::ForceRelease(){
	ObjectLock guard(*this);
	ShouldQuit = true;
	Quit = true;

	if(_Engine)
		_Engine->Release(true);

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

	_Engine->PreFirstTick();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::LeviathanApplication::RunMessageLoop(){
	// This is almost at tick so call this outside the loop for performance //
	PreFirstTick();

	// For reporting wait failures //
	int FailCount = 0;

	while(!_Engine->HasPreRleaseBeenDone()){
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
	ObjectLock guard(*this);
	// Report problems //
	if(FailCount)
		DEBUG_OUTPUT_AUTO(wstring(L"Application main loop sleep fails: "+Convert::ToWstring(FailCount)));

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

DLLEXPORT void Leviathan::LeviathanApplication::MarkAsClosing(){
	QuitSometime = true;
}
// ------------------------------------ //
