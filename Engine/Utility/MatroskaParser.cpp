// ------------------------------------ //
#include "MatroskaParser.h"

#include "Convert.h"
#include "Exceptions.h"

#include "boost/endian/conversion.hpp"

using namespace Leviathan;
// ------------------------------------ //
// Constants for matroska parsing
// Official reference: https://matroska.org/technical/specs/index.html

// Header values
constexpr uint32_t EBML_MAGIC = 0x1A45DFA3;
constexpr uint64_t EBML_FIELD_IDENTIFIER_VERSION = 0x4286;
constexpr uint64_t EBML_FIELD_IDENTIFIER_READ_VERSION = 0x42F7;
constexpr uint64_t EBML_FIELD_IDENTIFIER_MAX_ID_LENGTH = 0x42F2;
constexpr uint64_t EBML_FIELD_IDENTIFIER_MAX_SIZE_LENGTH = 0x42F3;
constexpr uint64_t EBML_FIELD_IDENTIFIER_DOCTYPE = 0x4282;
constexpr uint64_t EBML_FIELD_IDENTIFIER_DOCTYPE_VERSION = 0x4287;
constexpr uint64_t EBML_FIELD_IDENTIFIER_DOCTYPE_READ_VERSION = 0x4285;
// L1 elements
constexpr uint64_t ELEMENT_TYPE_SEGMENT = 0x18538067;
// L2 elements
constexpr uint64_t ELEMENT_TYPE_SEEK_HEAD = 0x114D9B74;
constexpr uint64_t ELEMENT_TYPE_VOID = 0xEC;
constexpr uint64_t ELEMENT_TYPE_INFO = 0x1549A966;
constexpr uint64_t ELEMENT_TYPE_TRACKS = 0x1654AE6B;
constexpr uint64_t ELEMENT_TYPE_TAGS = 0x1254C367;
constexpr uint64_t ELEMENT_TYPE_CLUSTER = 0x1F43B675;
constexpr uint64_t ELEMENT_TYPE_CUES = 0x1C53BB6B;

// L3 elements
// Info children
constexpr uint64_t ELEMENT_TYPE_TIMECODE_SCALE = 0x2AD7B1;
constexpr uint64_t ELEMENT_TYPE_MUXIN_APP = 0x4D80;
constexpr uint64_t ELEMENT_TYPE_WRITING_APP = 0x5741;
constexpr uint64_t ELEMENT_TYPE_DURATION = 0x4489;
// Tracks children
constexpr uint64_t ELEMENT_TYPE_TRACK_ENTRY = 0xAE;
// General
constexpr uint64_t ELEMENT_TYPE_CRC_32 = 0xBF;
constexpr uint64_t ELEMENT_TYPE_TIMECODE = 0xE7;
constexpr uint64_t ELEMENT_TYPE_SIMPLE_BLOCK = 0xA3;

// L4 elements
// Track entry children
constexpr uint64_t ELEMENT_TYPE_TRACK_NUMBER = 0xD7;
constexpr uint64_t ELEMENT_TYPE_TRACK_UID = 0x73C5;
constexpr uint64_t ELEMENT_TYPE_FLAG_LACING = 0x9C;
constexpr uint64_t ELEMENT_TYPE_LANGUAGE = 0x22B59C;
constexpr uint64_t ELEMENT_TYPE_CODEC_ID = 0x86;
constexpr uint64_t ELEMENT_TYPE_TRACK_TYPE = 0x83;
constexpr uint64_t ELEMENT_TYPE_DEFAULT_DURATION = 0x23E383;
constexpr uint64_t ELEMENT_TYPE_VIDEO = 0xE0;
constexpr uint64_t ELEMENT_TYPE_AUDIO = 0xE1;
constexpr uint64_t ELEMENT_TYPE_CODEC_PRIVATE = 0x63A2;

// L5 elements
// Video element children
constexpr uint64_t ELEMENT_TYPE_PIXEL_WIDTH = 0xB0;
constexpr uint64_t ELEMENT_TYPE_PIXEL_HEIGHT = 0xBA;
constexpr uint64_t ELEMENT_TYPE_DISPLAY_UNIT = 0x54B2;
constexpr uint64_t ELEMENT_TYPE_COLOUR = 0x55B0;
// Audio element children
constexpr uint64_t ELEMENT_TYPE_CHANNELS = 0x9F;
constexpr uint64_t ELEMENT_TYPE_SAMPLING_FREQUENCY = 0xB5;
constexpr uint64_t ELEMENT_TYPE_BIT_DEPTH = 0x6264;

