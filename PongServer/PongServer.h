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

        void Tick(int mspassed) override;

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

		//! Changes the server to preparing screen
		void OnStartPreMatch();

		PongServerNetworking* GetServerNetworkInterface(){

			return _PongServerNetworking;
		}
        
		void SetScoreLimit(int scorelimit){
			ScoreLimit = scorelimit;
		}

        //! \brief Called when scored, will handle everything
		int PlayerScored(Leviathan::BasePhysicsObject* goalptr){
			// Don't count if the player whose goal the ball is in is the last one to touch it or if none have touched it //
			if(PlayerIDMatchesGoalAreaID(LastPlayerHitBallID, goalptr) || LastPlayerHitBallID == -1){

				return 1;
			}

			// Add point to the player who scored //

			// Look through all players and compare PlayerIDs //
			for(size_t i = 0; i < _PlayerList.Size(); i++){

				PlayerSlot* slotptr = _PlayerList[i];

				while(slotptr){


					if(LastPlayerHitBallID == slotptr->GetPlayerNumber()){
						// Found right player //
						slotptr->SetScore(slotptr->GetScore()+SCOREPOINT_AMOUNT);
						goto playrscorelistupdateendlabel;
					}

					slotptr = slotptr->GetSplit();
				}
			}
			// No players got points! //

playrscorelistupdateendlabel:

            GameArena->LetGoOfBall();

            Leviathan::ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind<void>([](int LastPlayerHitBallID,
                            PongServer* instance) -> void
                {

                    // Send ScoreUpdated event //
                    Leviathan::EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(new wstring(L"ScoreUpdated"),
                            new NamedVars(shared_ptr<NamedVariableList>(new NamedVariableList(L"ScoredPlayer", new
                                        Leviathan::VariableBlock(LastPlayerHitBallID))))));

                    // Serve new ball //
                    instance->GameArena->ServeBall();

                    // Check for game end //
                    instance->ServerCheckEnd();

                    
                }, LastPlayerHitBallID.GetValue(), this)));

			return 0;
		}

		//! This function sets the player ID who should get points for scoring //
		void _SetLastPaddleHit(Leviathan::BasePhysicsObject* objptr, Leviathan::BasePhysicsObject* objptr2){
			// Note: the object pointers can be in any order they want //

			Leviathan::BasePhysicsObject* realballptr = dynamic_cast<Leviathan::BasePhysicsObject*>(
                GameArena->GetBallPtr().get());

			// Look through all players and compare paddle ptrs //
			for(size_t i = 0; i < _PlayerList.Size(); i++){

				PlayerSlot* slotptr = _PlayerList[i];

				while(slotptr){

					Leviathan::BasePhysicsObject* castedptr = dynamic_cast<Leviathan::BasePhysicsObject*>(
                        slotptr->GetPaddle().get());

					if((objptr == castedptr && objptr2 == realballptr) ||
                        (objptr2 == castedptr && objptr == realballptr))
                    {
						// Found right player //
                        if(LastPlayerHitBallID != slotptr->GetPlayerNumber()){
                            LastPlayerHitBallID = slotptr->GetPlayerNumber();
                            SetBallLastHitColour();
                        }
                        
						return;
					}

					slotptr = slotptr->GetSplit();
				}
			}
		}

		//! Handles score increase from scoring and destruction of ball. The second parameter is used to
        //! ensuring it is the right ball
		int _BallEnterGoalArea(Leviathan::BasePhysicsObject* goal, Leviathan::BasePhysicsObject* ballobject){
			// Note: the object pointers can be in any order they want //

			Leviathan::BasePhysicsObject* castedptr = dynamic_cast<Leviathan::BasePhysicsObject*>(
                GameArena->GetBallPtr().get());

			if(ballobject == castedptr){
				// goal is actually the goal area //
				return PlayerScored(goal);
			} else if(goal == castedptr){
				// ballobject is actually the goal area //
				return PlayerScored(ballobject);
			}
			return 0;
		}

		void _DisposeOldBall(){

			// Tell arena to let go of old ball //
			GameArena->LetGoOfBall();

			// Reset variables //
			LastPlayerHitBallID = -1;
			StuckThresshold = 0;
			// This should reset the ball trail colour //
			SetBallLastHitColour();
		}

		void GameMatchEnded(){
			// This can be called from script so ensure that these are set //
			GameArena->LetGoOfBall();

			CustomizedGameEnd();
		}

        static PongServer* Get();

        
        static void BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep, int threadIndex){

			// Call the callback //
			Staticaccess->_SetLastPaddleHit(reinterpret_cast<Leviathan::BasePhysicsObject*>(
                    NewtonBodyGetUserData(NewtonJointGetBody0(contact))), reinterpret_cast<
                Leviathan::BasePhysicsObject*>(NewtonBodyGetUserData(NewtonJointGetBody1(contact))));
		}
		static void BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep, int threadIndex){
			// Call the function and set the collision state as the last one //
			NewtonJointSetCollisionState(contact, Staticaccess->_BallEnterGoalArea(reinterpret_cast<
                    Leviathan::BasePhysicsObject*>(NewtonBodyGetUserData(NewtonJointGetBody0(contact))),
                    reinterpret_cast<Leviathan::BasePhysicsObject*>(NewtonBodyGetUserData(
                            NewtonJointGetBody1(contact)))));
		}



        virtual PhysicsMaterialContactCallback GetBallPaddleCallback(){

            return BallContactCallbackPaddle;
        }

        virtual PhysicsMaterialContactCallback GetBallGoalAreaCallback(){

            return BallContactCallbackGoalArea;
        }

	protected:

		virtual void ServerCheckEnd();
		virtual void DoSpecialPostLoad();
		virtual void CustomizedGameEnd();

		virtual bool MoreCustomScriptTypes(asIScriptEngine* engine);
		virtual void MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids);

		// Server specific connection handling //


		PongServerNetworking* _PongServerNetworking;

		shared_ptr<GameInputController> ServerInputHandler;


		//! Ball's position during last tick. This is used to see if the ball is "stuck"
		Float3 BallLastPos;
        
		//! Direction in which the ball can get stuck
		Float3 DeadAxis;
		int StuckThresshold;

        
		static PongServer* Staticaccess;
	};

}
#endif
