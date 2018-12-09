#include "Networking/Connection.h"
#include "Networking/NetworkRequest.h"
#include "Networking/NetworkResponse.h"
#include "Networking/SentNetworkThing.h"
#include "Networking/WireData.h"

#include "../PartialEngine.h"

#include "../NetworkTestHelpers.h"

#include "catch.hpp"

/**!
 * @brief \file Tests that check that the \ref networkformat Is followed
 */

using namespace Leviathan;
using namespace Leviathan::Test;


// This is sort of unnecessary with the test "Packet header bytes test"
// But I guess it's fine to test the same thing but directly with WireData...
TEST_CASE("Normal message format directly with WireData", "[networking]")
{
    sf::Packet packet;

    SECTION("Keepalive response")
    {
        WireData::FormatResponseBytes(
            ResponseNone(NETWORK_RESPONSE_TYPE::Keepalive, 0), 11, 1, nullptr, packet);

        SECTION("Response header")
        {
            // First needs to be leviathan identification //
            uint16_t leviathanMagic = 0;

            packet >> leviathanMagic;
            REQUIRE(packet);

            // Normal packet check 0x4C6E (Ln)
            CHECK(0x6E == 'n');
            CHECK(static_cast<int>(leviathanMagic & 0xFF) == 'n');

            CHECK(0x4C == 'L');
            CHECK(static_cast<int>((leviathanMagic & 0xFF00) >> 8) == 'L');

            // ID number. Must be 1
            uint32_t packetID = 5;

            packet >> packetID;
            REQUIRE(packet);

            CHECK(packetID == 1);

            // Acks. Needs to be 0 and the next fields need to be missing
            uint32_t startAck = 5;

            packet >> startAck;
            REQUIRE(packet);

            CHECK(startAck == 0);

            // And then message count which should be 1
            uint8_t messageCount = 0;

            packet >> messageCount;
            REQUIRE(packet);

            CHECK(messageCount == 1);
        }

        uint8_t messageType = 1;

        packet >> messageType;
        REQUIRE(packet);

        CHECK(messageType == NORMAL_RESPONSE_TYPE);

        uint32_t messageNumber = 656;

        packet >> messageNumber;
        REQUIRE(packet);

        CHECK(messageNumber == 11);

        uint16_t responseTypeRaw = 500;

        packet >> responseTypeRaw;
        REQUIRE(packet);

        CHECK(static_cast<NETWORK_RESPONSE_TYPE>(responseTypeRaw) ==
              NETWORK_RESPONSE_TYPE::Keepalive);
    }

    SECTION("Echo request")
    {
        WireData::FormatRequestBytes(
            RequestNone(NETWORK_REQUEST_TYPE::Echo), 12, 1, nullptr, packet);

        SECTION("Request header")
        {
            // First needs to be leviathan identification //
            uint16_t leviathanMagic = 0;

            packet >> leviathanMagic;
            REQUIRE(packet);

            // Normal packet check 0x4C6E (Ln)
            CHECK(0x6E == 'n');
            CHECK(static_cast<int>(leviathanMagic & 0xFF) == 'n');

            CHECK(0x4C == 'L');
            CHECK(static_cast<int>((leviathanMagic & 0xFF00) >> 8) == 'L');

            // ID number. Must be 1
            uint32_t packetID = 5;

            packet >> packetID;
            REQUIRE(packet);

            CHECK(packetID == 1);

            // Acks. Needs to be 0 and the next fields need to be missing
            uint32_t startAck = 5;

            packet >> startAck;
            REQUIRE(packet);

            CHECK(startAck == 0);

            // And then message count which should be 1
            uint8_t messageCount = 0;

            packet >> messageCount;
            REQUIRE(packet);

            CHECK(messageCount == 1);
        }

        uint8_t messageType = 1;

        packet >> messageType;
        REQUIRE(packet);

        CHECK(messageType == NORMAL_REQUEST_TYPE);

        uint32_t messageNumber = 656;

        packet >> messageNumber;
        REQUIRE(packet);

        CHECK(messageNumber == 12);

        uint16_t responseTypeRaw = 500;

        packet >> responseTypeRaw;
        REQUIRE(packet);

        CHECK(
            static_cast<NETWORK_REQUEST_TYPE>(responseTypeRaw) == NETWORK_REQUEST_TYPE::Echo);
    }
}


