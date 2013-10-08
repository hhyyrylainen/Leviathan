#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LEAPMANAGER
#include "LeapManager.h"
#endif
#include "Leap.h"
#include "Engine.h"
using namespace Leviathan;
using namespace Leap;
// ------------------------------------ //
DLLEXPORT Leviathan::LeapManager::LeapManager(Engine* engineinstance) : EngineAccess(engineinstance), MainController(NULL), MainListener(NULL){
	// default values to gesture variables //
	TimeSinceReset = 0;

	SweepDownShutdown = 0;
}

DLLEXPORT Leviathan::LeapManager::~LeapManager(){
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::LeapManager::Init(){
	// initialize leap interface //
	MainController = new Controller();

	// create listener //
	MainListener = new LeapListener(this);

	// start listening //
	if(!MainController->addListener(*MainListener)){
		// no leap controller! //
		Logger::Get()->Warning(L"LeapManager: Failed to start listening for Leap motion controller");
	}

	// now listening //
	return true;
}

DLLEXPORT void Leviathan::LeapManager::Release(){
	// we need to unregister listener //
	if(MainListener){
		// unregister from controller //
		MainController->removeListener(*MainListener);
		Logger::Get()->Info(L"LeapManager: unconnected from Leap");
	}
	// unallocate //
	SAFE_DELETE(MainListener);
	SAFE_DELETE(MainController);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeapManager::OnTick(const int &mspassed){
	// add to time //
	TimeSinceReset += mspassed;

	if(TimeSinceReset >= GESTURESTATERESETTIME){
		// reset all gesture variables //
		SweepDownShutdown = 0;

		// reset time //
		TimeSinceReset = 0;
	}
	// could get state from controller here //


	// check for action //
	if(SweepDownShutdown >= SHUTDOWNSWEEPTHRESSHOLD){

		Logger::Get()->Info(L"LeapManager: Input: downward swipe threshold passed, shutting down");

		// close the window so the program quits after this //
		EngineAccess->GetWindowEntity()->GetWindow()->CloseDown();
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeapManager::DownWardSwipeThresshold(const int &change){
	SweepDownShutdown += change;
}
// ------------------------------------ //






