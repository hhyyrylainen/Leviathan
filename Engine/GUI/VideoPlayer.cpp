// ------------------------------------ //
#include "VideoPlayer.h"

#include "Utility/Codec.h"
#include "Utility/MatroskaParser.h"

#include "Engine.h"

#include "CoreThread/BsCoreThread.h"
#include "bsfCore/Image/BsTexture.h"

#include <filesystem>
#include <optional>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
// NOTE: even though these formats are called RGBA they are actually stored in ARGB order on
// little endian machines, for some insane reason, but one that luckily seems to be a
// convention everywhere

// TODO: add support for disabling alpha if not needed

constexpr auto BS_PIXEL_FORMAT = bs::PF_RGBA8;

// This must match the above definition
constexpr auto FRAME_CONVERT_FORMAT = DecodedFrame::Image::IMAGE_TARGET_FORMAT::RGBA;


constexpr auto DEFAULT_AUDIO_BUFFER_RESERVED_SPACE = 64000;
// ------------------------------------ //
// VideoPlayer::Implementation
struct VideoPlayer::Implementation {
public:
    struct PendingAudioData {
        void Clear()
        {
            Data.clear();
            NextReadOffset = 0;
            Empty = true;
        }

        std::vector<uint8_t> Data;
        size_t NextReadOffset = 0;
        bool Empty = true;
    };

    enum class LAST_USED {
        None = 0,
        First,
        Second,
    };

public:
    ~Implementation()
    {
        CloseCodecs();
    }

    void CloseCodecs()
    {
        VideoCodec.reset();
        AudioCodec.reset();

        VideoParser.reset();
        AudioParser.reset();
    }

    //! Mutex that must be locked when destroying or creating any resource in this class that
    //! is used from multiple threads. Also needs to be locked when reading or writing
    //! PendingAudioDataBuffers
    Mutex ThreadSafetyMutex;

    std::unique_ptr<Codec> VideoCodec;
    std::unique_ptr<Codec> AudioCodec;

    std::optional<MatroskaParser> VideoParser;
    std::optional<MatroskaParser> AudioParser;

    MatroskaParser::TrackInfo VideoTrack;
    MatroskaParser::TrackInfo AudioTrack;

    std::optional<DecodedFrame> CurrentlyDecodedFrame;

    bool FrameNeedsGPUUpload = false;

    bs::SPtr<bs::PixelData> GPUUploadBuffer;

    float CurrentlyDecodedFrameTimeStamp = -1.f;
    float NextFrameTimeStamp = -1.f;

    //! This buffers audio data that the audio library couldn't take at once if the decoder
    //! gives us more data than requested
    PendingAudioData _PendingAudioData;

    //! When true the player waits for the video texture to finish uploading on each frame
    bool WaitForTextureToUpload = false;
};

// ------------------------------------ //
// VideoPlayer
DLLEXPORT VideoPlayer::VideoPlayer() : Pimpl(std::make_unique<Implementation>()) {}

DLLEXPORT VideoPlayer::~VideoPlayer()
{
    UnRegisterAllEvents();

    // Ensure all decoding resources are cleared
    Stop();
}
// ------------------------------------ //
DLLEXPORT bool VideoPlayer::Play(const std::string& videofile)
{
    // Make sure nothing is playing currently //
    Stop();

    if(!std::filesystem::exists(videofile)) {

        LOG_ERROR("VideoPlayer: Play: file doesn't exist: " + videofile);
        return false;
    }

    VideoFile = videofile;

    try {
        // Parse stream data to know how big our textures need to be //
        if(!OpenCodecsForFile()) {

            LOG_ERROR("VideoPlayer: Play: failed to parse file or open codecs for file");
            Stop();
            return false;
        }

        if(!CreateOutputTexture()) {

            LOG_ERROR("VideoPlayer: Play: output video texture creation failed");
            Stop();
            return false;
        }
    } catch(const Exception& e) {
        LOG_ERROR("VideoPlayer: Play: exception happened on initializing playback: ");
        e.PrintToLog();
        Stop();
        return false;
    }

    // Make tick run
    IsPlaying = true;
    RegisterForEvent(EVENT_TYPE_FRAME_BEGIN);
    return true;
}

