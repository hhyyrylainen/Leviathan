#pragma once
#ifndef LEVIATHAN_LEAPMANAGER
#define LEVIATHAN_LEAPMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "LeapListener.h"
#include "Leap.h"

#define SHUTDOWNSWEEPTHRESSHOLD			20000
#define GESTURESTATERESETTIME			1000

// Define when using async handling
#define LEAP_USE_ASYNC


namespace Leviathan{

	class Engine;

	class LeapManager : public EngineComponent{
		friend LeapListener;
	public:
		DLLEXPORT LeapManager(Engine* engineinstance);
		DLLEXPORT ~LeapManager();
		// actual methods //
		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();

		DLLEXPORT void OnTick(const int &mspassed);

		DLLEXPORT void DownWardSwipeThresshold(const int &change);

	private:
		// leap listener //
		LeapListener* MainListener;
		Leap::Controller* MainController;
        
	protected:
		// current gesture stored states //
		int SweepDownShutdown;


		int TimeSinceReset;

#ifndef LEAP_USE_ASYNC
        //! Avoids processing a single frame multiple times
        //! \todo Use this to avoid missing frames, too
        int64_t LastFrameID;
        
#endif //LEAP_USE_ASYNC

		// access to various components //
		Engine* EngineAccess;
	};

}
#endif
