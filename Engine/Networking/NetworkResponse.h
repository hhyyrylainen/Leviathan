#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"


namespace Leviathan{

//! Defines the type of response that the packet contains
enum NETWORKRESPONSETYPE{
    //! Sent in response to a NETWORKREQUESTTYPE_IDENTIFICATION contains a user readable
    //! string, game name, game version and leviathan version strings
    NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS,
    NETWORKRESPONSETYPE_KEEPALIVE,
    NETWORKRESPONSETYPE_CLOSECONNECTION,
    NETWORKRESPONSETYPE_REMOTECONSOLECLOSED,
    NETWORKRESPONSETYPE_REMOTECONSOLEOPENED,
    NETWORKRESPONSETYPE_INVALIDREQUEST,
    //! Sent by a server when it disallows a made request
    NETWORKRESPONSETYPE_SERVERDISALLOW,
    //! Sent by a server when a request is allowed
    NETWORKRESPONSETYPE_SERVERALLOW,
    //! Returns anonymous data about the server
    NETWORKRESPONSETYPE_SERVERSTATUS,
    //! Sends a update/new SyncedValue
    NETWORKRESPONSETYPE_SYNCVALDATA,
    //! Send after all NETWORKRESPONSETYPE_SYNCVALDATA has been sent and indicates
    //! whether they should have arrived correctly
    NETWORKRESPONSETYPE_SYNCDATAEND,
    //! Contains SyncedResource update notification
    NETWORKRESPONSETYPE_SYNCRESOURCEDATA,
    //! Contains a new NetworkedInput
    NETWORKRESPONSETYPE_CREATENETWORKEDINPUT,
    //! Contains control state updates regarding a NetworkedInput
    NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT,

    //! Client sents this when they want input to be destroyed
    NETWORKRESPONSETYPE_DISCONNECTINPUT,

    //! Contains one or more full entities sent by the server
    NETWORKRESPONSETYPE_INITIAL_ENTITY,

    //! Contains update data for a single entity
    NETWORKRESPONSETYPE_ENTITY_UPDATE,

    //! Contains (list) an ID for entity to be deleted
    NETWORKRESPONSETYPE_ENTITY_DESTRUCTION,

    //! Contains an updated AI cache variable
    NETWORKRESPONSETYPE_AI_CACHE_UPDATED,

    //! Contains the name of a removed AI cache variable
    NETWORKRESPONSETYPE_AI_CACHE_REMOVED,

    //! Instructs a world to create or destroy a constraint
    NETWORKRESPONSETYPE_ENTITY_CONSTRAINT,

    //! Sent when the server changes physics frozen state
    NETWORKRESPONSETYPE_WORLD_FROZEN,

    //! A server heartbeat packet
    NETWORKRESPONSETYPE_SERVERHEARTBEAT,

    //! Marks that the client is required to send heartbeats
    NETWORKRESPONSETYPE_STARTHEARTBEATS,

    //! The packet is a game specific packet!
    //! \see GameSpecificPacketHandler BaseGameSpecificFactory BaseGameSpecificResponsePacket
    NETWORKRESPONSETYPE_CUSTOM,

    //! Empty response, used for keeping alive/nothing
    NETWORKRESPONSETYPE_NONE
};
	

//! Defines in what way a request was invalid also
//! defines why a server disallowed a request
enum NETWORKRESPONSE_INVALIDREASON{

    //! Returned when the connection is anonymous (the other client hasn't requested verified connection)
    NETWORKRESPONSE_INVALIDREASON_UNAUTHENTICATED,
    //! Returned when we don't implement the wanted action (for example if we are asked our server status
    //! and we aren't a server)
    NETWORKRESPONSE_INVALIDREASON_UNSUPPORTED,
    //! Server has maximum number of players
    NETWORKRESPONSE_INVALIDREASON_SERVERFULL,
    //! Server is not accepting players
    NETWORKRESPONSE_INVALIDREASON_SERVERNOTACCEPTINGPLAYERS,
    //! The client isn't properly authenticated for that action or the server received mismatching security/
    //! id numbers
    NETWORKRESPONSE_INVALIDREASON_NOT_AUTHORIZED,

    //! The client has already connected to the server, and must disconnect before trying again
    NETWORKRESPONSE_INVALIDREASON_SERVERALREADYCONNECTEDTOYOU,

    //! The server has used a custom rule to disallow this
    NETWORKRESPONSE_INVALIDREASON_SERVERCUSTOM
};

//! Defines what request the server accepted and any potential data
enum NETWORKRESPONSE_SERVERACCEPTED_TYPE{

