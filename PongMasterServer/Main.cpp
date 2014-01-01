#include "PongMasterServerIncludes.h"
#include "PongMasterServer.h"

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
	int Return = 0;
#ifdef _WIN32
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if(SUCCEEDED(CoInitialize(NULL))){
#else

#endif

#ifdef LEVIATHAN_USES_VLD
		// now that we are in code we can start tracking //

		VLDEnable();
#endif // LEVIATHAN_USES_VLD


		// create program object //
		PongMasterServer app;


		unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine(L"PongMaster", L"./EngineConf.conf", L"./Pong.conf", L"", &PongMasterServer::CheckGameConfigurationVariables,
			&PongMasterServer::CheckGameKeyConfigVariables));
		// customize values //
#ifdef _WIN32
		ProgramDefinition->SetHInstance(hInstance);
#endif
		ProgramDefinition->SetMasterServerParameters(MasterServerInformation(true, L"Pong_" GAME_VERSIONS));

		// create window last //
		ProgramDefinition->StoreWindowDetails(PongMasterServer::GenerateWindowTitle(), true,
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
		game.PassCommandLine(commandline);
#endif

		if(app.Initialize(ProgramDefinition.get())){

			// this is where the game should customize the engine //
			app.CustomizeEnginePostLoad();

			// After everything is ready the command line should be flushed //
			app.FlushCommandLine();


			Logger::Get()->Info(L"Engine successfully initialized", true);
			Return = app.RunMessageLoop();
		} else {
			Logger::Get()->Error(L"App init failed, closing", true);
			app.Release();
			Return = 5;
		}
#ifdef _WIN32
	}
	//_CrtDumpMemoryLeaks();
	CoUninitialize();
#endif

	return Return;
}