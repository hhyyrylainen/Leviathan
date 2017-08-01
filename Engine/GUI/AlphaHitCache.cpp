// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
// ------------------------------------ //
#include "AlphaHitCache.h"

#include "Iterators/StringIterator.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

AlphaHitCache::AlphaHitCache(){

    LEVIATHAN_ASSERT(!StaticInstance, "AlphaHitCache::StaticInstance not null");
    StaticInstance = this;
}

AlphaHitCache::~AlphaHitCache(){

    StaticInstance = nullptr;
}

AlphaHitCache* AlphaHitCache::StaticInstance = nullptr;
// ------------------------------------ //
std::shared_ptr<AlpaHitStoredTextureData> AlphaHitCache::GetDataForImageProperty(
    const std::string &str)
{
    // First return a cached image if one exists //
    auto existing = LoadedImageData.find(str);

    if(existing != LoadedImageData.end()){

        return existing->second;
    }
    
    std::shared_ptr<AlpaHitStoredTextureData> newImage;

    if(!newImage){

        LOG_ERROR("AlpaHitCache: GetDataForImageProperty: failed to load image for "
            "property: '" + str + "'");
        return nullptr;
    }


    // Store it for future calls //
    LoadedImageData.insert(std::make_pair(str, newImage));
    return newImage;
}
// ------------------------------------ //
std::tuple<std::string, std::string> AlphaHitCache::ParseImageProperty(const std::string &str){

    
    // Extract the part of name after /.
    // Here's an example: ThriveGeneric/MenuNormal
    const auto slash = str.find_last_of('/');

    if(slash == std::string::npos)
        return std::make_tuple("", "");

    std::string name = str.substr(slash + 1);
    std::string schema = str.substr(0, slash);

    return std::make_tuple(schema, name);
}

ImageSetSubImage AlphaHitCache::LoadImageAreaFromImageSet(
    const std::tuple<std::string, std::string> &schemaandname)
{
    return {"", 0, 0, 0, 0};
}

// ------------------------------------ //
// AlpaHitStoredTextureData
uint8_t AlpaHitStoredTextureData::GetPixel(uint32_t x, uint32_t y) const{

    DEBUG_BREAK;
    return 0;
}

