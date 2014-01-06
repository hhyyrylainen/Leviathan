#include "Define.h"
#include "TestFunction.h"

class DummyNetworkHandler : public Leviathan::NetworkInterface{
public:
	DLLEXPORT virtual void HandleResponseOnlyPacket(shared_ptr<NetworkResponse> message, ConnectionInfo* connection){
		throw std::exception("The method or operation is not implemented.");
	}
};

// ------------------ ProgramConfiguration ------------------ //
	
#define ENGINECONFIGURATION				L"./EngineConf.conf"
#define PROGRAMCONFIGURATION			L"./Tests.conf"
#define PROGRAMKEYCONFIGURATION			L""
#define PROGRAMCHECKCONFIGFUNCNAME		LeviathanApplication::DummyGameConfigurationVariables
#define PROGRAMCHECKKEYCONFIGFUNCNAME	LeviathanApplication::DummyGameKeyConfigVariables
#define PROGRAMMASTERSERVERINFO			MasterServerInformation()
#define WINDOWTITLEGENFUNCTION			L"LeviathanTest for version " VERSIONS L" built on " __WDATE__ L" " __WTIME__

#define USERREADABLEIDENTIFICATION		L"Leviathan test" VERSIONS
#define GAMENAMEIDENTIFICATION			L"Test"
#define GAMEVERSIONIDENTIFICATION		VERSIONS


#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD




#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
		//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

#else
int main(int argcount, char* args[]){
#endif

	int Return = 0;
#ifdef _WIN32
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL))){
#else

#endif

#ifdef LEVIATHAN_USES_VLD
		// now that we are in code we can start tracking //
		VLDEnable();
#endif // LEVIATHAN_USES_VLD

#ifdef _WIN32
		// set path //
		SYSTEMTIME tdate;
		GetLocalTime(&tdate);



		wstring times = Convert::IntToWstring(tdate.wYear)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wDay)+L" "+Convert::IntToWstring(tdate.wHour)+L"."+Convert::IntToWstring(tdate.wMinute);

#else
		wstring times = L"No time on linux";
#endif

		LeviathanApplication app;
		DummyNetworkHandler network;

		unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine(times, ENGINECONFIGURATION, PROGRAMCONFIGURATION, PROGRAMKEYCONFIGURATION, 
			&PROGRAMCHECKCONFIGFUNCNAME, &PROGRAMCHECKKEYCONFIGFUNCNAME));
		// customize values //
#ifdef _WIN32
		ProgramDefinition->SetHInstance(hInstance);
#endif
		ProgramDefinition->SetMasterServerParameters(PROGRAMMASTERSERVERINFO).SetPacketHandler(&network).SetApplicationIdentification(
			USERREADABLEIDENTIFICATION, GAMENAMEIDENTIFICATION, GAMEVERSIONIDENTIFICATION);

		bool Passed = true;

		// run pre-engine tests //
		Logger::Get()->Info(WINDOWTITLEGENFUNCTION);
		Logger::Get()->Info(L"-------------------- STARTING PRE-ENGINE TESTS --------------------\n\n\n", true);

		TimingMonitor::StartTiming(L"All tests timer");

		if(TestPreEngine()){
			Logger::Get()->Write(L"\n\n", false);
			Logger::Get()->Error(L"!----! Some Tests [Failed] !----!", true);
			Passed = false;
		} else {
			Logger::Get()->Write(L"\n\n", false);
			Logger::Get()->Info(L"-------------------- ALL TESTS PASSED --------------------\n", true);
		}

		TimingMonitor::StopTiming(L"All tests timer");

		// customize values //
		ProgramDefinition->StoreWindowDetails(WINDOWTITLEGENFUNCTION, true,
#ifdef _WIN32
			LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
#endif
			&app);

#ifdef _WIN32
		app.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));
#else
		wstring commandline = L"";
		for(int i = 1; i < argcount; i++){
			commandline += L" "+Leviathan::Convert::StringToWstring(args[i]);
		}
		app.PassCommandLine(commandline);
#endif


		if(app.Initialize(ProgramDefinition.get())){

			// After everything is ready the command line should be flushed //
			app.FlushCommandLine();

			Logger::Get()->Info(L"Engine successfully initialized", false);

			// run tests //
			TimingMonitor::StartTiming(L"All tests timer");

			if(TestEngine(app.GetEngine())){
				Logger::Get()->Write(L"\n\n", false);
				Logger::Get()->Error(L"!----! Some Tests [Failed] !----!", true);
				Passed = false;
			} else {
				Logger::Get()->Write(L"\n\n", false);
				Logger::Get()->Info(L"-------------------- ALL ENGINE TESTS PASSED --------------------\n", true);
			}

			TimingMonitor::StopTiming(L"All tests timer");

			if(Passed){
				// CMake testing will detect this
				Logger::Get()->Info(L"***************** Testing completed succesfully *****************", true);
				cout << "testing completed successfully" << endl;
			}

			// finish //
			app.StartRelease();
			// receive close message //
			Return = app.RunMessageLoop();

		} else {
			Logger::Get()->Error(L"App init failed, closing",0, true);
			//app.Close();

			cout << "Testing failed" << endl;

			Return = 005;
		}

		//un-init
		app.Release();
#ifdef _WIN32
	}
	//_CrtDumpMemoryLeaks();
	CoUninitialize();
#endif
	return Return;
}