    //! Server has accepted your join request
    NETWORKRESPONSE_SERVERACCEPTED_TYPE_CONNECT_ACCEPTED,
    //! Server has accepted the request and will handle it soon
    NETWORKRESPONSE_SERVERACCEPTED_TYPE_REQUEST_QUEUED
};

//! Defines server join protection status (who can join the server)
enum NETWORKRESPONSE_SERVERJOINRESTRICT{

};

//! Base class for all data objects that can be sent with the NETWORKRESPONSETYPE
//! \note Even though it cannot be required by the base class, sub classes should implement a constructor
//! taking in an sf::Packet object
class BaseNetworkResponseData{
public:

    DLLEXPORT virtual ~BaseNetworkResponseData(){};

    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) = 0;
};

//! Stores data for NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS
class NetworkResponseDataForIdentificationString : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForIdentificationString(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForIdentificationString(
        const std::string &userreadableidentification,
        const std::string &gamename, const std::string &gameversion,
        const std::string &leviathanversion);

    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);
		

    // Data //
    std::string UserReadableData;
    std::string GameName;
    std::string GameVersionString;
    std::string LeviathanVersionString;
};

//! Stores data for NETWORKRESPONSETYPE_INVALIDREQUEST
class NetworkResponseDataForInvalidRequest : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForInvalidRequest(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForInvalidRequest(NETWORKRESPONSE_INVALIDREASON reason,
        const std::string &additional
        = std::string());
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);


    NETWORKRESPONSE_INVALIDREASON Invalidness;
    std::string AdditionalInfo;
};

//! Stores data for NETWORKRESPONSETYPE_SERVERSTATUS
class NetworkResponseDataForServerStatus : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForServerStatus(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForServerStatus(const std::string &servername,
        bool isjoinable,
        NETWORKRESPONSE_SERVERJOINRESTRICT whocanjoin, int players, int maxplayers, int bots,
        NETWORKRESPONSE_SERVERSTATUS currentstatus, int serverflags);
        
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! Contains the name of the server, should be limited to max 100 letters
    std::string ServerNameString;
    //! States if the server is joinable (has started, doesn't take slots into account)
    bool Joinable;

    //! Defines the type of join authentication the server uses (restricts who can join)
    NETWORKRESPONSE_SERVERJOINRESTRICT JoinRestriction;

    //! Current human players on the server
    int Players;
    //! Maximum human players
    int MaxPlayers;

    //! Current bots on the server
    int Bots;

    //! The current status of the server. Used to define what the server is doing
    NETWORKRESPONSE_SERVERSTATUS ServerStatus;

    //! The flags of the server. These can be used based on the game for example to define
    //! game mode or level requirements or something else
    int AdditionalFlags;
};

    
//! \brief Stores data for NETWORKRESPONSETYPE_DISCONNECTINPUT,
class NetworkResponseDataForDisconnectInput : public BaseNetworkResponseData{
public:
    NetworkResponseDataForDisconnectInput(int inputid, int ownerid);

    NetworkResponseDataForDisconnectInput(sf::Packet &packet);

    DLLEXPORT void AddDataToPacket(sf::Packet &packet) override;

    //! The ID of the input that should be closed
    int InputID;

    //! ID of the player the input belongs to, provided as a sanity check
    int OwnerID;
};


//! \brief Stores data about a server disallow response
class NetworkResponseDataForServerDisallow : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForServerDisallow(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON reason,
        const std::string &message = "Default disallow");
        
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! \brief An user readable disallow string
    //! \note Should be limited to a maximum of 100 characters
    std::string Message;

    //! The reason why this request was dropped
    NETWORKRESPONSE_INVALIDREASON Reason;
};

//! \brief Stores data about a server allow response
class NetworkResponseDataForServerAllow : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForServerAllow(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForServerAllow(
        NETWORKRESPONSE_SERVERACCEPTED_TYPE whataccepted, const std::string &message = "");
        
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! What the server accepted
    NETWORKRESPONSE_SERVERACCEPTED_TYPE ServerAcceptedWhat;

    //! \brief An user readable disallow string
    //! \note Should be limited to a maximum of 100 characters
    std::string Message;
};

//! \brief Stores data about a synced variable
class NetworkResponseDataForSyncValData : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForSyncValData(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForSyncValData(NamedVariableList* newddata);
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! The variable that is passed/received
    std::shared_ptr<NamedVariableList> SyncValueData;
};


