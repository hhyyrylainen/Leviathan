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

	//! \brief Mainly a FPS limiter
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

		FORCE_INLINE static void MakeSureHasEnoughRoom(std::vector<int> &tarvec, const size_t &accessspot){
			
			if(tarvec.size() <= accessspot+1){
				if(tarvec.size() > 5000){

					tarvec.resize((size_t)(tarvec.size()*1.35f));
					Logger::Get()->Warning(L"RenderingStatistics: large frame time tracking buffer is getting larger, "
                        "size: "+Convert::ToWstring(tarvec.size()));

				} else {

					tarvec.resize((size_t)(tarvec.size()*1.8f));
				}
			}


		}

		// ------------------------------------ //
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
		std::vector<int> LastMinuteFPS;
		size_t LastMinuteFPSPos;

		std::vector<int> LastMinuteRenderTimes;
		size_t LastMinuteRenderTimesPos;


		bool EraseOld;



	};

}
#endif
