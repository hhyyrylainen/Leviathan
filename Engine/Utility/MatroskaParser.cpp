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

// Block flags
constexpr uint8_t MATROSKA_BLOCK_FLAG_INVISIBLE = 0x8;
constexpr uint8_t MATROSKA_BLOCK_FLAG_LACING = 0x30;


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
    EBMLLengthValue(const EBMLLengthValue& other) = default;
    EBMLLengthValue(EBMLLengthValue& other) = default;
    EBMLLengthValue& operator=(const EBMLLengthValue& other) = default;
    EBMLLengthValue& operator=(EBMLLengthValue& other) = default;

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
    EBMLLIdentifierValue(const EBMLLIdentifierValue& other) = default;
    EBMLLIdentifierValue(EBMLLIdentifierValue& other) = default;
    EBMLLIdentifierValue& operator=(const EBMLLIdentifierValue& other) = default;
    EBMLLIdentifierValue& operator=(EBMLLIdentifierValue& other) = default;

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
    EBMLElement(const EBMLElement& other) = default;
    EBMLElement(EBMLElement& other) = default;
    EBMLElement& operator=(const EBMLElement& other) = default;
    EBMLElement& operator=(EBMLElement& other) = default;

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

//! A block inside a cluster
struct ClusterBlockHeader {
public:
    ClusterBlockHeader(const ClusterBlockHeader& other) = default;
    ClusterBlockHeader(ClusterBlockHeader& other) = default;
    ClusterBlockHeader& operator=(const ClusterBlockHeader& other) = default;
    ClusterBlockHeader& operator=(ClusterBlockHeader& other) = default;

    //! Read from stream
    template<class T>
    ClusterBlockHeader(T& stream) : TrackIdentifier(stream)
    {
        if(!stream.good())
            return;

        int16_t tmp;

        stream.read(reinterpret_cast<char*>(&tmp), sizeof(tmp));

        RelativeTimecode = boost::endian::big_to_native(tmp);

        uint8_t flags = stream.get();

        Invisible = flags & MATROSKA_BLOCK_FLAG_INVISIBLE;
        Lacing = flags & MATROSKA_BLOCK_FLAG_LACING;
    }

    //! This element type uses masking to remove the highest bits from the identifier
    EBMLLengthValue TrackIdentifier;
    int16_t RelativeTimecode;

    // Flags
    bool Invisible;
    int Lacing;
};

//! Xiph lacing handling
//! First is the item count - 1 (so 0 means there's 1 item). Then there is "item count - 1"
//! number of object sizes. The object sizes are encoded as bytes with the first bite less than
//! 255 ending the number. The last object's size is not encoded (it takes up the rest of the
//! space after the other sizes are subtrackted), this is why the item count - 1 is the number
//! in the data.
class XiphLacing {
public:
    XiphLacing(const uint8_t* data, size_t length)
    {
        if(length < 1)
            throw InvalidArgument("data length is 0");

        int count = data[0];
        size_t readOffset = 1;

        // The last item is implicit
        ItemCount = count + 1;

        size_t otherObjectSizes = 0;

        // Read the object lengths
        for(int i = 0; i < count; ++i) {
            size_t currentLength = 0;
            int byte;
            do {
                if(readOffset >= length)
                    throw InvalidArgument("data ended while parsing laced lengths");

                byte = data[readOffset];
                ++readOffset;

                currentLength += byte;

            } while(byte >= 255);

            ItemSizes.push_back(currentLength);
            otherObjectSizes += currentLength;
        }

        // Header ended
        LacedHeaderLenght = static_cast<int>(readOffset);

        // The last object is the rest of the length
        const int64_t lastObjectSize = length - LacedHeaderLenght - otherObjectSizes;

        if(lastObjectSize <= 0)
            throw InvalidArgument(
                "laced data length left for last item is zero or negative: " +
                std::to_string(lastObjectSize));

        ItemSizes.push_back(lastObjectSize);
    }

    int ItemCount;
    std::vector<size_t> ItemSizes;

    //! The total lacing header size, used to get from start of the data to the first object
    int LacedHeaderLenght;
};

} // namespace Leviathan
// ------------------------------------ //
// MatroskaParser::BlockSearchInfo
struct MatroskaParser::BlockSearchInfo {
    std::optional<size_t> CurrentClusterEnd;
    uint64_t ClusterTimeCode;
    std::optional<size_t> NextCluster;

    //! Search next pos. Caller must set this with for example from ClusterBlockIterator after
    //! calling JumpToFirstCluster
    std::optional<size_t> NextReadPos;
};