//! \brief Tells whether a variable sync all succeeded or failed
class NetworkResponseDataForSyncDataEnd : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForSyncDataEnd(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForSyncDataEnd(bool succeeded);
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);


    bool Succeeded;
};

//! \brief Contains custom data for SyncedResource to handle
class NetworkResponseDataForSyncResourceData : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForSyncResourceData(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForSyncResourceData(const std::string &containeddata);
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! The packet's binary data is stored as characters
    std::string OurCustomData;
};


//! \brief Used for BaseGameSpecificResponsePacket storing
class NetworkResponseDataForCustom : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForCustom(GameSpecificPacketData* newdpacketdata);
    DLLEXPORT NetworkResponseDataForCustom(BaseGameSpecificResponsePacket* newddata);

    DLLEXPORT NetworkResponseDataForCustom(sf::Packet &frompacket);
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);


    std::shared_ptr<GameSpecificPacketData> ActualPacketData;
};

//! \brief Used for storing data related to creating a NetworkedInput
class NetworkResponseDataForCreateNetworkedInput : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForCreateNetworkedInput(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForCreateNetworkedInput(NetworkedInput &tosend);
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! This contains the data required to create the object
    sf::Packet DataForObject;
};
	
//! \brief Used for storing data related to updating a NetworkedInput
class NetworkResponseDataForUpdateNetworkedInput : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForUpdateNetworkedInput(sf::Packet &frompacket);
    DLLEXPORT NetworkResponseDataForUpdateNetworkedInput(NetworkedInput &object);
    DLLEXPORT NetworkResponseDataForUpdateNetworkedInput(
        const NetworkResponseDataForUpdateNetworkedInput &other);
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

    //! The ID of the input used to match the player to the input
    int InputID;

    //! This contains the custom data which actually has the good stuff in it
    sf::Packet UpdateData;
};


//! \brief Used for storing data for number of whole entitites
class NetworkResponseDataForInitialEntity : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForInitialEntity(sf::Packet &frompacket);
        
    //! \brief Creates a response with a single entity
    DLLEXPORT NetworkResponseDataForInitialEntity(int worldid,
        std::unique_ptr<sf::Packet> &entity1data);
        
    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) override;

    //! \brief Puts the data of an object to a sf::Packet
    //! \note This will overwrite all the data in the packet
    //! \return True when the index is valid
    DLLEXPORT std::shared_ptr<sf::Packet> GetDataForEntity(size_t index) const;
        
    //! The ID of the world to which the entities belong
    int WorldID;

    //! The data for the entitites is here as binary data
    std::vector<std::shared_ptr<sf::Packet>> EntityData;
};

//! \brief Holds data regarding a constraint between two entities
class NetworkResponseDataForEntityConstraint : public BaseNetworkResponseData{
public:
        
    DLLEXPORT NetworkResponseDataForEntityConstraint(int worldid, int constraintid,
        ObjectID entity1, ObjectID entity2, bool create, ENTITY_CONSTRAINT_TYPE type,
        std::shared_ptr<sf::Packet> &data);

    DLLEXPORT NetworkResponseDataForEntityConstraint(int worldid, int constraintid,
        ObjectID entity1, ObjectID entity2, bool create, ENTITY_CONSTRAINT_TYPE type);

    DLLEXPORT NetworkResponseDataForEntityConstraint(sf::Packet &frompacket);

    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) override;
        
    //! When false the constraint is to be deleted
    bool Create;
        
    //! The ID of the world to which the entities belong
    int WorldID;

    //! The ID of the constraint
    int ConstraintID;

    //! The first entity
    ObjectID EntityID1;
        
    //! The second entity
    ObjectID EntityID2;
        
    ENTITY_CONSTRAINT_TYPE Type;

    //! Data for the constraint
    std::shared_ptr<sf::Packet> ConstraintData;
};

//! \brief Holds data for updating an entity
class NetworkResponseDataForEntityUpdate : public BaseNetworkResponseData{
public:
        
    DLLEXPORT NetworkResponseDataForEntityUpdate(int worldid, int entityid, int ticknumber,
        int referencetick, std::shared_ptr<sf::Packet> data);

