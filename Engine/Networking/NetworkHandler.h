// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "CommonNetwork.h"
#include "MasterServerInfo.h"
#include "NetworkClientInterface.h"
#include "NetworkInterface.h"
#include "NetworkServerInterface.h"

#include "Connection.h"


#include "Common/ThreadSafe.h"

#include "SFML/Network/UdpSocket.hpp"

#include <future>
#include <memory>
#include <thread>

namespace Leviathan {

class Engine;

void RunGetResponseFromMaster(
    NetworkHandler* instance, std::shared_ptr<std::promise<std::string>> resultvar);

//! \brief Handles everything related to connections
class NetworkHandler : public ThreadSafe {
    friend void RunGetResponseFromMaster(
        NetworkHandler* instance, std::shared_ptr<std::promise<std::string>> resultvar);

    friend Connection;
    friend Engine;

public:
    // Either a client or a server handler //
    DLLEXPORT NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler);
    DLLEXPORT virtual ~NetworkHandler();

    DLLEXPORT virtual bool Init(const MasterServerInformation& info);

    //! Don't use this version. This just creates a socket with the port
    DLLEXPORT bool Init(uint16_t port = 0);

    // \note This waits for all connections to terminate
    DLLEXPORT virtual void Release();

    //! \note  Call as often as possible to receive responses
    DLLEXPORT virtual void UpdateAllConnections();

    DLLEXPORT virtual void RemoveClosedConnections(Lock& guard);

    DLLEXPORT std::shared_ptr<std::promise<std::string>> QueryMasterServer(
        const MasterServerInformation& info);

    //! \brief Checks whether a connection is open and ready for sending
    //! \note Will check whether the connection is in Make sure it is in OpenConnections or not
    //! Which may be a waste of time if only the connection needs to be checked for openness
    DLLEXPORT bool IsConnectionValid(Connection& connection) const;

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
    DLLEXPORT std::shared_ptr<Connection> OpenConnectionTo(const std::string& targetaddress);

    //! \brief Variant for already resolved address
    //! \see OpenConnectionTo
    DLLEXPORT std::shared_ptr<Connection> OpenConnectionTo(
        Lock& guard, const sf::IpAddress& targetaddress, unsigned short port);

    inline std::shared_ptr<Connection> OpenConnectionTo(
        const sf::IpAddress& targetaddress, unsigned short port)
    {
        GUARD_LOCK();
        return OpenConnectionTo(guard, targetaddress, port);
    }

    //! Returns the port to which our socket has been bind
    inline uint16_t GetOurPort() const
    {
        return PortNumber;
    }

    //! \brief Gets the type of network this program uses
    //!
    //! Will usually be NETWORKED_TYPE_CLIENT or NETWORKED_TYPE_SERVER
    inline NETWORKED_TYPE GetNetworkType() const
    {
        return AppType;
    }

    inline const char* GetNetworkTypeStr() const
    {
        if(AppType == NETWORKED_TYPE::Client)
            return "client";
        if(AppType == NETWORKED_TYPE::Server)
            return "server";
        if(AppType == NETWORKED_TYPE::Master)
            return "master";

        return "unknown";
    }

    //! \brief Returns interface object. Type depends on AppType
    inline NetworkInterface* GetInterface()
    {
        if(AppType == NETWORKED_TYPE::Client)
            return ClientInterface;
        if(AppType == NETWORKED_TYPE::Server)
            return ServerInterface;

        return nullptr;
    }

    inline NetworkClientInterface* GetClientInterface()
    {

        return ClientInterface;
    }

    inline NetworkCache* GetCache()
    {

        return _NetworkCache.get();
    }

    inline SyncedVariables* GetSyncedVariables()
    {

        return VariableSyncer.get();
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
    DLLEXPORT virtual void CloseConnection(Connection& connection);

    // Common network functions //
    //! \brief Retrieves base address from an http protocol URL
    //! For example if passed http://boostslair.com/Pong/MastersList.php
    //! returns http://boostslair.com/ //
    DLLEXPORT static std::string GetServerAddressPartOfAddress(
        const std::string& fulladdress, const std::string& regextouse = "http://.*?/");


    //! Adds a connection to the list of open connections. Use ONLY if you have manually
    //! created a Connection object
    DLLEXPORT void _RegisterConnection(std::shared_ptr<Connection> connection);


protected:
    //! \brief Unhooks the NetworkInterfaces from this object.
    //!
    //! This uses locking but the reading methods don't use locks. So this should only
    //! be called from the main thread
    DLLEXPORT void DisconnectInterface();


    Lock LockSocketForUse();

    // Closes the socket //
    void _ReleaseSocket();

    //! \brief Returns false if the socket has been closed
    bool _RunUpdateOnce(Lock& guard);

    //! \brief Constantly listens for packets in a blocked state
    void _RunListenerThread();

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

    //! If true uses a blocking socket and async handling
    bool BlockingMode = false;

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

    std::string MasterServerMustPassIdentification;
};

} // namespace Leviathan
