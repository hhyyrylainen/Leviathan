#ifndef PONG_GAME
#define PONG_GAME
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Arena.h"
#include "PlayerSlot.h"

#define SCRIPT_REGISTERFAIL	Logger::Get()->Error(L"PongGame: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return;

namespace Pong{

	class PongGame : public Leviathan::LeviathanApplication{
	public:
		PongGame();
		~PongGame();

		int TryStartGame();

		void CustomizeEnginePostLoad();

		static wstring GenerateWindowTitle();
		// posts a quit message to quit after script has returned //
		void ScriptCloseGame();

		static PongGame* Get();

		void inline SetError(const string &error){
			ErrorState = error;
		}
		string GetErrorString();

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
	protected:
		// game objects //
		unique_ptr<Arena> GameArena;


		vector<PlayerSlot*> PlayerList;


		// stores last error string for easy access from scripts //
		string ErrorState;

		static PongGame* StaticAccess;
	};

}
// ------------------------------------ //

#endif