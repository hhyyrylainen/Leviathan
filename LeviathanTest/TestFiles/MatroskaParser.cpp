#include "Utility/MatroskaParser.h"

#include "../PartialEngine.h"

#include "catch.hpp"

#include <filesystem>

using namespace Leviathan;
using namespace Leviathan::Test;

constexpr auto TEST_VIDEO = "Data/Videos/SampleVideo.mkv";

TEST_CASE("Matroska test file exists", "[video]")
{
    CHECK(std::filesystem::exists(TEST_VIDEO));
}

TEST_CASE("Matroska header parsing succeeds", "[video]")
{
    TestLogger log;

    REQUIRE(std::filesystem::exists(TEST_VIDEO));

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    CHECK(parser.Good());
    CHECK(parser.GetHeader().DocType == "matroska");
}

TEST_CASE("Matroska info element is parsed correctly", "[video]")
{
    TestLogger log;

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    CHECK(parser.Good());

    const auto& parsed = parser.GetHeader();

    CHECK(parsed.TimecodeScale == 1000000);
    CHECK(parsed.MuxinApp == "Lavf58.20.100");
    CHECK(parsed.WritingApp == "Lavf58.20.100");
    CHECK(parsed.Duration == 10333.f);
}

TEST_CASE("Copying matroska parser preserves info", "[video]")
{
    TestLogger log;

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    REQUIRE(parser.Good());

    SECTION("copy constructor")
    {
        MatroskaParser parser2 = parser;

        CHECK(parser2.Good());
        parser2.FindTopLevelElements();
        INFO(parser2.GetErrorMessage());
        CHECK(parser2.Good());
    }
}

TEST_CASE("Matroska parser finds clusters", "[video]")
{
    TestLogger log;

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    CHECK(parser.Good());

    const auto& parsed = parser.GetHeader();

    REQUIRE(parsed.Clusters.size() == 3);
    CHECK(parsed.Clusters[0].Lenght == 188023);
    CHECK(parsed.Clusters[0].DataStart == 5059);
    CHECK(parsed.Clusters[1].Lenght == 91034);
    CHECK(parsed.Clusters[1].DataStart == 193094);
    CHECK(parsed.Clusters[2].Lenght == 2306);
    CHECK(parsed.Clusters[2].DataStart == 284140);
}

TEST_CASE("Matroska parser correctly handles tracks", "[video]")
{
    TestLogger log;

    MatroskaParser parser(TEST_VIDEO);

    INFO(parser.GetErrorMessage());
    CHECK(parser.Good());

    const auto& parsed = parser.GetHeader();

    CHECK(parsed.VideoTrackCount == 1);
    CHECK(parsed.AudioTrackCount == 1);

    const auto& tracks = parser.GetTracks();
    REQUIRE(tracks.size() == 2);

    auto const* track1 = &tracks[0];
    auto const* track2 = &tracks[1];

    // Swap tracks if they are the wrong way around
    if(track2->TrackType == MatroskaParser::TRACK_TYPE::Video) {
        auto tmp = track1;
        track1 = track2;
        track2 = tmp;
    }

    // Video track info
    CHECK(track1->TrackNumber == 1);
    CHECK(track1->TrackUID == 1);
    CHECK(track1->Lacing == false);
    CHECK(track1->Language == "und");
    CHECK(track1->CodecID == MatroskaParser::CODEC_TYPE_AV1);
    CHECK(track1->TrackType == MatroskaParser::TRACK_TYPE::Video);
    CHECK(track1->DefaultDuration == 33333333);
    CHECK(
        std::get<MatroskaParser::TrackInfo::Video>(track1->TrackTypeData).PixelWidth == 1920);
    CHECK(
        std::get<MatroskaParser::TrackInfo::Video>(track1->TrackTypeData).PixelHeight == 1080);
    CHECK(track1->CodecPrivateLength == 4);
    CHECK(track1->CodecPrivateOffset == 469 + 2 + 1);

    // Audio track info
    CHECK(track2->TrackNumber == 2);
    CHECK(track2->TrackUID == 2);
    CHECK(track2->Lacing == false);
    CHECK(track2->Language == "und");
    CHECK(track2->CodecID == MatroskaParser::CODEC_TYPE_VORBIS);
    CHECK(track2->TrackType == MatroskaParser::TRACK_TYPE::Audio);
    CHECK(std::get<MatroskaParser::TrackInfo::Audio>(track2->TrackTypeData).Channels == 2);
    CHECK(
        std::get<MatroskaParser::TrackInfo::Audio>(track2->TrackTypeData).SamplingFrequency ==
        44100.f);
    CHECK(track2->CodecPrivateLength == 3959);
    CHECK(track2->CodecPrivateOffset == 541 + 2 + 2);
}
