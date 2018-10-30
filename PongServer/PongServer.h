#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Application/ServerApplication.h"
#include "CommonPong.h"
#include "GameInputController.h"
#include "PongServerNetworking.h"
#include <memory>

namespace Pong{

    using namespace std;

    class PongServer : public CommonPongParts<Leviathan::ServerApplication, true>{
    public:
        PongServer();
        ~PongServer();
        

        void Tick(int mspassed) override;

        void TryStartMatch();
        void CheckForGameEnd();


        static string GenerateWindowTitle();
        

        // Game configuration checkers //
        static void CheckGameConfigurationVariables(Lock &guard, GameConfiguration* configobj);
        static void CheckGameKeyConfigVariables(Lock &guard, KeyConfiguration* keyconfigobj);

        //! Used to set the server status as joinable (it has started)
        virtual void PreFirstTick() override;

        //! This doesn't need any handling
        virtual void OnPlayerStatsUpdated(PlayerList* list) override{}

        //! Changes the server to preparing screen
        void OnStartPreMatch();

        // PongServerNetworking& GetServerNetworkInterface() {

        //     return *ServerInterface;
        // }

        
        void SetScoreLimit(int scorelimit);


        //! \brief Called when scored, will handle everything
        int PlayerScored(ObjectID goal);

        //! This function sets the player ID who should get points for scoring //
        void _SetLastPaddleHit(ObjectID objptr, ObjectID objptr2);
        

        //! Handles score increase from scoring and destruction of ball.
        //! The second parameter is used to ensuring it is the right ball
        int _BallEnterGoalArea(ObjectID goal, ObjectID ballobject);

        void _DisposeOldBall();
        
        void GameMatchEnded();

        static PongServer* Get();

        
        // static void BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep,
        //     int threadIndex);
        
        // static void BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep,
        //     int threadIndex);

        // PhysicsMaterialContactCallback GetBallPaddleCallback() override;

        // PhysicsMaterialContactCallback GetBallGoalAreaCallback() override;

	protected:

        Leviathan::NetworkInterface* _GetApplicationPacketHandler() override;
        void _ShutdownApplicationPacketHandler() override;
        
        virtual void ServerCheckEnd();
        
        void DoSpecialPostLoad() override;
        void CustomizedGameEnd() override;

        bool MoreCustomScriptTypes(asIScriptEngine* engine) override;

        //! \brief For testing AI with valgrind
        //! \todo Add a score limit and a way to go back to default state afterwards
        void RunAITestMatch();

    protected:

        // Server specific connection handling //

        std::unique_ptr<PongServerNetworking> ServerInterface;

        shared_ptr<GameInputController> ServerInputHandler;


        //! Ball's position during last tick. This is used to see if the ball is "stuck"
        Float3 BallLastPos;
        
        //! Direction in which the ball can get stuck
        Float3 DeadAxis;
        int StuckThresshold;

        
        static PongServer* Staticaccess;
    };
}

