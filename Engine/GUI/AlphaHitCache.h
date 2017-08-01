// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <vector>
#include <memory>
#include <map>


namespace Leviathan{
namespace GUI{

//! \brief Data holder for AlphaHitCache
class AlpaHitStoredTextureData{
public:

    //! \returns The alpha value of pixel at x, y. 0 means fully transparent
    //! \exception InvalidArgument if x or y out of range
    uint8_t GetPixel(uint32_t x, uint32_t y) const;

protected:

    uint32_t Width;
    uint32_t Height;

    std::vector<uint8_t> AlphaValues;
};

//! \brief Stores data for use by widgets that do hit detection based on their image
class AlphaHitCache{
public:

    
    //! \brief Helper for parsing CEGUI image names
    //!
    //! Takes in a raw string pointer to work easier with CEGUI strings
    //! \returns Tuple schema, name
    static std::tuple<std::string, std::string> ParseImageProperty(const char* str,
        size_t strlen);

    //! same as above except automatically determines the length
    static std::tuple<std::string, std::string> ParseImageProperty(const char* str);
    

    //! \brief Helper for finding images matching parsed CEGUI names
    
protected:

    //! Holds all the loaded images. The key is the CEGUI "Image" property,
    //! for example: TaharezLook/ButtonMiddleNormal
    std::map<std::string, std::shared_ptr<AlpaHitStoredTextureData>> LoadedImageData;
};

}
}
