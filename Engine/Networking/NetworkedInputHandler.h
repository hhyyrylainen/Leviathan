#pragma once
#ifndef LEVIATHAN_NETWORKEDINPUTHANDLER
#define LEVIATHAN_NETWORKEDINPUTHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input/InputController.h"
#include "boost/function.hpp"
#include "Common/ThreadSafe.h"



namespace Leviathan{

	//! \brief A factory provider required for other instances to be able to replicate other's input objects
	class NetworkInputFactory{
	public:

		//! \brief Won't actually be called by the engine but this should be used locally
        //! to create new instances
		std::unique_ptr<NetworkedInput> CreateNewInstanceForLocalStart(
            int inputid, bool isclient);


		//! \brief Called when a new input needs to be created through a networked packet
		//! \note The returned object will get custom data from the packet after this call
        //! so it isn't necessary to fill in all the data
		DLLEXPORT virtual std::unique_ptr<NetworkedInput> CreateNewInstanceForReplication(
            int inputid, int ownerid) = 0;


		//! \brief Called when a input is finally accepted and the construction should be finished
		//! \note This is after it has been filled with data from the packet so you should only link this to
        //! other objects here
		//! \see CreateNewInstanceForReplication
		DLLEXPORT virtual void ReplicationFinalized(NetworkedInput* input) = 0;


		//! \brief Called when an input is no longer needed
		//!
		//! This can be used, for example, to remove hooks from physics objects or character controllers
        //! \note This should use dynamic cast to find cases where the object is already being destructed and
        //! is now only the base class and cannot be properly casted. The cast should only fail if
        //! NetworkedInput::GetState return NETWORKEDINPUT_STATE_DESTRUCTED
		DLLEXPORT virtual void NoLongerNeeded(NetworkedInput &todiscard, Lock &parentlock) = 0;


		//! \brief Called on the server to verify whether a new input object can be created
		DLLEXPORT virtual bool DoesServerAllowCreate(NetworkedInput* input, ConnectionInfo* connection) = 0;

		//! \brief Called when the handler needs to know if a connection belongs to a player/input owner
		DLLEXPORT virtual bool IsConnectionAllowedToUpdate(NetworkedInput* input, ConnectionInfo* connection) = 0;

	};




	//! \brief Main class for controlling input between the server and clients
	//! \warning This class won't work unless you replace the GraphicalInputEntity's input controller with this AND
    //! register it with either
	//! the NetworkClientInterface or the NetworkServerInterface depending on whether this is a server
	//! \note As a server usually doesn't have a window nor it's own input setting this to the GraphicalInputEntity
    //! can be skipped as such entity might not have even been created in non-GUI mode
    //!
    //! Only one NetworkInputHandler should be created
    //! \note The behaviour is changed by engine networking strategy, which may make some assumptions in documentation
    //! invalid or not applicable
    //! \see NETWORK_USE_SNAPSHOTS
	class NetworkedInputHandler : public InputController{
		friend NetworkedInput;
	public:

		//! \brief Creates a NetworkedInputHandler for client applications
		//! \param objectcreater The factory object used to instantiate objects, the pointer won't be released by this
		DLLEXPORT NetworkedInputHandler(NetworkInputFactory* objectcreater, NetworkClientInterface* isclient);

		//! \brief Creates a NetworkedInputHandler for server applications
		//! \param objectcreater The factory object used to instantiate objects, the pointer won't be released by this
		DLLEXPORT NetworkedInputHandler(NetworkInputFactory* objectcreater, NetworkServerInterface* isserver);

		DLLEXPORT virtual ~NetworkedInputHandler();


        //! \brief Destroyes all input objects
        DLLEXPORT void Release();


		//! \brief Handles an input update packet
		//!
		//! The packet might cause creation of additional objects, delete existing or just alter state of them
		//! \return Returns true when the packet is a networked input related packet even if it ISN'T handled
		DLLEXPORT virtual bool HandleInputPacket(std::shared_ptr<NetworkRequest> request,
            ConnectionInfo* connection);

