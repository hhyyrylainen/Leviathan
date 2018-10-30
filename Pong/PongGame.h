#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
#include "GameInputController.h"
#include "GUI/GuiManager.h"
#include "Application/GameConfiguration.h"
#include "Application/KeyConfiguration.h"
#include "Application/ClientApplication.h"
#include "CommonPong.h"

#include "Events/EventHandler.h"

namespace Pong{

    class PongNetHandler;


    class PongGame : public CommonPongParts<Leviathan::ClientApplication, false>,
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
        bool Connect(const string &address, string &errorstr);

        //! \brief Connect method with no result and no error return
        void ConnectNoError(const string &address){
            
            string errorcatcher;
            Connect(address, errorcatcher);
        }

        //! \brief Returns our client ID player number
        int GetOurPlayerID();

        void Tick(int mspassed) override;
        
        //! \brief Sends a command to the current server if connected
        //! \return True if connected, false otherwise
        bool SendServerCommand(const string &command);

        void AllowPauseMenu();

        //! Verifies that the GUI displays correct state
        void VerifyCorrectState(PONG_JOINGAMERESPONSE_TYPE serverstatus);


        PongNetHandler& GetInterface() const{

            return *ClientInterface;
        }

        static string GenerateWindowTitle();

        static PongGame* Get();

        // Game configuration checkers //
        static void CheckGameConfigurationVariables(Lock &guard, GameConfiguration* configobj);
        static void CheckGameKeyConfigVariables(Lock &guard, KeyConfiguration* keyconfigobj);

        GameInputController* GetInputController(){

            return GameInputHandler.get();
        }

        virtual int OnEvent(Event* event) override;
        virtual int OnGenericEvent(GenericEvent* event) override{
            return -1;
        }
        
        // static void BallContactCallbackPaddle(const NewtonJoint* contact, dFloat timestep,
        //     int threadIndex)
        // {

        // }
        
        // static void BallContactCallbackGoalArea(const NewtonJoint* contact, dFloat timestep,
        //     int threadIndex)
        // {

        //     // The ball will always go through it... //
        //     NewtonJointSetCollisionState(contact, 0);
        // }



        // virtual PhysicsMaterialContactCallback GetBallPaddleCallback() override{

        //     return BallContactCallbackPaddle;
        // }

        // virtual PhysicsMaterialContactCallback GetBallGoalAreaCallback() override{

        //     return BallContactCallbackGoalArea;
        // }
        
    protected:

        Leviathan::NetworkInterface* _GetApplicationPacketHandler() override;
        void _ShutdownApplicationPacketHandler() override;        

        virtual void DoSpecialPostLoad() override;
        virtual void CustomizedGameEnd() override;
        virtual bool MoreCustomScriptTypes(asIScriptEngine* engine) override;


        //! \brief Sends updates to the GUI
        //! \todo Implement this
        virtual void OnPlayerStatsUpdated(PlayerList* list) override;

        // ------------------------------------ //
        Leviathan::GUI::GuiManager* GuiManagerAccess;
        shared_ptr<GameInputController> GameInputHandler;

        std::unique_ptr<PongNetHandler> ClientInterface;

#ifdef _WIN32

        HANDLE ServerProcessHandle;

#endif // _WIN32


        static PongGame* StaticGame;
    };

}
// ------------------------------------ //