TEST_CASE_METHOD(UDPSocketAndClientFixture, "Packet header bytes test", "[networking]")
{
    SECTION("Header format")
    {
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

        CHECK(packetID == PACKET_NUMBERING_OFFSET + 1);

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

        SECTION("Hello packet payload")
        {
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

            SECTION("Data size is correct")
            {
                // Consume all the data in the Connect request and then verify that the
                // packet is empty
                for(size_t i = 0; i < connectSize; ++i) {

                    uint8_t dummy;
                    received >> dummy;
                    INFO("On loop: " + std::to_string(i));
                    REQUIRE(received);
                }

                uint8_t invalidread;
                received >> invalidread;
                CHECK(!received);
            }

            SECTION("Connect Request payload is correct")
            {
                int32_t checkValue = 0;

                received >> checkValue;
                REQUIRE(received);

                CHECK(checkValue == 42);
            }
        }
    }
}

TEST_CASE("Packet serialization and deserialization", "[networking]")
{
    PartialEngine<false> reporter;

    // Requests //
    SECTION("Connect basic data")
    {
        sf::Packet packet;

        RequestConnect request;

        request.AddDataToPacket(packet);

        auto loaded = NetworkRequest::LoadFromPacket(packet, 0);

        REQUIRE(loaded);
        REQUIRE(loaded->GetType() == NETWORK_REQUEST_TYPE::Connect);

        auto* deserialized = static_cast<RequestConnect*>(loaded.get());

        CHECK(deserialized->CheckValue == request.CheckValue);
    }

    SECTION("RequestSecurity")
    {
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

    SECTION("RequestAuthenticate")
    {
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
    SECTION("ResponseAuthenticate")
    {
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

    SECTION("ResponseSecurity")
    {
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

TEST_CASE("Packet serialization and then deserialization with WireData", "[networking]")
{
    sf::Packet packet;

    SECTION("ResponseConnect packet with no acks")
    {
        ResponseConnect response(2);

        WireData::FormatResponseBytes(response, 2, 1, nullptr, packet);

        REQUIRE(packet.getDataSize() > 0);

        bool received = false;

        WireData::DecodeIncomingData(packet,
            [&](NetworkAckField& acks) -> WireData::DECODE_CALLBACK_RESULT {
                CHECK(acks.FirstPacketID == 0);
                CHECK(acks.Acks.size() == 0);
                return WireData::DECODE_CALLBACK_RESULT::Continue;
            },
            [&](uint32_t ack) -> void { CHECK(false); },
            [&](uint32_t packetnumber) -> WireData::DECODE_CALLBACK_RESULT {
                CHECK(!received);

                CHECK(packetnumber == 1);

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            },
            [&](uint8_t messagetype, uint32_t messagenumber,
                sf::Packet& packet) -> WireData::DECODE_CALLBACK_RESULT {
                CHECK(!received);

                CHECK(messagenumber == 2);

                CHECK(messagetype == NORMAL_RESPONSE_TYPE);

                std::shared_ptr<NetworkResponse> decodedResponse;
                CHECK_NOTHROW(decodedResponse = NetworkResponse::LoadFromPacket(packet));

                REQUIRE(decodedResponse);
                REQUIRE(decodedResponse->GetType() == NETWORK_RESPONSE_TYPE::Connect);

                CHECK(response.CheckValue ==
                      static_cast<ResponseConnect*>(decodedResponse.get())->CheckValue);

                received = true;

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            });

        CHECK(received);
    }

    SECTION("ResponseKeepAlive packet with one byte fitting acks")
    {
        ResponseNone response(NETWORK_RESPONSE_TYPE::Keepalive);

        NetworkAckField::PacketReceiveStatus ackStatus;

        const std::vector<uint32_t> expectedAcks = {1, 2, 5, 6};

        for(auto ack : expectedAcks)
            ackStatus[ack] = RECEIVED_STATE::StateReceived;


        NetworkAckField sentAcks(1, 20, ackStatus);

        WireData::FormatResponseBytes(response, 8, 4, &sentAcks, packet);

        REQUIRE(packet.getDataSize() > 0);

        bool received = false;

        WireData::DecodeIncomingData(packet,
            [&](NetworkAckField& acks) -> WireData::DECODE_CALLBACK_RESULT {
                CHECK(acks.FirstPacketID == 1);
                // Fits in one byte
                CHECK(acks.Acks.size() == 1);

                std::vector<uint32_t> receivedAcks;

                acks.InvokeForEachAck([&](uint32_t ack) { receivedAcks.push_back(ack); });

                CHECK(receivedAcks == expectedAcks);

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            },
            [&](uint32_t ack) -> void { CHECK(false); },
            [&](uint32_t packetnumber) -> WireData::DECODE_CALLBACK_RESULT {
                CHECK(!received);

                CHECK(packetnumber == 4);

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            },
            [&](uint8_t messagetype, uint32_t messagenumber,
                sf::Packet& packet) -> WireData::DECODE_CALLBACK_RESULT {
                CHECK(!received);

                CHECK(messagenumber == 8);

                CHECK(messagetype == NORMAL_RESPONSE_TYPE);

                std::shared_ptr<NetworkResponse> decodedResponse;
                CHECK_NOTHROW(decodedResponse = NetworkResponse::LoadFromPacket(packet));

                REQUIRE(decodedResponse);
                REQUIRE(decodedResponse->GetType() == NETWORK_RESPONSE_TYPE::Keepalive);

                received = true;

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            });

        CHECK(received);
    }
}


TEST_CASE("Ack field filling", "[networking]")
{

    SECTION("Invoking on each set")
    {
        std::vector<uint32_t> ids{1, 5, 6, 7, 12};

        NetworkAckField::PacketReceiveStatus packetsreceived;

        for(auto id : ids)
            packetsreceived[id] = RECEIVED_STATE::StateReceived;

        NetworkAckField acks(1, 32, packetsreceived);

        std::vector<uint32_t> invoked;

        acks.InvokeForEachAck([&](uint32_t id) { invoked.push_back(id); });

        CHECK(ids == invoked);
    }

    SECTION("Basic construction")
    {
        SECTION("Single Byte")
        {
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

        SECTION("Multiple Bytes")
        {
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

    SECTION("Empty field to packet has no length value")
    {
        NetworkAckField::PacketReceiveStatus first;
        first[1] = RECEIVED_STATE::NotReceived;

        NetworkAckField tosend(1, 32, first);

        CHECK(tosend.FirstPacketID == 0);
        CHECK(tosend.Acks.size() == 0);

        sf::Packet packet;

        tosend.AddDataToPacket(packet);

        CHECK(packet.getDataSize() == sizeof(uint32_t));

        SECTION("Loading")
        {
            NetworkAckField unserialized(packet);

            CHECK(packet);

            CHECK(unserialized.FirstPacketID == 0);
        }
    }

    SECTION("Direct manipulation")
    {
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

    SECTION("Serialize size test")
    {
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

        SECTION("Three bytes")
        {

            first[12] = RECEIVED_STATE::StateReceived;
            first[18] = RECEIVED_STATE::StateReceived;

            NetworkAckField tosend(1, 32, first);

            sf::Packet packet;

            tosend.AddDataToPacket(packet);

            CHECK(packet.getDataSize() ==
                  sizeof(uint32_t) + sizeof(uint8_t) + (sizeof(uint8_t) * 3));
        }
    }
}

class AckFillConnectionTest : public Connection {
public:
    AckFillConnectionTest() : Connection(sf::IpAddress::LocalHost, 33030) {}

    void FillJustAcks(sf::Packet& tofill)
    {
        const auto fullpacketid = ++LastUsedLocalID;
        auto acks = _GetAcksToSend(fullpacketid);

        WireData::FillHeaderAckData(acks.get(), tofill);
    }

    void SetPacketReceived(uint32_t packetid, RECEIVED_STATE state)
    {
        ReceivedRemotePackets[packetid] = state;
    }

    void SetDone(uint32_t packetid)
    {
        auto iter = ReceivedRemotePackets.find(packetid);

        if(iter != ReceivedRemotePackets.end())
            ReceivedRemotePackets.erase(iter);
    }
};

TEST_CASE("Creating Acks in headers", "[networking]")
{
    AckFillConnectionTest connection;
    auto& packetlist = connection.GetReceivedPackets();

    sf::Packet received;

    SECTION("No packets")
    {
        connection.FillJustAcks(received);

        CHECK(received.getDataSize() == sizeof(uint32_t));

        uint32_t count = -1;
        received >> count;
        REQUIRE(received);

        CHECK(count == 0);
    }

    SECTION("1 packet")
    {
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

        SECTION("Ack is marked as sent")
        {
            const auto sentstuff = connection.GetCurrentlySentAcks();

            CHECK(std::find(sentstuff.begin(), sentstuff.end(), 1) != sentstuff.end());
        }
    }

    SECTION("2 packets")
    {
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

        SECTION("Ack is marked as sent")
        {

            const auto sentstuff = connection.GetCurrentlySentAcks();

            CHECK(std::find(sentstuff.begin(), sentstuff.end(), 1) != sentstuff.end());
            CHECK(std::find(sentstuff.begin(), sentstuff.end(), 2) != sentstuff.end());
            CHECK(std::find(sentstuff.begin(), sentstuff.end(), 3) == sentstuff.end());
        }
    }

    SECTION("Extra bytes")
    {
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

    SECTION("Back acks")
    {
        for(int i = 2; i < DEFAULT_ACKCOUNT * 2; ++i)
            connection.SetPacketReceived(i, RECEIVED_STATE::StateReceived);

        for(int i = 1; i < 11; ++i)
            connection.SetPacketReceived(
                i + DEFAULT_ACKCOUNT * 5, RECEIVED_STATE::StateReceived);

        // All of the acks cannot fit into a single packet
        connection.FillJustAcks(received);

        CHECK(received.getDataSize() ==
              sizeof(uint32_t) + sizeof(uint8_t) + (DEFAULT_ACKCOUNT / 8));

        uint32_t startID = -1;
        received >> startID;
        REQUIRE(received);

        CHECK(startID == 2);


        uint8_t count = -1;

        received >> count;
        REQUIRE(received);

        CHECK(count == DEFAULT_ACKCOUNT / 8);


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


TEST_CASE_METHOD(
    UDPSocketAndClientFixture, "Manually constructed packet decoding", "[networking]")
{
    // Throw away the first packet
    sf::Packet received;
    sf::IpAddress sender;
    unsigned short sentport;

    REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

    // Construct a packet that has a single message RequestConnect in it
    sf::Packet requestPacket;

    requestPacket << LEVIATHAN_NORMAL_PACKET << uint32_t(1) << uint32_t(0) << uint8_t(1)
                  << NORMAL_REQUEST_TYPE << uint32_t(1);

    {
        RequestConnect requestConnect;
        requestConnect.AddDataToPacket(requestPacket);
    }

    ClientConnection->HandlePacket(requestPacket);

    auto& packetlist = ClientConnection->GetReceivedPackets();

    {
        auto iter = packetlist.find(1);
        REQUIRE(iter != packetlist.end());
        CHECK(iter->second == RECEIVED_STATE::StateReceived);

        const auto sentstuff = ClientConnection->GetCurrentlySentAcks();

        CHECK(std::find(sentstuff.begin(), sentstuff.end(), 1) != sentstuff.end());
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

    CHECK(ClientConnection->GetState() != CONNECTION_STATE::NothingReceived);

    SECTION("Received Response correct format")
    {
        // First needs to be leviathan identification //
        uint16_t leviathanMagic = 0;

        received >> leviathanMagic;
        REQUIRE(received);
        CHECK(leviathanMagic == LEVIATHAN_NORMAL_PACKET);

        uint32_t packetID;
        received >> packetID;
        REQUIRE(received);

        CHECK(packetID == PACKET_NUMBERING_OFFSET + 2);

        uint32_t startAck;
        received >> startAck;
        REQUIRE(received);

        // Acks for the first thing
        CHECK(startAck == 1);

        uint8_t ackBytes = 0;
        received >> ackBytes;
        REQUIRE(received);

        for(uint8_t i = 0; i < ackBytes; ++i) {

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

        SECTION("Payload is correct")
        {

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


TEST_CASE("Finalize callback works", "[networking]")
{
    auto response = std::make_shared<SentResponse>(
        1, 1, RECEIVE_GUARANTEE::Critical, std::make_shared<ResponseConnect>(0));

    bool successSet = false;
    bool callbackCalled = false;

    response->SetCallbackFunc([&](bool success, SentNetworkThing& thing) {
        callbackCalled = true;
        successSet = success;
    });

    SECTION("Success")
    {
        response->OnFinalized(true);
        CHECK(callbackCalled);
        CHECK(successSet);
    }

    SECTION("Failure")
    {
        response->OnFinalized(false);
        CHECK(callbackCalled);
        CHECK(!successSet);
    }
}

TEST_CASE_METHOD(UDPSocketAndClientFixture, "Client Requests get completed", "[networking]")
{
    auto request = std::make_shared<RequestNone>(NETWORK_REQUEST_TYPE::Echo);

    auto sent = ClientConnection->SendPacketToConnection(request, RECEIVE_GUARANTEE::Critical);

    bool successSet = false;
    bool callbackCalled = false;

    sent->SetCallbackFunc([&](bool success, SentNetworkThing& thing) {
        callbackCalled = true;
        successSet = success;
    });

    // Create packet //
    sf::Packet response;

    response << LEVIATHAN_NORMAL_PACKET << uint32_t(1) << uint32_t(0) << uint8_t(0x1)
             << NORMAL_RESPONSE_TYPE << uint32_t(1);

    {
        ResponseServerAllow responseAllow(
            sent->MessageNumber, SERVER_ACCEPTED_TYPE::RequestQueued, "response");
        responseAllow.AddDataToPacket(response);
    }

    REQUIRE(socket.send(response, sf::IpAddress::LocalHost, Client.GetOurPort()) ==
            sf::Socket::Done);

    Client.UpdateAllConnections();

    CHECK(callbackCalled);
    CHECK(successSet);
}

TEST_CASE_METHOD(
    UDPSocketAndClientFixture, "Resend shouldn't happen inside a test", "[networking]")
{
    auto response = std::make_shared<ResponseNone>(NETWORK_RESPONSE_TYPE::Keepalive);

    auto sent =
        ClientConnection->SendPacketToConnection(response, RECEIVE_GUARANTEE::Critical);

    const auto inPacket = sent->PacketNumber;

    CHECK(inPacket == PACKET_NUMBERING_OFFSET + 2);

    sf::Packet received;

    sf::IpAddress sender;
    unsigned short sentport;

    // Connect request
    REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

    // Response
    REQUIRE(socket.receive(received, sender, sentport) == sf::Socket::Done);

    // Shouldn't be a resend
    REQUIRE(socket.receive(received, sender, sentport) != sf::Socket::Done);

    // Even after running update once
    Client.UpdateAllConnections();

    REQUIRE(socket.receive(received, sender, sentport) != sf::Socket::Done);

    CHECK(inPacket == sent->PacketNumber);
}

TEST_CASE_METHOD(UDPSocketAndClientFixture, "Client Responses get acks", "[networking]")
{
    auto response = std::make_shared<ResponseNone>(NETWORK_RESPONSE_TYPE::Keepalive);

    auto sent =
        ClientConnection->SendPacketToConnection(response, RECEIVE_GUARANTEE::Critical);

    const auto inPacket = sent->PacketNumber;

    CHECK(inPacket == PACKET_NUMBERING_OFFSET + 2);

    bool successSet = false;
    bool callbackCalled = false;

    sent->SetCallbackFunc([&](bool success, SentNetworkThing& thing) {
        callbackCalled = true;
        successSet = success;
    });

    // Create packet //
    sf::Packet ackPacket;

    SECTION("From normal packet")
    {

        SECTION("Packet object")
        {

            NetworkAckField::PacketReceiveStatus fakeReceived;
            fakeReceived[inPacket] = RECEIVED_STATE::StateReceived;

            NetworkAckField tosend(PACKET_NUMBERING_OFFSET + 1, 32, fakeReceived);

            ackPacket << LEVIATHAN_NORMAL_PACKET << uint32_t(1);
            // Ack header start
            tosend.AddDataToPacket(ackPacket);

            // No messages
            ackPacket << uint8_t(0);
        }

        SECTION("Manual bytes")
        {
            ackPacket << LEVIATHAN_NORMAL_PACKET
                      << uint32_t(1)
                      // Ack header start
                      << uint32_t(PACKET_NUMBERING_OFFSET + 2) << uint8_t(1) << uint8_t(0x1) <<
                // No messages
                uint8_t(0);
        }
    }

    SECTION("Ack only packet")
    {
        ackPacket << LEVIATHAN_ACK_PACKET << uint8_t(1) << inPacket;
    }


    REQUIRE(socket.send(ackPacket, sf::IpAddress::LocalHost, Client.GetOurPort()) ==
            sf::Socket::Done);

    Client.UpdateAllConnections();

    CHECK(inPacket == sent->PacketNumber);

    CHECK(callbackCalled);
    CHECK(successSet);
}
