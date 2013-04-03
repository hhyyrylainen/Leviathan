#ifndef LEVIATHAN_CPUMONITOR
#define LEVIATHAN_CPUMONITOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#pragma comment(lib, "pdh.lib")
#include <pdh.h>

namespace Leviathan{

	class CpuMonitor : public EngineComponent{
	public:
		CpuMonitor();

		bool Init();
		void Release();
		void Frame();
		int GetCpuPercentage();

	private:
		bool CanReadCPU;
		PDH_HQUERY QueryHandle;
		PDH_HCOUNTER FramecounterHandle;
		unsigned long LastCheckTime;
		long CpuUsage;
	};

}
#endif