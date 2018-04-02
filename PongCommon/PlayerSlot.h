#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
#include "Entities/Components.h"
#include "Networking/SyncedResource.h"
#include "Common/ThreadSafe.h"
#include <functional>


#define INPUTFORCE_APPLYSCALE		20.f
#define INPUT_TRACK_ADVANCESPEED	2.0f

namespace Pong{

    using namespace Leviathan;

	enum PLAYERTYPE {
        PLAYERTYPE_HUMAN = 0, PLAYERTYPE_COMPUTER, PLAYERTYPE_EMPTY, PLAYERTYPE_CLOSED
    };
    
	enum PLAYERCONTROLS
    {
        PLAYERCONTROLS_NONE = 0, PLAYERCONTROLS_AI, PLAYERCONTROLS_WASD, PLAYERCONTROLS_ARROWS, PLAYERCONTROLS_IJKL,
        PLAYERCONTROLS_NUMPAD, PLAYERCONTROLS_CONTROLLER
    };
    
	enum CONTROLKEYACTION
    {
        CONTROLKEYACTION_LEFT = 1, CONTROLKEYACTION_RIGHT, CONTROLKEYACTION_POWERUPDOWN, CONTROLKEYACTION_POWERUPUP
    };

	class PlayerList;
	class PongNInputter;

	//! \todo implement powerups
	class PlayerSlot : public Leviathan::ThreadSafe{
		friend PlayerList;
	public:
		// creates an empty slot //
		PlayerSlot(int slotnumber, PlayerList* owner);
		~PlayerSlot();


		//! Call to set the startup parameters
		//!
		//! Doesn't actually initialize anything
		void Init(PLAYERTYPE type = PLAYERTYPE_EMPTY, int PlayerNumber = 0, PLAYERCONTROLS controltype =
            PLAYERCONTROLS_NONE, int ctrlidentifier = 0, int playercontrollerid = -1,
            const Float4 &playercolour = Float4::GetColourWhite());

		//! Serializes this object to a packet
		void AddDataToPacket(sf::Packet &packet);

		//! Updates this object's data from a packet
		void UpdateDataFromPacket(sf::Packet &packet, Lock &listlock);

		void SetPlayer(PLAYERTYPE type, int identifier);
		PLAYERTYPE GetPlayerType();
		int GetPlayerNumber();

		int GetPlayerControllerID();

		void SetControls(PLAYERCONTROLS type, int identifier);
		PLAYERCONTROLS GetControlType();
		int GetControlIdentifier();

		int GetSlotNumber();
		int GetScore();
		inline void SetScore(int score){
			Score = score;
		}
		bool IsVerticalSlot();

		void AddEmptySubSlot();

		bool DoesPlayerNumberMatchThisOrParent(int number);

		float GetTrackProgress();

		// active should be if the key is down (or false if it is up) //
		void PassInputAction(CONTROLKEYACTION actiontoperform, bool active);
		// resets all input state //
		void InputDisabled();

		ObjectID GetPaddle(){
			return PaddleObject;
		}
		// Warning increases reference count //
		ObjectID GetTrackController(){
	
			return TrackObject;
		}
        
		void SetPaddleObject(ObjectID obj){
			PaddleObject = obj;
		}
        
		ObjectID GetGoalArea(){
			return GoalAreaObject;
		}

		void SetGoalAreaObject(ObjectID obj){
			GoalAreaObject = obj;
		}
        
		void SetTrackObject(ObjectID obj){

			TrackObject = obj;
		}

		inline Float4 GetColour(){
			return Colour;
		}
		void SetColour(const Float4 &colour){
			Colour = colour;
		}

		//! This should only be called on the server
		void SetNetworkedInputID(int id){
			NetworkedInputID = id;
		}

		int GetNetworkedInputID() const{

			return NetworkedInputID;
		}

		//! Returns true if player type isn't empty or closed
		inline bool IsSlotActive(){

			return PlayerType != PLAYERTYPE_CLOSED && PlayerType != PLAYERTYPE_EMPTY;
		}

		// Recursively looks through children and counts maximum depth (usually 0 or 1) //
		int GetSplitCount();

		PlayerSlot* GetSplit(){
			return SplitSlot;
		}


