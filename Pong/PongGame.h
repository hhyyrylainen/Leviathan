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
#include "Entities\Bases\BasePhysicsObject.h"
#include "Utility\DataHandling\SimpleDataBase.h"
#include "GUI\GuiManager.h"

#define SCRIPT_REGISTERFAIL	Logger::Get()->Error(L"PongGame: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return;

#define BALLSTUCK_THRESHOLD		0.045f
#define BALLSTUCK_COUNT			8
#define SCOREPOINT_AMOUNT		1

namespace Pong{

	class PongGame : public Leviathan::LeviathanApplication{
		friend Arena;
	public:
		PongGame();
		~PongGame();

		int TryStartGame();
		void GameMatchEnded();

		virtual void Tick(int mspassed);

		void CustomizeEnginePostLoad();
		void EnginePreShutdown();

		static wstring GenerateWindowTitle();
		// posts a quit message to quit after script has returned //
		void ScriptCloseGame();

		// Updates the ball trail based on the player colour //
		void SetBallLastHitColour();

		static PongGame* Get();

		// Called when scored, will handle everything //
		int PlayerScored(Leviathan::BasePhysicsObject* goalptr);

		// Will determine if a paddle could theoretically hit the ball //
		bool IsBallInGoalArea();

		PlayerSlot* GetPlayerSlot(int id);

		void inline SetError(const string &error){
			ErrorState = error;
		}
		string GetErrorString();

		int GetScoreLimit();
		void SetScoreLimit(int scorelimit);

		bool PlayerIDMatchesGoalAreaID(int plyid, Leviathan::BasePhysicsObject* goalptr);

		void SetPauseState(bool paused);

		void CheckForGameEnd();

		Leviathan::SimpleDatabase* GetGameDatabase();

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
		virtual void RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager);

		// Ball handling callback //
		static void BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep, int threadIndex);
		static void BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep, int threadIndex);

	protected:

		// This function sets the player ID who should get points for scoring //
		void _SetLastPaddleHit(Leviathan::BasePhysicsObject* objptr, Leviathan::BasePhysicsObject* objptr2);
		// Handles score increase from scoring and destruction of ball. The second parameter is used to ensuring it is the right ball //
		int _BallEnterGoalArea(Leviathan::BasePhysicsObject* goal, Leviathan::BasePhysicsObject* ballobject);

		void _DisposeOldBall();
		// ------------------------------------ //

		// game objects //
		unique_ptr<Arena> GameArena;
		shared_ptr<GameWorld> WorldOfPong;
		Leviathan::Gui::GuiManager* GuiManagerAccess;

		// AI module //
		GameModule* GameAI;


		int LastPlayerHitBallID;

		bool GamePaused;
		int ScoreLimit;



		vector<PlayerSlot*> PlayerList;
		GameInputController* GameInputHandler;

		// stores last error string for easy access from scripts //
		string ErrorState;

		// Used to count ticks to not have to call set apply force every tick //
		int Tickcount;
		// Ball's position during last tick. This is used to see if the ball is "stuck" //
		Float3 BallLastPos;
		// Direction in which the ball can get stuck //
		Float3 DeadAxis;
		int StuckThresshold;

		// Configuration data //
		Leviathan::SimpleDatabase GameConfigurationData;

		static PongGame* StaticAccess;
	};

}
// ------------------------------------ //

#endif