#ifndef PONG_INPUTCONTROLLER
#define PONG_INPUTCONTROLLER
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
// ---- includes ---- //
#include "PlayerSlot.h"
#include "Networking/NetworkedInputHandler.h"
#include "Networking/NetworkedInput.h"
#include "Common/ThreadSafe.h"
#include <memory>


namespace Pong{
	
    using namespace std;

	//! \brief The factory that handles creating the things
	class PongInputFactory : public Leviathan::NetworkInputFactory{
	public:


		//! \brief Returns the static instance
		static PongInputFactory* Get();

		DLLEXPORT virtual unique_ptr<NetworkedInput> CreateNewInstanceForLocalStart(int inputid, bool isclient);

		DLLEXPORT virtual unique_ptr<NetworkedInput> CreateNewInstanceForReplication(int inputid, int ownerid);

		DLLEXPORT virtual void NoLongerNeeded(NetworkedInput &todiscard);

		DLLEXPORT virtual bool DoesServerAllowCreate(NetworkedInput* input, ConnectionInfo* connection);

		DLLEXPORT virtual void ReplicationFinalized(NetworkedInput* input);
		
		DLLEXPORT virtual bool IsConnectionAllowedToUpdate(NetworkedInput* input, ConnectionInfo* connection);

	protected:

		//! \brief Returns true when the connection matches the player who owns the input with the id
		bool IsConnectionAllowed(NetworkedInput* input, ConnectionInfo* connection);


		static PongInputFactory* Staticinstance;
	};

	//! \brief The bit flags for encoding input status
	enum PONG_INPUT_FLAGS {PONG_INPUT_FLAGS_LEFT = 0x1, 
		PONG_INPUT_FLAGS_RIGHT = 0x2, 
		PONG_INPUT_FLAGS_POWERUP = 0x4, 
		PONG_INPUT_FLAGS_POWERDOWN = 0x8
	};

	//! \brief A single input receiver that handles a group of keys
	class PongNInputter : public Leviathan::NetworkedInput, public ThreadSafe{
		friend PongInputFactory;
	public:

		PongNInputter(int ownerid, int networkid, PlayerSlot* controlthis, PLAYERCONTROLS typetoreceive);
		~PongNInputter();


		virtual bool ReceiveInput(OIS::KeyCode key, int modifiers, bool down);

		virtual void ReceiveBlockedInput(OIS::KeyCode key, int modifiers, bool down);

		virtual bool OnMouseMove(int xmove, int ymove);

		//! \brief Called by a PlayerSlot to prevent us using an invalid pointer
		void StopSendingInput(PlayerSlot* tohere);

		//! \brief Adds a PlayerSlot later
		void StartSendingInput(PlayerSlot* target);

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


		bool _HandleKeyThing(OIS::KeyCode key, bool down);

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




	class GameInputController : public Leviathan::NetworkedInputHandler{
	public:
		GameInputController();
		~GameInputController();



		std::map<OIS::KeyCode, CONTROLKEYACTION>& MapControlsToKeyGrouping(
            PLAYERCONTROLS controls);
		

		static GameInputController* Get();

	protected:
		void _SetupControlGroups();

		// ------------------------------------ //


		//! Translates a control group to a map of keys that correspond to that group
		std::map<PLAYERCONTROLS, std::map<OIS::KeyCode, CONTROLKEYACTION>> GroupToKeyMap;


		// To avoid having to pass an extra parameter to PongNInputter //
		static GameInputController* Staticistance;
	};

}
#endif