// Possible ELEMENT_TYPE_TRACK_TYPE values
constexpr uint64_t EBML_TRACK_TYPE_VIDEO = 1;
constexpr uint64_t EBML_TRACK_TYPE_AUDIO = 2;
constexpr uint64_t EBML_TRACK_TYPE_COMPLEX = 3;
constexpr uint64_t EBML_TRACK_TYPE_LOGO = 16;
constexpr uint64_t EBML_TRACK_TYPE_SUBTITLE = 17;
constexpr uint64_t EBML_TRACK_TYPE_BUTTONS = 18;
constexpr uint64_t EBML_TRACK_TYPE_CONTROL = 32;


// Variable length parsing constants
constexpr uint8_t EBML_LENGTH_ONE_BYTES = 0x80;
constexpr uint8_t EBML_LENGTH_ONE_MASK = EBML_LENGTH_ONE_BYTES - 1;
constexpr uint8_t EBML_LENGTH_TWO_BYTES = 0x40;
constexpr uint8_t EBML_LENGTH_TWO_MASK = EBML_LENGTH_TWO_BYTES - 1;
constexpr uint8_t EBML_LENGTH_THREE_BYTES = 0x20;
constexpr uint8_t EBML_LENGTH_THREE_MASK = EBML_LENGTH_THREE_BYTES - 1;
constexpr uint8_t EBML_LENGTH_FOUR_BYTES = 0x10;
constexpr uint8_t EBML_LENGTH_FOUR_MASK = EBML_LENGTH_FOUR_BYTES - 1;
constexpr uint8_t EBML_LENGTH_FIVE_BYTES = 0x8;
constexpr uint8_t EBML_LENGTH_FIVE_MASK = EBML_LENGTH_FIVE_BYTES - 1;
constexpr uint8_t EBML_LENGTH_SIX_BYTES = 0x4;
constexpr uint8_t EBML_LENGTH_SIX_MASK = EBML_LENGTH_SIX_BYTES - 1;
constexpr uint8_t EBML_LENGTH_SEVEN_BYTES = 0x2;
constexpr uint8_t EBML_LENGTH_SEVEN_MASK = EBML_LENGTH_SEVEN_BYTES - 1;
constexpr uint8_t EBML_LENGTH_EIGHT_BYTES = 0x1;
constexpr uint8_t EBML_LENGTH_EIGHT_MASK = EBML_LENGTH_EIGHT_BYTES - 1;