// ------------------------------------ //
// MatroskaParser::BlockSearchResult
struct MatroskaParser::BlockSearchResult {
    BlockSearchInfo Search;
    std::optional<ClusterBlockHeader> FoundBlock;
    std::optional<EBMLElement> BlockElement;
    BlockInfo BlockData;
};
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
DLLEXPORT float MatroskaParser::GetDurationInSeconds() const
{
    if(Parsed.Duration < 0)
        return -1.f;
    return Parsed.Duration * MATROSKA_DURATION_TO_SECONDS;
}
// ------------------------------------ //
DLLEXPORT const MatroskaParser::TrackInfo& MatroskaParser::GetFirstVideoTrack() const
{
    for(const auto& track : Parsed.Tracks) {
        if(track.TrackType == TRACK_TYPE::Video)
            return track;
    }

    throw Leviathan::InvalidState("no video tracks exist");
}

DLLEXPORT const MatroskaParser::TrackInfo& MatroskaParser::GetFirstAudioTrack() const
{
    for(const auto& track : Parsed.Tracks) {
        if(track.TrackType == TRACK_TYPE::Audio)
            return track;
    }

    throw Leviathan::InvalidState("no audio tracks exist");
}
// ------------------------------------ //
DLLEXPORT std::vector<uint8_t> MatroskaParser::ReadTrackCodecPrivateData(
    const TrackInfo& track)
{
    if(!track.CodecPrivateOffset || !track.CodecPrivateLength || !Good())
        return {};

    std::vector<uint8_t> result;
    result.resize(*track.CodecPrivateLength);

    Reader.seekg(*track.CodecPrivateOffset);

    Reader.read(reinterpret_cast<char*>(result.data()), result.size());

    if(!Reader.good()) {
        SetError("data ended while reading track codec private data");
        return {};
    }

    return result;
}
// ------------------------------------ //
DLLEXPORT void MatroskaParser::JumpToFirstCluster()
{
    if(Parsed.Clusters.empty()) {
        ClusterBlockIterator.reset();
        return;
    }

    ClusterBlockIterator = BlockIteratorInfo(Parsed.Clusters.front().DataStart);
}

DLLEXPORT std::tuple<const uint8_t*, size_t, MatroskaParser::BlockInfo>
    MatroskaParser::GetNextBlockForTrack(int tracknumber)
{
    BlockSearchResult result;
    if(!_SearchForNextBlock(tracknumber, result)) {
        // Out of data
        ClusterBlockIterator.reset();
        return std::make_tuple(nullptr, 0, BlockInfo{});
    }

    // Found a block, read its data to pass to our caller
    const size_t dataBegin = Reader.tellg();

    const auto currentElementEnd =
        result.BlockElement->DataStart + result.BlockElement->Length.Value;

    const auto nonHeaderBytes = currentElementEnd - dataBegin;
    ClusterBlockIterator->DataBuffer.resize(nonHeaderBytes);

    Reader.read(reinterpret_cast<char*>(ClusterBlockIterator->DataBuffer.data()),
        ClusterBlockIterator->DataBuffer.size());

    // Set the next position
    if(result.Search.NextReadPos) {
        ClusterBlockIterator->NextReadPos = *result.Search.NextReadPos;
    } else {
        // No more data
        ClusterBlockIterator->LastBlockRead = true;
    }

    // Return the block data
    return std::make_tuple(ClusterBlockIterator->DataBuffer.data(),
        ClusterBlockIterator->DataBuffer.size(), result.BlockData);
}

DLLEXPORT std::tuple<bool, size_t, MatroskaParser::BlockInfo>
    MatroskaParser::PeekNextBlockForTrack(int tracknumber) const
{
    BlockSearchResult result;
    if(!_SearchForNextBlock(tracknumber, result)) {
        // No data found
        return std::make_tuple(false, 0, BlockInfo{});
    }

    return std::make_tuple(true, result.BlockElement->Length.Value, result.BlockData);
}
// ------------------------------------ //
DLLEXPORT bool MatroskaParser::_UpdateFindClusterInfo(BlockSearchInfo& info) const
{
    if(Error || !info.NextReadPos) {
        return false;
    }

    info.CurrentClusterEnd.reset();
    info.NextCluster.reset();


    // TODO: this loop is also not optimal if there are a ton of clusters (could be
    // replaced with a binary search or caching)
    for(size_t i = Parsed.Clusters.size() - 1;; --i) {
        const auto& cluster = Parsed.Clusters[i];

        if(cluster.DataStart <= *info.NextReadPos) {
            // In this cluster
            info.CurrentClusterEnd = cluster.DataStart + cluster.Lenght;
            info.ClusterTimeCode = cluster.Timecode;

            if(i + 1 < Parsed.Clusters.size()) {
                // There is a next cluster
                info.NextCluster = Parsed.Clusters[i + 1].DataStart;
            }
            break;
        }

        if(i == 0) {
            LOG_ERROR("MatroskaParser: could not find which cluster current position is in: " +
                      std::to_string(*info.NextReadPos));
            return false;
        }
    }

    if(!info.CurrentClusterEnd) {
        LOG_ERROR("MatroskaParser: could not find which cluster current "
                  "read position is in");
        return false;
    }

    return true;
}

