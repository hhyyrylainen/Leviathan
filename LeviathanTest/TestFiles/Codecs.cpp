//! \file Tests for engine supported codecs. Needs the matroska parser to work right

#include "Utility/Codec.h"
#include "Utility/MatroskaParser.h"

#include "../PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

constexpr auto TEST_VIDEO = "Data/Videos/SampleVideo.mkv";

std::tuple<int, int, int, int> GetPixelValueRGBA(
    const std::vector<uint8_t>& image, uint32_t width, uint32_t height, size_t x, size_t y)
{
    return std::make_tuple(image[(y * width * 4) + (x * 4) + 0],
        image[(y * width * 4) + (x * 4) + 1], image[(y * width * 4) + (x * 4) + 2],
        image[(y * width * 4) + (x * 4) + 3]);
}
// ------------------------------------ //
TEST_CASE("AV1 can decode first matroska included frame", "[video]")
{
    TestLogger log;

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    CHECK(parser.Good());
    REQUIRE(parser.GetVideoTrackCount() >= 0);

    const auto videoTrack = parser.GetFirstVideoTrack();

    AV1Codec codec;

    parser.JumpToFirstCluster();

    auto [data, length, opts] = parser.GetNextBlockForTrack(videoTrack.TrackNumber);

    CHECK(length > 0);
    REQUIRE(data);
    CHECK(opts.Timecode == 0);

    CHECK(codec.FeedRawFrame(data, length));

    bool gotFrame = false;

    codec.ReceiveDecodedFrames([&](const DecodedFrame& frame) {
        CHECK(!gotFrame);
        gotFrame = true;

        uint32_t width;
        uint32_t height;

        REQUIRE_NOTHROW(height = std::get<DecodedFrame::Image>(frame.TypeSpecificData).Height);
        REQUIRE_NOTHROW(width = std::get<DecodedFrame::Image>(frame.TypeSpecificData).Width);

        CHECK(height == 1080);
        CHECK(width == 1920);

        std::vector<uint8_t> rgbImage;
        rgbImage.resize(height * width * 4);

        REQUIRE(std::get<DecodedFrame::Image>(frame.TypeSpecificData)
                    .ConvertImage(rgbImage.data(), rgbImage.size(),
                        DecodedFrame::Image::IMAGE_TARGET_FORMAT::RGBA));

        // Check some pixels
        auto pixel = GetPixelValueRGBA(rgbImage, width, height, 1162, 292);
        CHECK(std::get<0>(pixel) == 255);
        CHECK(std::get<1>(pixel) == 255);
        CHECK(std::get<2>(pixel) == 255);
        CHECK(std::get<3>(pixel) == 255);

        pixel = GetPixelValueRGBA(rgbImage, width, height, 649, 353);
        CHECK(std::get<0>(pixel) == 68);
        CHECK(std::get<1>(pixel) == 143); // could also be 142
        CHECK(std::get<2>(pixel) == 120);
        CHECK(std::get<3>(pixel) == 255);

        pixel = GetPixelValueRGBA(rgbImage, width, height, 100, 100);
        CHECK(std::get<0>(pixel) == 0);
        CHECK(std::get<1>(pixel) == 0);
        CHECK(std::get<2>(pixel) == 0);
        CHECK(std::get<3>(pixel) == 255);

        pixel = GetPixelValueRGBA(rgbImage, width, height, 783, 969);
        CHECK(std::get<0>(pixel) == 159);
        CHECK(std::get<1>(pixel) == 77); // could also be 76
        CHECK(std::get<2>(pixel) == 99); // could also be 100
        CHECK(std::get<3>(pixel) == 255);

        // Alpha should be one everywhere
        for(size_t y = 0; y < height; ++y) {
            for(size_t x = 0; x < width; ++x) {
                if(rgbImage[(y * width * 4) + (x * 4) + 3] != 255) {
                    FAIL("alpha not 255 on pixel");
                }
            }
        }

        // Stop after this frame
        return false;
    });

    CHECK(gotFrame);
}

TEST_CASE("Vorbis can decode first matroska included frame", "[video]")
{
    TestLogger log;

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    CHECK(parser.Good());
    REQUIRE(parser.GetAudioTrackCount() >= 0);

    const auto audioTrack = parser.GetFirstAudioTrack();

    const auto codecPrivateData = parser.ReadTrackCodecPrivateData(audioTrack);

    CHECK(codecPrivateData.size() > 0);
    VorbisCodec codec(codecPrivateData.data(), codecPrivateData.size());

    parser.JumpToFirstCluster();

    bool gotFrame = false;

    bool first = true;

    // Differently from the video, this codec needs to be fed blocks until it returns something
    while(!gotFrame) {

        auto [data, length, opts] = parser.GetNextBlockForTrack(audioTrack.TrackNumber);

        if(first) {
            CHECK(length > 0);
            REQUIRE(data);
            CHECK(opts.Timecode == 0);
            first = false;
        }

        if(!data)
            break;

        CHECK(codec.FeedRawFrame(data, length));

        codec.ReceiveDecodedFrames([&](const DecodedFrame& frame) {
            CHECK(!gotFrame);
            gotFrame = true;

            uint32_t samples;
            uint32_t channels;

            REQUIRE_NOTHROW(
                channels = std::get<DecodedFrame::Sound>(frame.TypeSpecificData).Channels);
            REQUIRE_NOTHROW(
                samples = std::get<DecodedFrame::Sound>(frame.TypeSpecificData).Samples);

            CHECK(channels == 2);
            CHECK(samples >= 100);

            std::vector<int16_t> interleavedData;
            interleavedData.resize(samples * channels, -1);

            REQUIRE(std::get<DecodedFrame::Sound>(frame.TypeSpecificData)
                        .ConvertSamples(reinterpret_cast<uint8_t*>(interleavedData.data()),
                            interleavedData.size() * sizeof(int16_t),
                            DecodedFrame::Sound::SOUND_TARGET_FORMAT::INTERLEAVED_INT16));

            // Sound should be 0 (quiet) everywhere
            for(size_t i = 0; i < interleavedData.size(); ++i) {
                if(interleavedData[i] != 0) {
                    FAIL("non-zero decoded audio sample");
                }
            }

            return false;
        });
    }

    CHECK(gotFrame);
}
