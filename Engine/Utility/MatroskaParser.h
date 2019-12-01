// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <fstream>
#include <optional>
#include <variant>
#include <vector>

namespace Leviathan {

struct EBMLElement;

//! \brief Basic parser for Matroska (.mkv) container for use with GUI::VideoPlayer
//!
//! This parser is written based on the following documents:
//! https://matroska.org/technical/diagram/index.html
//! https://matroska.org/technical/specs/index.html
class MatroskaParser {
public:
    // Codec names
    // Official list is here https://www.matroska.org/technical/specs/codecid/index.html but
    // that is missing many things, like AV1 and opus, so we are using the value ffmpeg
    // generates for them
    static constexpr auto CODEC_TYPE_AV1 = "V_AV1";
    static constexpr auto CODEC_TYPE_VORBIS = "A_VORBIS";
    static constexpr auto CODEC_TYPE_OPUS = "A_OPUS";

    enum class TRACK_TYPE {
        Video = 1,
        Audio = 2,
        Complex = 3,
        Logo = 16,
        Subtitle = 17,
        Buttons = 18,
        Control = 32,
    };

    struct ClusterInfo {
        size_t DataStart = -1;
        size_t Lenght = -1;
        uint64_t Timecode = -1;
    };

    struct TrackInfo {
        struct Video {
            int PixelWidth = -1;
            int PixelHeight = -1;
            int DisplayUnit = 4;
        };

        struct Audio {
            int Channels = 2;
            double SamplingFrequency = 44100.f;
            // Only used for PCM
            int BitDepth = 32;
        };

        int TrackNumber = -1;
        int TrackUID = -1;
        bool Lacing = false;
        std::string Language = "und";
        std::string CodecID = "";
        TRACK_TYPE TrackType = TRACK_TYPE::Video;

        std::variant<std::monostate, Video, Audio> TrackTypeData;

        uint64_t DefaultDuration = 33'333'333;

        std::optional<size_t> CodecPrivateOffset;
        std::optional<size_t> CodecPrivateLength;
    };

    struct ParsedInformation {
        // The top level "Segment" offset
        size_t FirstEBMLElementOffset = -1;

        // Header info
        int EBMLVersion = -1;
        int EBMLReadVersion = -1;
        int EBMLMaxIDLength = -1;
        int EBMLMaxSizeLength = -1;
        std::string DocType = "unknown";
        int DocTypeVersion = -1;
        int DocTypeReadVersion = -1;

        // Info element contents
        int TimecodeScale = 1'000'000;
        std::string MuxinApp = "unknown";
        std::string WritingApp = "unknown";
        double Duration = -1.f;

        // Track information
        int VideoTrackCount = 0;
        int AudioTrackCount = 0;

        std::vector<TrackInfo> Tracks;

        // List of clusters
        std::vector<ClusterInfo> Clusters;
    };

public:
    //! \note Parses the matroska header. Check Good() after construction to see if the file
    //! was valid
    DLLEXPORT MatroskaParser(const std::string& file);

    inline MatroskaParser(const MatroskaParser& other) :
        Error(other.Error), ErrorMessage(other.ErrorMessage), Parsed(other.Parsed),
        File(other.File)
    {
        Reader.open(File, std::ios::binary);
        Reader.seekg(other.Reader.tellg());
    }

    MatroskaParser& operator=(const MatroskaParser& other)
    {
        Error = other.Error;
        ErrorMessage = other.ErrorMessage;
        File = other.File;
        Parsed = other.Parsed;
        Reader.open(File, std::ios::binary);
        Reader.seekg(other.Reader.tellg());
        return *this;
    }

    DLLEXPORT void FindTopLevelElements();

    inline bool Good() const
    {
        return !Error && Reader.good();
    }

    inline const auto& GetErrorMessage() const
    {
        return ErrorMessage;
    }

    inline const auto& GetHeader() const
    {
        return Parsed;
    }

    inline const auto GetTracks() const
    {
        return Parsed.Tracks;
    }

    //! \brief Reads a variable length unsigned integer (in big endian form) into an integer
    //! \param length The length in bytes to read, must be > 0 && < data.length()
    DLLEXPORT static uint64_t ReadVariableLengthUnsignedInteger(
        const std::vector<uint8_t>& data, int length);

    //! \brief Reads a variable length float (in big endian form)
    //! \param length The length in bytes to read, must be > 0 && < data.length()
    DLLEXPORT static double ReadVariableLengthFloat(
        const std::vector<uint8_t>& data, int length);

protected:
    inline void SetError(const std::string& error)
    {
        Error = true;
        if(ErrorMessage.empty()) {
            ErrorMessage = error;
        } else {
            ErrorMessage += "; " + error;
        }
    }

    //! \returns True if can be parsed
    //! \note Reader must be at the start of the list of header data
    DLLEXPORT bool ParseHeaderValues(int headerSizeLeft);

    DLLEXPORT void HandleInfoElement(const EBMLElement& element);
    DLLEXPORT void HandleTracksElement(const EBMLElement& element);
    DLLEXPORT void HandleTrackEntryElement(const EBMLElement& element);
    DLLEXPORT void HandleClusterElement(const EBMLElement& element);

private:
    bool Error = false;
    std::string ErrorMessage;

    //! All parsed data like different section offsets, etc. are stored here to make copying
    //! easier
    ParsedInformation Parsed;

    std::string File;
    //! This is mutable to allow calling tellg when copying
    mutable std::ifstream Reader;
};
} // namespace Leviathan
