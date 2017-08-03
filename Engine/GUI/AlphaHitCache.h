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

class AlphaHitCache;

//! \brief Data holder for AlphaHitCache
class AlpaHitStoredTextureData{
    friend AlphaHitCache;
public:
    //! \brief Initializes AlphaValues with the correct size for width and height
    //! \protected
    AlpaHitStoredTextureData(uint32_t width, uint32_t height);

    //! \returns The alpha value of pixel at x, y. 0 means fully transparent
    //! \exception InvalidArgument if x or y out of range
    uint8_t GetPixel(uint32_t x, uint32_t y) const;

    //! \brief Verifies that at least one alpha pixel is > 0
    bool HasNonZeroPixels() const;

protected:

    uint32_t Width;
    uint32_t Height;

    std::vector<uint8_t> AlphaValues;
};

//! \brief Region inside a GUI region
struct ImageSetSubImage{
    //! \brief Invalid state constructor
    ImageSetSubImage() :
        ImageFile(""), X(0), Y(0), Width(0), Height(0)
    {
    }

    ImageSetSubImage(const std::string &imagefile, uint32_t x, uint32_t y, uint32_t width,
        uint32_t height) : 
        ImageFile(imagefile), X(x), Y(y), Width(width), Height(height)
    {
    }

    const std::string ImageFile;
    const uint32_t X;
    const uint32_t Y;
    const uint32_t Width;
    const uint32_t Height;
};

//! \brief Stores data for use by widgets that do hit detection based on their image
//! \todo Make sure that each Ogre image would be loaded only once
class AlphaHitCache{
public:

    AlphaHitCache();
    ~AlphaHitCache();


    //! \brief Handles loading image data for the specific Image property
    std::shared_ptr<AlpaHitStoredTextureData> GetDataForImageProperty(const std::string &str);

    
    //! \brief Helper for parsing CEGUI image names
    //!
    //! \returns Tuple schema, name. If the string is not properly formed these can be empty
    static std::tuple<std::string, std::string> ParseImageProperty(const std::string &str);

    //! \brief Helper for finding images matching parsed CEGUI names
    //! \returns The image area. Name and sizes will be empty if it wasn't found
    //! \todo Parse imagefile="TaharezLook.png" in case the iamge is not the same name
    //! as the imageset
    static ImageSetSubImage LoadImageAreaFromImageSet(
        const std::tuple<std::string, std::string> &schemaandname);

    static AlphaHitCache* Get();
    
protected:

    //! Holds all the loaded images. The key is the CEGUI "Image" property,
    //! for example: TaharezLook/ButtonMiddleNormal
    std::map<std::string, std::shared_ptr<AlpaHitStoredTextureData>> LoadedImageData;

    static AlphaHitCache* StaticInstance;
};

}
}
