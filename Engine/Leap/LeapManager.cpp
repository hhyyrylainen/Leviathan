// ------------------------------------ //
#include "LeapManager.h"

#include "../Application/Application.h"
#include "Engine.h"
#include "Leap.h"
using namespace Leviathan;
using namespace Leap;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::LeapManager::LeapManager(Engine* engineinstance) :
    EngineAccess(engineinstance), MainController(NULL), MainListener(NULL)
{
#ifndef LEAP_USE_ASYNC
    LastFrameID = 0;
#endif // LEAP_USE_ASYNC

    // Default values to gesture variables //
    TimeSinceReset = 0;
}

DLLEXPORT Leviathan::LeapManager::~LeapManager() {}
// ------------------------------------ //
DLLEXPORT bool Leviathan::LeapManager::Init()
{
    // Initialize leap interface //
    MainController = new Controller();

    // Create listener //
    MainListener = new LeapListener(this);

#ifdef LEAP_USE_ASYNC
    // Start listening //
    if(!MainController->addListener(*MainListener)) {

        // No leap controller! //
        Logger::Get()->Warning(
            "LeapManager: Failed to start listening for Leap motion controller");
    }
#endif // LEAP_USE_ASYNC

    return true;
}

DLLEXPORT void Leviathan::LeapManager::Release()
{
    // We need to unregister listener //
    if(MainListener) {
#ifdef LEAP_USE_ASYNC
        MainController->removeListener(*MainListener);
        Logger::Get()->Info("LeapManager: disconnected from Leap");
#endif // LEAP_USE_ASYNC
    }

    // Unallocate //
    SAFE_DELETE(MainListener);
    SAFE_DELETE(MainController);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::LeapManager::OnTick(const int& mspassed)
{
    // Add to time //
    TimeSinceReset += mspassed;

#ifndef LEAP_USE_ASYNC
    // Poll frame //
    const auto frame = MainController->frame();

    auto id = frame.id();

    if(id != LastFrameID) {

        LastFrameID = id;
        MainListener->HandleFrame(frame, *MainController);
    }

#endif // LEAP_USE_ASYNC

    // Check for action //



    if(TimeSinceReset >= GESTURESTATERESETTIME) {
        // Reset time //
        TimeSinceReset = 0;
    }
}
// ------------------------------------ //
