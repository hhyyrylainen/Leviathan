// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
// ------------------------------------ //
#include "AlphaHitCache.h"

#include "FileSystem.h"
#include "Iterators/StringIterator.h"

#include "OgreImage.h"
#include "OgreException.h"

#include <iostream>

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

    const auto regionData = LoadImageAreaFromImageSet(ParseImageProperty(str));
    
    std::shared_ptr<AlpaHitStoredTextureData> newImage;

    if(!regionData.ImageFile.empty()){

        // The file should have been confirmed to exist, so read it and extract the wanted area
        Ogre::Image img;
        try{
            img.load(StringOperations::RemovePath<std::string>(regionData.ImageFile),
                "MainTexturesFolder");
        } catch(const Ogre::Exception &e){

            LOG_ERROR("AlpaHitCache: GetDataForImageProperty: failed to load image file "
                "to Ogre::Image: " +
                StringOperations::RemovePath<std::string>(regionData.ImageFile) +
                ", exception: " + std::string(e.what()));
            return nullptr;            
        }

        const auto format = img.getFormat();

        switch(format){
        case Ogre::PixelFormat::PF_R8G8B8A8:
        {
            
            
        }
        default:
        {
            LOG_ERROR("AlpaHitCache: GetDataForImageProperty: unknown Ogre image format "
                "for: " + StringOperations::RemovePath<std::string>(regionData.ImageFile) +
                ", format: " + Convert::ToString<int>(format));
            break;
        }
        }
    }

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
    const auto file = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_SCRIPT,
        std::get<0>(schemaandname), "imageset");

    if(file.empty()){

        LOG_ERROR("AlphaHitCache: LoadImageAreaFromImageSet: imageset file for '" +
            std::get<0>(schemaandname) + "' wasn't found");
        return ImageSetSubImage();
    }
    
    std::ifstream reader(file);

    if(!reader.good()){

        LOG_ERROR("AlphaHitCache: failed to read file: " + file);
        return ImageSetSubImage();
    }

    const auto imageFile = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_TEXTURE,
        std::get<0>(schemaandname), "png|jpg|dds");

    if(imageFile.empty() || !FileSystem::FileExists(imageFile)){

        LOG_ERROR("AlphaHitCache: LoadImageAreaFromImageSet: failed to find matching texture "
            "file for: " + file);
        return ImageSetSubImage();
    }

    uint32_t lineNumber = 0;

    while(reader.good()){

        std::string line;
        std::getline(reader, line);
        ++lineNumber;

        if(line.empty())
            continue;

        // Try to find the current image on this line //
        if(line.find(std::get<1>(schemaandname)) != std::string::npos){

            uint32_t x = -1;
            uint32_t y = -1;
            uint32_t width = -1;
            uint32_t height = -1;

            // Parse the data //
            // We could use an xml parser, but meh...
            
            

            std::stringstream message;
            message << "AlphaHitCache: LoadImageAreaFromImageSet: parsed '" <<
                std::get<0>(schemaandname) << "/" << std::get<1>(schemaandname) <<
                "' image data(" << file << " line " << lineNumber <<
                "): x = " << x << " y = " << y << " width = " << width << " height = " <<
                height;
            
            LOG_INFO(message.str());
            return ImageSetSubImage(imageFile, x, y, width, height);
        }
        
    }

    LOG_ERROR("AlphaHitCache: failed to find image '" + std::get<1>(schemaandname) +
        "' in file: " + file);
    return ImageSetSubImage();
}

// ------------------------------------ //
// AlpaHitStoredTextureData
uint8_t AlpaHitStoredTextureData::GetPixel(uint32_t x, uint32_t y) const{

    DEBUG_BREAK;
    return 0;
}

