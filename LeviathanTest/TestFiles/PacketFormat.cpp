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

    SECTION("Header format"){

        // Test size of Connection request
        RequestConnect requestTest;

        sf::Packet sizetest;
        requestTest._SerializeCustom(sizetest);

        const auto connectSize = sizetest.getDataSize();

        CHECK(connectSize > 0);

        // Init has already sent the hello packet //
        sf::Packet received;

        sf::IpAddress sender;
        unsigned short sentport;
        
        REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

        CHECK(sender == sf::IpAddress::LocalHost);
        CHECK(sentport == Client.GetOurPort());

        // Check size //
        CHECK(received.getDataSize() == 18 + connectSize);

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
        
        SECTION("Hello packet payload"){

            uint8_t messageType = 1;
            
            received >> messageType;
            REQUIRE(received);

            CHECK(messageType == NORMAL_REQUEST_TYPE);

            uint32_t messageNumber = 656;

            received >> messageNumber;
            REQUIRE(received);

            CHECK(messageNumber == 1);

            uint16_t responseTypeRaw = 500;

            received >> responseTypeRaw;
            REQUIRE(received);

            CHECK(static_cast<NETWORK_REQUEST_TYPE>(responseTypeRaw) ==
                NETWORK_REQUEST_TYPE::Connect);

            SECTION("Data size is correct"){

                
                // Consume all the data in the Connect request and then verify that the
                // packet is empty
                for(size_t i = 0; i < connectSize; ++i){
                    
                    uint8_t dummy;
                    received >> dummy;
                    INFO("On loop: " + std::to_string(i));
                    REQUIRE(received);
                }

                uint8_t invalidread;
                received >> invalidread;
                CHECK(!received);
            }

            SECTION("Connect Request payload is correct"){

                int32_t checkValue = 0;

                received >> checkValue;
                REQUIRE(received);

                CHECK(checkValue == 42);
            }

        }
    }
}

TEST_CASE("Packet serialization and deserialization", "[networking]"){

    PartialEngine<false> reporter;

    // Requests //
    SECTION("Connect basic data"){

        sf::Packet packet;

        RequestConnect request;

        request.AddDataToPacket(packet);

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Connect);

        auto* deserialized = static_cast<RequestConnect*>(loaded.get());

        CHECK(deserialized->CheckValue == request.CheckValue);
    }
    
    SECTION("RequestSecurity"){

        sf::Packet packet;

        RequestSecurity request(CONNECTION_ENCRYPTION::Standard, "1235312125", "315247268");

        request.AddDataToPacket(packet);

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Security);

        auto* deserialized = static_cast<RequestSecurity*>(loaded.get());

        CHECK(deserialized->SecureType == request.SecureType);
        CHECK(deserialized->PublicKey == request.PublicKey);
        CHECK(deserialized->AdditionalSettings == request.AdditionalSettings);
    }
    
    SECTION("RequestAuthenticate"){

        sf::Packet packet;

        RequestAuthenticate request("griefer-123", 57332578, "my-pass");

        request.AddDataToPacket(packet);

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Authenticate);

        auto* deserialized = static_cast<RequestAuthenticate*>(loaded.get());

        CHECK(deserialized->UserName == request.UserName);
        CHECK(deserialized->AuthToken == request.AuthToken);
        CHECK(deserialized->AuthPasswd == request.AuthPasswd);
    }



    // Responses //
    SECTION("ResponseAuthenticate"){

        sf::Packet packet;

        ResponseAuthenticate response(0, 1001, 523980358035209);

        response.AddDataToPacket(packet);

        auto loaded = NetworkResponse::LoadFromPacket(packet);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_RESPONSE_TYPE::Authenticate);

        auto* deserialized = static_cast<ResponseAuthenticate*>(loaded.get());

        CHECK(deserialized->GetResponseID() == response.GetResponseID());
        CHECK(deserialized->UserID == response.UserID);
        CHECK(deserialized->UserToken == response.UserToken);
    }
    
    SECTION("ResponseSecurity"){

        sf::Packet packet;

        ResponseSecurity response(712, CONNECTION_ENCRYPTION::Standard, "523980358035209",
            "234650879135789035209547");

        response.AddDataToPacket(packet);

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

TEST_CASE("Ack field filling", "[networking]") {

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

    SECTION("Serialize size test"){

        NetworkAckField::PacketReceiveStatus first;
        first[1] = RECEIVED_STATE::StateReceived;
        first[2] = RECEIVED_STATE::StateReceived;
        first[3] = RECEIVED_STATE::StateReceived;
        first[4] = RECEIVED_STATE::StateReceived;
        first[6] = RECEIVED_STATE::StateReceived;

        NetworkAckField tosend(1, 32, first);

        sf::Packet packet;

        tosend.AddDataToPacket(packet);

        CHECK(packet.getDataSize() == sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t));

        SECTION("Three bytes"){

            first[12] = RECEIVED_STATE::StateReceived;
            first[18] = RECEIVED_STATE::StateReceived;

            NetworkAckField tosend(1, 32, first);

            sf::Packet packet;

            tosend.AddDataToPacket(packet);

            CHECK(packet.getDataSize() == sizeof(uint32_t) + sizeof(uint8_t) + (
                    sizeof(uint8_t) * 3));
        }
    }
}

