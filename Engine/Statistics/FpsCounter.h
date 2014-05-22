#ifndef LEVIATHAN_FPSCOUNTER
#define LEVIATHAN_FPSCOUNTER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class FpsCounter : public EngineComponent{
	public:
		FpsCounter();
		

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