// Variable length parsing helper macros for shorter code
#define CHECK_BYTE_COUNT(countname, count)                                      \
    if(byte & EBML_LENGTH_##countname##_BYTES) {                                \
        ReadNextBytes(stream, byte& EBML_LENGTH_##countname##_MASK, count - 1); \
        return;                                                                 \
    }

#define CHECK_BYTE_COUNT_ID(countname, count)    \
    if(byte & EBML_LENGTH_##countname##_BYTES) { \
        ReadNextBytes(stream, byte, count - 1);  \
        return;                                  \
    }

// ------------------------------------ //
namespace Leviathan {
struct CommonVariableLengthValue {
protected:
    template<class T>
    void ReadNextBytes(T& stream, int firstbyte, int count)
    {
        ByteLength = count + 1;
        Value = static_cast<uint64_t>(firstbyte) << (count * BITS_IN_BYTE);

        for(; count > 0; --count) {
            int byte = stream.get();
            Value += static_cast<uint64_t>(byte) << ((count - 1) * BITS_IN_BYTE);
        }
    }

public:
    //! The contents of this value
    uint64_t Value;

    //! The length in bytes that this value took in the file
    int ByteLength = 1;
};

//! Decodes a length encoded in EBML
struct EBMLLengthValue : public CommonVariableLengthValue {
public:
    //! Read from stream
    template<class T>
    EBMLLengthValue(T& stream)
    {
        int byte = 0;
        byte = stream.get();

        CHECK_BYTE_COUNT(ONE, 1);
        CHECK_BYTE_COUNT(TWO, 2);
        CHECK_BYTE_COUNT(THREE, 3);
        CHECK_BYTE_COUNT(FOUR, 4);
        CHECK_BYTE_COUNT(FIVE, 5);
        CHECK_BYTE_COUNT(SIX, 6);
        CHECK_BYTE_COUNT(SEVEN, 7);
        CHECK_BYTE_COUNT(EIGHT, 8);

        throw InvalidState(
            "MatroskaParser: EBML variable length, length field could not be handled, "
            "first byte: " +
            std::to_string(byte));
    }
};

//! Decodes an indentifier encoded in EBML
struct EBMLLIdentifierValue : public CommonVariableLengthValue {
public:
    //! Read from stream
    template<class T>
    EBMLLIdentifierValue(T& stream)
    {
        int byte = 0;
        byte = stream.get();

        CHECK_BYTE_COUNT_ID(ONE, 1);
        CHECK_BYTE_COUNT_ID(TWO, 2);
        CHECK_BYTE_COUNT_ID(THREE, 3);
        CHECK_BYTE_COUNT_ID(FOUR, 4);

        throw InvalidState(
            "MatroskaParser: EBML identifier could not be handled, first byte: " +
            std::to_string(byte));
    }
};

//! A basic EBML element
struct EBMLElement {
public:
    //! Read from stream
    template<class T>
    EBMLElement(T& stream) : Identifier(stream), Length(stream)
    {
        if(stream.good())
            DataStart = stream.tellg();
    }

    template<class T, class Callback>
    int ReadChildElements(T& stream, Callback callback) const
    {
        stream.seekg(DataStart);

        if(!stream.good())
            return -1;

        const auto dataEnd = DataStart + Length.Value;
        int foundCount = 0;

        std::vector<uint8_t> dataBuffer;

        while(true) {

            EBMLElement childElement(stream);

            if(!stream.good())
                return -1;

            auto readData = [&]() -> std::vector<uint8_t>& {
                dataBuffer.resize(childElement.Length.Value);
                stream.seekg(childElement.DataStart);
                stream.read(reinterpret_cast<char*>(dataBuffer.data()), dataBuffer.size());

                return dataBuffer;
            };

            const bool continueLoop = callback(childElement, readData);

            ++foundCount;

            // Jump past this element
            const auto jumpPos = childElement.DataStart + childElement.Length.Value;

            if(jumpPos >= dataEnd)
                break;

            stream.seekg(jumpPos);

            if(!continueLoop)
                break;
        }

        return foundCount;
    }

    EBMLLIdentifierValue Identifier;
    EBMLLengthValue Length;
    size_t DataStart = -1;
};
} // namespace Leviathan
// ------------------------------------ //
// MatroskaParser
DLLEXPORT MatroskaParser::MatroskaParser(const std::string& file) :
    File(file), Reader(File, std::ios::binary)
{
    if(!Reader.good()) {
        SetError("File can't be read");
        return;
    }

    // EBML header
    uint32_t magic;
    Reader.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    magic = boost::endian::big_to_native(magic);

    if(magic != 0x1A45DFA3) {
        SetError("wrong magic, got: " + Convert::ToHexadecimalString(magic) +
                 " expected: " + Convert::ToHexadecimalString(EBML_MAGIC));
        return;
    }

    EBMLLengthValue headerLength(Reader);

    if(!Reader.good()) {
        SetError("reading header length failed");
        return;
    }

    LOG_INFO("Reading matroska file: " + file +
             ", magic is correct, header length: " + std::to_string(headerLength.Value));

    Parsed.FirstEBMLElementOffset =
        sizeof(magic) + headerLength.ByteLength + headerLength.Value;

    // Parse the rest of the header. This sets any errors it encounters and returns false if
    // the file is unsupported
    if(!ParseHeaderValues(headerLength.Value)) {
        Error = true;
        return;
    }

    if(static_cast<size_t>(Reader.tellg()) != Parsed.FirstEBMLElementOffset) {
        SetError("reader ended up at the wrong position, it should have ended up as the first "
                 "segment start");
    }

    // Look for all top level elements for later use
    FindTopLevelElements();
}
// ------------------------------------ //
DLLEXPORT bool MatroskaParser::ParseHeaderValues(int headerSizeLeft)
{
    std::vector<uint8_t> dataBuffer;

    while(headerSizeLeft > 0) {
        EBMLLIdentifierValue valueIdentifier(Reader);

        if(!Reader.good()) {
            SetError("data ended while reading a header identifier");
            return false;
        }

        EBMLLengthValue valueLength(Reader);

        if(!Reader.good()) {
            SetError("data ended while reading a header variable length");
            return false;
        }

        // Read the data
        // This shouldn't shrink automatically
        dataBuffer.resize(valueLength.Value);

        Reader.read(reinterpret_cast<char*>(dataBuffer.data()), dataBuffer.size());

        if(!Reader.good()) {
            SetError("data ended while reading a header variable's data of length: " +
                     std::to_string(dataBuffer.size()));
            return false;
        }

        // Decrement the left number of bytes in the header by the number of bytes just
        // processed
        headerSizeLeft -=
            valueIdentifier.ByteLength + valueLength.ByteLength + valueLength.Value;

        // Handle variable type if it is known
        switch(valueIdentifier.Value) {
        case EBML_FIELD_IDENTIFIER_VERSION:
            Parsed.EBMLVersion =
                ReadVariableLengthUnsignedInteger(dataBuffer, valueLength.Value);
            break;
        case EBML_FIELD_IDENTIFIER_READ_VERSION:
            Parsed.EBMLReadVersion =
                ReadVariableLengthUnsignedInteger(dataBuffer, valueLength.Value);
            break;
        case EBML_FIELD_IDENTIFIER_MAX_ID_LENGTH:
            Parsed.EBMLMaxIDLength =
                ReadVariableLengthUnsignedInteger(dataBuffer, valueLength.Value);
            break;
        case EBML_FIELD_IDENTIFIER_MAX_SIZE_LENGTH:
            Parsed.EBMLMaxSizeLength =
                ReadVariableLengthUnsignedInteger(dataBuffer, valueLength.Value);
            break;
        case EBML_FIELD_IDENTIFIER_DOCTYPE:
            Parsed.DocType =
                std::string(reinterpret_cast<char*>(dataBuffer.data()), valueLength.Value);
            break;
        case EBML_FIELD_IDENTIFIER_DOCTYPE_VERSION:
            Parsed.DocTypeVersion =
                ReadVariableLengthUnsignedInteger(dataBuffer, valueLength.Value);
            break;
        case EBML_FIELD_IDENTIFIER_DOCTYPE_READ_VERSION:
            Parsed.DocTypeReadVersion =
                ReadVariableLengthUnsignedInteger(dataBuffer, valueLength.Value);
            break;
        }
    }

    // Check the header info for whether we can understand the file or not
    if(Parsed.EBMLVersion != 1) {
        SetError("unknown EBML version: " + std::to_string(Parsed.EBMLVersion));
        return false;
    }

    if(Parsed.EBMLReadVersion != 1) {
        SetError("unknown EBML read version: " + std::to_string(Parsed.EBMLReadVersion));
        return false;
    }

    if(Parsed.DocType != "matroska") {
        SetError("unknown doctype: " + Parsed.DocType);
        return false;
    }

    // This parser is written for version 4 but assume it works just as fine for lower versions
    if(Parsed.DocTypeVersion > 4 || Parsed.DocTypeVersion < 1) {
        SetError("unknown doctype version: " + std::to_string(Parsed.DocTypeVersion));
        return false;
    }

    if(Parsed.DocTypeReadVersion != 2) {
        SetError("unknown doctype read version: " + std::to_string(Parsed.DocTypeReadVersion));
        return false;
    }

    return true;
}
// ------------------------------------ //
DLLEXPORT void MatroskaParser::FindTopLevelElements()
{
    Reader.seekg(Parsed.FirstEBMLElementOffset);

    if(!Reader.good()) {
        SetError("Parsed.FirstEBMLElementOffset is invalid offset");
        return;
    }

    EBMLElement segment(Reader);

    if(!Reader.good()) {
        SetError("Segment ended while parsing its header");
        return;
    }

    if(static_cast<size_t>(Reader.tellg()) != segment.DataStart) {
        SetError("Segment start pos mismatched with data start");
        return;
    }

    if(segment.Identifier.Value != ELEMENT_TYPE_SEGMENT) {
        SetError("unexpected element identifier where Segment should begin");
        return;
    }

    int result =
        segment.ReadChildElements(Reader, [&](const EBMLElement& element, auto readdata) {
            switch(element.Identifier.Value) {
            case ELEMENT_TYPE_INFO: HandleInfoElement(element); break;
            case ELEMENT_TYPE_TRACKS: HandleTracksElement(element); break;
            case ELEMENT_TYPE_CLUSTER:
                HandleClusterElement(element);
                break;
                // Not useful elements
            case ELEMENT_TYPE_CUES:
            case ELEMENT_TYPE_TAGS:
            case ELEMENT_TYPE_VOID:
            case ELEMENT_TYPE_SEEK_HEAD:
            default: break;
            }

            return true;
        });

    if(result < 0) {
        SetError("reading file data failed while parsing EBML element, at pos: " +
                 std::to_string(Reader.tellg()));
        return;
    }

    if(Parsed.Duration < 0.f) {
        SetError("Failed to find info element with duration");
    }
}
// ------------------------------------ //
DLLEXPORT void MatroskaParser::HandleInfoElement(const EBMLElement& element)
{
    int result =
        element.ReadChildElements(Reader, [&](const EBMLElement& element, auto readdata) {
            switch(element.Identifier.Value) {
            case ELEMENT_TYPE_TIMECODE_SCALE:
                Parsed.TimecodeScale =
                    ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
                break;
            case ELEMENT_TYPE_MUXIN_APP:
                Parsed.MuxinApp = std::string(
                    reinterpret_cast<char*>(readdata().data()), element.Length.Value);
                break;
            case ELEMENT_TYPE_WRITING_APP:
                Parsed.WritingApp = std::string(
                    reinterpret_cast<char*>(readdata().data()), element.Length.Value);
                break;
            case ELEMENT_TYPE_DURATION:
                Parsed.Duration = ReadVariableLengthFloat(readdata(), element.Length.Value);
                break;
            }

            return true;
        });

    if(result < 0) {
        SetError("reading info element contents failed");
    }
}

DLLEXPORT void MatroskaParser::HandleTracksElement(const EBMLElement& element)
{
    int result =
        element.ReadChildElements(Reader, [&](const EBMLElement& element, auto readdata) {
            switch(element.Identifier.Value) {
            case ELEMENT_TYPE_TRACK_ENTRY: HandleTrackEntryElement(element); break;
            }

            return true;
        });

    if(result < 0) {
        SetError("reading tracks element contents failed");
    }
}

DLLEXPORT void MatroskaParser::HandleTrackEntryElement(const EBMLElement& element)
{
    TrackInfo track;
    bool error = false;

    int result = element.ReadChildElements(Reader, [&](const EBMLElement& element,
                                                       auto readdata) {
        switch(element.Identifier.Value) {
        case ELEMENT_TYPE_TRACK_NUMBER:
            track.TrackNumber =
                ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
            break;
        case ELEMENT_TYPE_TRACK_UID:
            track.TrackUID =
                ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
            break;
        case ELEMENT_TYPE_FLAG_LACING:
            track.Lacing =
                ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value) != 0;
            break;
        case ELEMENT_TYPE_LANGUAGE:
            track.Language =
                std::string(reinterpret_cast<char*>(readdata().data()), element.Length.Value);
            break;
        case ELEMENT_TYPE_CODEC_ID:
            track.CodecID =
                std::string(reinterpret_cast<char*>(readdata().data()), element.Length.Value);
            break;
        case ELEMENT_TYPE_TRACK_TYPE: {
            int type = ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);

            switch(type) {
            case EBML_TRACK_TYPE_VIDEO: track.TrackType = TRACK_TYPE::Video; break;
            case EBML_TRACK_TYPE_AUDIO: track.TrackType = TRACK_TYPE::Audio; break;
            case EBML_TRACK_TYPE_COMPLEX: track.TrackType = TRACK_TYPE::Complex; break;
            case EBML_TRACK_TYPE_LOGO: track.TrackType = TRACK_TYPE::Logo; break;
            case EBML_TRACK_TYPE_SUBTITLE: track.TrackType = TRACK_TYPE::Subtitle; break;
            case EBML_TRACK_TYPE_BUTTONS: track.TrackType = TRACK_TYPE::Buttons; break;
            case EBML_TRACK_TYPE_CONTROL: track.TrackType = TRACK_TYPE::Control; break;
            default:
                error = true;
                SetError("unknown track type: " + std::to_string(type));
                return false;
            }
        }
        case ELEMENT_TYPE_DEFAULT_DURATION:
            track.DefaultDuration =
                ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
            break;
            // These two have important child elements
        case ELEMENT_TYPE_VIDEO: {
            TrackInfo::Video video;
            int result = element.ReadChildElements(Reader, [&](const EBMLElement& element,
                                                               auto readdata) {
                switch(element.Identifier.Value) {
                case ELEMENT_TYPE_PIXEL_WIDTH:
                    video.PixelWidth =
                        ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
                    break;
                case ELEMENT_TYPE_PIXEL_HEIGHT:
                    video.PixelHeight =
                        ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
                    break;
                case ELEMENT_TYPE_DISPLAY_UNIT:
                    video.DisplayUnit =
                        ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
                    break;
                }
                return true;
            });

            if(result < 1) {
                SetError("parsing Video element failed");
                error = true;
                return false;
            }

            track.TrackTypeData = video;
            break;
        }
        case ELEMENT_TYPE_AUDIO: {
            TrackInfo::Audio audio;
            int result = element.ReadChildElements(Reader, [&](const EBMLElement& element,
                                                               auto readdata) {
                switch(element.Identifier.Value) {
                case ELEMENT_TYPE_CHANNELS:
                    audio.Channels =
                        ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
                    break;
                case ELEMENT_TYPE_SAMPLING_FREQUENCY:
                    audio.SamplingFrequency =
                        ReadVariableLengthFloat(readdata(), element.Length.Value);
                    break;
                case ELEMENT_TYPE_BIT_DEPTH:
                    audio.BitDepth =
                        ReadVariableLengthUnsignedInteger(readdata(), element.Length.Value);
                    break;
                }
                return true;
            });

            if(result < 1) {
                SetError("parsing Audio element failed");
                error = true;
                return false;
            }

            track.TrackTypeData = audio;
            break;
        }
        case ELEMENT_TYPE_CODEC_PRIVATE:
            track.CodecPrivateOffset = element.DataStart;
            track.CodecPrivateLength = element.Length.Value;
            break;
        }

        return true;
    });

    // Error checking before adding this track to avoid adding invalid tracks
    if(error)
        return;

    if(result < 1) {
        SetError("reading a track entry element contents failed (or it is empty)");
        return;
    }

    if(track.TrackType == TRACK_TYPE::Video &&
        !std::holds_alternative<TrackInfo::Video>(track.TrackTypeData)) {
        SetError("parsed a track of type video but it didn't contain a Video element");
        return;
    }

    if(track.TrackType == TRACK_TYPE::Audio &&
        !std::holds_alternative<TrackInfo::Audio>(track.TrackTypeData)) {
        SetError("parsed a track of type audio but it didn't contain a Audio element");
        return;
    }

    // Add the parsed track to the list of tracks and update the easy to read variables for how
    // many tracks of each type we have
    Parsed.Tracks.push_back(track);

    if(track.TrackType == TRACK_TYPE::Video)
        ++Parsed.VideoTrackCount;
    if(track.TrackType == TRACK_TYPE::Audio)
        ++Parsed.AudioTrackCount;
}

