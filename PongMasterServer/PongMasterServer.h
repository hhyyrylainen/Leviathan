#ifndef PONG_MASTERSERVER
#define PONG_MASTERSERVER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application/MasterServerApplication.h"


namespace Pong{

	class PongMasterServer : public Leviathan::MasterServerApplication{
	public:
		PongMasterServer();
		~PongMasterServer();

		virtual void Tick(int mspassed);

		void CustomizeEnginePostLoad();
		void EnginePreShutdown();

		static wstring GenerateWindowTitle();

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
		virtual void RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager);

		// Game configuration checkers //
		static void CheckGameConfigurationVariables(GameConfiguration* configobj);
		static void CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj);

	protected:

	};

}
#endif
