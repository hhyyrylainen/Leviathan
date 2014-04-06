#ifndef PONG_GAME
#define PONG_GAME
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "GameInputController.h"
#include "GUI/GuiManager.h"
#include "Application/GameConfiguration.h"
#include "Application/KeyConfiguration.h"
#include "CommonPong.h"

namespace Pong{

	class PongGame : public CommonPongParts<Leviathan::LeviathanApplication, false>{
	public:
		PongGame();
		~PongGame();

		int StartServer();

		//! \brief Called when game returns from win screen to the lobby screen
		void MoveBackToLobby();

		void StartInputHandling();

		//! \brief Called when the game wants to exit current game/lobby
		void Disconnect(const string &reasonstring);

		//! \brief Connects to a server specified by an address string
		//! \note Only allows connections to be made to one server at a time (excluding remote console connections)
		bool Connect(const wstring &address, wstring &errorstr);

		void ConnectProxy(const wstring &address){
			wstring error;
			Connect(address, error);
		}

		void AllowPauseMenu();

		static wstring GenerateWindowTitle();

		static PongGame* Get();

		// Game configuration checkers //
		static void CheckGameConfigurationVariables(GameConfiguration* configobj);
		static void CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj);

	protected:

		virtual void DoSpecialPostLoad();
		virtual void CustomizedGameEnd();
		virtual void MoreCustomScriptTypes(asIScriptEngine* engine);
		virtual void MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids);


		//! \brief Sends updates to the GUI
		//! \todo Implement this
		virtual void OnPlayerStatsUpdated(PlayerList* list);

		// ------------------------------------ //
		Leviathan::Gui::GuiManager* GuiManagerAccess;
		GameInputController* GameInputHandler;

#ifdef _WIN32

		HANDLE ServerProcessHandle;

#endif // _WIN32


		static PongGame* StaticGame;
	};

}
// ------------------------------------ //

#endif