class AckFillConnectionTest : public Connection{
public:

    AckFillConnectionTest() : Connection(sf::IpAddress::LocalHost, 33030)
    {
        
    }

    void FillJustAcks(sf::Packet &tofill){

        GUARD_LOCK();
        _FillHeaderAcks(guard, ++LastUsedLocalID, tofill);
    }

    void SetPacketReceived(uint32_t packetid, RECEIVED_STATE state){

        ReceivedRemotePackets[packetid] = state;
    }

    void SetDone(uint32_t packetid){

        auto iter = ReceivedRemotePackets.find(packetid);

        if(iter != ReceivedRemotePackets.end())
            ReceivedRemotePackets.erase(iter);
    }

};

TEST_CASE("Creating Acks in headers", "[networking]"){

    AckFillConnectionTest connection;
    auto& packetlist = connection.GetReceivedPackets();
    
    sf::Packet received;
    
    SECTION("No packets"){

        connection.FillJustAcks(received);

        CHECK(received.getDataSize() == sizeof(uint32_t));

        uint32_t count = -1;
        received >> count;
        REQUIRE(received);

        CHECK(count == 0);
    }

    SECTION("1 packet"){

        CHECK(packetlist.find(1) == packetlist.end());
        
        connection.SetPacketReceived(1, RECEIVED_STATE::StateReceived);

        {
            REQUIRE(!packetlist.empty());
            auto iter = packetlist.find(1);
            REQUIRE(iter != packetlist.end());
            const RECEIVED_STATE state = iter->second;
            CHECK(state == RECEIVED_STATE::StateReceived);
        }

        connection.FillJustAcks(received);

        CHECK(received.getDataSize() == sizeof(uint32_t) + sizeof(uint8_t) * 2);

        uint32_t startID = -1;
        received >> startID;
        REQUIRE(received);

        CHECK(startID == 1);

        
        uint8_t count = -1;

        received >> count;
        REQUIRE(received);

        CHECK(count == 1);

        SECTION("Ack is marked as sent"){
            
            auto iter = packetlist.find(1);
            REQUIRE(iter != packetlist.end());
            const RECEIVED_STATE state = iter->second;
            CHECK(state == RECEIVED_STATE::AcksSent);
        }
        
    }

    SECTION("2 packets"){

        connection.SetPacketReceived(1, RECEIVED_STATE::StateReceived);
        connection.SetPacketReceived(2, RECEIVED_STATE::StateReceived);

        connection.FillJustAcks(received);

        CHECK(received.getDataSize() == sizeof(uint32_t) + sizeof(uint8_t) * 2);

        uint32_t startID = -1;
        received >> startID;
        REQUIRE(received);

        CHECK(startID == 1);

        
        uint8_t count = -1;

        received >> count;
        REQUIRE(received);

        CHECK(count == 1);

        SECTION("Ack is marked as sent"){
            
            auto iter = packetlist.find(1);
            REQUIRE(iter != packetlist.end());
            CHECK(iter->second == RECEIVED_STATE::AcksSent);

            iter = packetlist.find(2);
            REQUIRE(iter != packetlist.end());
            CHECK(iter->second == RECEIVED_STATE::AcksSent);

            iter = packetlist.find(3);
            CHECK(iter == packetlist.end());
        }
    }

    SECTION("Extra bytes"){
        
        connection.SetPacketReceived(1, RECEIVED_STATE::StateReceived);
        connection.SetPacketReceived(2, RECEIVED_STATE::StateReceived);
        connection.SetPacketReceived(5, RECEIVED_STATE::StateReceived);
        connection.SetPacketReceived(7, RECEIVED_STATE::StateReceived);
        connection.SetPacketReceived(8, RECEIVED_STATE::StateReceived);
        connection.SetPacketReceived(9, RECEIVED_STATE::StateReceived);

        connection.FillJustAcks(received);

        CHECK(received.getDataSize() == sizeof(uint32_t) + sizeof(uint8_t) * 3);

        uint32_t startID = -1;
        received >> startID;
        REQUIRE(received);

        CHECK(startID == 1);

        
        uint8_t count = -1;

        received >> count;
        REQUIRE(received);

        CHECK(count == 2);
    }

    SECTION("Back acks"){

        for(int i = 2; i < DEFAULT_ACKCOUNT * 2; ++i)
            connection.SetPacketReceived(i, RECEIVED_STATE::StateReceived);

        for(int i = 1; i < 11; ++i)
            connection.SetPacketReceived(i + DEFAULT_ACKCOUNT * 5,
                RECEIVED_STATE::StateReceived);

        // All of the acks cannot fit into a single packet
        connection.FillJustAcks(received);

        CHECK(received.getDataSize() == sizeof(uint32_t) + sizeof(uint8_t) + (
                DEFAULT_ACKCOUNT/8));

        uint32_t startID = -1;
        received >> startID;
        REQUIRE(received);

        CHECK(startID == 2);

        
        uint8_t count = -1;

        received >> count;
        REQUIRE(received);

        CHECK(count == DEFAULT_ACKCOUNT/8);


        received.clear();
        connection.FillJustAcks(received);

        CHECK(received.getDataSize() >= sizeof(uint32_t) + sizeof(uint8_t) * 2);

        startID = -1;
        received >> startID;
        REQUIRE(received);

        CHECK(startID > DEFAULT_ACKCOUNT);

        
        count = -1;

        received >> count;
        REQUIRE(received);

        CHECK(count > 0);
    }
}

