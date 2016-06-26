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

TEST_CASE("Ack field filling", "networking"){

    // We cannot test any client or server specific code, so hopefully using master disables those
    PartialEngine<false, NETWORKED_TYPE_MASTER> engine;

    ConnectionInfo conn(sf::IpAddress::LocalHost, 256);


    
}
