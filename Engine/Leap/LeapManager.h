#pragma once
// ------------------------------------ //
#include "../ForwardDeclarations.h"
#include "Include.h"

#include "LeapListener.h"
#include "Leap.h"

#define GESTURESTATERESETTIME			1000

// Define when using async handling
#define LEAP_USE_ASYNC


namespace Leviathan{

	class Engine;

	class LeapManager{
		friend LeapListener;
	public:
		DLLEXPORT LeapManager(Engine* engineinstance);
		DLLEXPORT virtual ~LeapManager();
		// actual methods //
		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();

		DLLEXPORT void OnTick(const int &mspassed);

	private:
		// leap listener //
		LeapListener* MainListener;
		Leap::Controller* MainController;
        
	protected:


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

