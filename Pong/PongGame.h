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

#define SCRIPT_REGISTERFAIL	Logger::Get()->Error(L"PongGame: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return;

#define BALLSTUCK_THRESHOLD		0.03f
#define SCOREPOINT_AMOUNT		1

namespace Pong{

	struct StoredCollisionData{
		// Used to store pointers in AABB callback to be used in Contact callback //
		StoredCollisionData() : Body0(NULL), Body1(NULL){
		};
		StoredCollisionData(const NewtonBody* body, const NewtonBody* body2) : Body0(body), Body1(body2){
		};

		const NewtonBody* Body0;
		const NewtonBody* Body1;
	};

	class PongGame : public Leviathan::LeviathanApplication{
		friend Arena;
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

		// Called when scored, will handle everything //
		int PlayerScored(Leviathan::BasePhysicsObject* goalptr);


		PlayerSlot* GetPlayerSlot(int id);

		void inline SetError(const string &error){
			ErrorState = error;
		}
		string GetErrorString();

		int GetScoreLimit();
		void SetScoreLimit(int scorelimit);

		bool PlayerIDMatchesGoalAreaID(int plyid, Leviathan::BasePhysicsObject* goalptr);

		void CheckForGameEnd();

		// customized callbacks //
		virtual void InitLoadCustomScriptTypes(asIScriptEngine* engine);
		virtual void RegisterCustomScriptTypes(asIScriptEngine* engine, std::map<int, wstring> &typeids);
		virtual void RegisterApplicationPhysicalMaterials(Leviathan::PhysicsMaterialManager* manager);

		// Ball handling callback //
		static int BallAABBCallbackPaddle(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1, int threadIndex);
		static void BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep, int threadIndex);
		static int BallAABBCallbackGoalArea(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1, int threadIndex);
		static void BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep, int threadIndex);

	protected:

		// This function sets the player ID who should get points for scoring //
		void _SetLastPaddleHit(Leviathan::BasePhysicsObject* objptr, Leviathan::BasePhysicsObject* objptr2);
		// Handles score increase from scoring and destruction of ball. The second parameter is used to ensuring it is the right ball //
		int _BallEnterGoalArea(Leviathan::BasePhysicsObject* goal, Leviathan::BasePhysicsObject* ballobject);
		// ------------------------------------ //

		// game objects //
		unique_ptr<Arena> GameArena;

		int LastPlayerHitBallID;

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

		// Map for storing collision data between the callbacks //
		std::map<int, StoredCollisionData> ThreadIDStoredBodyPtrsMap;
		std::map<int, StoredCollisionData> ThreadIDStoredBallGoalHitPtrsMap;

		static PongGame* StaticAccess;
	};

}
// ------------------------------------ //

#endif