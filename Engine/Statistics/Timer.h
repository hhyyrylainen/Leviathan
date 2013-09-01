#ifndef LEVIATHAN_TIMER
#define LEVIATHAN_TIMER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{
	class Timer{
	public:
		DLLEXPORT Timer();

		DLLEXPORT void Update();
		DLLEXPORT void Reset();

		DLLEXPORT int Framerate();
		DLLEXPORT int MaxFramerate();
		DLLEXPORT int FrameCount();
		DLLEXPORT float RunTime();
		DLLEXPORT float Elapsed();

		DLLEXPORT void SetFixedStep(float step);


	private:
		float Delta;
		float FramesSecond;
		int MaxframesSecond;
		int iFrameCount;

		float Fixeddeltatime;
		bool UseFixedDelta;

		//unsigned __int64 TicksPerSecond64;
		//unsigned __int64 StartTimeTicks64;
		//unsigned __int64 CurrentTicks64;
		//unsigned __int64 OneSecTicks64;
		//unsigned __int64 LastTick64;
		__int64 TicksPerSecond64;
		__int64 StartTimeTicks64;
		__int64 CurrentTicks64;
		__int64 OneSecTicks64;
		__int64 LastTick64;
	};
}









#endif