DLLEXPORT bool MatroskaParser::_FindNextBlock(int tracknumber, BlockSearchResult& result) const
{
    if(!_UpdateFindClusterInfo(result.Search))
        return false;

    Reader.seekg(*result.Search.NextReadPos);

    if(Error || !Reader.good()) {
        LOG_ERROR("MatroskaParser: start search pos is invalid or error is set");
        return false;
    }

    bool working = true;

    while(working) {
        EBMLElement currentElement(Reader);

        if(!Reader.good()) {
            LOG_ERROR("MatroskaParser: element ended while parsing its header");
            return false;
        }

        const auto currentElementEnd = currentElement.DataStart + currentElement.Length.Value;

        bool clusterEnded = false;

        // Don't jump past the ond of the current cluster (without properly going to the
        // next cluster)
        auto setNextReadPos = [&]() {
            if(currentElementEnd >= *result.Search.CurrentClusterEnd) {
                // Needs to jump to next cluster
                clusterEnded = true;

                if(result.Search.NextCluster) {
                    result.Search.NextReadPos = *result.Search.NextCluster;
                } else {
                    // No more clusters
                    result.Search.NextReadPos.reset();
                }
            } else {
                result.Search.NextReadPos = currentElementEnd;
            }
        };

        switch(currentElement.Identifier.Value) {
            // The current cluster already contains the time code
            // TODO: that is not optimal if there are a ton of clusters (ie. longer videos are
            // attempted to be played)
            // case ELEMENT_TYPE_TIMECODE:
        case ELEMENT_TYPE_SIMPLE_BLOCK: {
            ClusterBlockHeader foundBlock(Reader);

            if(!Reader.good()) {
                LOG_ERROR("MatroskaParser:: data ended while decoding a cluster block header");
                working = false;
                break;
            }

            if(static_cast<int>(foundBlock.TrackIdentifier.Value) == tracknumber) {

                result.BlockElement = currentElement;
                result.FoundBlock = foundBlock;

                result.BlockData = BlockInfo{ApplyRelativeTimecode(
                    result.Search.ClusterTimeCode, result.FoundBlock->RelativeTimecode)};

                // We need to set the read pos for the next block in case our caller needs it
                setNextReadPos();

                return true;
            }
        }
            // Else let the default case handle jumping over the current data as we didn't want
            // this block
        default:
            // Unknown data, we need to jump past
            setNextReadPos();

            // If the jump is past the end of the current cluster we need to update which
            // cluster we are in
            if(clusterEnded) {
                if(!_UpdateFindClusterInfo(result.Search)) {
                    // Ran out of clusters
                    working = false;
                    break;
                }
            }

            Reader.seekg(*result.Search.NextReadPos);
        }
    }

    return false;
}

DLLEXPORT bool MatroskaParser::_SearchForNextBlock(
    int tracknumber, BlockSearchResult& result) const
{
    if(!ClusterBlockIterator || ClusterBlockIterator->LastBlockRead)
        return false;

    result.Search.NextReadPos = ClusterBlockIterator->NextReadPos;

    if(!_FindNextBlock(tracknumber, result) || !result.FoundBlock) {
        // Out of data
        return false;
    }

    return true;
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
// ------------------------------------ //
DLLEXPORT uint64_t MatroskaParser::ApplyRelativeTimecode(uint64_t base, int16_t relative)
{
    // Make sure not to add too big negative number to cause a time point wrap to a huge value
    if(relative < 0) {
        if(base >= static_cast<unsigned>(-1 * relative)) {
            return base + relative;
        } else {
            return 0;
        }
    }

    return base + relative;
}
// ------------------------------------ //
DLLEXPORT std::vector<std::tuple<const uint8_t*, size_t>>
    MatroskaParser::SplitVorbisPrivateSetupData(
        const uint8_t* codecprivatedata, size_t datalength)
{
    try {
        XiphLacing laced(codecprivatedata, datalength);

        std::vector<std::tuple<const uint8_t*, size_t>> result;

        if(laced.ItemCount != 3) {
            LOG_WARNING("MatroskaParser: Vorbis private data: expected packet count to be 3, "
                        "but got: " +
                        std::to_string(laced.ItemCount));
        }

        const uint8_t* currentData = codecprivatedata + laced.LacedHeaderLenght;

        for(int i = 0; i < laced.ItemCount; ++i) {
            const auto size = laced.ItemSizes[i];

            if(currentData + size - codecprivatedata > static_cast<std::ptrdiff_t>(datalength))
                throw InvalidArgument(
                    "there isn't enough data in the buffer for the current item");

            result.push_back(std::make_tuple(currentData, size));
            currentData += size;
        }

        return result;

    } catch(const InvalidArgument& e) {
        LOG_ERROR(
            "MatroskaParser: SplitVorbisPrivateSetupData: data is malformed, exception: ");
        e.PrintToLog();
        return {};
    }
}
