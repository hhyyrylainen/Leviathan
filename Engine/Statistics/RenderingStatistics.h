#ifndef LEVIATHAN_RENDERINGSTATISTICS
#define LEVIATHAN_RENDERINGSTATISTICS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/DataStore.h"

namespace Leviathan{

	class RenderingStatistics : public Object{
	public:
		DLLEXPORT RenderingStatistics();
		DLLEXPORT ~RenderingStatistics();

		DLLEXPORT void RenderingStart();
		DLLEXPORT void RenderingEnd();

		DLLEXPORT bool CanRenderNow(int maxfps, int& TimeSinceLastFrame);

		DLLEXPORT void ReportStats(DataStore* dstore);

	private:
		void HalfMinuteMark();
		void SecondMark();

		// ---------------------- //
		__int64 HalfMinuteStartTime;
		__int64 SecondStartTime;
		__int64 RenderingStartTime;
		__int64 RenderingEndTime;
		// measured values //
		int Frames;

		int FPS;
		int RenderMCRSeconds;

		int MinFPS;
		int MaxFPS;
		int AverageFps;

		int MaxFrameTime;
		int MinFrameTime;
		int AverageRenderTime;

		int DoubtfulCancel;

		bool IsFirstFrame;

		// stored values //
		vector<int> LastMinuteFPS;
		vector<int> LastMinuteRenderTimes;

		bool EraseOld;



	};

}
#endif
