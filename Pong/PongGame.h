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

#include "Events/EventHandler.h"

namespace Pong{

	class PongNetHandler;


	class PongGame : public CommonPongParts<Leviathan::LeviathanApplication, false>,
                       public Leviathan::CallableObject
    {
	public:
		PongGame();
		~PongGame();

		int StartServer();

		//! \brief Called when game returns from win screen to the lobby screen
		void MoveBackToLobby();

		void StartInputHandling();

        void CustomEnginePreShutdown() override;

		//! \brief Called when the game wants to exit current game/lobby
		void Disconnect(const string &reasonstring);

		//! \brief Connects to a server specified by an address string
		//! \note Only allows connections to be made to one server at a time (excluding remote console connections)
		bool Connect(const wstring &address, wstring &errorstr);

		bool ConnectProxy(const string &address, string &error);

		//! \brief Connect method with no result and no error return
		void Connect(const wstring &address){
			
			wstring errorcatcher;
			Connect(address, errorcatcher);
		}


		//! \brief Sends a command to the current server if connected
		//! \return True if connected, false otherwise
		bool SendServerCommand(const string &command);

		void AllowPauseMenu();

		//! Verifies that the GUI displays correct state
		void VerifyCorrectState(PONG_JOINGAMERESPONSE_TYPE serverstatus);


		PongNetHandler* GetInterface() const{

			return ClientInterface;
		}

		static wstring GenerateWindowTitle();

		static PongGame* Get();

		// Game configuration checkers //
		static void CheckGameConfigurationVariables(GameConfiguration* configobj);
		static void CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj);

		GameInputController* GetInputController(){

			return GameInputHandler.get();
		}

        virtual int OnEvent(Event** pEvent) override;
        virtual int OnGenericEvent(GenericEvent** pevent) override{
            return -1;
        }

        
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
		shared_ptr<GameInputController> GameInputHandler;


		//! Cached version of client network interface
		PongNetHandler* ClientInterface;

#ifdef _WIN32

		HANDLE ServerProcessHandle;

#endif // _WIN32


		static PongGame* StaticGame;
	};

}
// ------------------------------------ //

#endif
