#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LEAPMANAGER
#include "LeapManager.h"
#endif
#include "Leap.h"
#include "Engine.h"
#include "Application/Application.h"
using namespace Leviathan;
using namespace Leap;
// ------------------------------------ //
DLLEXPORT Leviathan::LeapManager::LeapManager(Engine* engineinstance) :
    EngineAccess(engineinstance), MainController(NULL), MainListener(NULL), LastFrameID(0)
{
	// Default values to gesture variables //
	TimeSinceReset = 0;

	SweepDownShutdown = 0;
}

DLLEXPORT Leviathan::LeapManager::~LeapManager(){
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::LeapManager::Init(){
	// Initialize leap interface //
	MainController = new Controller();

	// Create listener //
	MainListener = new LeapListener(this);

#ifdef LEAP_USE_ASYNC
	// Start listening //
	if(!MainController->addListener(*MainListener)){
        
		// No leap controller! //
		Logger::Get()->Warning(L"LeapManager: Failed to start listening for Leap motion controller");
	}
#endif //LEAP_USE_ASYNC

	return true;
}

DLLEXPORT void Leviathan::LeapManager::Release(){
	// We need to unregister listener //
	if(MainListener){
#ifdef LEAP_USE_ASYNC
		MainController->removeListener(*MainListener);
		Logger::Get()->Info(L"LeapManager: unconnected from Leap");
#endif //LEAP_USE_ASYNC
	}
    
	// Unallocate //
	SAFE_DELETE(MainListener);
	SAFE_DELETE(MainController);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeapManager::OnTick(const int &mspassed){
	// Add to time //
	TimeSinceReset += mspassed;

#ifndef LEAP_USE_ASYNC
    // Poll frame //
    const auto frame = MainController.frame();

    auto id = frame.id();

    if(id != LastFrameID){

        LastFrameID = id;
        MainListener->HandleFrame(frame);
    }
    
#endif //LEAP_USE_ASYNC
    

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
		GUARD_LOCK_OTHER_OBJECT(Leviathan::LeviathanApplication::GetApp());
		Leviathan::LeviathanApplication::GetApp()->MarkAsClosing();
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeapManager::DownWardSwipeThresshold(const int &change){
	SweepDownShutdown += change;
}
// ------------------------------------ //






