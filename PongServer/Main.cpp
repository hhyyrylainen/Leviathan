#include "PongServerIncludes.h"

#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
	//_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

	int Return = 0;
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);


	if(SUCCEEDED(CoInitialize(NULL))){

#ifdef LEVIATHAN_USES_VLD
		// now that we are in code we can start tracking //

		VLDEnable();
#endif // LEVIATHAN_USES_VLD


		// create game object //
		PongGame game;



		unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine());
		// customize values //
		ProgramDefinition->SetHInstance(hInstance);

		// create window parameters last //
		ProgramDefinition->StoreWindowDetails(PongGame::GenerateWindowTitle(), true, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), &game);


		if(game.Initialize(ProgramDefinition.get())){
			game.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));
			// this is where the game should customize the engine //
			game.CustomizeEnginePostLoad();

			Logger::Get()->Info(L"Engine successfully initialized", true);
			Return = game.RunMessageLoop();
		} else {
			Logger::Get()->Error(L"App init failed, closing", true);
			game.Release();
			Return = 005;
		}
	}

	CoUninitialize();
	return Return;
}