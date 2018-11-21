#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Addons/GameModule.h"
#include "Application/Application.h"
#include "Application/GameConfiguration.h"
#include "Arena.h"
#include "Entities/Components.h"
#include "Entities/GameWorld.h"
#include "Events/EventHandler.h"
#include "GUI/GuiManager.h"
#include "GameInputController.h"
#include "Networking/NetworkHandler.h"
#include "Networking/NetworkInterface.h"
#include "Networking/SyncedResource.h"
#include "Physics/PhysicalMaterial.h"
#include "Physics/PhysicsMaterialManager.h"
#include "PlayerSlot.h"
#include "PongPackets.h"
#include "Script/ScriptExecutor.h"
#include "Statistics/TimingMonitor.h"
#include "Threading/QueuedTask.h"
#include "Threading/ThreadingManager.h"
#include "Window.h"
#include "add_on/autowrapper/aswrappedcall.h"

#include "Generated/StandardWorld.h"

#include <functional>

#define SCRIPT_REGISTERFAIL                                                                \
    Logger::Get()->Error("PongGame: AngelScript: register global failed in file " __FILE__ \
                         " on line " +                                                     \
                         Convert::ToString(__LINE__));                                     \
    return false;

#define BALLSTUCK_THRESHOLD 0.065f
#define BALLSTUCK_COUNT 8
#define SCOREPOINT_AMOUNT 1

#define BALL_SPEED_MAX 35
#define BALL_SPEED_MULT 1.00001f

namespace Pong {

using namespace std;

class BasePongParts;
//! \brief Should be in BasePongParts, used for static access
//!
//! Why is gcc so stupid on linux that it does not allow __declspec(selectany)
extern BasePongParts* BasepongStaticAccess;

//! \brief A parent class for the CommonPongParts class to allow non-template use
//!
//! Mainly required for passing CommonPongParts to non-template functions
class BasePongParts {
    friend Arena;

public:
    static void StatUpdater(PlayerList* list)
    {
        Get()->OnPlayerStatsUpdated(list);
    }


    BasePongParts() :
        _NetworkInterface(nullptr), LastPlayerHitBallID("LastPlayerHitBallID", -1),
        GamePaused("GamePaused", false), ScoreLimit("ScoreLimit", 20),
        _PlayerList(std::function<void(PlayerList*)>(&StatUpdater), 4), ErrorState("No error")
    {
        BasepongStaticAccess = this;
    }

    ~BasePongParts()
    {

        SAFE_DELETE(GameAI);
        BasepongStaticAccess = NULL;
    }

    void SetInterface(NetworkInterface* networkinterface)
    {

        _NetworkInterface = networkinterface;
    }

    //! \brief Updates the ball trail based on the player colour
    void SetBallLastHitColour()
    {
        // Find the player with the last hit identifier and apply that player's colour //
        auto tmplist = _PlayerList.GetVec();

        for(size_t i = 0; i < tmplist.size(); i++) {

            PlayerSlot* slotptr = tmplist[i];

            while(slotptr) {

                if(LastPlayerHitBallID == slotptr->GetPlayerNumber()) {
                    // Set colour //
                    GameArena->ColourTheBallTrail(slotptr->GetColour());
                    return;
                }

                slotptr = slotptr->GetSplit();
            }
        }

        // No other colour is applied so set the default colour //
        GameArena->ColourTheBallTrail(Float4(1.f, 1.f, 1.f, 1.f));
    }


    //! \brief posts a quit message to quit after script has returned
    void ScriptCloseGame()
    {

        Leviathan::LeviathanApplication::Get()->MarkAsClosing();
    }


    //! \brief Will determine if a paddle could theoretically hit the ball
    bool IsBallInGoalArea()
    {
        // Tell arena to handle this //
        return GameArena->IsBallInPaddleArea();
    }

    //! \brief Called before closing the engine, use to release some stuff
    virtual void CustomEnginePreShutdown()
    {

        if(GameAI) {
            GameAI->ReleaseScript();
        }
    }



    bool PlayerIDMatchesGoalAreaID(int plyid, ObjectID goal)
    {
        // Look through all players and compare find the right PlayerID and compare goal
        // area ptr
        for(size_t i = 0; i < _PlayerList.Size(); i++) {

            PlayerSlot* slotptr = _PlayerList[i];

            while(slotptr) {

                if(plyid == slotptr->GetPlayerNumber()) {

                    if(goal == slotptr->GetGoalArea()) {
                        // Found matching goal area //
                        return true;
                    }
                }

                slotptr = slotptr->GetSplit();
            }
        }

        // Not found //
        return false;
    }


