#include "Define.h"
#include "TestFunction.h"

#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
		//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

	int Return = 0;
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL))){

#ifdef LEVIATHAN_USES_VLD
		// now that we are in code we can start tracking //
		VLDEnable();
#endif // LEVIATHAN_USES_VLD

		wstring tittle = L"LeviathanTest for version ";
		tittle += VERSIONS;
		tittle += L" built on ";
		tittle += __WDATE__ L" " __WTIME__;

		LeviathanApplication app;

		// create custom logger //
		Logger* customlogger = new Logger();
		// set path //
		SYSTEMTIME tdate;
		GetLocalTime(&tdate);

		bool Passed = true;

		wstring times = Convert::IntToWstring(tdate.wYear)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wDay)+L" "+Convert::IntToWstring(tdate.wHour)+L"."+Convert::IntToWstring(tdate.wMinute);

		customlogger->SetSavePath(L".\\TestLog "+times+L".txt");

		// run pre-engine tests //
		Logger::Get()->Info(tittle, false);
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

		unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine());
		// customize values //
		ProgramDefinition->SetHInstance(hInstance);

		// create window last //
		ProgramDefinition->StoreWindowDetails(tittle, true, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), &app);



		if(app.Initialize(ProgramDefinition.get())){
			app.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));

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
				cout << "testing completed succesfully" << endl;
			}

			// finish //
			app.GetEngine()->GetDefinition()->GetWindow()->CloseDown();
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
	}
	//_CrtDumpMemoryLeaks();
	CoUninitialize();
	return Return;
}