    DLLEXPORT NetworkResponseDataForEntityUpdate(sf::Packet &frompacket);

    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) override;
        
    //! The ID of the world to which the entities belong
    int WorldID;

    //! The ID of the entity
    int EntityID;

    //! The tick on which this was generated
    int TickNumber;

    //! The tick number against which this update has been created
    //!
    //! Special case is -1 which notes that there is no reference tick
    int ReferenceTick;
        
    //! Data for updating the entity 
    std::shared_ptr<sf::Packet> UpdateData;
};

//! \brief Contains entities to destroy
class NetworkResponseDataForEntityDestruction : public BaseNetworkResponseData{
public:
    DLLEXPORT NetworkResponseDataForEntityDestruction(int worldid, int entityid);

    DLLEXPORT NetworkResponseDataForEntityDestruction(sf::Packet &frompacket);

    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) override;

        
    //! The ID of the world to which the entities belong
    int WorldID;

    //! The ID of the entity
    int EntityID;
};

    
//! \brief Holds world physics frozen state change information
class NetworkResponseDataForWorldFrozen : public BaseNetworkResponseData{
public:

    DLLEXPORT NetworkResponseDataForWorldFrozen(int worldid, bool frozen, int ontick);

    DLLEXPORT NetworkResponseDataForWorldFrozen(sf::Packet &frompacket);

    DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) override;

    //! World for which this packet applies
    int WorldID;

    //! Whether to freeze or unfreeze the world
    bool Frozen;

    //! The tick number at which this applies
    //! \todo Make the GameWorld
    int TickNumber;
        
};

//! \brief Holds data for upated AI cache variable
class NetworkResponseDataForAICacheUpdated : public BaseNetworkResponseData{
public:

    //! \exception ExceptionInvalidArgument When the variable is NULL
    DLLEXPORT NetworkResponseDataForAICacheUpdated(
        std::shared_ptr<NamedVariableList> variable);

    //! \exception ExceptionInvalidArgument When packet cannot be unserialized
    DLLEXPORT NetworkResponseDataForAICacheUpdated(sf::Packet &frompacket);

    DLLEXPORT void AddDataToPacket(sf::Packet &packet) override;

    //! The received variable
    std::shared_ptr<NamedVariableList> Variable;
};

//! \brief Holds data for upated AI cache variable
class NetworkResponseDataForAICacheRemoved : public BaseNetworkResponseData{
public:

    DLLEXPORT NetworkResponseDataForAICacheRemoved(const std::string &name);

    //! \exception ExceptionInvalidArgument When packet cannot be unserialized
    DLLEXPORT NetworkResponseDataForAICacheRemoved(sf::Packet &frompacket);

    DLLEXPORT void AddDataToPacket(sf::Packet &packet) override;

    //! The name of the removed variable
    std::string Name;
};


//! \brief Represents a response type packet sent through a ConnectionInfo
//! \todo Refactor all packets to check if the packet is valid after loading all data
class NetworkResponse{
public:
    DLLEXPORT NetworkResponse(int inresponseto, PACKET_TIMEOUT_STYLE timeout,
        int timeoutvalue);
        
    // This is for constructing these on the receiver side //
    DLLEXPORT NetworkResponse(sf::Packet &receivedresponse);
    DLLEXPORT ~NetworkResponse();

    // Named "constructors" for different types //
    // todo: these could be changed to take parameters for the object's constructors instead of pointers
    DLLEXPORT void GenerateIdentificationStringResponse(
        NetworkResponseDataForIdentificationString* newddata);
    DLLEXPORT void GenerateInvalidRequestResponse(
        NetworkResponseDataForInvalidRequest* newddata);
    DLLEXPORT void GenerateServerStatusResponse(
        NetworkResponseDataForServerStatus* newddata);
    DLLEXPORT void GenerateServerDisallowResponse(
        NetworkResponseDataForServerDisallow* newddata);
    DLLEXPORT void GenerateServerAllowResponse(
        NetworkResponseDataForServerAllow* newddata);
    DLLEXPORT void GenerateValueSyncResponse(
        NetworkResponseDataForSyncValData* newddata);
    DLLEXPORT void GenerateValueSyncEndResponse(
        NetworkResponseDataForSyncDataEnd* newddata);
    DLLEXPORT void GenerateResourceSyncResponse(const char* dataptr, size_t datasize);
    DLLEXPORT void GenerateCreateNetworkedInputResponse(
        NetworkResponseDataForCreateNetworkedInput* newddata);
    DLLEXPORT void GenerateUpdateNetworkedInputResponse(
        NetworkResponseDataForUpdateNetworkedInput* newddata);
    DLLEXPORT void GenerateInitialEntityResponse(
        NetworkResponseDataForInitialEntity* newddata);
    DLLEXPORT void GenerateEntityConstraintResponse(
        NetworkResponseDataForEntityConstraint* newddata);
    DLLEXPORT void GenerateWorldFrozenResponse(NetworkResponseDataForWorldFrozen* newddata);
    DLLEXPORT void GenerateEntityUpdateResponse(NetworkResponseDataForEntityUpdate* newddata);
    DLLEXPORT void GenerateEntityDestructionResponse(
        NetworkResponseDataForEntityDestruction* newddata);
    DLLEXPORT void GenerateAICacheUpdatedResponse(
        NetworkResponseDataForAICacheUpdated* newddata);
    DLLEXPORT void GenerateAICacheRemovedResponse(
        NetworkResponseDataForAICacheRemoved* newddata);
    DLLEXPORT void GenerateDisconnectInputResponse(
        NetworkResponseDataForDisconnectInput* newddata);

