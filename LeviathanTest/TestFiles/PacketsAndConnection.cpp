#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "PartialEngine.h"
#include "../DummyLog.h"

#include "NetworkTestHelpers.h"

#include "catch.hpp"

using namespace Leviathan;



class TestClientGetSpecificPacket : public NetworkClientInterface{
public:

    TestClientGetSpecificPacket(NETWORK_RESPONSE_TYPE typetocheckfor) :
        CheckForType(typetocheckfor)
    {
    }
    
    virtual void HandleResponseOnlyPacket(std::shared_ptr<NetworkResponse> message, 
        Connection &connection) override 
    {
        if(message->GetType() == CheckForType){

            ++ReceivedCount;
            
        } else {

            WARN("TestClientGetSpecificPacket: got something else than CheckForType");
        }
    }

    virtual void _OnStartApplicationConnect() override {
    }


    NETWORK_RESPONSE_TYPE CheckForType;

    //! How many packets of CheckForType type has been received
    int ReceivedCount = 0;
};

class GapingConnectionTest : public Connection{
public:

    GapingConnectionTest(unsigned int port) : Connection(sf::IpAddress::LocalHost, port)
    {
        State = CONNECTION_STATE::Authenticated;
    }

};


TEST_CASE("Connection correctly decodes ServerAllow and passes to ClientInterface",
    "[networking]")
{
    // Sender socket //
    sf::UdpSocket socket;
    socket.setBlocking(false);
    REQUIRE(socket.bind(sf::Socket::AnyPort) == sf::Socket::Done);

    PartialEngine<false> engine;

    TestClientGetSpecificPacket ClientInterface(NETWORK_RESPONSE_TYPE::ServerAllow);

    NetworkHandler Client(NETWORKED_TYPE::Client, &ClientInterface);

    REQUIRE(Client.Init(sf::Socket::AnyPort));

    auto ClientConnection = std::make_shared<GapingConnectionTest>(socket.getLocalPort());

    Client._RegisterConnection(ClientConnection);
    ClientConnection->Init(&Client);

    // Create packet //
    sf::Packet response;

    response << LEVIATHAN_NORMAL_PACKET << uint32_t(1) << uint32_t(0) << uint8_t(1) <<
        NORMAL_RESPONSE_TYPE << uint32_t(1);

    {
        ResponseServerAllow responseAllow(0, SERVER_ACCEPTED_TYPE::RequestQueued,
            "not a chance");
        responseAllow.AddDataToPacket(response);
    }

    SECTION("Direct pass packet"){

        ClientConnection->HandlePacket(response);
    }

    SECTION("Through socket"){

        REQUIRE(socket.send(response, sf::IpAddress::LocalHost, Client.GetOurPort()) ==
            sf::Socket::Done);
        
        Client.UpdateAllConnections();
    }

    // Needs to be marked as received //
    CHECK(ClientInterface.ReceivedCount == 1);
}







class ConnectionTestFixture {
protected:

    ConnectionTestFixture() : 
        Client(NETWORKED_TYPE::Client, &ClientInterface),
        Server(NETWORKED_TYPE::Server, &ServerInterface)
    {

        REQUIRE(Client.Init(sf::Socket::AnyPort));
        REQUIRE(Server.Init(sf::Socket::AnyPort));

        ClientConnection = std::make_shared<Connection>(
            sf::IpAddress::LocalHost, Server.GetOurPort());

        ServerConnection = std::make_shared<Connection>(
            sf::IpAddress::LocalHost, Client.GetOurPort());

        Server._RegisterConnection(ServerConnection);
        Client._RegisterConnection(ClientConnection);

        ClientConnection->Init(&Client);
        ServerConnection->Init(&Server);

        CHECK(ClientConnection->GetState() == CONNECTION_STATE::NothingReceived);
        CHECK(ServerConnection->GetState() == CONNECTION_STATE::NothingReceived);
    }

    void RunListeningLoop(int times = 3) {
        
        for (int i = 0; i < times; ++i) {

            Server.UpdateAllConnections();
            Client.UpdateAllConnections();
        }
    }

protected:

    PartialEngine<false> engine;

    TestClientInterface ClientInterface;
    NetworkHandler Client;


    TestServerInterface ServerInterface;
    NetworkHandler Server;

    std::shared_ptr<Connection> ClientConnection;
    std::shared_ptr<Connection> ServerConnection;
};

TEST_CASE_METHOD(ConnectionTestFixture, "Connect to localhost socket", "[networking]") {

    RunListeningLoop(6);

    // Should no longer be in initial state //
    CHECK(ClientConnection->GetState() != CONNECTION_STATE::NothingReceived);
    CHECK(ServerConnection->GetState() != CONNECTION_STATE::NothingReceived);

    CHECK(ClientConnection->IsValidForSend());
    CHECK(ServerConnection->IsValidForSend());

    // Should have been enough time to move to CONNECTION_STATE::Authenticated
    CHECK(ClientConnection->GetState() == CONNECTION_STATE::Authenticated);
    CHECK(ServerConnection->GetState() == CONNECTION_STATE::Authenticated);

}



