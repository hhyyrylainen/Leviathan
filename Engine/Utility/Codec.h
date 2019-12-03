// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "MatroskaParser.h"

#include <functional>
#include <memory>

struct aom_image;

namespace Leviathan {

//! \brief Holds decoded data. This is only valid until the frame receive callback ends
struct DecodedFrame {
public:
    struct Image {
        enum class IMAGE_TARGET_FORMAT {
            //! \note Due to little endian shenanigans in memory this is stored as ARGB
            RGBA
        };

        unsigned Width;
        unsigned Height;
        unsigned BitDepth;

        std::function<bool(uint8_t* target, size_t targetlength, IMAGE_TARGET_FORMAT format)>
            ConvertImage;
    };

    struct Sound {
        enum class SOUND_TARGET_FORMAT {
            // Samples are int16_t layout in this order: channel 1 sample 1, channel 2 sample
            // 1, channel 1 sample 2 etc.
            // Requires buffer size to be sizeof(int16_t) * Channels * Samples
            INTERLEAVED_INT16
        };

        int Channels;
        int Samples;

        std::function<bool(uint8_t* target, size_t targetlength, SOUND_TARGET_FORMAT format)>
            ConvertSamples;
    };

public:
    DLLEXPORT DecodedFrame(aom_image* img);
    //! \param pcm Is a multidimensional array where each array is of size samples, channels
    //! tells how many arrays there are in pcm
    DLLEXPORT DecodedFrame(const float* const* pcm, int samples, int channels);

    // //! Raw frame data. Note some formats don't fill these as they are only converted on
    // demand const uint8_t* RawData = nullptr; size_t DataLength = 0;

    std::variant<std::monostate, Image, Sound> TypeSpecificData;
};

//! \brief Base codec interface for all supported video and audio codecs
class Codec {
public:
    using FrameCallback = std::function<bool(const DecodedFrame& frame)>;

public:
    Codec() = default;
    Codec(const Codec& other) = delete;
    Codec& operator=(const Codec& other) = delete;

    //! \brief Sends the raw data frame to the codec
    //! \returns True on success. False if the codec couldn't handle the frame and is now
    //! possibly in broken state
    virtual bool FeedRawFrame(const uint8_t* data, size_t length) = 0;

    //! \brief Returns decoded frames one by one after they are ready
    //!
    //! Call FeedRawFrame and then call this afterwards, there may be more than 1 or no decoded
    //! frames for each raw frame. If the callback returns false the loop for iterating the
    //! finished frames stops before all frames are passed to the callback
    virtual void ReceiveDecodedFrames(FrameCallback callback) = 0;
};

//! \brief AOM AV1 video codec
class AV1Codec : public Codec {
    struct Implementation;

public:
    DLLEXPORT AV1Codec();
    DLLEXPORT ~AV1Codec();

    DLLEXPORT bool FeedRawFrame(const uint8_t* data, size_t length) override;
    DLLEXPORT void ReceiveDecodedFrames(FrameCallback callback) override;

private:
    std::unique_ptr<Implementation> Pimpl;
};

//! \brief Vorbis audio codec
class VorbisCodec : public Codec {
    struct Implementation;

public:
    DLLEXPORT VorbisCodec(const uint8_t* codecprivatedata, size_t datalength);
    DLLEXPORT ~VorbisCodec();

    DLLEXPORT bool FeedRawFrame(const uint8_t* data, size_t length) override;
    DLLEXPORT void ReceiveDecodedFrames(FrameCallback callback) override;


private:
    std::unique_ptr<Implementation> Pimpl;
};

//! \brief Opus audio codec
//! \todo Implement this, currently this has no implementation
class OpusCodec : public Codec {
    struct Implementation;

public:
    DLLEXPORT OpusCodec();
    DLLEXPORT ~OpusCodec();

    // DLLEXPORT bool FeedRawFrame(const uint8_t* data, size_t length) override;
    // DLLEXPORT void ReceiveDecodedFrames(FrameCallback callback) override;

private:
    std::unique_ptr<Implementation> Pimpl;
};

}; // namespace Leviathan
