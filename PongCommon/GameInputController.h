#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
// ---- includes ---- //
#include "PlayerSlot.h"
#include "Common/ThreadSafe.h"
#include "Input/InputController.h"
#include <memory>


namespace Pong{
    
    //! \brief The bit flags for encoding input status
    enum PONG_INPUT_FLAGS {PONG_INPUT_FLAGS_LEFT = 0x1, 
        PONG_INPUT_FLAGS_RIGHT = 0x2, 
        PONG_INPUT_FLAGS_POWERUP = 0x4, 
        PONG_INPUT_FLAGS_POWERDOWN = 0x8
    };

    //! \brief A single input receiver that handles a group of keys
    class PongNInputter : public Leviathan::InputReceiver, public ThreadSafe{
    public:

        PongNInputter(int ownerid, int networkid, PlayerSlot* controlthis, PLAYERCONTROLS typetoreceive);
        ~PongNInputter();


        bool ReceiveInput(int32_t key, int modifiers, bool down) override;

        void ReceiveBlockedInput(int32_t key, int modifiers, bool down) override;

        bool OnMouseMove(int xmove, int ymove) override;

        //! \brief Called by a PlayerSlot to prevent us using an invalid pointer
        void StopSendingInput(PlayerSlot* tohere);

        //! \brief Adds a PlayerSlot later
        void StartSendingInput(Lock &guard, PlayerSlot* target);

        //! \brief Update settings reflecting new options
        //! \todo Add control ID setting for controller support
        //! \note This is only available on the client that created this object
        void UpdateSettings(PLAYERCONTROLS newcontrols);

        // The default functions that need overloading //
        virtual void InitializeLocal();

        virtual void OnLoadCustomFullDataFrompacket(sf::Packet &packet);

        virtual void OnLoadCustomUpdateDataFrompacket(sf::Packet &packet);

        virtual void OnAddFullCustomDataToPacket(sf::Packet &packet);

        virtual void OnAddUpdateCustomDataToPacket(sf::Packet &packet);

    protected:



        virtual void _OnInputChanged();


        bool _HandleKeyThing(int32_t key, bool down);

        // ------------------------------------ //

        //! The slot that this controls, maybe NULL if the factory is doing something wonky
        PlayerSlot* ControlledSlot;

        //! Used for finding the right keys
        //! These actually don't have to be exactly synced between all clients, thus this is only sent in the full packet
        PLAYERCONTROLS CtrlGroup;


        //! The actual state keys
        //! \see PONG_INPUT_FLAGS
        char ControlStates;


        //! The changed keys
        char ChangedKeys;


        //! Blocks input handling on all but the client that created this
        bool CreatedByUs;

    };



    class GameInputController : public Leviathan::InputController{
    public:
        GameInputController();
        ~GameInputController();



        std::map<int32_t, CONTROLKEYACTION>& MapControlsToKeyGrouping(
            PLAYERCONTROLS controls);
        
    protected:
        void _SetupControlGroups();

        // ------------------------------------ //


        //! Translates a control group to a map of keys that correspond to that group
        std::map<PLAYERCONTROLS, std::map<int32_t, CONTROLKEYACTION>> GroupToKeyMap;
    };

}
