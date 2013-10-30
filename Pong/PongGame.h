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
#include "GameInputController.h"

#define SCRIPT_REGISTERFAIL	Logger::Get()->Error(L"PongGame: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return;

namespace Pong{

	class PongGame : public Leviathan::LeviathanApplication{
	public:
		PongGame();
		~PongGame();

		int TryStartGame();
		void GameMatchEnded();

		// function called after input has been received (should be just before rendering) //
		void ProcessPlayerInputsAndState();

		virtual void Tick(int mspassed);

		void CustomizeEnginePostLoad();

		static wstring GenerateWindowTitle();
		// posts a quit message to quit after script has returned //
		void ScriptCloseGame();

		static PongGame* Get();


		PlayerSlot* GetPlayerSlot(int id);

		void inline SetError(const string &error){
			ErrorState = error;
		}
		string GetErrorString();

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
		virtual void RegisterApplicationPhysicalMaterials(PhysicsMaterialManager* manager);

	protected:
		// game objects //
		unique_ptr<Arena> GameArena;


		vector<PlayerSlot*> PlayerList;
		GameInputController* GameInputHandler;

		// stores last error string for easy access from scripts //
		string ErrorState;

		// Used to count ticks to not have to call set apply force every tick //
		int Tickcount;

		static PongGame* StaticAccess;
	};

}
// ------------------------------------ //

#endif