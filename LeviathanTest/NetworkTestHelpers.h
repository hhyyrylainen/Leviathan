// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once

#include "Networking/NetworkClientInterface.h"
#include "Networking/NetworkServerInterface.h"
#include "Networking/NetworkRequest.h"
#include "Networking/Connection.h"
#include "Networking/WireData.h"

#include "PartialEngine.h"

namespace Leviathan{
namespace Test{

class TestClientInterface : public NetworkClientInterface {
public:
    
    virtual void HandleResponseOnlyPacket(std::shared_ptr<NetworkResponse> message, 
        Connection &connection) override 
    {
    }

protected:
    virtual void _OnStartApplicationConnect() override {
    }

};

class TestServerInterface : public NetworkServerInterface {
public:

    TestServerInterface() : NetworkServerInterface(1, "TestServer"){ }
    
    virtual void HandleResponseOnlyPacket(std::shared_ptr<NetworkResponse> message, 
        Connection &connection) override 
    {
    }
};

class GapingConnectionTest : public Connection{
public:

    GapingConnectionTest(unsigned int port) : Connection(sf::IpAddress::LocalHost, port)
    {
        State = CONNECTION_STATE::Authenticated;
    }

};

//! Test fixture for testing client to server messaging
class ConnectionTestFixture{
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


//! Test fixture for testing client sent messages to a raw socket
class ClientConnectionTestFixture{
protected:

    ClientConnectionTestFixture() : 
        Client(NETWORKED_TYPE::Client, &ClientInterface)
    {
        
        RawSocket.setBlocking(false);
        REQUIRE(RawSocket.bind(sf::Socket::AnyPort) == sf::Socket::Done);

        REQUIRE(Client.Init(sf::Socket::AnyPort));

        ClientConnection = std::make_shared<Connection>(
            sf::IpAddress::LocalHost, RawSocket.getLocalPort());

        Client._RegisterConnection(ClientConnection);

        ClientConnection->Init(&Client);

        CHECK(ClientConnection->GetState() == CONNECTION_STATE::NothingReceived);
    }

