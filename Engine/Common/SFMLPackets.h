#pragma once
#ifndef LEVIATHAN_SFMLPACKETS
#define LEVIATHAN_SFMLPACKETS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Types.h"
#include "SFML/Network/Packet.hpp"

//! \file
//! Contains SFML packet includes and common overloaded packet
//! operators for some types.

namespace Leviathan{
    

    // ------------------ Float3 ------------------ //
    DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const Float3& data);

    DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, Float3& data);
    // ------------------ Float4 ------------------ //
    DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const Float4& data);
    
    DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, Float4& data);
}
#endif