DLLEXPORT void VideoPlayer::Stop()
{
    // Close all codec resources //
    StreamValid = false;

    // Stop audio playing first //
    if(IsPlayingAudio) {
        IsPlayingAudio = false;
    }

    if(AudioStreamData) {

        AudioStreamData->Detach();
        AudioStreamData.reset();
    }

    if(AudioStream) {

        AudioStream->Stop();
        AudioStream.reset();
    }

    Lock lock(Pimpl->ThreadSafetyMutex);

    // Dump audio data
    Pimpl->_PendingAudioData.Clear();

    Pimpl->CurrentlyDecodedFrame.reset();

    Pimpl->CloseCodecs();

    // Let go of our textures and things //
    VideoFile = "";

    VideoOutputTexture = nullptr;

    Pimpl->GPUUploadBuffer = nullptr;

    IsPlaying = false;
}
// ------------------------------------ //
DLLEXPORT float VideoPlayer::GetDuration() const
{
    if(!Pimpl->VideoParser)
        return -1.f;

    return Pimpl->VideoParser->GetDurationInSeconds();
}
// ------------------------------------ //
DLLEXPORT float VideoPlayer::GetCurrentTime() const
{
    return Pimpl->CurrentlyDecodedFrameTimeStamp;
}
// ------------------------------------ //
bool VideoPlayer::CreateOutputTexture()
{
    Pimpl->GPUUploadBuffer =
        bs::PixelData::create(FrameWidth, FrameHeight, 1, BS_PIXEL_FORMAT);

    if(!Pimpl->GPUUploadBuffer)
        return false;

    VideoOutputTexture = bs::Texture::create(Pimpl->GPUUploadBuffer, bs::TU_DYNAMIC);
    return VideoOutputTexture != nullptr;
}
// ------------------------------------ //
bool VideoPlayer::OpenCodecsForFile()
{
    Pimpl->VideoParser = MatroskaParser(VideoFile);

    if(!Pimpl->VideoParser->Good()) {
        LOG_ERROR("VideoPlayer: failed to parse video file (is it a matroska file?): " +
                  Pimpl->VideoParser->GetErrorMessage());
        return false;
    }

    VideoTimeBase = Pimpl->VideoParser->GetHeader().TimecodeScale;

    // Jump to where the video data starts
    Pimpl->VideoParser->JumpToFirstCluster();

    // Copy track info
    Pimpl->VideoTrack = Pimpl->VideoParser->GetFirstVideoTrack();

    // Duplicate the parser state for the audio stream
    if(Pimpl->VideoParser->GetAudioTrackCount() > 0) {
        HasAudioStream = true;

        Pimpl->AudioParser = Pimpl->VideoParser;

        Pimpl->AudioTrack = Pimpl->AudioParser->GetFirstAudioTrack();

        // Initialize audio codec
        if(Pimpl->AudioTrack.CodecID == MatroskaParser::CODEC_TYPE_VORBIS) {

            const auto codecPrivateData =
                Pimpl->AudioParser->ReadTrackCodecPrivateData(Pimpl->AudioTrack);

            Pimpl->AudioCodec = std::make_unique<VorbisCodec>(
                codecPrivateData.data(), codecPrivateData.size());
        } else {
            LOG_ERROR("VideoPlayer: unknown audio codec: " + Pimpl->AudioTrack.CodecID);
            HasAudioStream = false;
        }

        if(HasAudioStream) {
            SampleRate = static_cast<int>(
                std::get<MatroskaParser::TrackInfo::Audio>(Pimpl->AudioTrack.TrackTypeData)
                    .SamplingFrequency);
            ChannelCount =
                std::get<MatroskaParser::TrackInfo::Audio>(Pimpl->AudioTrack.TrackTypeData)
                    .Channels;

            // Create sound object //
            AudioStreamDataProperties.SampleRate = SampleRate;

            bool valid = false;

            if(ChannelCount == 1) {
                AudioStreamDataProperties.Channels = alure::ChannelConfig::Mono;
                valid = true;
            } else if(ChannelCount == 2) {
                AudioStreamDataProperties.Channels = alure::ChannelConfig::Stereo;
                valid = true;
            } else if(ChannelCount == 4) {
                AudioStreamDataProperties.Channels = alure::ChannelConfig::Quad;
                valid = true;
            } else if(ChannelCount == 6) {
                AudioStreamDataProperties.Channels = alure::ChannelConfig::X51;
                valid = true;
            } else if(ChannelCount == 7) {
                AudioStreamDataProperties.Channels = alure::ChannelConfig::X61;
                valid = true;
            } else if(ChannelCount == 8) {
                AudioStreamDataProperties.Channels = alure::ChannelConfig::X71;
                valid = true;
            }

            // TODO: alure supports float32 format, we could do without converting that from
            // the codec (if the codec internally returns floats)
            AudioStreamDataProperties.SampleType = alure::SampleType::Int16;

            if(!valid) {
                LOG_ERROR("VideoPlayer: invalid channel configuration for audio: " +
                          std::to_string(ChannelCount));
                return false;
            }
        }

        // Allocate some audio buffer space
        Pimpl->_PendingAudioData.Data.reserve(DEFAULT_AUDIO_BUFFER_RESERVED_SPACE);

    } else {
        HasAudioStream = false;
    }

    // Initialize the video codec
    if(Pimpl->VideoTrack.CodecID == MatroskaParser::CODEC_TYPE_AV1) {

        Pimpl->VideoCodec = std::make_unique<AV1Codec>();
    } else {
        LOG_ERROR("VideoPlayer: unknown video codec: " + Pimpl->VideoTrack.CodecID);
        return false;
    }

    FrameWidth =
        std::get<MatroskaParser::TrackInfo::Video>(Pimpl->VideoTrack.TrackTypeData).PixelWidth;
    FrameHeight = std::get<MatroskaParser::TrackInfo::Video>(Pimpl->VideoTrack.TrackTypeData)
                      .PixelHeight;


    PassedTimeSeconds = 0.f;

    // Reset frame status
    Pimpl->CurrentlyDecodedFrameTimeStamp = -1.f;
    Pimpl->NextFrameTimeStamp = -1.f;

    StreamValid = true;
    return true;
}
// ------------------------------------ //
bool VideoPlayer::HandleFrameVideoUpdate()
{
    // If we don't have a frame decoded we should decode one one
    if(Pimpl->CurrentlyDecodedFrameTimeStamp < 0) {
        // This is the first frame of the video

        // Reset the elapsed time to not skip the initial frame
        PassedTimeSeconds = 0.f;

        // Decode the first frame
        if(!DecodeVideoFrame())
            return false;

        Pimpl->FrameNeedsGPUUpload = true;

        // And get the timestamp for the next video
        PeekNextFrameTimeStamp();
    } else {
        // Loop until we reach a state where the next frame is > PassedTimeSeconds
        while(PassedTimeSeconds >= Pimpl->NextFrameTimeStamp) {
            // Decode the next frame
            if(!DecodeVideoFrame()) {
                // We ran out of data
                LOG_INFO("VideoPlayer: reached end of video stream");
                OnStreamEndReached();
                return false;
            }

            Pimpl->FrameNeedsGPUUpload = true;

            // Get the time for the next frame
            // In case we ran out of data this won't update the next frame time, and this will
            // loop again. During that loop the next frame decode fails and this breaks, so we
            // don't need to check the return value here
            PeekNextFrameTimeStamp();
        }
    }

    // Copy decoded frame to GPU if it was updated
    if(Pimpl->FrameNeedsGPUUpload)
        UpdateTexture();

    return true;
}