DLLEXPORT void MatroskaParser::HandleClusterElement(const EBMLElement& element)
{
    int result =
        element.ReadChildElements(Reader, [&](const EBMLElement& childElement, auto readdata) {
            switch(childElement.Identifier.Value) {
            case ELEMENT_TYPE_TIMECODE:
                // Just the time code is grabbed and this cluster info is them stored
                const auto time =
                    ReadVariableLengthUnsignedInteger(readdata(), childElement.Length.Value);

                Parsed.Clusters.push_back(
                    ClusterInfo{element.DataStart, element.Length.Value, time});
                return false;
            }
            return true;
        });

    if(result < 0) {
        SetError("reading a cluster element contents failed");
    }
}
// ------------------------------------ //
DLLEXPORT uint64_t MatroskaParser::ReadVariableLengthUnsignedInteger(
    const std::vector<uint8_t>& data, int length)
{
    uint64_t result = 0;

    for(int i = 0; i < length && i < static_cast<int>(data.size()); ++i) {
        result += static_cast<uint64_t>(data[i]) << (length - 1 - i) * BITS_IN_BYTE;
    }

    return result;
}

DLLEXPORT double MatroskaParser::ReadVariableLengthFloat(
    const std::vector<uint8_t>& data, int length)
{
    if(length == 4) {
        float result;

        uint32_t tmp = ReadVariableLengthUnsignedInteger(data, length);
        static_assert(sizeof(tmp) == sizeof(result), "float size mismatch on integer type");

        std::memcpy(&result, &tmp, sizeof(result));
        return result;

    } else if(length == 8) {
        double result;

        uint64_t tmp = ReadVariableLengthUnsignedInteger(data, length);
        static_assert(sizeof(tmp) == sizeof(result), "float size mismatch on integer type");

        std::memcpy(&result, &tmp, sizeof(result));
        return result;
    }

    LOG_ERROR("MatroskaParser: ReadVariableLengthFloat: cannot handle float of length: " +
              std::to_string(length));
    return -1.f;
}
