// ------------------------------------ //
#include "VideoPlayer.h"

#include "Common/DataStoring/DataBlock.h"

#include "Engine.h"

#include "bsfCore/Image/BsTexture.h"

#include <filesystem>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

// // Ogre::PF_R8G8B8A8 results in incorrect images, for some reason
// // RGBA is an alias for: PF_BYTE_RGBA = PF_A8B8G8R8 so that probably explains.
// // Luckily it seems ffmpeg RGBA is also in the same order
// constexpr Ogre::PixelFormat OGRE_IMAGE_FORMAT = Ogre::PF_BYTE_RGBA;

constexpr auto BS_PIXEL_FORMAT = bs::PF_RGBA8;

// This must match the above definition
constexpr AVPixelFormat FFMPEG_DECODE_TARGET = AV_PIX_FMT_RGBA;

// TODO: add support for disabling alpha if not needed
// constexpr Ogre::PixelFormat OGRE_IMAGE_FORMAT_NO_ALPHA = Ogre::PF_BYTE_RGB;
// // This must match the above definition
// constexpr AVPixelFormat FFMPEG_DECODE_TARGET_NO_ALPHA = AV_PIX_FMT_RGB24;


DLLEXPORT VideoPlayer::VideoPlayer() {}

DLLEXPORT VideoPlayer::~VideoPlayer()
{
    UnRegisterAllEvents();

    // Ensure all FFMPEG resources are closed
    Stop();
}
// ------------------------------------ //
DLLEXPORT bool VideoPlayer::Play(const std::string& videofile)
{
    // Make sure nothing is playing currently //
    Stop();

    // Make sure ffmpeg is loaded //
    LoadFFMPEG();

    if(!std::filesystem::exists(videofile)) {

        LOG_ERROR("VideoPlayer: Play: file doesn't exist: " + videofile);
        return false;
    }

    VideoFile = videofile;

    // Parse stream data to know how big our textures need to be //
    if(!FFMPEGLoadFile()) {

        LOG_ERROR("VideoPlayer: Play: ffmpeg failed to parse / setup playback for the file");
        Stop();
        return false;
    }

    if(!OnVideoDataLoaded()) {

        LOG_ERROR("VideoPlayer: Play: output video texture creation failed");
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
    // Close all ffmpeg resources //
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

    // Dump remaining packet data frames //
    {
        Lock lock(ReadPacketMutex);

        WaitingVideoPackets.clear();
        WaitingAudioPackets.clear();
    }

    // Close down audio portion //
    {
        Lock lock(AudioMutex);

        // PlayingSource = nullptr;
        ReadAudioDataBuffer.clear();
    }

    // Video and Audio codecs are released by Context, but we still free them here?
    if(VideoCodec)
        avcodec_free_context(&VideoCodec);
    if(AudioCodec)
        avcodec_free_context(&AudioCodec);

    VideoCodec = nullptr;
    AudioCodec = nullptr;

    if(ImageConverter) {

        sws_freeContext(ImageConverter);
        ImageConverter = nullptr;
    }

    if(AudioConverter)
        swr_free(&AudioConverter);

    // These are set to null automatically //
    if(DecodedFrame)
        av_frame_free(&DecodedFrame);
    if(DecodedAudio)
        av_frame_free(&DecodedAudio);
    if(ConvertedFrameBuffer)
        av_freep(&ConvertedFrameBuffer);
    if(ConvertedFrame)
        av_frame_free(&ConvertedFrame);

    // if(ResourceReader){

    //     if(ResourceReader->buffer){

    //         av_free(ResourceReader->buffer);
    //         ResourceReader->buffer = nullptr;
    //     }

    //     av_free(ResourceReader);
    //     ResourceReader = nullptr;
    // }

    if(FormatContext) {
        // The doc says this is the right method to close it after
        // avformat_open_input has succeeded

        // avformat_free_context(FormatContext);
        avformat_close_input(&FormatContext);
        FormatContext = nullptr;
    }

    // if(VideoParser){
    //     av_parser_close(VideoParser);
    //     VideoParser = nullptr;
    // }

    // if(AudioParser){
    //     av_parser_close(AudioParser);
    //     AudioParser = nullptr;
    // }


    // Let go of our textures and things //
    VideoFile = "";

    VideoOutputTexture = nullptr;
    ClearDataBuffers();

    IsPlaying = false;
}
// ------------------------------------ //
DLLEXPORT float VideoPlayer::GetDuration() const
{
    if(!FormatContext)
        return 0;

    return static_cast<float>(FormatContext->duration) / AV_TIME_BASE;
}
// ------------------------------------ //
bool VideoPlayer::OnVideoDataLoaded()
{
    auto buffer = GetNextDataBuffer();
    if(!buffer)
        return false;

    VideoOutputTexture = bs::Texture::create(*buffer, bs::TU_DYNAMIC);
    return true;
}
// ------------------------------------ //
bool VideoPlayer::FFMPEGLoadFile()
{
    FormatContext = avformat_alloc_context();
    if(!FormatContext) {
        LOG_ERROR("VideoPlayer: FFMPEG: avformat_alloc_context failed");
        return false;
    }

    // Not using custom reader
    // FormatContext->pb = ResourceReader;

    if(avformat_open_input(&FormatContext, VideoFile.c_str(), nullptr, nullptr) < 0) {

        // Context was freed automatically
        FormatContext = nullptr;
        LOG_ERROR("VideoPlayer: FFMPEG: FFMPEG failed to open video stream file resource");
        return false;
    }

    if(avformat_find_stream_info(FormatContext, nullptr) < 0) {

        LOG_ERROR("VideoPlayer: FFMPEG: Failed to read video stream info");
        return false;
    }

    // Find audio and video streams //
    unsigned int foundVideoStreamIndex = std::numeric_limits<unsigned int>::max();
    unsigned int foundAudioStreamIndex = std::numeric_limits<unsigned int>::max();

    for(unsigned int i = 0; i < FormatContext->nb_streams; ++i) {

        if(FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

            foundVideoStreamIndex = i;
            continue;
        }

        if(FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {

            foundAudioStreamIndex = i;
            continue;
        }
    }

    // Fail if didn't find a stream //
    if(foundVideoStreamIndex >= FormatContext->nb_streams) {

        LOG_WARNING("VideoPlayer: FFMPEG: Video didn't have a video stream");
        return false;
    }


    if(foundVideoStreamIndex < FormatContext->nb_streams) {

        // Found a video stream, play it
        if(!OpenStream(foundVideoStreamIndex, true)) {

            LOG_ERROR("VideoPlayer: FFMPEG: Failed to open video stream");
            return false;
        }
    }

    if(foundAudioStreamIndex < FormatContext->nb_streams) {

        // Found an audio stream, play it
        if(!OpenStream(foundAudioStreamIndex, false)) {

            LOG_WARNING("VideoPlayer: FFMPEG: Failed to open audio stream, "
                        "playing without audio");
        }
    }

    DecodedFrame = av_frame_alloc();
    ConvertedFrame = av_frame_alloc();
    DecodedAudio = av_frame_alloc();

    if(!DecodedFrame || !ConvertedFrame || !DecodedAudio) {
        LOG_ERROR("VideoPlayer: FFMPEG: av_frame_alloc failed");
        return false;
    }

    FrameWidth = FormatContext->streams[foundVideoStreamIndex]->codecpar->width;
    FrameHeight = FormatContext->streams[foundVideoStreamIndex]->codecpar->height;

    // Calculate required size for the converted frame
    ConvertedBufferSize =
        av_image_get_buffer_size(FFMPEG_DECODE_TARGET, FrameWidth, FrameHeight, 1);

    ConvertedFrameBuffer =
        reinterpret_cast<uint8_t*>(av_malloc(ConvertedBufferSize * sizeof(uint8_t)));

    if(!ConvertedFrameBuffer) {
        LOG_ERROR("VideoPlayer: FFMPEG: av_malloc failed for ConvertedFrameBuffer");
        return false;
    }

    if(ConvertedBufferSize != static_cast<size_t>(FrameWidth * FrameHeight * 4)) {

        LOG_ERROR("VideoPlayer: FFMPEG: FFMPEG and Ogre image data sizes don't match! "
                  "Check selected formats");
        return false;
    }

    if(av_image_fill_arrays(ConvertedFrame->data, ConvertedFrame->linesize,
           ConvertedFrameBuffer, FFMPEG_DECODE_TARGET, FrameWidth, FrameHeight, 1) < 0) {
        LOG_ERROR("VideoPlayer: FFMPEG: av_image_fill_arrays failed");
        return false;
    }

    // Converting images to be ogre compatible is done by this
    // TODO: allow controlling how good conversion is done
    // SWS_FAST_BILINEAR is the fastest
    ImageConverter = sws_getContext(FrameWidth, FrameHeight,
        static_cast<AVPixelFormat>(FormatContext->streams[VideoIndex]->codecpar->format),
        FrameWidth, FrameHeight, FFMPEG_DECODE_TARGET, SWS_BICUBIC, nullptr, nullptr, nullptr);

    if(!ImageConverter) {

        LOG_ERROR("VideoPlayer: FFMPEG: sws_getContext failed");
        return false;
    }

    if(AudioCodec) {

        // Setup audio playing //
        SampleRate = AudioCodec->sample_rate;
        ChannelCount = AudioCodec->channels;

        // This may or may not be a limitation anymore
        // Especially not sure the channel count applies with SFML
        if(ChannelCount <= 0 || ChannelCount > 2) {

            LOG_ERROR("VideoPlayer: FFMPEG: Unsupported audio channel count, "
                      "only 1 or 2 are supported");
            return false;
        }

        // cAudio expects AV_SAMPLE_FMT_S16
        // AudioCodec->sample_fmt;
        if(av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) != 2) {

            LOG_FATAL("AV_SAMPLE_FMT_S16 size has changed");
            return false;
        }

        AudioConverter = swr_alloc();

        if(!AudioConverter) {

            LOG_ERROR("VideoPlayer: FFMPEG: swr_alloc failed");
            return false;
        }

        const auto ChannelLayout = AudioCodec->channel_layout != 0 ?
                                       AudioCodec->channel_layout :
                                       // Guess
                                       av_get_default_channel_layout(AudioCodec->channels);

        // Check above for the note about the target audio format
        AudioConverter = swr_alloc_set_opts(AudioConverter, ChannelLayout, AV_SAMPLE_FMT_S16,
            AudioCodec->sample_rate, ChannelLayout, AudioCodec->sample_fmt,
            AudioCodec->sample_rate, 0, nullptr);

        if(swr_init(AudioConverter) < 0) {

            LOG_ERROR("VideoPlayer: FFMPEG: Failed to initialize audio converter for stream");
            return false;
        }

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

        AudioStreamDataProperties.SampleType = alure::SampleType::Int16;

        if(!valid) {
            LOG_ERROR("VideoPlayer: invalid channel configuration for audio: " +
                      std::to_string(ChannelCount));
            return false;
        }
    }

    DumpInfo();

    // Clock is now reset in the callback
    FirstCallbackAfterPlay = true;

    PassedTimeSeconds = 0.f;
    NextFrameReady = false;
    // This is -1 to make this smaller than 0
    CurrentlyDecodedTimeStamp = -1.f;

    StreamValid = true;

    LOG_INFO("VideoPlayer: successfully opened all the ffmpeg streams for video file");

    return true;
}

bool VideoPlayer::OpenStream(unsigned int index, bool video)
{
    auto* thisStreamCodec =
        avcodec_find_decoder(FormatContext->streams[index]->codecpar->codec_id);

    if(!thisStreamCodec) {

        LOG_ERROR("VideoPlayer: FFMPEG: unsupported codec used in video file");
        return false;
    }

    auto* thisCodecContext = avcodec_alloc_context3(thisStreamCodec);

    if(!thisCodecContext) {

        LOG_ERROR("VideoPlayer: FFMPEG: failed to allocate codec context");
        return false;
    }

    // Try copying parameters //
    if(avcodec_parameters_to_context(
           thisCodecContext, FormatContext->streams[index]->codecpar) < 0) {
        avcodec_free_context(&thisCodecContext);
        LOG_ERROR("VideoPlayer: FFMPEG: failed to copy parameters to codec context");
        return false;
    }

    // Open the codec this is important to avoid segfaulting //
    // FFMPEG documentation warns that this is not thread safe
    const auto codecOpenResult = avcodec_open2(thisCodecContext, thisStreamCodec, nullptr);

    if(codecOpenResult < 0) {

        std::string errorMessage;
        errorMessage.resize(40);
        memset(&errorMessage[0], ' ', errorMessage.size());
        av_strerror(codecOpenResult, &errorMessage[0], errorMessage.size());

        LOG_ERROR("VideoPlayer: FFMPEG: Error opening codec context: " + errorMessage);

        avcodec_free_context(&thisCodecContext);
        LOG_ERROR("VideoPlayer: FFMPEG: codec failed to open");
        return false;
    }

    // This should probably be done by the caller of this method...
    if(video) {

        // VideoParser = ThisCodecParser;
        VideoCodec = thisCodecContext;
        VideoIndex = static_cast<int>(index);
        VideoTimeBase = static_cast<float>(FormatContext->streams[index]->time_base.num) /
                        static_cast<float>(FormatContext->streams[index]->time_base.den);
        // VideoTimeBase = static_cast<float>(VideoCodec->time_base.num) /
        //     static_cast<float>(VideoCodec->time_base.den);

    } else {

        // AudioParser = ThisCodecParser;
        AudioCodec = thisCodecContext;
        AudioIndex = static_cast<int>(index);
    }

    return true;
}
// ------------------------------------ //
bool VideoPlayer::DecodeVideoFrame()
{
    const auto result = avcodec_receive_frame(VideoCodec, DecodedFrame);

    if(result >= 0) {

        // Worked //

        // Convert the image from its native format to RGB
        if(sws_scale(ImageConverter, DecodedFrame->data, DecodedFrame->linesize, 0,
               FrameHeight, ConvertedFrame->data, ConvertedFrame->linesize) < 0) {
            // Failed to convert frame //
            LOG_ERROR("Converting video frame failed");
            return false;
        }

        // Seems like DecodedFrame->pts contains garbage
        // and packet.pts is the timestamp in VideoCodec->time_base
        // so we access that through pkt_pts
        // CurrentlyDecodedTimeStamp = DecodedFrame->pkt_pts * VideoTimeBase;
        // VideoTimeBase = VideoCodec->time_base.num / VideoCodec->time_base.den;
        // CurrentlyDecodedTimeStamp = DecodedFrame->pkt_pts * VideoTimeBase;

        // Seems that the latest FFMPEG version has fixed this.
        // I would put this in a #IF macro bLock if ffmpeg provided a way to check the
        // version at compile time
        CurrentlyDecodedTimeStamp = DecodedFrame->pts * VideoTimeBase;
        return true;
    }

    if(result == AVERROR(EAGAIN)) {

        // Waiting for data //
        return false;
    }

    LOG_ERROR("VideoPlayer: DecodeVideoFrame: frame receive failed, error: " +
              std::to_string(result));
    return false;
}

VideoPlayer::PacketReadResult VideoPlayer::ReadOnePacket(
    Lock& packetmutex, DecodePriority priority)
{
    if(!FormatContext || !StreamValid)
        return PacketReadResult::Ended;

    // Decode queued packets first
    if(priority == DecodePriority::Video && !WaitingVideoPackets.empty()) {

        // Try to send it //
        const auto Result =
            avcodec_send_packet(VideoCodec, &WaitingVideoPackets.front()->packet);

        if(Result == AVERROR(EAGAIN)) {

            // Still wailing to send //
            return PacketReadResult::QueueFull;
        }

        if(Result < 0) {

            // An error occured //
            LOG_ERROR("Video stream send error from queue, stopping playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        // Successfully sent the first in the queue //
        WaitingVideoPackets.pop_front();
        return PacketReadResult::Ok;
    }

    if(priority == DecodePriority::Audio && !WaitingAudioPackets.empty()) {

        // Try to send it //
        const auto Result =
            avcodec_send_packet(AudioCodec, &WaitingAudioPackets.front()->packet);

        if(Result == AVERROR(EAGAIN)) {

            // Still wailing to send //
            return PacketReadResult::QueueFull;
        }

        if(Result < 0) {

            // An error occured //
            LOG_ERROR("VideoPlayer: Audio stream send error from queue, stopping playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        // Successfully sent the first in the queue //
        WaitingAudioPackets.pop_front();
        return PacketReadResult::Ok;
    }

    // If we had nothing in the right queue try to read more frames //

    AVPacket Packet;
    // av_init_packet(&packet);

    if(av_read_frame(FormatContext, &Packet) < 0) {

        // Stream ended //
        // av_packet_unref(&packet);
        return PacketReadResult::Ended;
    }

    if(!StreamValid) {

        av_packet_unref(&Packet);
        return PacketReadResult::Ended;
    }

    // Is this a packet from the video stream?
    if(Packet.stream_index == VideoIndex) {

        // If not wanting this stream don't send it //
        if(priority != DecodePriority::Video) {

            WaitingVideoPackets.push_back(
                std::unique_ptr<ReadPacket>(new ReadPacket(&Packet)));

            return PacketReadResult::Ok;
        }

        // Send it to the decoder //
        const auto Result = avcodec_send_packet(VideoCodec, &Packet);

        if(Result == AVERROR(EAGAIN)) {

            // Add to queue //
            WaitingVideoPackets.push_back(
                std::unique_ptr<ReadPacket>(new ReadPacket(&Packet)));
            return PacketReadResult::QueueFull;
        }

        av_packet_unref(&Packet);

        if(Result < 0) {

            LOG_ERROR("VideoPlayer:Video stream send error, stopping playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        return PacketReadResult::Ok;

    } else if(Packet.stream_index == AudioIndex && AudioCodec) {

        // If audio codec is null audio playback is disabled //

        // If not wanting this stream don't send it //
        if(priority != DecodePriority::Audio) {

            WaitingAudioPackets.push_back(
                std::unique_ptr<ReadPacket>(new ReadPacket(&Packet)));
            return PacketReadResult::Ok;
        }

        const auto Result = avcodec_send_packet(AudioCodec, &Packet);

        if(Result == AVERROR(EAGAIN)) {

            // Add to queue //
            WaitingAudioPackets.push_back(
                std::unique_ptr<ReadPacket>(new ReadPacket(&Packet)));
            return PacketReadResult::QueueFull;
        }

        av_packet_unref(&Packet);

        if(Result < 0) {

            LOG_ERROR("Audio stream send error, stopping audio playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        // This is probably not needed? and was an error before
        // av_packet_unref(&Packet);
        return PacketReadResult::Ok;
    }

    // Unknown stream, ignore
    av_packet_unref(&Packet);
    return PacketReadResult::Ok;
}
// ------------------------------------ //
void VideoPlayer::UpdateTexture()
{
    auto buffer = GetNextDataBuffer();
    if(!buffer) {
        LOG_WARNING("VideoPlayer: all texture data buffers are still locked, skipping update");
        return;
    }

    std::memcpy((*buffer)->getData(), ConvertedFrameBuffer, ConvertedBufferSize);
    VideoOutputTexture->writeData(*buffer, 0, 0, true);
}

bs::SPtr<bs::PixelData> VideoPlayer::_OnNewBufferNeeded()
{
    return bs::PixelData::create(FrameWidth, FrameHeight, 1, BS_PIXEL_FORMAT);
}
// ------------------------------------ //
size_t VideoPlayer::ReadAudioData(uint8_t* output, size_t amount)
{
    Lock lock(AudioMutex);
    Lock lock2(ReadPacketMutex);

    if(!AudioCodec || !StreamValid) {
        return 0;
    }

    // This is verified in open when setting up converting
    // av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) could also be used here
    const auto bytesPerSample = 2;
    amount *= bytesPerSample * ChannelCount;

    size_t readAmount = 0;

    // Read audio data until the stream ends or we have reached amount
    while(amount > 0) {

        // First return from queue //
        if(!ReadAudioDataBuffer.empty()) {

            // Try to read from the queue //
            const auto read = ReadDataFromAudioQueue(lock, output, amount);

            if(read == 0) {

                // Queue is invalid... //
                LOG_ERROR("Invalid audio queue, emptying the queue");
                ReadAudioDataBuffer.clear();
                continue;
            }

            // Adjust pointer and amount and try to read again if still some size left
            readAmount += read;
            output += read;
            amount -= read;
            continue;
        }

        const auto result = avcodec_receive_frame(AudioCodec, DecodedAudio);

        if(result == AVERROR(EAGAIN)) {

            if(this->ReadOnePacket(lock2, DecodePriority::Audio) == PacketReadResult::Ended) {

                // Stream ended //
                return readAmount / bytesPerSample / ChannelCount;
            }

            // We have now read more data, try decoding again
            continue;
        }

        if(result < 0) {

            // Some error //
            LOG_ERROR("Failed receiving audio packet, stopping audio playback");
            StreamValid = false;
            return 0;
        }

        // Received audio data //

        const auto totalSize = bytesPerSample * (DecodedAudio->nb_samples * ChannelCount);

        if(amount >= static_cast<size_t>(totalSize)) {

            // Lets try to directly feed the converted data to the requester //
            if(swr_convert(AudioConverter, &output, totalSize,
                   const_cast<const uint8_t**>(DecodedAudio->data),
                   DecodedAudio->nb_samples) < 0) {
                LOG_ERROR("Invalid audio stream, converting to audio read buffer failed");
                StreamValid = false;
                return 0;
            }

            // Adjust pointer and amount and try to read again if still some size left
            readAmount += totalSize;
            output += totalSize;
            amount -= totalSize;
            continue;
        }

        // We need a buffer //
        ReadAudioPacket* buffer = nullptr;

        // Try to find a recycled one
        for(auto iter = ReadAudioDataBuffer.begin(); iter != ReadAudioDataBuffer.end();
            ++iter) {

            if((*iter)->Played) {

                buffer = iter->get();

                // It needs to be moved to the back to play them in order
                ReadAudioDataBuffer.splice(
                    ReadAudioDataBuffer.end(), ReadAudioDataBuffer, iter);
                break;
            }
        }

        // Or make a new one
        if(!buffer) {
            ReadAudioDataBuffer.emplace_back(new ReadAudioPacket());
            buffer = ReadAudioDataBuffer.back().get();
        }

        buffer->DecodedData.resize(totalSize);

        uint8_t* decodeOutput = buffer->DecodedData.data();

        // Convert into the output data
        if(swr_convert(AudioConverter, &decodeOutput, totalSize,
               const_cast<const uint8_t**>(DecodedAudio->data),
               DecodedAudio->nb_samples) < 0) {
            LOG_ERROR("Invalid audio stream, converting failed");
            StreamValid = false;
            return 0;
        }

        LEVIATHAN_ASSERT(decodeOutput == buffer->DecodedData.data(),
            "Audio convert messed with our pointer");

        // Reset played and offset
        buffer->Played = false;
        buffer->CurrentReadOffset = 0;

        // Now we have more data so we can loop again to read from the buffer
        continue;
    }

    return readAmount / bytesPerSample / ChannelCount;
}

size_t VideoPlayer::ReadDataFromAudioQueue(Lock& audiolocked, uint8_t* output, size_t amount)
{
    // Find one that wasn't played
    for(auto iter = ReadAudioDataBuffer.begin(); iter != ReadAudioDataBuffer.end(); ++iter) {

        if((*iter)->Played)
            continue;

        // This hasn't been played
        auto buffer = iter->get();
        auto& dataVector = buffer->DecodedData;

        const auto dataLeft = dataVector.size() - buffer->CurrentReadOffset;

        if(amount >= dataLeft) {

            // Can move an entire packet //
            std::memcpy(output, dataVector.data() + buffer->CurrentReadOffset, dataLeft);

            // Mark it as played
            ReadAudioDataBuffer.pop_front();

            return dataLeft;
        }

        // Only partial data is wanted
        std::memcpy(output, dataVector.data() + buffer->CurrentReadOffset, amount);

        buffer->CurrentReadOffset += amount;
        return amount;
    }

    return 0;
}

// ------------------------------------ //
void VideoPlayer::ResetClock()
{
    LastUpdateTime = ClockType::now();
}

void VideoPlayer::OnStreamEndReached()
{
    auto vars = NamedVars::MakeShared<NamedVars>();

    vars->AddVar(std::make_shared<NamedVariableList>("oldvideo", new StringBlock(VideoFile)));

    Stop();
    OnPlayBackEnded.Call(vars);
}

void VideoPlayer::SeekVideo(float time)
{

    if(time < 0)
        time = 0;

    const auto seekPos = static_cast<uint64_t>(time * AV_TIME_BASE);

    const auto timeStamp = av_rescale_q(seekPos,
#ifdef _MSC_VER
        // Copy pasted from the definition of AV_TIME_BASE_Q
        // TODO: check is this still required
        AVRational{1, AV_TIME_BASE},
#else
        AV_TIME_BASE_Q,
#endif
        FormatContext->streams[VideoIndex]->time_base);

    av_seek_frame(FormatContext, VideoIndex, timeStamp, AVSEEK_FLAG_BACKWARD);

    LOG_WARNING("VideoPlayer: SeekVideo: audio seeking not implemented!");
}
// ------------------------------------ //
void VideoPlayer::DumpInfo() const
{

    if(FormatContext) {
        // Passing VideoFile here passes the name onto output, it's not needed
        // but it differentiates the output by file name
        av_dump_format(FormatContext, 0, VideoFile.c_str(), 0);
    }
}
// ------------------------------------ //
DLLEXPORT int VideoPlayer::OnEvent(Event* event)
{

    switch(event->GetType()) {
    case EVENT_TYPE_FRAME_BEGIN: {

        // If we are no longer player unregister
        if(!IsPlaying)
            return -1;

        if(!IsStreamValid()) {

            LOG_WARNING("VideoPlayer: Stream is invalid, closing playback");
            OnStreamEndReached();
            return -1;
        }

        const auto now = ClockType::now();

        if(FirstCallbackAfterPlay) {

            // Reset time now to get an accurate starting time
            LastUpdateTime = now;

            FirstCallbackAfterPlay = false;
        }

        const auto elapsed = now - LastUpdateTime;
        LastUpdateTime = now;

        PassedTimeSeconds +=
            std::chrono::duration_cast<std::chrono::duration<float>>(elapsed).count();

        // Start playing audio. Hopefully at the same time as the first frame of the
        // video is decoded
        if(!IsPlayingAudio && AudioCodec) {

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

        // This loops until we are displaying a frame we should be showing at this time //
        while(CurrentlyDecodedTimeStamp <= PassedTimeSeconds) {

            // Make sure the next frame is ready
            if(!NextFrameReady) {
                Lock lock(ReadPacketMutex);

                while(!NextFrameReady) {

                    // Decode a packet if none are in queue
                    if(ReadOnePacket(lock, DecodePriority::Video) == PacketReadResult::Ended) {

                        // There are no more frames, end the playback
                        lock.unlock();
                        OnStreamEndReached();
                        return -1;
                    }

                    NextFrameReady = DecodeVideoFrame();
                }
            }

            // Don't show a frame yet if it is too soon
            if(PassedTimeSeconds < CurrentlyDecodedTimeStamp)
                break;

            UpdateTexture();
            NextFrameReady = false;
        }

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

// ------------------------------------ //
static bool FFMPEGLoadedAlready = false;
static Mutex FFMPEGLoadMutex;

namespace Leviathan {

//! This is for storing the ffmpeg output lines until a full line is outputted
static std::string FFMPEGOutBuffer = "";
static Mutex FFMPEGOutBufferMutex;

//! \brief Custom callback for ffmpeg to pipe output to our logger class
void FFMPEGCallback(void* ptr, int level, const char* fmt, va_list varg)
{

    if(level > AV_LOG_INFO)
        return;

    // Format message //
    std::string formatedMessage;

    constexpr auto FORMAT_BUFFER_SIZE = 1024;
    char strBuffer[FORMAT_BUFFER_SIZE];
    static int print_prefix = 1;

    int result = av_log_format_line2(
        ptr, level, fmt, varg, strBuffer, sizeof(strBuffer), &print_prefix);

    if(result < 0 || result >= FORMAT_BUFFER_SIZE) {

        LOG_WARNING("FFMPEG log message was too long and is truncated");
    }

    formatedMessage = strBuffer;

    // Store the output until it ends with a newline, then output it //
    Lock lock(FFMPEGOutBufferMutex);

    FFMPEGOutBuffer += formatedMessage;

    if(FFMPEGOutBuffer.empty() || FFMPEGOutBuffer.back() != '\n')
        return;

    FFMPEGOutBuffer.pop_back();

    if(level <= AV_LOG_FATAL) {

        LOG_ERROR("[FFMPEG FATAL] " + FFMPEGOutBuffer);

    } else if(level <= AV_LOG_ERROR) {

        LOG_ERROR("[FFMPEG] " + FFMPEGOutBuffer);

    } else if(level <= AV_LOG_WARNING) {

        LOG_WARNING("[FFMPEG] " + FFMPEGOutBuffer);

    } else {

        LOG_INFO("[FFMPEG] " + FFMPEGOutBuffer);
    }

    FFMPEGOutBuffer.clear();
}

} // namespace Leviathan

void VideoPlayer::LoadFFMPEG()
{

    // Makes sure all threads can pass only when ffmpeg is loaded
    Lock lock(FFMPEGLoadMutex);

    if(FFMPEGLoadedAlready)
        return;

    FFMPEGLoadedAlready = true;

    av_log_set_callback(Leviathan::FFMPEGCallback);

    avcodec_register_all();
    av_register_all();
}
