#ifndef PONG_SERVER
#define PONG_SERVER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application/ServerApplication.h"


namespace Pong{

	class PongServer : public Leviathan::ServerApplication{
	public:
		PongServer();
		~PongServer();

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
