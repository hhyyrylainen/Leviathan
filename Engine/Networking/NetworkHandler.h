// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "NetworkInterface.h"
#include "NetworkServerInterface.h"
#include "NetworkClientInterface.h"
#include "CommonNetwork.h"

#include "Connection.h"


#include "Common/ThreadSafe.h"

#include "SFML/Network/UdpSocket.hpp"

#include <future>
#include <thread>
#include <memory>

namespace Leviathan{

void RunGetResponseFromMaster(NetworkHandler* instance,
    std::shared_ptr<std::promise<std::string>> resultvar);
	
// Used to pass master server info to the application //
struct MasterServerInformation{
    MasterServerInformation(bool iammaster, const std::string &identificationstr) :
        MasterServerIdentificationString(identificationstr), RequireMaster(false),
        IAmMyOwnMaster(true)
    {

    }
    MasterServerInformation() : RequireMaster(false), IAmMyOwnMaster(false){
    }
    MasterServerInformation(const std::string &masterslistfile,
        const std::string &identification,
        const std::string &masterserverlistaddress,
        const std::string &masterserverlistpagename,
        const std::string &loginsession, bool requireconnection = false) :
        MasterListFetchServer(masterserverlistaddress),
        MasterListFetchPage(masterserverlistpagename),
        StoredListFile(masterslistfile), MasterServerIdentificationString(identification),
        LoginStoreFile(loginsession), RequireMaster(requireconnection), IAmMyOwnMaster(false)
    {

    }
        
        
    std::string MasterListFetchServer;
    std::string MasterListFetchPage;
    std::string StoredListFile;
    std::string MasterServerIdentificationString;
    std::string LoginStoreFile;
    bool RequireMaster;
    bool IAmMyOwnMaster;
};

//! \brief Handles everything related to connections
class NetworkHandler : public ThreadSafe {
    friend void RunGetResponseFromMaster(NetworkHandler* instance,
        std::shared_ptr<std::promise<std::string>> resultvar);

    friend Connection;
public:
    // Either a client or a server handler //
    DLLEXPORT NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler);
    DLLEXPORT virtual ~NetworkHandler();

    DLLEXPORT virtual bool Init(const MasterServerInformation &info);
        
    // \note This waits for all connections to terminate
    DLLEXPORT virtual void Release();

    //! \note  Call as often as possible to receive responses
    DLLEXPORT virtual void UpdateAllConnections();

    //! \brief Called by Engine to stop own connection update thread
    DLLEXPORT void StopOwnUpdaterThread(Lock &guard);

    DLLEXPORT virtual void RemoveClosedConnections();

    DLLEXPORT std::shared_ptr<std::promise<std::string>> QueryMasterServer(
        const MasterServerInformation &info);

    //! \brief Checks whether a connection is open and ready for sending
    DLLEXPORT bool IsConnectionValid(Connection &connection) const;

    //! \brief Returns a persistent pointer to a connection
    DLLEXPORT std::shared_ptr<Connection> GetConnection(Connection* directptr) const;
    
    //! \brief Opens a new connection to the provided address
    //!
    //! \param targetaddress The input should be in a form that has address:port in it.
    //! The address should be like 'google.fi' or '192.168.1.1'
    //! \note This function doesn't verify that there actually is something on the target.
    //! \note  This will return an existing connection if one matches the targetaddress
    //! \note The connection will be automatically closed if the remote doesn't properly
    //! respond to our open request
    DLLEXPORT std::shared_ptr<Connection> OpenConnectionTo(
        const std::string &targetaddress);

    //! \brief Variant for already resolved address
    //! \see OpenConnectionTo
    DLLEXPORT std::shared_ptr<Connection> OpenConnectionTo(
        const sf::IpAddress &targetaddress, unsigned short port);

    //! Returns the port to which our socket has been bind
    inline uint16_t GetOurPort() const{
        
        return PortNumber;
    }
    
    //! \brief Gets the type of network this program uses
    //!
    //! Will usually be NETWORKED_TYPE_CLIENT or NETWORKED_TYPE_SERVER
    inline NETWORKED_TYPE GetNetworkType() const{
        
        return AppType;
    }
    
    //! \brief Returns interface object. Type depends on AppType
    inline NetworkInterface* GetInterface(){
        
        if(AppType == NETWORKED_TYPE::Client)
            return ClientInterface;
        if(AppType == NETWORKED_TYPE::Server)
            return ServerInterface;

        return nullptr;
    }

    //! \brief Destroys the network cache permanently
    //!
    //! Called by the engine when quitting the game
    DLLEXPORT void ShutdownCache();

    //! \brief Destroys the networked input handler and all input objects
    //!
    //! Called by the engine when quitting the game
    DLLEXPORT void ReleaseInputHandler();
    
    //! \brief Marks a connection as closing
    //!
    //! The connection will send a close packet and if provided a reason a reason packet,
    //! also. The connection will no longer be processed in update
    DLLEXPORT virtual void CloseConnection(Connection &connection);
    
    // Common network functions //
    //! \brief Retrieves base address from an http protocol URL
    //! For example if passed http://boostslair.com/Pong/MastersList.php
    //! returns http://boostslair.com/ //
    DLLEXPORT static std::string GetServerAddressPartOfAddress(const std::string &fulladdress,
        const std::string &regextouse = "http://.*?/");
    
    
protected:
    
    Lock LockSocketForUse();
    
    // Closes the socket //
    void _ReleaseSocket();
    
    //! \brief Constantly listens for packets in a blocked state
    void _RunListenerThread();
    
    //! \brief Does temporary connection updating
    void _RunTemporaryUpdaterThread();
    
    void _SaveMasterServerList();
    bool _LoadMasterServerList();
    
    // ------------------------------------ //
    
    // Internal listing of all connections //
    
    //! Closes connections on next update
    Mutex ConnectionsToTerminateMutex;
    std::vector<Connection*> ConnectionsToTerminate;
    
    std::vector<std::shared_ptr<Connection>> OpenConnections;

    //! Type of application
    NETWORKED_TYPE AppType;

    NetworkServerInterface* ServerInterface = nullptr;
    NetworkClientInterface* ClientInterface = nullptr;

    //! Main socket for listening for incoming packets and sending
    sf::UdpSocket _Socket;
    //! Used to control the locking of the socket
    Mutex SocketMutex;

    //! Our local port number
    uint16_t PortNumber;

    //! The syncable variable holder associated with this instance
    std::unique_ptr<SyncedVariables> VariableSyncer;

    //! Networked variable cache
    std::unique_ptr<NetworkCache> _NetworkCache;
    

    //! Game specific packet handler that allows programs to register their own packets
    std::unique_ptr<GameSpecificPacketHandler> _GameSpecificPacketHandler;

    // The master server list //
    std::vector<std::shared_ptr<std::string>> MasterServers;

    //! Stores a "working" (meaning the server has responded something) master server address
    std::shared_ptr<Connection> MasterServerConnection;

    MasterServerInformation StoredMasterServerInfo;

    //! Makes sure that master server thread is graciously closed
    std::thread MasterServerConnectionThread;
    bool CloseMasterServerConnection;

    //! Thread that constantly blocks on the socket and waits for packets
    std::thread ListenerThread;

    //! Temporary thread for getting responses while the game is starting
    std::thread TemporaryUpdateThread;
    bool UpdaterThreadStop;

    std::condition_variable_any NotifyTemporaryUpdater;

    std::string MasterServerMustPassIdentification;
};

}

