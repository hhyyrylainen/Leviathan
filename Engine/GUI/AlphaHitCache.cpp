// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
// ------------------------------------ //
#include "AlphaHitCache.h"

#include "Iterators/StringIterator.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //



// ------------------------------------ //
std::tuple<std::string, std::string> AlphaHitCache::ParseImageProperty(const char* str,
    size_t strlen)
{
    StringIterator itr{new UTF8PointerDataIterator(str, str + strlen)};

    std::string schema;
    std::string name;

    return std::make_tuple(schema, name);
}

std::tuple<std::string, std::string> AlphaHitCache::ParseImageProperty(const char* str){

    return ParseImageProperty(str, std::char_traits<char>::length(str));
}


// ------------------------------------ //
// AlpaHitStoredTextureData
uint8_t AlpaHitStoredTextureData::GetPixel(uint32_t x, uint32_t y) const{

    DEBUG_BREAK;
}