bool VideoPlayer::DecodeVideoFrame()
{
    bool frameReady = false;

    while(!frameReady) {
        // Try to get a frame first
        Pimpl->VideoCodec->ReceiveDecodedFrames([&](const DecodedFrame& frame) {
            Pimpl->CurrentlyDecodedFrame = frame;

            frameReady = true;

            // We want only one frame at a time
            return false;
        });

        if(frameReady)
            break;

        // If we didn't get a frame send more data to the decoder
        auto [data, length, opts] =
            Pimpl->VideoParser->GetNextBlockForTrack(Pimpl->VideoTrack.TrackNumber);

        // If we ran out of data there's nothing to do
        if(!data)
            break;

        // CurrentlyDecodedTimeStamp = opts.Timecode / VideoTimeBase;
        Pimpl->CurrentlyDecodedFrameTimeStamp =
            opts.Timecode * MatroskaParser::MATROSKA_DURATION_TO_SECONDS;


        if(!Pimpl->VideoCodec->FeedRawFrame(data, length)) {
            LOG_ERROR("VideoCodec: failed to send raw frame to video codec");
        }
    }

    return frameReady;
}

bool VideoPlayer::PeekNextFrameTimeStamp()
{
    auto [found, length, opts] =
        Pimpl->VideoParser->PeekNextBlockForTrack(Pimpl->VideoTrack.TrackNumber);

    // We ran out of data
    if(!found)
        return false;

    Pimpl->NextFrameTimeStamp = opts.Timecode * MatroskaParser::MATROSKA_DURATION_TO_SECONDS;
    return true;
}
// ------------------------------------ //
void VideoPlayer::UpdateTexture()
{
    if(Pimpl->WaitForTextureToUpload) {
        // Wait until buffer is no longer locked
        if(Pimpl->GPUUploadBuffer->isLocked()) {

            // Using true here majorly stalls rendering
            bs::gCoreThread().submit(false);

            do {
                std::this_thread::yield();
            } while(Pimpl->GPUUploadBuffer->isLocked());
        }
    } else {
        // Skip updating until the texture is no longer locked for update
        if(Pimpl->GPUUploadBuffer->isLocked())
            return;
    }

    // For maximum performance we directly convert the frame to the GPU buffer
    const auto& imageData =
        std::get<DecodedFrame::Image>(Pimpl->CurrentlyDecodedFrame->TypeSpecificData);

    if(imageData.Width != static_cast<uint32_t>(FrameWidth) ||
        imageData.Height != static_cast<uint32_t>(FrameHeight)) {
        LOG_ERROR("VideoPlayer: decoded frame size is different than what file header "
                  "info said");
        return;
    }

    if(!imageData.ConvertImage(Pimpl->GPUUploadBuffer->getData(),
           Pimpl->GPUUploadBuffer->getSize(),
           DecodedFrame::Image::IMAGE_TARGET_FORMAT::RGBA)) {
        LOG_ERROR("VideoPlayer: frame convert failed, likely due to mismatch between graphics "
                  "buffer size and what is needed for frame conversion");
        return;
    }

    VideoOutputTexture->writeData(Pimpl->GPUUploadBuffer, 0, 0, true);
    Pimpl->FrameNeedsGPUUpload = false;
}
// ------------------------------------ //
size_t VideoPlayer::ReadAudioData(uint8_t* output, size_t amount)
{
    Lock lock(Pimpl->ThreadSafetyMutex);

    if(!HasAudioStream || !StreamValid || amount < 1) {
        return 0;
    }

    // Convert amount to byte count
    const auto bytesPerSample = 2;
    amount *= bytesPerSample * ChannelCount;

    size_t readAmount = 0;

    // Read audio data until the stream ends or we have reached amount
    while(amount > 0) {
        // First return from queue //
        if(!Pimpl->_PendingAudioData.Empty) {

            // Try to read from the queue //
            const auto read = ReadDataFromAudioQueue(lock, output, amount);

            if(read == 0) {
                // Queue is invalid... //
                LOG_ERROR("Invalid audio queue, emptying the queue");
                Pimpl->_PendingAudioData.Clear();
            } else {

                // Adjust pointer and amount and try to read again if still some size left
                readAmount += read;
                output += read;
                amount -= read;
                continue;
            }
        }

        bool wroteToBuffer = false;

        // Try to read audio data from the codec
        Pimpl->AudioCodec->ReceiveDecodedFrames([&](const DecodedFrame& frame) {
            const auto& soundData = std::get<DecodedFrame::Sound>(frame.TypeSpecificData);

            if(soundData.Channels != ChannelCount) {
                LOG_ERROR("VideoPlayer: decoded audio data has different channel count than "
                          "what file header info said");
                return false;
            }

            // Received audio data //

            // First check if we can fit the whole data to the receiver buffer
            const auto totalSize = bytesPerSample * (soundData.Samples * ChannelCount);

            if(amount >= static_cast<size_t>(totalSize) // && !wroteToBuffer
            ) {
                // Directly feed the converted data to the requested
                if(!soundData.ConvertSamples(output, totalSize,
                       DecodedFrame::Sound::SOUND_TARGET_FORMAT::INTERLEAVED_INT16)) {
                    LOG_ERROR(
                        "VideoPlayer: converting audio data failed (directly to receiver)");
                    return false;
                }

                // Adjust pointer and amount and try to read again if still some size left
                readAmount += totalSize;
                output += totalSize;
                amount -= totalSize;

                // Continue reading blocks if amount not full
                return amount > 0;
            } else {
                // We need a buffer //
                wroteToBuffer = true;

                const auto previousBufferSize = Pimpl->_PendingAudioData.Data.size();
                const auto& bufferWritePos =
                    (&Pimpl->_PendingAudioData.Data[previousBufferSize - 1]) + 1;

                Pimpl->_PendingAudioData.Data.resize(previousBufferSize + totalSize);

                if(!soundData.ConvertSamples(bufferWritePos, totalSize,
                       DecodedFrame::Sound::SOUND_TARGET_FORMAT::INTERLEAVED_INT16)) {
                    LOG_ERROR("VideoPlayer: converting audio data failed");
                    return false;
                }

                Pimpl->_PendingAudioData.Empty = false;

                // Now that there is data we can loop again to get some data from the buffer
                return false;
            }
        });

        // Stop if we got enough data
        if(amount <= 0)
            break;

        // Read from buffer without looping again if we wrote to it to not pass the codec more
        // data in case there was still some leftover data
        if(wroteToBuffer)
            continue;

        // Not enough data could be read, read next block for the audio stream
        auto [data, length, opts] =
            Pimpl->AudioParser->GetNextBlockForTrack(Pimpl->AudioTrack.TrackNumber);

        if(!data) {
            // Ran out of audio data
            LOG_INFO("VideoPlayer: audio stream reached end");
            break;
        }

        if(!Pimpl->AudioCodec->FeedRawFrame(data, length)) {
            LOG_ERROR("VideoPlayer: failed to send raw data block to audio codec");
        }

        // Now we have more data so we can loop again to read from the buffer
    }

    return readAmount / bytesPerSample / ChannelCount;
}

