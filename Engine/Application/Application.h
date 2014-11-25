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
#include "Script/ScriptInterface.h"

namespace Leviathan{

	class LeviathanApplication : public Object, public ThreadSafe{
	public:
		// creation and initialization //
		DLLEXPORT LeviathanApplication();
		DLLEXPORT virtual ~LeviathanApplication();

		DLLEXPORT virtual bool Initialize(AppDef* configuration);

		//! \brief Used to immediately terminate the program
		//! \note Should be only called if initialization fails
		DLLEXPORT void ForceRelease();

		//! \brief Safely releases the Application //
		//! \note This should be used instead of Release
		DLLEXPORT virtual void StartRelease();

		//! \brief Thread safely marks the game to close sometime
		//!
		//! The closing should happen in around 2 ticks (100ms)
		DLLEXPORT void MarkAsClosing();

		// perform actions //
		DLLEXPORT virtual int RunMessageLoop();
		DLLEXPORT virtual void Render();
		DLLEXPORT virtual void PassCommandLine(const wstring &params);
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

        //! \brief Used to query a world for specific id
        //!
        //! This is called when the world holder couldn't find a world with the id
        DLLEXPORT virtual shared_ptr<GameWorld> GetGameWorld(int id);

        
		// static access method for getting instance of this class //
		DLLEXPORT static LeviathanApplication* GetApp();
        DLLEXPORT static LeviathanApplication* Get();
        
		// Some dummy functions for ease of use //
		DLLEXPORT static void DummyGameConfigurationVariables(GameConfiguration* configobj);
		DLLEXPORT static void DummyGameKeyConfigVariables(KeyConfiguration* keyconfigobj);

	protected:

		//! \brief Performs the final steps in the release process
		//! \warning This should not be called directly
		DLLEXPORT virtual void Release();

		// called just before returning from initialization, and can be used setting start time etc. //
		DLLEXPORT virtual void _InternalInit();
		// ------------------------------------ //

		bool Quit;
		bool ShouldQuit;
		//! This can be quickly set anywhere to quit sometime in the future 
		bool QuitSometime;

		Engine* _Engine;
		AppDef* ApplicationConfiguration;
        
		// static part //
		static LeviathanApplication* Curapp;
	};


}
#endif
