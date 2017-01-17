#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Types.h"
#include "SFML/Network/Packet.hpp"

//! \file
//! Contains SFML packet includes and common overloaded packet
//! operators for some types.

//! Define when SFML 2.2 or higher is used
//! \todo Make this work
#define SFML_HAS_64_BIT_VALUES_PACKET

namespace Leviathan{
    

// ------------------ Float3 ------------------ //
DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const Float3 &data);

DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, Float3 &data);
// ------------------ Float4 ------------------ //
DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const Float4 &data);
    
DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, Float4 &data);

// ------------------ NamedVariableList ------------------ //
DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, const NamedVariableList &data);
DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, const NamedVariableList &data);

// ------------------ SFML Packet into a packet ------------------ //
DLLEXPORT sf::Packet& operator <<(sf::Packet& packet, sf::Packet& packetinner);
DLLEXPORT sf::Packet& operator >>(sf::Packet& packet, sf::Packet& packetinner);

}

