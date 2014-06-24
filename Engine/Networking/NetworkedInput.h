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


	//! \brief Represents a single input source (group of keys) that are an input method that is to be synchronized between the server and the clients
	//! \note This class inherits from InputReceiver and you will need to implement it's input processing methods
	class NetworkedInput : public InputReceiver{
	public:
		DLLEXPORT NetworkedInput(int ownerid);
		DLLEXPORT virtual ~NetworkedInput();




		DLLEXPORT virtual void AddFullDataToPacket(sf::Packet &packet);


		DLLEXPORT virtual void AddChangesToPacket(sf::Packet &packet);



		DLLEXPORT virtual void LoadDataFromFullPacket(sf::Packet &packet);


		DLLEXPORT virtual void LoadUpdatesFromPacket(sf::Packet &packet);


		//! \brief Call when the input states have been updated
		DLLEXPORT virtual void OnUpdateInputStates();


		//! \brief Call to establish a connection with the server
		DLLEXPORT virtual bool ConnectToServersideInput();


	protected:

		//! \brief Called before this is destroyed by NetworkedInputHandler
		//!
		//! This is usually a result of the server not allowing ConnectToServersideInput or the server removing our access at a later time
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



		//! The ID for this object, this is only used in some checks to make sure that right input is going to the right place
		//! \todo The server needs to be asked about this ID
		int InputID;

	};

}
#endif