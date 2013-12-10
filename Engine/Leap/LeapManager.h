#ifndef LEVIATHAN_LEAPMANAGER
#define LEVIATHAN_LEAPMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
#ifdef _DEBUG
//#define LEAP_DEBUGOUTPUT_DATA		100
#endif // _DEBUG
// ---- includes ---- //
#include "LeapListener.h"
#include "Leap.h"

#define SHUTDOWNSWEEPTHRESSHOLD			20000
#define GESTURESTATERESETTIME			1000

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
		// access to various components //
		Engine* EngineAccess;
	};

}
#endif