    ObjectID GetBall()
    {

        return GameArena->GetBall();
    }

    Leviathan::StandardWorld* GetGameWorld()
    {
        return WorldOfPong.get();
    }
    int GetLastHitPlayer()
    {
        return LastPlayerHitBallID;
    }

    // Variable set/get //
    PlayerSlot* GetPlayerSlot(int id)
    {
        return _PlayerList[id];
    }

    void inline SetError(const string& error)
    {
        ErrorState = error;
    }
    string GetErrorString()
    {
        return ErrorState;
    }

    int GetScoreLimit()
    {
        return ScoreLimit;
    }

    BaseNotifiableAll* GetPlayersAsNotifiable()
    {

        return static_cast<BaseNotifiableAll*>(&_PlayerList);
    }

    static BasePongParts* Get()
    {
        return BasepongStaticAccess;
    }

    PlayerList* GetPlayers()
    {

        return &_PlayerList;
    }

    Arena* GetArena()
    {

        return GameArena.get();
    }

protected:
    // These should be overridden by the child class //
    virtual void DoSpecialPostLoad() = 0;
    virtual void CustomizedGameEnd() = 0;
    virtual void OnPlayerStatsUpdated(PlayerList* list) = 0;

    // ------------------------------------ //

    NetworkInterface* _NetworkInterface = nullptr;

    // game objects //
    std::unique_ptr<Arena> GameArena;
    std::shared_ptr<StandardWorld> WorldOfPong;

    // AI module //
    GameModule* GameAI = nullptr;

    SyncedPrimitive<int> LastPlayerHitBallID;

    SyncedPrimitive<bool> GamePaused;
    SyncedPrimitive<int> ScoreLimit;

    PlayerList _PlayerList;

    //! stores last error string for easy access from scripts
    std::string ErrorState;
};



//! \brief Class that contains common functions required both by Pong and PongServer
//!
//! Set the program type to match the proper class LeviathanApplication or ServerApplication
//! and the IsServer bool to match it (to make syncing with the server work)
template<class ProgramType, bool IsServer>
class CommonPongParts : public BasePongParts, public ProgramType {
public:
    CommonPongParts() {}
    ~CommonPongParts() {}




    virtual shared_ptr<GameWorld> GetGameWorld(int id) override
    {
        // The IDs won't probably match, so return our only world anyways //
        // if(!id != WorldOfPong->GetID()){

        //     Logger::Get()->Error("Pong asked to return a world that isn't WorldOfPong, ID:
        //     "+
        //         Convert::ToString(id)+", WorldOfPong:
        //         "+Convert::ToString(WorldOfPong->GetID()));
        //     return nullptr;
        // }

        return WorldOfPong;
    }

