// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
// ------------------------------------ //
#include "AlphaHitCache.h"

#include "FileSystem.h"
#include "Iterators/StringIterator.h"

#include "OgreImage.h"
#include "OgreException.h"
#include "OgrePixelBox.h"
#include "OgrePixelFormat.h"

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

AlphaHitCache* AlphaHitCache::Get(){
    return StaticInstance;
}

AlphaHitCache* AlphaHitCache::StaticInstance = nullptr;
// ------------------------------------ //
bool AlphaHitCache::PreLoadImage(const std::string &imageproperty){

    auto loaded = GetDataForImageProperty(imageproperty);

    return loaded.operator bool();
}
// ------------------------------------ //
std::shared_ptr<AlphaHitLoadedTexture> AlphaHitCache::GetImageData(const std::string &name){

    const auto existing = LoadedFullImages.find(name);

    if(existing != LoadedFullImages.end()){

        return existing->second;
    }

    const auto actualFile = FileSystem::Get()->SearchForFile(Leviathan::FILEGROUP_TEXTURE,
        name, "png|jpg|dds");

    if(actualFile.empty() || !FileSystem::FileExists(actualFile)){

        LOG_ERROR("AlphaHitCache: GetImageData: failed to find matching texture "
            "file for: " + name);
        return nullptr;
    }

    Ogre::Image img;
    try{
        img.load(StringOperations::RemovePath<std::string>(actualFile),
            "MainTexturesFolder");
    } catch(const Ogre::Exception &e){

        LOG_ERROR("AlpaHitCache: GetDataForImageProperty: failed to load image file "
            "to Ogre::Image: " +
            StringOperations::RemovePath<std::string>(actualFile) +
            ", exception: " + std::string(e.what()));
        return nullptr;            
    }

    // Convert to only alpha channel //
    auto newImage = std::make_shared<AlphaHitLoadedTexture>(img.getWidth(),
        img.getHeight());

    Ogre::PixelBox dest(newImage->Width, newImage->Height, 0, Ogre::PF_A8,
        newImage->AlphaValues.data());

    try{
        Ogre::PixelUtil::bulkPixelConversion(img.getPixelBox(), dest);
    } catch(const Ogre::Exception &e){

        LOG_ERROR("AlpaHitCache: GetDataForImageProperty: failed to convert Ogre image "
            "to alpha only: " +
            StringOperations::RemovePath<std::string>(actualFile) +
            ", exception: " + std::string(e.what()));
        return nullptr; 
    }

    // newImage->AlphaValues should now have the alpha values of the image

    // Store it for future calls //
    LoadedFullImages.insert(std::make_pair(name, newImage));
    LOG_INFO("AlphaHitCache: GetImageData: loaded new lookup texture: " + actualFile);
    return newImage;
}
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

        const auto imageObject = GetImageData(regionData.ImageFile);

        if(!imageObject){

            LOG_ERROR("AlpaHitCache: GetDataForImageProperty: failed to load image file: " +
                regionData.ImageFile);
            return nullptr;
        }


        try{
            newImage = std::make_shared<AlpaHitStoredTextureData>(regionData.X, regionData.Y,
                regionData.Width, regionData.Height, imageObject);
        } catch(const InvalidArgument &e){

            LOG_ERROR("AlpaHitCache: GetDataForImageProperty: image region is invalid. "
                "Exception: ");
            e.PrintToLog();
            return nullptr;
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

    const auto imageFile = std::get<0>(schemaandname);
    
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
            StringIterator itr;

            // Height
            itr.ReInit(new UTF8PointerDataIterator(line));
            itr.GetUntilCharacterSequence<std::string>("height");

            auto str = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

            if(str)
                height = Convert::StringTo<uint32_t>(*str);

            // Width
            itr.ReInit(new UTF8PointerDataIterator(line));
            itr.GetUntilCharacterSequence<std::string>("width");

            str = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

            if(str)
                width = Convert::StringTo<uint32_t>(*str);

            // X
            itr.ReInit(new UTF8PointerDataIterator(line));
            itr.GetUntilCharacterSequence<std::string>("xPos");

            str = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

            if(str)
                x = Convert::StringTo<uint32_t>(*str);

            // Y
            itr.ReInit(new UTF8PointerDataIterator(line));
            itr.GetUntilCharacterSequence<std::string>("yPos");

            str = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_NONE);

            if(str)
                y = Convert::StringTo<uint32_t>(*str);

            std::stringstream message;
            message << "AlphaHitCache: LoadImageAreaFromImageSet: parsed '" <<
                std::get<0>(schemaandname) << "/" << std::get<1>(schemaandname) <<
                "' image data(" << file << " line " << lineNumber <<
                "): x = " << x << " y = " << y << " width = " << width << " height = " <<
                height;
            LOG_INFO(message.str());

            if(x == static_cast<uint32_t>(-1) || y == static_cast<uint32_t>(-1) ||
                width == static_cast<uint32_t>(-1) || height == static_cast<uint32_t>(-1))
            {
                LOG_ERROR("AlphaHitCache: LoadImageAreaFromImageSet data is invalid!");
                return ImageSetSubImage();
            }
            
            return ImageSetSubImage(imageFile, x, y, width, height);
        }
        
    }

    LOG_ERROR("AlphaHitCache: failed to find image '" + std::get<1>(schemaandname) +
        "' in file: " + file);
    return ImageSetSubImage();
}
// ------------------------------------ //
// AlphaHitLoadedTexture
AlphaHitLoadedTexture::AlphaHitLoadedTexture(uint32_t width, uint32_t height)  :
    Width(width), Height(height)
{
    // sizeof(uint8_t) == 1
    AlphaValues.resize(width * height * 1);
}

// ------------------------------------ //
// AlpaHitStoredTextureData
AlpaHitStoredTextureData::AlpaHitStoredTextureData(uint32_t xoffset, uint32_t yoffset,
    uint32_t width, uint32_t height, const std::shared_ptr<AlphaHitLoadedTexture> &data) :
    Width(width), Height(height), XOffset(xoffset), YOffset(yoffset), TextureData(data)
{
    LEVIATHAN_ASSERT(data, "Null AlphaHitLoadedTexture passed to AlpaHitStoredTextureData");

    if(xoffset + width > data->Width || yoffset + height > data->Height)
        throw InvalidArgument("AlpaHitStoredTextureData subregion is out of range for "
            "the data");
}
// ------------------------------------ //
uint8_t AlpaHitStoredTextureData::GetPixel(uint32_t x, uint32_t y) const{

    if(x >= Width || y >= Height)
        throw InvalidArgument("AlpaHitStoredTextureData: GetPixel: x or y out of range");    

    const auto arrayIndex = ((y + YOffset) * TextureData->Width) + (x + XOffset);

    const auto& values = TextureData->AlphaValues;

    if(arrayIndex >= values.size())
        throw InvalidArgument("AlpaHitStoredTextureData: GetPixel: parent "
            "texture out of range?");

    return values[arrayIndex];
}
// ------------------------------------ //
bool AlpaHitStoredTextureData::HasNonZeroPixels() const{

    const auto& values = TextureData->AlphaValues;
    
    for(uint32_t x = 0; x < Width; ++x){
        for(uint32_t y = 0; y < Height; ++y){

            const auto arrayIndex = ((y + YOffset) * Width) + (x + XOffset);

            if(values[arrayIndex] > 0)
                return true;            
        }
    }
    
    return false;
}

