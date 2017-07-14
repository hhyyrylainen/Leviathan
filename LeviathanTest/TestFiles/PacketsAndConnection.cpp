#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "../DummyLog.h"

#include "../NetworkTestHelpers.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


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

// ------------------------------------ //

TEST_CASE_METHOD(ClientConnectionTestFixture, "ClientConnectionTestFixture can manually open "
    "a connection to a client", "[networking]")
{
    DoConnectionOpening();
}

// ------------------------------------ //
TEST_CASE_METHOD(ClientConnectionTestFixture, "Client sends JoinServer message when joining",
    "[networking]")
{
    DoConnectionOpening();

    REQUIRE(ClientInterface.JoinServer(ClientConnection));
}



// ------------------------------------ //
TEST_CASE_METHOD(ConnectionTestFixture, "Connect to localhost socket", "[networking]"){

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

// TEST_CASE_METHOD(ConnectionTestFixture, "No infinite acks", "[networking]"){

//     RunListeningLoop(6);

//     // Should have been enough time to move to CONNECTION_STATE::Authenticated
//     CHECK(ClientConnection->GetState() == CONNECTION_STATE::Authenticated);
//     CHECK(ServerConnection->GetState() == CONNECTION_STATE::Authenticated);

//     // Acks should quiet down after a while

//     // I have no clue how to test this without making the Timer class
//     // have a possibility for a dummy time variable
//     RunListeningLoop(6);
// }


// TEST_CASE_METHOD(ConnectionTestFixture, "Test server join", "[networking]"){

//     RunListeningLoop(6);

//     // Should have been enough time to move to CONNECTION_STATE::Authenticated
//     CHECK(ClientConnection->GetState() == CONNECTION_STATE::Authenticated);
//     CHECK(ServerConnection->GetState() == CONNECTION_STATE::Authenticated);

//     // Client requests to join game //
//     REQUIRE(ClientInterface.JoinServer(ClientConnection));

//     RunListeningLoop(6);

//     CHECK(ClientInterface.GetServerConnectionState() ==
//         NetworkClientInterface::CLIENT_CONNECTION_STATE::Connected);
    
    
// }



