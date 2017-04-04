#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "PartialEngine.h"
#include "../DummyLog.h"

#include "NetworkTestHelpers.h"

#include "catch.hpp"

using namespace Leviathan;



TEST_CASE("Ack field filling", "networking") {

    SECTION("Basic construction") {

        SECTION("Single Byte") {

            NetworkAckField::PacketReceiveStatus packetsreceived;
            packetsreceived[1] = RECEIVED_STATE::StateReceived;
            packetsreceived[2] = RECEIVED_STATE::StateReceived;
            packetsreceived[3] = RECEIVED_STATE::StateReceived;
            packetsreceived[4] = RECEIVED_STATE::StateReceived;
            packetsreceived[6] = RECEIVED_STATE::StateReceived;
            packetsreceived[12] = RECEIVED_STATE::StateReceived;
            packetsreceived[18] = RECEIVED_STATE::StateReceived;

            NetworkAckField tosend(1, 32, packetsreceived);

            CHECK(tosend.FirstPacketID == 1);
            CHECK(tosend.IsAckSet(0));
            CHECK(tosend.IsAckSet(1));
            CHECK(!tosend.IsAckSet(4));
            CHECK(tosend.IsAckSet(5));
            CHECK(!tosend.IsAckSet(6));
            CHECK(tosend.IsAckSet(11));
            CHECK(tosend.IsAckSet(17));
            CHECK(!tosend.IsAckSet(93));
        }

        SECTION("Multiple Bytes") {

            NetworkAckField::PacketReceiveStatus packetsreceived;
            packetsreceived[1] = RECEIVED_STATE::StateReceived;
            packetsreceived[2] = RECEIVED_STATE::StateReceived;
            packetsreceived[3] = RECEIVED_STATE::StateReceived;
            packetsreceived[14] = RECEIVED_STATE::StateReceived;
            packetsreceived[19] = RECEIVED_STATE::StateReceived;
            packetsreceived[25] = RECEIVED_STATE::StateReceived;
            packetsreceived[28] = RECEIVED_STATE::StateReceived;
            packetsreceived[48] = RECEIVED_STATE::StateReceived;
            packetsreceived[50] = RECEIVED_STATE::StateReceived;
            packetsreceived[128] = RECEIVED_STATE::StateReceived;

            NetworkAckField tosend(1, 32, packetsreceived);

            CHECK(tosend.FirstPacketID == 1);
            CHECK(tosend.Acks.size() > 1);
            CHECK(tosend.IsAckSet(0));
            CHECK(tosend.IsAckSet(1));
            CHECK(!tosend.IsAckSet(4));
            CHECK(tosend.IsAckSet(13));
            CHECK(tosend.IsAckSet(18));
            CHECK(tosend.IsAckSet(24));
            CHECK(tosend.IsAckSet(27));
            CHECK(!tosend.IsAckSet(30));
            CHECK(!tosend.IsAckSet(42));
            // Max count is respected if these pass 
            CHECK(!tosend.IsAckSet(47));
            CHECK(!tosend.IsAckSet(49));
            CHECK(!tosend.IsAckSet(127));
        }
    }

    SECTION("Empty field to packet has no length value") {

        NetworkAckField::PacketReceiveStatus first;
        first[1] = RECEIVED_STATE::NotReceived;

        NetworkAckField tosend(1, 32, first);

        CHECK(tosend.FirstPacketID == 0);
        CHECK(tosend.Acks.size() == 0);

        sf::Packet packet;

        tosend.AddDataToPacket(packet);

        CHECK(packet.getDataSize() == sizeof(uint32_t));

        SECTION("Loading") {

            NetworkAckField unserialized(packet);

            CHECK(packet);

            CHECK(unserialized.FirstPacketID == 0);
        }
    }

    SECTION("Direct manipulation") {


        NetworkAckField::PacketReceiveStatus first;
        first[1] = RECEIVED_STATE::StateReceived;
        first[2] = RECEIVED_STATE::StateReceived;
        first[3] = RECEIVED_STATE::StateReceived;
        first[4] = RECEIVED_STATE::StateReceived;
        first[6] = RECEIVED_STATE::StateReceived;
        first[12] = RECEIVED_STATE::StateReceived;
        first[18] = RECEIVED_STATE::StateReceived;
        // This shouldn't be included
        first[94] = RECEIVED_STATE::StateReceived;

        NetworkAckField tosend(1, 32, first);

        CHECK(tosend.FirstPacketID == 1);
        CHECK(tosend.IsAckSet(0));
        CHECK(tosend.IsAckSet(1));
        CHECK(!tosend.IsAckSet(4));
        CHECK(tosend.IsAckSet(5));
        CHECK(tosend.IsAckSet(11));
        CHECK(!tosend.IsAckSet(93));

        sf::Packet packet;

        tosend.AddDataToPacket(packet);

        CHECK(packet.getDataSize() > 0);

        NetworkAckField receive(packet);

        CHECK(receive.Acks.size() > 0);
        CHECK(receive.FirstPacketID == tosend.FirstPacketID);
        CHECK(receive.IsAckSet(0));
        CHECK(receive.IsAckSet(1));
        CHECK(!receive.IsAckSet(4));
        CHECK(receive.IsAckSet(5));
        CHECK(receive.IsAckSet(11));
        CHECK(!receive.IsAckSet(93));
    }
}

