#pragma once
// ------------------------------------ //

namespace Leviathan{

	class FpsCounter{
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
	};

}

