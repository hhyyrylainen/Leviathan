#ifndef LEVIATHAN_APPLICATION
#define LEVIATHAN_APPLICATION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Engine.h"
#include "Application/AppDefine.h"

namespace Leviathan{

	class LeviathanApplication : public Object, public ThreadSafe{
	public:
		// creation and initialization //
		DLLEXPORT LeviathanApplication();
		DLLEXPORT virtual ~LeviathanApplication();

		DLLEXPORT virtual bool Initialize(AppDef* configuration);
		DLLEXPORT virtual void Release();
		// sets should quit to true so that the main thread will stop everything and close //
		DLLEXPORT virtual void StartRelease();


		// perform actions //
		DLLEXPORT virtual int RunMessageLoop();
		DLLEXPORT virtual void Render();
		DLLEXPORT void PassCommandLine(const wstring &params);
		// Executes all pending command line arguments //
		DLLEXPORT void FlushCommandLine();
		DLLEXPORT virtual void Tick(int mspassed);
		DLLEXPORT virtual void PreFirstTick();
		
		// getting data from the class //
		DLLEXPORT bool Quitting(){ return Quit; };
		DLLEXPORT Engine* GetEngine(){ return _Engine;};
		DLLEXPORT AppDef* GetDefinition(){ return ApplicationConfiguration;};

		// callback functions called during engine initialization at appropriate times //
		DLLEXPORT virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		DLLEXPORT virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
		DLLEXPORT virtual void RegisterApplicationPhysicalMaterials(PhysicsMaterialManager* manager);
		DLLEXPORT virtual void EnginePreShutdown();

		// static access method for getting instance of this class //
		DLLEXPORT static LeviathanApplication* GetApp();
		// Some dummy functions for ease of use //
		DLLEXPORT static void DummyGameConfigurationVariables(GameConfiguration* configobj);
		DLLEXPORT static void DummyGameKeyConfigVariables(KeyConfiguration* keyconfigobj);

	protected:
		// called just before returning from initialization, and can be used setting start time etc. //
		DLLEXPORT virtual void _InternalInit();
		// ------------------------------------ //
		bool Quit;
		bool ShouldQuit;

		Engine* _Engine;
		AppDef* ApplicationConfiguration;

		// static part //
		static LeviathanApplication* Curapp;
	};


}
#endif