    // These handle the common code between the server and client //
    virtual void CustomizeEnginePostLoad() override
    {

        LEVIATHAN_ASSERT(_NetworkInterface, "SetInterface not called!");
        LEVIATHAN_ASSERT(_NetworkInterface->GetOwner(), "_NetworkInterface has no owner!");

        using namespace Leviathan;

        QUICKTIME_THISSCOPE;

        // Load the game AI //
        // GameAI = new GameModule("PongAIModule", "PongGameCore");
        LOG_WRITE("TODO: port over AI module load");


        // No AI for the game //
        Logger::Get()->Error("Failed to load AI!");

        // Load Pong specific packets //
        PongPackets::RegisterAllPongPacketTypes();

        // load predefined materials //
        auto PaddleMaterial =
            std::make_unique<Leviathan::PhysicalMaterial>("PaddleMaterial", 1);
        auto ArenaMaterial = std::make_unique<Leviathan::PhysicalMaterial>("ArenaMaterial", 2);
        auto ArenaBottomMaterial =
            std::make_unique<Leviathan::PhysicalMaterial>("ArenaBottomMaterial", 3);
        auto BallMaterial = std::make_unique<Leviathan::PhysicalMaterial>("BallMaterial", 4);
        auto GoalAreaMaterial =
            std::make_unique<Leviathan::PhysicalMaterial>("GoalAreaMaterial", 5);

        // Set callbacks //
        BallMaterial->FormPairWith(*PaddleMaterial)
            /*.SetSoftness(1.f)
            .SetElasticity(1.0f)
            .SetFriction(1.f, 1.f)*/
            /*.SetCallbacks(NULL, GetBallPaddleCallback())*/;
        BallMaterial->FormPairWith(*GoalAreaMaterial)
            // .SetCallbacks(NULL, GetBallGoalAreaCallback())
            ;

        PaddleMaterial->FormPairWith(*GoalAreaMaterial).SetCollidable(false);
        PaddleMaterial->FormPairWith(*ArenaMaterial).SetCollidable(false)
            /*.SetElasticity(0.f)
            .SetSoftness(0.f)*/
            ;
        PaddleMaterial->FormPairWith(*ArenaBottomMaterial).SetCollidable(false)
            /*.SetSoftness(0.f)
            .SetFriction(0.f, 0.f)
            .SetElasticity(0.f)*/
            ;
        PaddleMaterial->FormPairWith(*PaddleMaterial).SetCollidable(false);
        ArenaMaterial->FormPairWith(*GoalAreaMaterial).SetCollidable(false);
        ArenaMaterial->FormPairWith(*BallMaterial)
            /*.SetFriction(0.f, 0.f)
            .SetSoftness(1.f)
            .SetElasticity(1.f)*/
            ;
        ArenaBottomMaterial->FormPairWith(*BallMaterial)
            /*.SetElasticity(0.f)
            .SetFriction(0.f, 0.f)
            .SetSoftness(0.f)*/
            ;
        ArenaBottomMaterial->FormPairWith(*GoalAreaMaterial).SetCollidable(false);

        // Add the materials //
        auto tmp = std::make_unique<Leviathan::PhysicsMaterialManager>();

        tmp->LoadedMaterialAdd(std::move(PaddleMaterial));
        tmp->LoadedMaterialAdd(std::move(ArenaMaterial));
        tmp->LoadedMaterialAdd(std::move(BallMaterial));
        tmp->LoadedMaterialAdd(std::move(GoalAreaMaterial));
        tmp->LoadedMaterialAdd(std::move(ArenaBottomMaterial));

        // Setup world //
        WorldOfPong = std::dynamic_pointer_cast<Leviathan::StandardWorld>(
            Engine::GetEngine()->CreateWorld(Engine::Get()->GetWindowEntity(), 0,
                std::move(tmp), Leviathan::WorldNetworkSettings::GetSettingsForHybrid()));

        // create playing field manager with the world //
        GameArena = unique_ptr<Arena>(new Arena(WorldOfPong.get()));

        DoSpecialPostLoad();

        // Register the variables //
        auto& syncvars = *_NetworkInterface->GetOwner()->GetSyncedVariables();
        ScoreLimit.StartSync(syncvars);
        GamePaused.StartSync(syncvars);
        _PlayerList.StartSync(syncvars);
        LastPlayerHitBallID.StartSync(syncvars);
    }

    virtual void EnginePreShutdown() override
    {
        // Only the AI needs this //
        if(GameAI)
            GameAI->ReleaseScript();

        // Release object pointers //
        _PlayerList.Release();

        // Destroy the world //
        Engine::Get()->DestroyWorld(WorldOfPong);
        WorldOfPong.reset();

        CustomEnginePreShutdown();

        Logger::Get()->Info("Pong PreShutdown complete");
    }

    //! Override this
    virtual void Tick(int mspassed) override
    {

        DEBUG_BREAK;
    }