    DLLEXPORT void GenerateCustomResponse(GameSpecificPacketData* newdpacketdata);
    DLLEXPORT void GenerateCustomResponse(BaseGameSpecificResponsePacket* newdpacketdata);
		

    DLLEXPORT void GenerateKeepAliveResponse();
    DLLEXPORT void GenerateCloseConnectionResponse();
    DLLEXPORT void GenerateRemoteConsoleOpenedResponse();
    DLLEXPORT void GenerateRemoteConsoleClosedResponse();
    DLLEXPORT void GenerateHeartbeatResponse();
    DLLEXPORT void GenerateStartHeartbeatsResponse();


    DLLEXPORT void GenerateEmptyResponse();

    DLLEXPORT NETWORKRESPONSETYPE GetTypeOfResponse() const;
    DLLEXPORT NETWORKRESPONSETYPE GetType() const;

    DLLEXPORT sf::Packet GeneratePacketForResponse() const;

    DLLEXPORT int GetTimeOutValue() const;
    DLLEXPORT PACKET_TIMEOUT_STYLE GetTimeOutType() const;

    // De-coding functions //
    DLLEXPORT NetworkResponseDataForIdentificationString* GetResponseDataForIdentificationString() const;
    DLLEXPORT NetworkResponseDataForServerStatus* GetResponseDataForServerStatus() const;
    DLLEXPORT NetworkResponseDataForSyncValData* GetResponseDataForValueSyncResponse() const;
    DLLEXPORT NetworkResponseDataForSyncDataEnd* GetResponseDataForValueSyncEndResponse() const;
    DLLEXPORT NetworkResponseDataForCustom* GetResponseDataForGameSpecific() const;
    DLLEXPORT NetworkResponseDataForSyncResourceData* GetResponseDataForSyncResourceResponse() const;
    DLLEXPORT NetworkResponseDataForServerAllow* GetResponseDataForServerAllowResponse() const;
    DLLEXPORT NetworkResponseDataForCreateNetworkedInput* GetResponseDataForCreateNetworkedInputResponse() const;
    DLLEXPORT NetworkResponseDataForUpdateNetworkedInput* GetResponseDataForUpdateNetworkedInputResponse() const;
    DLLEXPORT NetworkResponseDataForInitialEntity* GetResponseDataForInitialEntity() const;
    DLLEXPORT NetworkResponseDataForEntityConstraint* GetResponseDataForEntityConstraint() const;
    DLLEXPORT NetworkResponseDataForWorldFrozen* GetResponseDataForWorldFrozen() const;
    DLLEXPORT NetworkResponseDataForEntityUpdate* GetResponseDataForEntityUpdate() const;
    DLLEXPORT NetworkResponseDataForEntityDestruction* GetResponseDataForEntityDestruction() const;
    DLLEXPORT NetworkResponseDataForAICacheUpdated* GetResponseDataForAICacheUpdated() const;
    DLLEXPORT NetworkResponseDataForAICacheRemoved* GetResponseDataForAICacheRemoved() const;
    DLLEXPORT NetworkResponseDataForDisconnectInput* GetResponseDataForDisconnectInput() const;
        

    DLLEXPORT int GetResponseID() const;

protected:

    int ResponseID;


    int TimeOutValue;
    PACKET_TIMEOUT_STYLE TimeOutStyle;

    NETWORKRESPONSETYPE ResponseType;

    // Holds the pointer to the struct that holds the response data //
    BaseNetworkResponseData* ResponseData = nullptr;
};

}

