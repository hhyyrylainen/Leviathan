#ifndef LEVIATHAN_SYNCEDVARIABLES
#define LEVIATHAN_SYNCEDVARIABLES
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{


	//! \brief Class that encapsulates a value that can be used for syncing
	//! \see SyncedVariables
	class SyncedValue{
		friend SyncedVariables;
	public:
		//! \brief Creates a new variable that can be synced with others
		//! \param newddata NamedVariableList allocated with the new operator and filled with the initial wanted data
		//! \param passtoclients Controls if this is actually sent, can be used to create hidden values
		//! \param allowevents If true send GenericEvent when value is updated
		DLLEXPORT SyncedValue(NamedVariableList* newddata, bool passtoclients = true, bool allowevents = true);
		DLLEXPORT ~SyncedValue();


		

		//! \brief Call after changing the HeldVariables
		DLLEXPORT void NotifyUpdated();

		//! \brief Gets a direct access to the variables
		//! \warning You will need to manually call NotifyUpdated to not break things
		DLLEXPORT NamedVariableList* GetVariableAccess() const;

	protected:

		//! \brief Sets the owner, which is very important
		//! \return Returns false when the name is invalid
		bool _MasterYouCalled(SyncedVariables* owner);


		// ------------------------------------ //

		//! Controls whether this variable is actually sent to clients
		//! \note This can be used to temporarily disable sending a variable to all clients
		bool PassToClients;

		//! If set to true sends generic update events
		bool AllowSendEvents;

		//! Holds the actual variables
		NamedVariableList* HeldVariables;

		SyncedVariables* Owner;
	};


	//! \brief Class that synchronizes some key variables with another instance
	//!
	//! By default this doesn't synchronize anything. You will have to manually add variables
	class SyncedVariables{
	public:
		//! \brief Construct an instance that must be owned by a NetworkHandler
		DLLEXPORT SyncedVariables(NetworkHandler* owner, bool amiaserver, NetworkInterface* handlinginterface);
		//! \brief The destructor doesn't force other instances to remove their variables
		DLLEXPORT ~SyncedVariables();


		//! \brief Adds a new variable to be synced
		//!
		//! The variable is automatically then broadcasted to all the connected instances (or the server if this instance is a client)
		//! \note (Verify this, might be false) The variable needs to be added on both the client and the server for this to work. 
		//! Not doing this may cause the client to get kicked
		//! \return The value is true if it was added
		//! \see SyncedValue
		DLLEXPORT bool AddNewVariable(shared_ptr<SyncedValue> newvalue);


		//! \brief Handles all requests aimed at the synchronized variables
		DLLEXPORT bool HandleSyncRequests(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);

		//! \brief Handles a response only packet, if it is a sync packet
		//! \note This will most likely only receive variable updated notifications
		DLLEXPORT bool HandleResponseOnlySync(shared_ptr<NetworkResponse> response, ConnectionInfo* connection);


		//! \brief Adds another instance to sync with
		DLLEXPORT void AddAnotherToSyncWith(ConnectionInfo* unsafeptr);

		DLLEXPORT void RemoveConnectionWithAnother(ConnectionInfo* unsafeptr);

		//! \brief Provided for NetworkServerInterface and NetworkClientInterface to access AddAnotherToSyncWith and other functions
		DLLEXPORT static SyncedVariables* Get();

	protected:


		// ------------------------------------ //

		//! Interface used to ask for permission to do many things (like add new client when they request it)
		NetworkInterface* CorrespondingInterface;

		//! NetworkHandler that owns this and is used to verify ConnectionInfo unsafe pointers when they actually need to be used
		NetworkHandler* Owner;

		//! Should be set to true when a server
		//! \note This is used to control if multiple other instances are allowed and if changed values are sent
		bool IsHost;

		//! Contains the values that are to be synced
		std::vector<shared_ptr<SyncedValue>> ToSyncValues;

		//! Keeps tract of other instances that are allowed to request from us
		//!
		//! New ones are added with the AddAnotherToSyncWith function or after querying the interface automatically
		std::vector<ConnectionInfo*> ConnectedToOthers;


		static SyncedVariables* Staticaccess;
	};

}
#endif