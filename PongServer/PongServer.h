#ifndef PONG_SERVER
#define PONG_SERVER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application/ServerApplication.h"
#include "CommonPong.h"
#include "GameInputController.h"
#include "PongServerNetworking.h"

namespace Pong{

	class PongServer : public CommonPongParts<Leviathan::ServerApplication, true>{
	public:
		PongServer();
		~PongServer();

		virtual void Tick(int mspassed);

		void TryStartMatch();
		void CheckForGameEnd();


		static wstring GenerateWindowTitle();
		

		// Game configuration checkers //
		static void CheckGameConfigurationVariables(GameConfiguration* configobj);
		static void CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj);

		//! Used to set the server status as joinable (it has started)
		virtual void PreFirstTick();


		//! Makes sure doesn't start in GUI mode
		virtual void PassCommandLine(const wstring &params);

		//! This doesn't need any handling
		virtual void OnPlayerStatsUpdated(PlayerList* list){

		}

		PongServerNetworking* GetServerNetworkInterface(){

			return _PongServerNetworking;
		}


		static PongServer* Get(); 

	protected:

		virtual void ServerCheckEnd();
		virtual void DoSpecialPostLoad();
		virtual void CustomizedGameEnd();

		virtual void MoreCustomScriptTypes(asIScriptEngine* engine);
		virtual void MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids);

		// Server specific connection handling //


		PongServerNetworking* _PongServerNetworking;

		shared_ptr<GameInputController> ServerInputHandler;


		static PongServer* Staticaccess;
	};

}
#endif