		//! \brief Handles an input update packet
		//!
		//! The packet might cause creation of additional objects, delete existing or just alter state of them
		//! \return Returns true when the packet is a networked input related packet even if it ISN'T handled
		DLLEXPORT virtual bool HandleInputPacket(std::shared_ptr<NetworkResponse> response,
            ConnectionInfo* connection);


		//! \brief Sends update packets
		//!
		//! What this actually does is dependent on whether this is a server or a client.
        //! This should be called in the corresponding interface's
		//! update function
		DLLEXPORT virtual void UpdateInputStatus();


		//! \brief Fetches an unique input identifier number from the server
		//!
		//! Or if this is on the server just returns the next number.
		//! \warning This may take a while to resolve especially on slow connections or heavy server load.
        //! The client should disconnect if the failure callback is called
		DLLEXPORT virtual void GetNextInputIDNumber(boost::function<void (int)> onsuccess,
            boost::function<void ()> onfailure);

		//! \brief Returns the server's next input ID
		//! \warning This function ONLY works on the server
		DLLEXPORT virtual int GetNextInputIDNumberOnServer();


		//! \brief Creates a new input source for syncing
		//!
		//! This will add the object to the list of active input sources. The NetworkedInput::ConnectToServersideInput
        //! function will be automatically
		//! called and the object will be replicated on the server
		//! \warning This is only available on a client
		//! \note The object should be created with the passed factory's
        //! NetworkInputFactory::CreateNewInstanceForLocalStart method. OR if the user
		//! knows that the factory won't do anything too important you can create this object directly
		//! \param iobject This is an instance of a custom subclass of NetworkedInput that
        //! implements the required methods. Also the ID should be 
		//! authorized by the server, either by directly requesting one or receiving one from the server
		//! \return True when it is added. However it can later be discarded by the server not accepting us
        //! hooking the input to the global pool of input objects on the server
		DLLEXPORT virtual bool RegisterNewLocalGlobalReflectingInputSource(std::shared_ptr<NetworkedInput> iobject);

		//! \brief Queues a NetworkedInput instance to be destroyed
		DLLEXPORT void QueueDeleteInput(NetworkedInput* inputobj);


		// Input receiving parts that aren't needed as the default implementations work just fine //

        DLLEXPORT static NetworkedInputHandler* Get();
        
	protected:

		//! \brief Overloaded to allow us discard stuff from GlobalOrLocalListeners
		DLLEXPORT virtual void _OnChildUnlink(InputReceiver* child);



		void _HandleConnectRequestPacket(Lock &guard, std::shared_ptr<NetworkRequest> request,
            ConnectionInfo* connection);

		//! \brief Handle update response
		//! \return false If the connection isn't authorized to update the input
        //! from this connection
        //! \todo Client: check that the connection is the server connection
		bool _HandleInputUpdateResponse(Lock &guard, std::shared_ptr<NetworkResponse> response,
            ConnectionInfo* connection);

        //! \brief Handle input create responses
        //! \note Should only be called on the client
        //! \todo Fail if connection isn't the server we are connected ton
        bool _HandleInputCreateResponse(Lock &guard, std::shared_ptr<NetworkResponse> response,
            ConnectionInfo* connection);

        //! \brief Handles destroying input handlers when clients request
        bool _HandleDisconnectRequestPacket(Lock &guard,
            std::shared_ptr<NetworkResponse> response, ConnectionInfo* connection);

		//! \brief Clears out the DeleteQueue
		void _HandleDeleteQueue(Lock &guard);

		// ------------------------------------ //

		//! True if this is a server's object
		bool IsOnTheServer;

		// Server only part //

		//! The last used ID of an input source
		int LastInputSourceID;

		//! On the server this has the list of global input listeners, on a local 
		std::vector<std::shared_ptr<NetworkedInput>> GlobalOrLocalListeners;


		//! The factory used for stuff
		NetworkInputFactory* _NetworkInputFactory;

		//! Pointer to the network interface when on a client
		NetworkClientInterface* ClientInterface;

		//! Pointer to the network interface when on a server
		NetworkServerInterface* ServerInterface;

		//! Vector of listeners that will be deleted soon
		std::vector<std::shared_ptr<NetworkedInput>> DeleteQueue;

        static NetworkedInputHandler* Staticinstance;
	};

}
#endif
