#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/BaseNotifiable.h"
#include "TimeIncludes.h"

#include "SFML/Network/IpAddress.hpp"

#include <memory>

namespace Leviathan{

class RemoteConsoleSession{
    friend RemoteConsole;
public:
    DLLEXPORT RemoteConsoleSession(const std::string &name,
        std::shared_ptr<Connection> connection, int32_t token) :
        ConnectionName(name), SessionToken(token), CorrespondingConnection(connection),
        IsOpened(true), TerminateSession(false)
    { }
    DLLEXPORT ~RemoteConsoleSession();

    DLLEXPORT Connection* GetConnection();

    //! \brief Sets the connection as closing
    DLLEXPORT void KillConnection();

private:
    std::string ConnectionName;
    int SessionToken;
    std::shared_ptr<Connection> CorrespondingConnection;

    //! Marks if we can send stuff here //
    bool IsOpened;
        
    //! \brief Sets connection as terminating
    //!
    //! Actual termination will happen next time RemoteConsole::UpdateStatus is called
    bool TerminateSession;
};


//! Class used to handle remote server commands and receiving messages
class RemoteConsole : public ThreadSafe{
    friend Engine;

    struct RemoteConsoleExpect{
        RemoteConsoleExpect(const std::string &name, int token, bool onlylocalhost,
            const MillisecondDuration &timeout);


        std::string ConnectionName;
        int SessionToken;
        bool OnlyLocalhost;

        WantedClockType::time_point TimeoutTime;
    };
public:
    DLLEXPORT RemoteConsole();
    DLLEXPORT ~RemoteConsole();

    //! \brief Called before packets are handled
    //!
    //! Checks statuses of remote connections and can perform some special tasks such as
    //! closing Connection objects
    DLLEXPORT void UpdateStatus();


    // Handle functions for interface to use //
    DLLEXPORT void HandleRemoteConsoleRequestPacket(std::shared_ptr<NetworkRequest> request,
        std::shared_ptr<Connection> connection);
    DLLEXPORT void HandleRemoteConsoleResponse(std::shared_ptr<NetworkResponse> response,
        Connection &connection, std::shared_ptr<NetworkRequest> potentialrequest);

    //! Does everything needed to allow the client on the connection to connect to us
    DLLEXPORT void OfferConnectionTo(std::shared_ptr<Connection> connectiontouse,
        const std::string &connectionname, int token);

    //! \brief Returns true if connections are marked as awaiting
    //!
    //! Connections are marked as expected when the ExpectNewConnection function is called
    DLLEXPORT bool IsAwaitingConnections();


    //! \brief Returns active number of connections
    DLLEXPORT size_t GetActiveConnectionCount();


    //! \brief Gets the corresponding Connection object from a
    //! RemoteConsoleSession session indicated by name
    DLLEXPORT std::shared_ptr<Connection> GetConnectionForRemoteConsoleSession(
        const std::string &name);

    //! \brief Sets the remote console to close the game if there are no connections
    //!
    //! \see CloseIfNoRemoteConsole
    DLLEXPORT void SetCloseIfNoRemoteConsole(bool state);

    //! \brief Gets a matching RemoteConsoleSession from Connection
    //!
    //! \return Returns a valid pointer to a RemoteConsoleSession or NULL
    //! \note The returned pointer will be guaranteed to be only valid while
    //! you have guard locked
    //! \param guard Lock with this RemoteConsole instance
    DLLEXPORT RemoteConsoleSession* GetRemoteConsoleSessionForConnection(Lock &guard,
        Connection &connection);

    //! \brief Returns true if request from connection is allowed to open a remote console
    //! session
    DLLEXPORT bool CanOpenNewConnection(std::shared_ptr<Connection> connection,
        std::shared_ptr<NetworkRequest> request);

    DLLEXPORT void ExpectNewConnection(int SessionToken, const std::string &assignname = "",
        bool onlylocalhost = false, const MillisecondDuration &timeout =
        std::chrono::seconds(30));

protected:

    //! \brief Called by Engine after command line has been processed
    void SetAllowClose();

private:


    // ------------------------------------ //
    // We need to store the requests until we get a response //
    std::vector<std::shared_ptr<NetworkRequest>> WaitingRequests;

    std::vector<std::shared_ptr<RemoteConsoleSession>> RemoteConsoleConnections;

    std::vector<std::shared_ptr<RemoteConsoleExpect>> AwaitingConnections;

    //! Sends a close signal to the application if has no AwaitingConnections or
    //! RemoteConsoleConnections
    bool CloseIfNoRemoteConsole;

    //! Prevents the program from closing before receiving the wanted connection info
    bool CanClose;
};

}
