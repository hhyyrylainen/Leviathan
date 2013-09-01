#include "Define.h"
#include "TestFunction.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
		//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
		//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif


		int Return = 0;
		HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

		if (SUCCEEDED(CoInitialize(NULL))){

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

			wstring times = Convert::IntToWstring(tdate.wYear)+L"."+Convert::IntToWstring(tdate.wMonth)+L"."+Convert::IntToWstring(tdate.wDay)+L" "+Convert::IntToWstring(tdate.wHour)+L"."+Convert::IntToWstring(tdate.wMinute);

			customlogger->SetSavePath(L".\\TestLog "+times+L".txt");

			// run pre-engine tests //
			Logger::Get()->Info(tittle, false);
			Logger::Get()->Info(L"-------------------- STARTING PRE-ENGINE TESTS --------------------\n\n\n", true);

			TimingMonitor::StartTiming(L"All tests timer");

			if(TestPreEngine()){
				Logger::Get()->Write(L"\n\n", false);
				Logger::Get()->Error(L"!----! Some Tests [Failed] !----!", true);
			} else {
				Logger::Get()->Write(L"\n\n", false);
				Logger::Get()->Info(L"-------------------- ALL TESTS PASSED --------------------\n", true);
			}

			TimingMonitor::StopTiming(L"All tests timer");

			unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine());
			// customize values //
			ProgramDefinition->SetHInstance(hInstance);

			// create window last //
			ProgramDefinition->StoreWindowDetails(tittle, true, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), WndProc, &app);



			if(app.Initialize(ProgramDefinition.get())){
				app.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));

				Logger::Get()->Info(L"Engine successfully initialized", false);

				// run tests //
				TimingMonitor::StartTiming(L"All tests timer");
				
				if(TestEngine(app.GetEngine())){
					Logger::Get()->Write(L"\n\n", false);
					Logger::Get()->Error(L"!----! Some Tests [Failed] !----!", true);
				} else {
					Logger::Get()->Write(L"\n\n", false);
					Logger::Get()->Info(L"-------------------- ALL ENGINE TESTS PASSED --------------------\n", true);
				}

				TimingMonitor::StopTiming(L"All tests timer");
				// finish //
				app.GetEngine()->GetDefinition()->GetWindow()->destroy();
				// receive close message //
				Return = app.RunMessageLoop();
			} else {
				Logger::Get()->Error(L"App init failed, closing",0, true);
				//app.Close();

				Return = 005;
			}

			Logger::Get()->Save();
			//un-init
			app.Release();
			//CoUninitialize();
		}
		//_CrtDumpMemoryLeaks();
		CoUninitialize();
		return Return;
}