    // customized callbacks //
    virtual bool InitLoadCustomScriptTypes(asIScriptEngine* engine) override
    {

        // register PongGame type //
        if(engine->RegisterObjectType("PongBase", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        // get function //
        if(engine->RegisterGlobalFunction(
               "PongBase@ GetPongBase()", WRAP_FN(&BasePongParts::Get), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        // functions //
        if(engine->RegisterObjectMethod("PongBase", "void Quit()",
               WRAP_MFN(BasePongParts, ScriptCloseGame), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterObjectMethod("PongBase", "string GetErrorString()",
               WRAP_MFN(BasePongParts, GetErrorString), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterObjectMethod("PongBase", "int GetLastHitPlayer()",
               WRAP_MFN(BasePongParts, GetLastHitPlayer), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        // For getting the game database //
        if(engine->RegisterObjectMethod("PongBase", "GameWorld& GetGameWorld()",
               WRAP_MFN(BasePongParts, GetGameWorld), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PongBase", "ObjectID GetBall()",
               WRAP_MFN(BasePongParts, GetBall), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterObjectMethod("PongBase", "BaseNotifiableAll& GetPlayerChanges()",
               asMETHOD(BasePongParts, GetPlayersAsNotifiable), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }



        // Type enums //
        if(engine->RegisterEnum("PLAYERTYPE") < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_CLOSED", PLAYERTYPE_CLOSED) <
            0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERTYPE", "PLAYERTYPE_COMPUTER", PLAYERTYPE_COMPUTER) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_EMPTY", PLAYERTYPE_EMPTY) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue("PLAYERTYPE", "PLAYERTYPE_HUMAN", PLAYERTYPE_HUMAN) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterEnum("PLAYERCONTROLS") < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_NONE", PLAYERCONTROLS_NONE) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_AI", PLAYERCONTROLS_AI) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_WASD", PLAYERCONTROLS_WASD) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_ARROWS", PLAYERCONTROLS_ARROWS) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_IJKL", PLAYERCONTROLS_IJKL) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_NUMPAD", PLAYERCONTROLS_NUMPAD) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "PLAYERCONTROLS", "PLAYERCONTROLS_CONTROLLER", PLAYERCONTROLS_CONTROLLER) < 0) {
            SCRIPT_REGISTERFAIL;
        }


        if(engine->RegisterEnum("CONTROLKEYACTION") < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "CONTROLKEYACTION", "CONTROLKEYACTION_LEFT", CONTROLKEYACTION_LEFT) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue(
               "CONTROLKEYACTION", "CONTROLKEYACTION_RIGHT", CONTROLKEYACTION_RIGHT) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue("CONTROLKEYACTION", "CONTROLKEYACTION_POWERUPDOWN",
               CONTROLKEYACTION_POWERUPDOWN) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterEnumValue("CONTROLKEYACTION", "CONTROLKEYACTION_POWERUPUP",
               CONTROLKEYACTION_POWERUPUP) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        // PlayerSlot //
        if(engine->RegisterObjectType("PlayerSlot", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        // get function //
        if(engine->RegisterObjectMethod("PongBase", "PlayerSlot@ GetSlot(int number)",
               WRAP_MFN(BasePongParts, GetPlayerSlot), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        // functions //
        if(engine->RegisterObjectMethod("PlayerSlot", "bool IsActive()",
               WRAP_MFN(PlayerSlot, IsSlotActive), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterObjectMethod("PlayerSlot", "PLAYERTYPE GetPlayerType()",
               asMETHOD(PlayerSlot, GetPlayerType), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }


        if(engine->RegisterObjectMethod("PlayerSlot", "int GetPlayerNumber()",
               WRAP_MFN(PlayerSlot, GetPlayerNumber), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterObjectMethod("PlayerSlot", "int GetScore()",
               WRAP_MFN(PlayerSlot, GetScore), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        if(engine->RegisterObjectMethod("PlayerSlot", "PlayerSlot@ GetSplit()",
               WRAP_MFN(PlayerSlot, GetSplit), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "PLAYERCONTROLS GetControlType()",
               WRAP_MFN(PlayerSlot, GetControlType), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "void AddEmptySubSlot()",
               WRAP_MFN(PlayerSlot, AddEmptySubSlot), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot",
               "void SetControls(PLAYERCONTROLS type, int identifier)",
               WRAP_MFN(PlayerSlot, SetControls), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot",
               "void PassInputAction(CONTROLKEYACTION actiontoperform, bool active)",
               WRAP_MFN(PlayerSlot, PassInputAction), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "bool IsVerticalSlot()",
               WRAP_MFN(PlayerSlot, IsVerticalSlot), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "float GetTrackProgress()",
               WRAP_MFN(PlayerSlot, GetTrackProgress), asCALL_GENERIC) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "ObjectID GetPaddle()",
               asMETHOD(PlayerSlot, GetPaddle), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "ObjectID GetGoalArea()",
               asMETHOD(PlayerSlot, GetGoalArea), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "ObjectID GetTrackController()",
               asMETHOD(PlayerSlot, GetTrackController), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot",
               "bool DoesPlayerNumberMatchThisOrParent(int number)",
               asMETHOD(PlayerSlot, DoesPlayerNumberMatchThisOrParent), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }
        if(engine->RegisterObjectMethod("PlayerSlot", "int GetPlayerID()",
               asMETHOD(PlayerSlot, GetPlayerID), asCALL_THISCALL) < 0) {
            SCRIPT_REGISTERFAIL;
        }

        return MoreCustomScriptTypes(engine);
    }

    // ------------------ Physics callbacks for game logic ------------------ //
    // Ball handling callback //

    // virtual PhysicsMaterialContactCallback GetBallPaddleCallback() = 0;

    // virtual PhysicsMaterialContactCallback GetBallGoalAreaCallback() = 0;

protected:
    virtual bool MoreCustomScriptTypes(asIScriptEngine* engine) = 0;

    // ------------------------------------ //
};

} // namespace Pong