    //! Does the standard connection opening, after this methods on
    //! the client can be tested to see what packets they send
    //! \note This acts as a test that verifies the sequence of the
    //! packets for opening a connection
    void DoConnectionOpening(){

        sf::Packet packet;

        // Read connect
        REQUIRE(ReadPacket(packet));
        REQUIRE(packet.getDataSize() > 0);

        std::shared_ptr<NetworkRequest> connect;

        WireData::DecodeIncomingData(packet,
            nullptr, nullptr, nullptr,
            [&](uint8_t messagetype, uint32_t messagenumber, sf::Packet &packet)
            -> WireData::DECODE_CALLBACK_RESULT
            {
                switch(messagetype){
                case NORMAL_RESPONSE_TYPE:
                {
                    REQUIRE(false);
                    break;
                }
                case NORMAL_REQUEST_TYPE:
                {
                    REQUIRE_NOTHROW(connect = NetworkRequest::LoadFromPacket(packet,
                            messagenumber));
                    REQUIRE(connect);
                    REQUIRE(connect->GetType() == NETWORK_REQUEST_TYPE::Connect);
                    break;
                }
                default:
                {
                    REQUIRE(false);
                    return WireData::DECODE_CALLBACK_RESULT::Error;
                }
                }

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            });

        REQUIRE(connect);

        // Send response
        WireData::FormatResponseBytes(ResponseConnect(connect->GetIDForResponse()), 1, 1,
            nullptr, packet);
        
        SendPacket(packet);

        RunListeningLoop();

        // Read security
        REQUIRE(ReadPacket(packet));
        REQUIRE(packet.getDataSize() > 0);

        std::shared_ptr<NetworkRequest> security;

        WireData::DecodeIncomingData(packet,
            nullptr, nullptr, nullptr,
            [&](uint8_t messagetype, uint32_t messagenumber, sf::Packet &packet)
            -> WireData::DECODE_CALLBACK_RESULT
            {
                switch(messagetype){
                case NORMAL_RESPONSE_TYPE:
                {
                    REQUIRE(false);
                    break;
                }
                case NORMAL_REQUEST_TYPE:
                {
                    REQUIRE_NOTHROW(security = NetworkRequest::LoadFromPacket(packet,
                            messagenumber));
                    REQUIRE(security);
                    REQUIRE(security->GetType() == NETWORK_REQUEST_TYPE::Security);
                    break;
                }
                default:
                {
                    REQUIRE(false);
                    return WireData::DECODE_CALLBACK_RESULT::Error;
                }
                }

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            });

        // TODO: flag that makes the client only accept unencrypted connections from localhost
        WireData::FormatResponseBytes(ResponseSecurity(security->GetIDForResponse(),
                CONNECTION_ENCRYPTION::None), 2, 2,
            nullptr, packet);

        SendPacket(packet);

        RunListeningLoop();

        // Read authenticate
        REQUIRE(ReadPacket(packet));
        REQUIRE(packet.getDataSize() > 0);

        std::shared_ptr<NetworkRequest> authenticate;

        WireData::DecodeIncomingData(packet,
            nullptr, nullptr, nullptr,
            [&](uint8_t messagetype, uint32_t messagenumber, sf::Packet &packet)
            -> WireData::DECODE_CALLBACK_RESULT
            {
                switch(messagetype){
                case NORMAL_RESPONSE_TYPE:
                {
                    REQUIRE(false);
                    break;
                }
                case NORMAL_REQUEST_TYPE:
                {
                    REQUIRE_NOTHROW(authenticate = NetworkRequest::LoadFromPacket(packet,
                            messagenumber));
                    REQUIRE(authenticate);
                    REQUIRE(authenticate->GetType() == NETWORK_REQUEST_TYPE::Authenticate);
                    break;
                }
                default:
                {
                    REQUIRE(false);
                    return WireData::DECODE_CALLBACK_RESULT::Error;
                }
                }

                return WireData::DECODE_CALLBACK_RESULT::Continue;
            });


        constexpr auto dummyID = 12;
        WireData::FormatResponseBytes(ResponseAuthenticate(security->GetIDForResponse(),
                dummyID), 2, 2, nullptr, packet);

        SendPacket(packet);

        RunListeningLoop();
        
        REQUIRE(ClientConnection->GetState() == CONNECTION_STATE::Authenticated);
    }

    bool ReadPacket(sf::Packet &packet){

        sf::IpAddress sender;
        unsigned short sentport;
        return RawSocket.receive(packet, sender, sentport) == sf::Socket::Done;
    }

    void SendPacket(sf::Packet &packet){

        RawSocket.send(packet, sf::IpAddress::LocalHost, Client.GetOurPort());
    }

    void RunListeningLoop(int times = 1) {
        
        for (int i = 0; i < times; ++i) {

            Client.UpdateAllConnections();
        }
    }

protected:

    PartialEngine<false> engine;

    TestClientInterface ClientInterface;
    NetworkHandler Client;

    std::shared_ptr<Connection> ClientConnection;

    sf::UdpSocket RawSocket;
};

//! Client only with disabled restriction checks
class UDPSocketAndClientFixture {
protected:

    UDPSocketAndClientFixture() : 
        Client(NETWORKED_TYPE::Client, &ClientInterface)
    {
        socket.setBlocking(false);
        REQUIRE(socket.bind(sf::Socket::AnyPort) == sf::Socket::Done);

        REQUIRE(Client.Init(sf::Socket::AnyPort));

        ClientConnection = std::make_shared<GapingConnectionTest>(socket.getLocalPort());

        REQUIRE(ClientConnection);

        Client._RegisterConnection(ClientConnection);

        ClientConnection->Init(&Client);
    }

protected:

    PartialEngine<false> engine;

    // Receiver socket //
    sf::UdpSocket socket;

    TestClientInterface ClientInterface;
    NetworkHandler Client;

    std::shared_ptr<Connection> ClientConnection;
};

// ------------------------------------ //

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


}}