		inline int GetPlayerID() const{

			return PlayerID;
		}


		void SlotJoinPlayer(Leviathan::ConnectedPlayer* ply, int uniqnumber);

        void AddServerAI(int uniquenumber, int aitype = 2);


		void SlotLeavePlayer();


		Leviathan::ConnectedPlayer* GetConnectedPlayer(){

			return SlotsPlayer;
		}


        void WriteInfoToLog(int depth = 0) const;


		//! \brief Sets the input object that sends input here
		//!
		//! This is used to notify it that we are no longer available
		//! \param input The input object or NULL if the value needs to be reset
		//! \param oldcheck If not NULL will only clear if the current one matches, useful to stop accidentally clearing new inputs
		void SetInputThatSendsControls(Lock &guard, PongNInputter* input);

        //! \brief Called when input object is destroyed externally
        //!
        //! Called when NetworkedInputHandler is released while there are inputs attached
        void InputDeleted(Lock &guard);

        PongNInputter* GetInputObj() const{

            return InputObj;
        }

        REFERENCE_HANDLE_UNCOUNTED_TYPE(PlayerSlot);

	private:

		//! \brief Updates other client's objects, should be called when this is updated
		void _NotifyListOfUpdate();

		//! \brief Resets the active network input
		void _ResetNetworkInput(Lock &guard);

        inline void _ResetNetworkInput(){

            GUARD_LOCK();
            _ResetNetworkInput(guard);
        }

		// ------------------------------------ //

		// data //
		int Slot;
		PLAYERTYPE PlayerType;
        //! Unique number that is set for all players and AI, used to link various objects to the slot
		int PlayerNumber;
		PLAYERCONTROLS ControlType;
		int ControlIdentifier;

		//! The control object identifier
		//! \note This is used to detect which client should control which paddle
		int PlayerControllerID;

		Float4 Colour;

		//! This is the player's points
		//! \todo It would be more efficient to have a global points manager than resend all data
        //! when the slot is updated
		int Score;

		int MoveState;

		ObjectID PaddleObject;
		ObjectID GoalAreaObject;

		ObjectID TrackObject;

		//! Slot splitting
		PlayerSlot* SplitSlot;
		//! For quick lookups
		PlayerSlot* ParentSlot;


		//! The controller attached to this slot, this will be not-NULL when a human is controlling this player
		PongNInputter* InputObj;


		//! For access to all player specific data, if a player is in this slot, this is only available on the server
		Leviathan::ConnectedPlayer* SlotsPlayer;

		//! The ID of the player which can be used for lookups, exists on all clients and the server
		int PlayerID;


		//! The control ID of the player's input this exists on the server and on ALL of the clients.
		//! This is used to hook right input objects to right world objects
		int NetworkedInputID;


		//! Notifying our parent of updates
		PlayerList* Parent;
	};


	//! \brief Holds all the games' PlayerSlots and manages syncing them
	//! \todo Improve performance somehow
	//! \todo It should be possible to only update the slot that was updated
	class PlayerList : public Leviathan::SyncedResource{
	public:

		PlayerList(std::function<void (PlayerList*)> callback, size_t playercount = 4);
		~PlayerList();


		PlayerSlot* operator [](size_t index){
			return GamePlayers[index];
		}

		std::vector<PlayerSlot*>& GetVec(){
			return GamePlayers;
		}


		size_t Size() const{
			return GamePlayers.size();
		}

        void Release(){

            SAFE_DELETE_VECTOR(GamePlayers);
        }

        //! \brief Writes player information to log
        void ReportPlayerInfoToLog() const;

        void UpdateCustomDataFromPacket(Lock &guard, sf::Packet &packet) override;

		void SerializeCustomDataToPacket(Lock &guard, sf::Packet &packet) override;

		void OnValueUpdated(Lock &guard) override;

		//! \exception Leviathan::ExceptionInvalidArgument when out of range
		PlayerSlot* GetSlot(size_t index);


	protected:


		//! The main list of our players
		std::vector<PlayerSlot*> GamePlayers;

		//! The function called when this is updated either through the network or locally
		std::function<void (PlayerList*)> CallbackFunc;
	};



}

