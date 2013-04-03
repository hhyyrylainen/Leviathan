#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CPUMONITOR
#include "CpuMonitor.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
CpuMonitor::CpuMonitor(){

}
// ------------------------------------ //
bool CpuMonitor::Init(){

	PDH_STATUS status;

	// Init the flag indicating whether this object can read the system cpu usage or not.
	CanReadCPU = true;

	// Create a query object to poll cpu usage.
	status = PdhOpenQuery(NULL, 0, &QueryHandle);
	if(status != ERROR_SUCCESS){
	
		Logger::Get()->Error(L"CpuMonitor failed to init, couldn't open PdhOpenQuery", GetLastError());
		CanReadCPU = false;
	}

	// Set query object to poll all cpus in the system.
	status = PdhAddCounterW(QueryHandle, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &FramecounterHandle);
	//PDH_CSTATUS_BAD_COUNTERNAME

	if(status != ERROR_SUCCESS){
	
		Logger::Get()->Error(L"CpuMonitor failed to init, failed to add counter", GetLastError());	
		CanReadCPU = false;
	}

	LastCheckTime = GetTickCount(); 

	CpuUsage = 0;

	return true;
}
void CpuMonitor::Release(){

	if(CanReadCPU){
		PdhCloseQuery(QueryHandle);
	}
}
// ------------------------------------ //
void CpuMonitor::Frame(){

	PDH_FMT_COUNTERVALUE value; 

	if(CanReadCPU){
		if((LastCheckTime + 1000) < GetTickCount()){
		
			LastCheckTime = GetTickCount(); 

			PdhCollectQueryData(QueryHandle);
        
			PdhGetFormattedCounterValue(FramecounterHandle, PDH_FMT_LONG, NULL, &value);

			CpuUsage = value.longValue;
		}
	}
}
// ------------------------------------ //
int CpuMonitor::GetCpuPercentage(){

	if(CanReadCPU){
		return (int)CpuUsage;
	} else {
		return -1;
	}
}
// ------------------------------------ //