size_t VideoPlayer::ReadDataFromAudioQueue(Lock& audiolocked, uint8_t* output, size_t amount)
{
    if(Pimpl->_PendingAudioData.Empty || Pimpl->_PendingAudioData.Data.empty() || amount == 0)
        return 0;

    const auto* readPtr =
        Pimpl->_PendingAudioData.Data.data() + Pimpl->_PendingAudioData.NextReadOffset;

    const auto dataAvailable =
        Pimpl->_PendingAudioData.Data.size() - Pimpl->_PendingAudioData.NextReadOffset;

    if(dataAvailable <= amount) {
        // Can read all
        std::memcpy(output, readPtr, dataAvailable);
        Pimpl->_PendingAudioData.Clear();
        return dataAvailable;
    } else {

        // Can only read part of the buffer
        std::memcpy(output, readPtr, amount);

        // Update read pos for next call
        Pimpl->_PendingAudioData.NextReadOffset += amount;
        return amount;
    }
}
// ------------------------------------ //
void VideoPlayer::OnStreamEndReached()
{
    auto vars = NamedVars::MakeShared<NamedVars>();

    vars->AddVar(std::make_shared<NamedVariableList>("oldvideo", new StringBlock(VideoFile)));

    Stop();
    OnPlayBackEnded.Call(vars);
}
// ------------------------------------ //
DLLEXPORT int VideoPlayer::OnEvent(Event* event)
{
    switch(event->GetType()) {
    case EVENT_TYPE_FRAME_BEGIN: {

        // If we are no longer playing, unregister
        if(!IsPlaying)
            return -1;

        if(!IsStreamValid()) {

            LOG_WARNING("VideoPlayer: Stream is invalid, closing playback");
            OnStreamEndReached();
            return -1;
        }

        const auto data = event->GetFloatDataForEvent();
        if(!data) {
            LOG_FATAL("VideoPlayer: got event with no elapsed data");
            return -1;
        }

        const auto elapsed = data->FloatDataValue;

        PassedTimeSeconds += elapsed;


        // Start playing audio. Hopefully at the same time as the first frame of the
        // video is decoded
        if(!IsPlayingAudio && HasAudioStream) {

            LOG_INFO("VideoPlayer: Starting audio playback from the video...");

            AudioStreamData = alure::MakeShared<Sound::ProceduralSoundData>(
                [=](void* output, unsigned amount) -> unsigned {
                    return this->ReadAudioData(static_cast<uint8_t*>(output), amount);
                },
                AudioStreamDataProperties);

            AudioStream =
                Engine::Get()->GetSoundDevice()->CreateProceduralSound(AudioStreamData);
            IsPlayingAudio = true;
        }

        bool stillGood = HandleFrameVideoUpdate();


        if(!stillGood)
            return -1;
        return 0;
    }
    default:
        // Unregister from other events
        return -1;
    }
}

DLLEXPORT int VideoPlayer::OnGenericEvent(GenericEvent* event)
{
    return 0;
}
