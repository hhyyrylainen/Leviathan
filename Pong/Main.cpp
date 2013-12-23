#include "PongIncludes.h"
#include "PongGame.h"

#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD

using namespace Pong;

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
    cout << "entering!" << __FILE__ << __LINE__ << endl;
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


		// create game object //
		PongGame game;

		unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine());
		// customize values //
#ifdef _WIN32
		ProgramDefinition->SetHInstance(hInstance);
#endif
ProgramDefinition->SetMasterServerParameters(MasterServerInformation(L"PongMasters.txt", L"Pong_" GAME_VERSIONS,
			L"http://boostslair.com/", L"/Pong/MastersList.php", L"PongCrecentials", false));

		// create window last //
		ProgramDefinition->StoreWindowDetails(PongGame::GenerateWindowTitle(), true,
#ifdef _WIN32
			LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
#endif
			&game);



		if(game.Initialize(ProgramDefinition.get())){
#ifdef _WIN32
			game.PassCommandLine(Convert::StringToWstringNonRef(lpCmdLine));
#else
			wstring commandline = L"";
			for(int i = 1; i < argcount; i++){
				commandline += L" "+Leviathan::Convert::StringToWstring(args[i]);
			}
			game.PassCommandLine(commandline);
#endif
			// this is where the game should customize the engine //
			game.CustomizeEnginePostLoad();

			Logger::Get()->Info(L"Engine successfully initialized", true);
			Return = game.RunMessageLoop();
		} else {
			Logger::Get()->Error(L"App init failed, closing", true);
			game.Release();
			Return = 005;
		}
#ifdef _WIN32
	}
	//_CrtDumpMemoryLeaks();
	CoUninitialize();
#endif


	return Return;
}
