// ------------------------------------ //
#ifndef LEVIATHAN_SFMLPACKETS
#include "SFMLPackets.h"
#endif
// ------------------------------------ //
namespace Leviathan{

    // ------------------ Float3 ------------------ //
    DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const Float3& data)
    {
        return packet << data.X << data.Y << data.Z;
    }

    DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, Float3& data)
    {
        return packet >> data.X >> data.Y >> data.Z;
    }

    // ------------------ Float4 ------------------ //
    DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const Float4& data)
    {
        return packet << data.X << data.Y << data.Z << data.W;
    }

    DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, Float4& data)
    {
        return packet >> data.X >> data.Y >> data.Z >> data.W;
    }






    
}
