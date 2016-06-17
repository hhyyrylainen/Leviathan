#pragma once
#ifndef LEVIATHAN_NETWORKEDINPUT
#define LEVIATHAN_NETWORKEDINPUT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input/InputController.h"
#include "SFML/Network/Packet.hpp"



namespace Leviathan{

	//! \brief Defines the state of an input object
	enum NETWORKEDINPUT_STATE{
		//! This is the initial state
		NETWOKREDINPUT_STATE_READY,
		//! The state while waiting for server response
		NETWORKEDINPUT_STATE_WAITING,
		//! The state when successfully connected
		NETWORKEDINPUT_STATE_CONNECTED,
		//! The state when something has failed
		NETWORKEDINPUT_STATE_FAILED,
		//! The state when the server has closed the connection
		NETWORKEDINPUT_STATE_CLOSED,
        //! This is set in the destructor and can be used to determine if NetworkInputFactory should ignore it
        NETWORKEDINPUT_STATE_DESTRUCTED
	};





	//! \brief Represents a single input source (group of keys) that are an input method that is to be synchronized
    //! between the server and the clients
	//! \note This class inherits from InputReceiver and you will need to implement it's input processing methods
	class NetworkedInput : public InputReceiver{
		friend NetworkedInputHandler;
	public:

		//! \brief Base class constructor for NetworkedInputs
		//! \param ownerid The ID of the owning player object
		//! \param networkid The ID retrieved from the server's NetworkedInputHandler
		DLLEXPORT NetworkedInput(int ownerid, int networkid);

		//! \brief Initializes data with information from create request
		DLLEXPORT NetworkedInput(sf::Packet &packet);

		DLLEXPORT virtual ~NetworkedInput();




		DLLEXPORT virtual void AddFullDataToPacket(sf::Packet &packet);
		

		DLLEXPORT virtual void AddChangesToPacket(sf::Packet &packet);


		//! \brief Used to set the initial data after receiving create notification
		DLLEXPORT virtual void LoadDataFromFullPacket(sf::Packet &packet);


		DLLEXPORT virtual void LoadUpdatesFromPacket(sf::Packet &packet);


		//! \brief Call when the input states have been updated
		DLLEXPORT virtual void OnUpdateInputStates();


		//! \brief Call to establish a connection with the server
		DLLEXPORT virtual bool ConnectToServersideInput();

		//! \brief Called before destruction
		//!
		//! This should send proper destroyed messages to others
        //! (server broadcast to all, and original creator to server)
		DLLEXPORT virtual void TerminateConnection();

		//! \brief Called when this is connected to a networked input handler
		DLLEXPORT virtual void NowOwnedBy(NetworkedInputHandler* owner);


		//! \brief Called when the handler or the network is no longer available and all connections should be stopped
		DLLEXPORT virtual void OnParentNoLongerAvailable();


		//! \brief Returns the state of this object
		DLLEXPORT NETWORKEDINPUT_STATE GetState() const;

		//! \brief Sets state for an instance created from a packet
		DLLEXPORT void SetNetworkReceivedState();


		DLLEXPORT static void LoadHeaderDataFromPacket(sf::Packet &packet, int &ownerid, int &inputid);


		//! \brief Called when this is created locally, should set the initial data
		DLLEXPORT virtual void InitializeLocal() = 0;

        //! \brief Called on the server to make this behave like the server's input should
        DLLEXPORT void SetAsServerSize();


		DLLEXPORT int GetID() const;

	protected:

		//! \brief Called before this is destroyed by NetworkedInputHandler
		//!
		//! This is usually a result of the server not allowing ConnectToServersideInput or the server
        //! removing our access at a later time
		DLLEXPORT virtual void _OnRemovedConnection();


		//! \brief Called when the local input is changed OR the server has relayed us an update packet
		//! \note Do not call OnUpdateInputStates from here, rather this should be called by that function
		virtual void _OnInputChanged() = 0;


		//! \brief Called when custom data needs to be un-serialized
		virtual void OnLoadCustomFullDataFrompacket(sf::Packet &packet) = 0;

		//! \brief Called when custom data needs to be un-serialized
		virtual void OnLoadCustomUpdateDataFrompacket(sf::Packet &packet) = 0;

		//! \brief Called when custom data needs to be packed into a packet
		virtual void OnAddFullCustomDataToPacket(sf::Packet &packet) = 0;


		//! \brief Called when custom data needs to be packed into a packet
		virtual void OnAddUpdateCustomDataToPacket(sf::Packet &packet) = 0;

		// ------------------------------------ //

		//! The owner ID for this object, this is usually a ConnectedPlayer ID
		int OwnerID;



		//! The ID for this object, this is only used in some checks to make sure that right input is going
        //! to the right place
		//! \todo The server needs to be asked about this ID
		int InputID;

        //! True when this is local. On the client
        //! Set to false in SetAsServerSize
        bool IsLocal;

		//! A pointer to the owning handler instance
		NetworkedInputHandler* OwningHandler = nullptr;


		//! The current state of the object
		NETWORKEDINPUT_STATE CurrentState;
	};

}
#endif
