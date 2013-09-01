#include "Define.h"
#include "App.h"

// visual leak detector //
#include <vld.h>



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif


	int Return = 0;
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	//_CrtSetBreakAlloc(5426);

	if(SUCCEEDED(CoInitialize(NULL))){

		//LeviathanApplication* instance = LeviathanApplication::GetApp(); 
		//if(instance != NULL){

		// now that we are in code we can start tracking //
		VLDEnable();

		// create app //
		SandBoxie::App app = SandBoxie::App();

		wstring tittle = L"wrkn SandBoxie v ";
		tittle += GAME_VERSIONS;
		tittle += L" Leviathan ";
		tittle += VERSIONS;

		unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine());
		// customize values //
		ProgramDefinition->SetHInstance(hInstance);

		// create window parameters last //
		ProgramDefinition->StoreWindowDetails(tittle, true, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), 
			SandBoxie::App::WndProc, &app);
			



		if(app.Initialize(ProgramDefinition.get())){
			app.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));
			// this is where the game should customize the engine //
			app.CustomizeEnginePostLoad();

			Logger::Get()->Info(L"Engine successfully initialized", true);
			Return = app.RunMessageLoop();
		} else {
			Logger::Get()->Error(L"App init failed, closing",0, true);
			app.Release();
			Return = 005;
		}
	}

	VLDReportLeaks();
	//_CrtDumpMemoryLeaks();

	CoUninitialize();
	return Return;
}