#ifndef PONG
#define PONG
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

#define SCRIPT_REGISTERFAIL	Logger::Get()->Error(L"PongGame: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return;

namespace Pong{

	class PongGame : public Leviathan::LeviathanApplication{
	public:
		PongGame();


		int TryStartGame();

		void CustomizeEnginePostLoad();

		static wstring GenerateWindowTitle();
		// posts a quit message to quit after script has returned //
		void ScriptCloseGame();

		static PongGame* Get();

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
	protected:



		static PongGame* StaticAccess;
	};

}
// ------------------------------------ //

#endif