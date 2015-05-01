#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Application/MasterServerApplication.h"


namespace Pong{

	class PongMasterServer : public Leviathan::MasterServerApplication{
	public:
		PongMasterServer();
		~PongMasterServer();

		virtual void Tick(int mspassed);

		void CustomizeEnginePostLoad();
		void EnginePreShutdown();

		static std::string GenerateWindowTitle();

		// customized callbacks //
		virtual bool InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine,
            std::map<int, std::string> &typeids);
		virtual void RegisterApplicationPhysicalMaterials(
            Leviathan::PhysicsMaterialManager* manager);

		// Game configuration checkers //
		static void CheckGameConfigurationVariables(Lock &guard, GameConfiguration* configobj);
		static void CheckGameKeyConfigVariables(Lock &guard, KeyConfiguration* keyconfigobj);

	protected:

	};

}