TEST_CASE("Packet serialization and deserialization", "networking") {

    PartialEngine<false> reporter;

    // Requests //
    SECTION("Connect basic data") {

        sf::Packet packet;

        RequestConnect request;

        request.AddDataToPacket(packet);

        bool dummy;
        packet >> dummy;

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Connect);

        auto* deserialized = static_cast<RequestConnect*>(loaded.get());

        CHECK(deserialized->CheckValue == request.CheckValue);
    }
    SECTION("RequestSecurity") {

        sf::Packet packet;

        RequestSecurity request(CONNECTION_ENCRYPTION::Standard, "1235312125", "315247268");

        request.AddDataToPacket(packet);

        bool dummy;
        packet >> dummy;

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Security);

        auto* deserialized = static_cast<RequestSecurity*>(loaded.get());

        CHECK(deserialized->SecureType == request.SecureType);
        CHECK(deserialized->PublicKey == request.PublicKey);
        CHECK(deserialized->AdditionalSettings == request.AdditionalSettings);
    }
    SECTION("RequestAuthenticate") {

        sf::Packet packet;

        RequestAuthenticate request("griefer-123", 57332578, "my-pass");

        request.AddDataToPacket(packet);

        bool dummy;
        packet >> dummy;

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Authenticate);

        auto* deserialized = static_cast<RequestAuthenticate*>(loaded.get());

        CHECK(deserialized->UserName == request.UserName);
        CHECK(deserialized->AuthToken == request.AuthToken);
        CHECK(deserialized->AuthPasswd == request.AuthPasswd);
    }



    // Responses //
    SECTION("ResponseAuthenticate") {

        sf::Packet packet;

        ResponseAuthenticate response(0, 1001, 523980358035209);

        response.AddDataToPacket(packet);

        bool dummy;
        packet >> dummy;

        auto loaded = NetworkResponse::LoadFromPacket(packet);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_RESPONSE_TYPE::Authenticate);

        auto* deserialized = static_cast<ResponseAuthenticate*>(loaded.get());

        CHECK(deserialized->GetResponseID() == response.GetResponseID());
        CHECK(deserialized->UserID == response.UserID);
        CHECK(deserialized->UserToken == response.UserToken);
    }
    SECTION("ResponseSecurity") {

        sf::Packet packet;

        ResponseSecurity response(712, CONNECTION_ENCRYPTION::Standard, "523980358035209",
            "234650879135789035209547");

        response.AddDataToPacket(packet);

        bool dummy;
        packet >> dummy;

        auto loaded = NetworkResponse::LoadFromPacket(packet);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_RESPONSE_TYPE::Security);

        auto* deserialized = static_cast<ResponseSecurity*>(loaded.get());

        CHECK(deserialized->GetResponseID() == response.GetResponseID());
        CHECK(deserialized->SecureType == response.SecureType);
        CHECK(deserialized->PublicKey == response.PublicKey);
        CHECK(deserialized->EncryptedSymmetricKey == response.EncryptedSymmetricKey);
    }
   
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



