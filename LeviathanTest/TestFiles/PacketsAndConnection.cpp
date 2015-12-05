#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"

#include "PartialEngine.h"

#include "catch.hpp"

using namespace std;
using namespace Leviathan;

void FakeKeepAlive(ConnectionInfo &from, ConnectionInfo &to){

    sf::Packet packet;

    auto data = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000);

    data->GenerateKeepAliveResponse();

    from.CreateFullSendablePacket(data, packet);
    
    to.HandlePacket(packet);
}

TEST_CASE("ConnectionInfo opening and basic acks", "networking"){

    // We cannot test any client or server specific code, so hopefully using master disables those
    PartialEngine<false, NETWORKED_TYPE_MASTER> engine;

    ConnectionInfo client(sf::IpAddress::LocalHost, 256);
    ConnectionInfo server(sf::IpAddress::LocalHost, 255);

    REQUIRE(client.Init());
    REQUIRE(server.Init());

    // Lets fake some packets //
    FakeKeepAlive(client, server);
    
    
}