TEST_CASE("Manually constructed packet decoding", "[networking]"){

    // Sender socket //
    sf::UdpSocket socket;
    socket.setBlocking(false);
    REQUIRE(socket.bind(sf::Socket::AnyPort) == sf::Socket::Done);

    PartialEngine<false> engine;

    TestClientInterface ClientInterface;

    NetworkHandler Client(NETWORKED_TYPE::Client, &ClientInterface);

    REQUIRE(Client.Init(sf::Socket::AnyPort));

    Connection ClientConnection(sf::IpAddress::LocalHost, socket.getLocalPort());

    ClientConnection.Init(&Client);

    // Throw away the first packet
    sf::Packet received;
    sf::IpAddress sender;
    unsigned short sentport;
        
    REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

    // Construct a packet that has a single message RequestConnect in it
    sf::Packet requestPacket;

    requestPacket << LEVIATHAN_NORMAL_PACKET << uint32_t(1) << uint32_t(0) << uint8_t(1) <<
        NORMAL_REQUEST_TYPE << uint32_t(1);

    {
        RequestConnect requestConnect;
        requestConnect.AddDataToPacket(requestPacket);
    }

    ClientConnection.HandlePacket(requestPacket);

    auto& packetlist = ClientConnection.GetReceivedPackets();

    {
        REQUIRE(!packetlist.empty());
        auto iter = packetlist.find(1);
        REQUIRE(iter != packetlist.end());
        const RECEIVED_STATE state = iter->second;
        CHECK(state == RECEIVED_STATE::AcksSent);
    }

    // We should have gotten a response ResponseConnect //
    REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

    {
        // Also we should have gotten a single packet //
        sf::Packet received;
        sf::IpAddress sender;
        unsigned short sentport;

        CHECK(socket.receive(received, sender, sentport) != sf::Socket::Done);
    }

    CHECK(ClientConnection.GetState() != CONNECTION_STATE::NothingReceived);

    SECTION("Received Response correct format"){

        // First needs to be leviathan identification //
        uint16_t leviathanMagic = 0;

        received >> leviathanMagic;
        REQUIRE(received);
        CHECK(leviathanMagic == LEVIATHAN_NORMAL_PACKET);

        uint32_t packetID;
        received >> packetID;
        REQUIRE(received);

        CHECK(packetID == 2);

        uint32_t startAck;
        received >> startAck;
        REQUIRE(received);

        // Acks for the first thing
        CHECK(startAck == 1);

        uint8_t ackBytes = 0;
        received >> ackBytes;
        REQUIRE(received);

        for(uint8_t i = 0; i < ackBytes; ++i){

            uint8_t ack;
            received >> ack;

            // Should have some bits set //
            CHECK(ack != 0);
            
            REQUIRE(received);
        }

        uint8_t messageCount = 0;
        received >> messageCount;
        REQUIRE(received);

        CHECK(messageCount == 1);

        // Header was correct

        SECTION("Payload is correct"){

            uint8_t messageType;
            received >> messageType;
            REQUIRE(received);

            CHECK(messageType == NORMAL_RESPONSE_TYPE);

            uint32_t messageNumber;
            received >> messageNumber;
            REQUIRE(received);

            CHECK(messageNumber == 2);

            uint16_t responseTypeRaw = 500;

            received >> responseTypeRaw;
            REQUIRE(received);

            CHECK(static_cast<NETWORK_RESPONSE_TYPE>(responseTypeRaw) ==
                NETWORK_RESPONSE_TYPE::Connect);

            // Response to thing //
            uint32_t responseto = 0;
            received >> responseto;
            REQUIRE(received);

            CHECK(responseto == 1);

            // Connect payload //
            int32_t checkValue = 0;

            received >> checkValue;
            REQUIRE(received);

            CHECK(checkValue == 42);

            // It ended //
            uint8_t dummy;
            received >> dummy;
            CHECK(!received);
        }
    }
    
}


