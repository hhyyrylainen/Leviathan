#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "PartialEngine.h"
//#include "../DummyLog.h"

#include "NetworkTestHelpers.h"

#include "catch.hpp"

/**!
* @brief \file Tests that check that the \ref networkformat Is followed
*/

using namespace Leviathan;



TEST_CASE("Packet header bytes test", "[networking]"){

    // Receiver socket //
    sf::UdpSocket socket;
    socket.setBlocking(false);
    REQUIRE(socket.bind(sf::Socket::AnyPort) == sf::Socket::Done);

    PartialEngine<false> engine;

    TestClientInterface ClientInterface;

    NetworkHandler Client(NETWORKED_TYPE::Client, &ClientInterface);

    REQUIRE(Client.Init(sf::Socket::AnyPort));

    Connection ClientConnection(sf::IpAddress::LocalHost, socket.getLocalPort());

    ClientConnection.Init(&Client);

    CHECK(ClientConnection.GetState() == CONNECTION_STATE::NothingReceived);

    SECTION("Hello packet format"){

        // Init has already sent the hello packet //
        sf::Packet received;

        sf::IpAddress sender;
        unsigned short sentport;
        
        REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

        CHECK(sender == sf::IpAddress::LocalHost);
        CHECK(sentport == Client.GetOurPort());

        // First needs to be leviathan identification //
        uint16_t leviathanMagic = 0;

        received >> leviathanMagic;
        REQUIRE(received);

        // Normal packet check 0x4C6E (Ln)
        CHECK(0x6E == 'n');
        CHECK(static_cast<int>(leviathanMagic & 0xFF) == 'n');

        CHECK(0x4C == 'L');
        CHECK(static_cast<int>((leviathanMagic & 0xFF00) >> 8) == 'L');

        // ID number. Must be 1
        uint32_t packetID = 5;

        received >> packetID;
        REQUIRE(received);

        CHECK(packetID == 1);

        // Acks. Needs to be 0 and the next fields need to be missing
        uint32_t startAck = 5;

        received >> startAck;
        REQUIRE(received);

        CHECK(startAck == 0);

        // And then message count which should be 1
        uint8_t messageCount = 0;

        received >> messageCount;
        REQUIRE(received);

        CHECK(messageCount == 1);

        // Header was correct
    }

    
}


