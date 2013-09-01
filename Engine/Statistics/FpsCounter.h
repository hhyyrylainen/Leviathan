#ifndef LEVIATHAN_FPSCOUNTER
#define LEVIATHAN_FPSCOUNTER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

namespace Leviathan{

	class FpsCounter : public EngineComponent{
	public:
		FpsCounter();

		bool Init();
		void FakeFrame(int mspassed);
		void Frame(int timepassed);
		int GetFps();

		bool ShouldRender(int passed, int maxfps);

		
	private:
		int passedtime;
		int MaxFps, MinFps;
		int Fps, Framecount;
		int SinceLast;

		//unsigned long Starttime;
	};

}
#endif