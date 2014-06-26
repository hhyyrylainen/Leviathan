#ifndef PONG_PLAYERSLOT
#define PONG_PLAYERSLOT
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Objects/TrackEntityController.h"
#include "Networking/SyncedResource.h"
#include "Common/ThreadSafe.h"


#define INPUTFORCE_APPLYSCALE		20.f
#define INPUT_TRACK_ADVANCESPEED	2.0f

namespace Pong{

	enum PLAYERTYPE {PLAYERTYPE_HUMAN = 0, PLAYERTYPE_COMPUTER, PLAYERTYPE_EMPTY, PLAYERTYPE_CLOSED};
	enum PLAYERCONTROLS {PLAYERCONTROLS_NONE = 0, PLAYERCONTROLS_AI, PLAYERCONTROLS_WASD, PLAYERCONTROLS_ARROWS, PLAYERCONTROLS_IJKL,
		PLAYERCONTROLS_NUMPAD, PLAYERCONTROLS_CONTROLLER};
	enum CONTROLKEYACTION {CONTROLKEYACTION_LEFT, CONTROLKEYACTION_RIGHT, CONTROLKEYACTION_POWERUPDOWN, CONTROLKEYACTION_POWERUPUP};

	class PlayerList;

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
		void Init(PLAYERTYPE type = PLAYERTYPE_EMPTY, int PlayerNumber = 0, PLAYERCONTROLS controltype = PLAYERCONTROLS_NONE, 
			int ctrlidentifier = 0, int playercontrollerid = -1, const Float4 &playercolour = Float4::GetColourWhite());

		//! Serializes this object to a packet
		void AddDataToPacket(sf::Packet &packet);

		//! Updates this object's data from a packet
		void UpdateDataFromPacket(sf::Packet &packet);

		void SetPlayer(PLAYERTYPE type, int identifier);
		void SetPlayerProxy(PLAYERTYPE type);
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

		shared_ptr<Leviathan::BaseObject> GetPaddle(){
			return PaddleObject;
		}
		// Warning increases reference count //
		Leviathan::BaseObject* GetPaddleProxy(){
			PaddleObject->AddRef();
			return PaddleObject.get();
		}
		// Warning increases reference count //
		Leviathan::Entity::TrackEntityController* GetTrackController(){
			TrackDirectptr->AddRef();
			return TrackDirectptr;
		}
		void SetPaddleObject(shared_ptr<Leviathan::BaseObject> obj){
			PaddleObject = obj;
		}
		shared_ptr<Leviathan::BaseObject> GetGoalArea(){
			return GoalAreaObject;
		}
		// Warning increases reference count //
		Leviathan::BaseObject* GetGoalAreaProxy(){
			GoalAreaObject->AddRef();
			return GoalAreaObject.get();
		}
		void SetGoalAreaObject(shared_ptr<Leviathan::BaseObject> obj){
			GoalAreaObject = obj;
		}
		void SetTrackObject(shared_ptr<Leviathan::BaseObject> obj, Leviathan::Entity::TrackEntityController* direct){
			TrackObject = obj;
			TrackDirectptr = direct;
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


		//! returns true if player type isn't empty or closed
		inline bool IsSlotActive(){

			return PlayerType != PLAYERTYPE_CLOSED && PlayerType != PLAYERTYPE_EMPTY;
		}

		// recursively looks through children and counts maximum depth (usually 0 or 1) //
		int GetSplitCount();

		PlayerSlot* GetSplit(){
			return SplitSlot;
		}


		inline int GetPlayerID() const{

			return PlayerID;
		}


		void SlotJoinPlayer(Leviathan::ConnectedPlayer* ply);


		void SlotLeavePlayer();


		Leviathan::ConnectedPlayer* GetConnectedPlayer(){

			return SlotsPlayer;
		}



		static int CurrentPlayerNumber;

	private:

		//! \brief Updates other client's objects, should be called when this is updated
		void _NotifyListOfUpdate();

		// ------------------------------------ //

		// data //
		int Slot;
		PLAYERTYPE PlayerType;
		int PlayerNumber;
		PLAYERCONTROLS ControlType;
		int ControlIdentifier;

		//! The control object identifier
		//! \note This is used to detect which client should control which paddle
		int PlayerControllerID;

		Float4 Colour;

		//! This is the player's points
		//! \todo It would be more efficient to have a global points manager than resend all data when the slot is updated
		int Score;

		int MoveState;

		shared_ptr<Leviathan::BaseObject> PaddleObject;
		shared_ptr<Leviathan::BaseObject> GoalAreaObject;

		shared_ptr<Leviathan::BaseObject> TrackObject;
		Leviathan::Entity::TrackEntityController* TrackDirectptr;

		//! Slot splitting
		PlayerSlot* SplitSlot;
		//! For quick lookups
		PlayerSlot* ParentSlot;

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

		PlayerList(boost::function<void (PlayerList*)> callback, size_t playercount = 4);
		~PlayerList();


		PlayerSlot* operator [](size_t index) THROWS{
			return GamePlayers[index];
		}

		std::vector<PlayerSlot*>& GetVec(){
			return GamePlayers;
		}


		size_t Size() const{
			return GamePlayers.size();
		}

		virtual void UpdateCustomDataFromPacket(sf::Packet &packet) THROWS;

		virtual void SerializeCustomDataToPacket(sf::Packet &packet);

		virtual void OnValueUpdated();

		//! \exception Leviathan::ExceptionInvalidArgument when out of range
		PlayerSlot* GetSlot(size_t index) THROWS;


	protected:



		//! The main list of our players
		std::vector<PlayerSlot*> GamePlayers;

		//! The function called when this is updated either through the network or locally
		boost::function<void (PlayerList*)> CallbackFunc;
	};



}
#endif