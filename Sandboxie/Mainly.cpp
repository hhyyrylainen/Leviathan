#include "Define.h"
#include "App.h"

using namespace SandBoxie;

// ------------------ ProgramConfiguration ------------------ //

#define PROGRAMCLASSNAME				App
#define PROGRAMLOG						L"Sandboxie"
#define ENGINECONFIGURATION				L"./EngineConf.conf"
#define PROGRAMCONFIGURATION			L"./Sandboxie.conf"
#define PROGRAMKEYCONFIGURATION			L"./SandboxieKeys.conf"
#define PROGRAMCHECKCONFIGFUNCNAME		App::CheckGameConfigurationVariables
#define PROGRAMCHECKKEYCONFIGFUNCNAME	App::CheckGameKeyConfigVariables
#define PROGRAMMASTERSERVERINFO			MasterServerInformation()
#define WINDOWTITLEGENFUNCTION			App::GenerateWindowTitle()

#define USERREADABLEIDENTIFICATION		L"Sandboxie version " GAME_VERSIONS
#define GAMENAMEIDENTIFICATION			L"Sandboxie"
#define GAMEVERSIONIDENTIFICATION		GAME_VERSIONS

// Don't look at the mess ahead, just set the previous things and customize using virtual functions //

#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
#if defined(DEBUG) | defined(_DEBUG)
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
	PROGRAMCLASSNAME app;

	unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine(PROGRAMLOG, ENGINECONFIGURATION, PROGRAMCONFIGURATION, PROGRAMKEYCONFIGURATION, 
		&PROGRAMCHECKCONFIGFUNCNAME, &PROGRAMCHECKKEYCONFIGFUNCNAME));
	// customize values //
#ifdef _WIN32
	ProgramDefinition->SetHInstance(hInstance);
#endif
	ProgramDefinition->SetMasterServerParameters(PROGRAMMASTERSERVERINFO).SetPacketHandler(NULL).SetApplicationIdentification(
		USERREADABLEIDENTIFICATION, GAMENAMEIDENTIFICATION, GAMEVERSIONIDENTIFICATION);

	// create window last //
	ProgramDefinition->StoreWindowDetails(WINDOWTITLEGENFUNCTION, true,
#ifdef _WIN32
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
#endif
		&app);

#ifdef _WIN32
	app.PassCommandLine(Convert::CharPtrToWstring(lpCmdLine));
#else
	wstring commandline = L"";
	for(int i = 1; i < argcount; i++){
		commandline += L" "+Leviathan::Convert::StringToWstring(args[i]);
	}
	app.PassCommandLine(commandline);
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
		app.ForceRelease();
		Return = 5;
	}
#ifdef _WIN32
}
//_CrtDumpMemoryLeaks();
CoUninitialize();
#endif

return Return;
}
