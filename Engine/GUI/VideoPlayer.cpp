// ------------------------------------ //
#include "VideoPlayer.h"

#include "Common/DataStoring/DataBlock.h"

#include "Engine.h"

#include "OgreHardwarePixelBuffer.h"
#include "OgrePixelBox.h"
#include "OgrePixelFormat.h"
#include "OgreTextureManager.h"

#include <boost/filesystem.hpp>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

// Ogre::PF_R8G8B8A8 results in incorrect images, for some reason
constexpr Ogre::PixelFormat OGRE_IMAGE_FORMAT = Ogre::PF_BYTE_RGBA;
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

std::atomic<int> VideoPlayer::TextureSequenceNumber = {0};
// ------------------------------------ //
DLLEXPORT bool VideoPlayer::Play(const std::string& videofile)
{
    // Make sure nothing is playing currently //
    Stop();

    // Make sure ffmpeg is loaded //
    LoadFFMPEG();

    if(!boost::filesystem::exists(videofile)) {

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

    // If Ogre isn't initialized we are going to pretend that we worked for testing purposes
    if(!Ogre::TextureManager::getSingletonPtr()) {

        LOG_INFO("VideoPlayer: Ogre hasn't been initialized fully (no TextureManager), "
                 "failing but pretending to have worked");
        return true;
    }

    if(!OnVideoDataLoaded()) {

        VideoOutputTexture.reset();
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

    // Clear stored video frames //
    DecodedVideoDataBuffer.clear();


    // Close down audio portion //
    {
        Lock lock(AudioMutex);
        DecodedAudioDataBuffer.clear();
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
    TextureName = "";

    if(VideoOutputTexture)
        Ogre::TextureManager::getSingleton().remove(VideoOutputTexture);

    VideoOutputTexture.reset();

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
    const int number = TextureSequenceNumber++;

    TextureName = "Leviathan_VideoPlayer_" + std::to_string(number);

    VideoOutputTexture = Ogre::TextureManager::getSingleton().createManual(TextureName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, FrameWidth,
        FrameHeight, 0, OGRE_IMAGE_FORMAT, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE, 0,
        // Gamma correction
        true);

    if(VideoOutputTexture.isNull()) {
        LOG_ERROR("VideoPlayer: Failed to create video output texture");
        return false;
    }

    LOG_INFO("VideoPlayer: Created video texture: " + std::to_string(FrameWidth) + "x" +
             std::to_string(FrameHeight));

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

    if(ConvertedBufferSize != static_cast<size_t>(FrameWidth * FrameHeight * 4)) {

        LOG_ERROR("VideoPlayer: FFMPEG: FFMPEG and Ogre image data sizes don't match! "
                  "Check selected formats");
        return false;
    }

    ConvertedFrameBuffer =
        reinterpret_cast<uint8_t*>(av_malloc(ConvertedBufferSize * sizeof(uint8_t)));

    if(!ConvertedFrameBuffer) {
        LOG_ERROR("VideoPlayer: FFMPEG: av_malloc failed for ConvertedFrameBuffer");
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
        ProceduralSoundData::SoundProperties properties;
        properties.SampleRate = SampleRate;
        properties.SourceName = "VideoPlayer";
        properties.Format =
            ChannelCount > 1 ? cAudio::EAF_16BIT_STEREO : cAudio::EAF_16BIT_MONO;

        AudioStreamData = ProceduralSoundData::MakeShared<ProceduralSoundData>(
            [=](void* output, int amount) -> int {
                return static_cast<int>(
                    this->ReadAudioData(static_cast<uint8_t*>(output), amount));
            },
            std::move(properties));

        AudioStream = Engine::Get()->GetSoundDevice()->CreateProceduralSound(
            AudioStreamData, VideoFile.c_str());
    }

    DumpInfo();

    FirstCallbackAfterPlay = true;

    PassedTimeSeconds = 0.f;

    StreamValid = true;
    ReadReachedEnd = false;

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

        DecodedVideoData* storetarget = nullptr;

        // Allocate space at once if the size is wrong
        if(DecodedVideoDataBuffer.size() < TARGET_BUFFERED_FRAMES) {
            DecodedVideoDataBuffer.resize(TARGET_BUFFERED_FRAMES);
        }

        // Find a place to store or make space
        for(auto iter = DecodedVideoDataBuffer.begin(); iter != DecodedVideoDataBuffer.end();
            ++iter) {

            if(iter->Played) {
                // Empty
                storetarget = &*iter;
            }
        }

        if(!storetarget) {

            // We allow a variable amount of audio data to be decoded as well so we just need
            // to keep creating these if the audio is not laid out optimally
            DecodedVideoDataBuffer.emplace_back();
            storetarget = &DecodedVideoDataBuffer.back();
        }

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
        const auto frameTime = DecodedFrame->pts * VideoTimeBase;

        // Make sure it is big enough and then just copy
        storetarget->FrameData.resize(ConvertedBufferSize);

        // The data[0] buffer has some junk before the actual data so don't use that
        /*&ConvertedFrame->data[0]*/
        std::memcpy(storetarget->FrameData.data(), ConvertedFrameBuffer, ConvertedBufferSize);

        // Other properties
        storetarget->Timestamp = frameTime;
        storetarget->Played = false;
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

bool VideoPlayer::DecodeAudioFrame()
{
    const auto result = avcodec_receive_frame(AudioCodec, DecodedAudio);

    if(result >= 0) {

        // Worked //
        // This is verified in open when setting up converting
        // av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) could also be used here
        const auto BytesPerSample = 2;

        const auto TotalSize = BytesPerSample * (DecodedAudio->nb_samples * ChannelCount);

        Lock guard(AudioMutex);

        // Find an output buffer or make one
        DecodedAudioData* buffer = nullptr;

        for(auto iter = DecodedAudioDataBuffer.begin(); iter != DecodedAudioDataBuffer.end();
            ++iter) {

            if(iter->Played) {
                // Can reuse this one

                // For that it needs to be moved to the back
                DecodedAudioDataBuffer.splice(
                    DecodedAudioDataBuffer.end(), DecodedAudioDataBuffer, iter);

                buffer = &DecodedAudioDataBuffer.back();
                break;
            }
        }

        if(!buffer) {

            // Nothing to reuse, make new one
            DecodedAudioDataBuffer.emplace_back();
            buffer = &DecodedAudioDataBuffer.back();
        }

        // Make sure it is big enough
        buffer->DecodedData.resize(TotalSize);

        // And convert the data directly into it
        uint8_t* ptr = buffer->DecodedData.data();
        if(swr_convert(AudioConverter, &ptr, TotalSize,
               const_cast<const uint8_t**>(DecodedAudio->data),
               DecodedAudio->nb_samples) < 0) {
            LOG_ERROR("Invalid audio stream, converting to audio read buffer failed");
            StreamValid = false;
            return false;
        }

        LEVIATHAN_ASSERT(ptr == buffer->DecodedData.data(),
            "Audio data decode moved the buffer, this is very bad");

        buffer->Played = false;
        buffer->CurrentReadOffset = 0;
        return true;
    }

    if(result == AVERROR(EAGAIN)) {

        // Waiting for data //
        return false;
    }

    LOG_ERROR("VideoPlayer: DecodeAudioFrame: frame receive failed, error: " +
              std::to_string(result));
    return false;
}

void VideoPlayer::FillPlaybackBuffers()
{
    if(!FormatContext || !StreamValid || ReadReachedEnd)
        return;

    AVPacket packet;
    // av_init_packet(&packet);

    // Read data until we have enough
    while(true) {

        // Check do we have enough frames
        if(CountReadyFrames() >= TARGET_BUFFERED_FRAMES &&
            CountReadyAudioBuffers() >= TARGET_BUFFERED_AUDIO) {
            break;
        }

        if(av_read_frame(FormatContext, &packet) < 0) {

            // Stream ended //
            ReadReachedEnd = true;
            LOG_INFO("VideoPlayer: video data read reached the end");
            break;
        }

        if(!StreamValid) {
            av_packet_unref(&packet);
            break;
        }

        // Is this a packet from the video stream?
        if(packet.stream_index == VideoIndex) {

            // Send it to the decoder //
            while(true) {

                const auto result = avcodec_send_packet(VideoCodec, &packet);

                if(result == AVERROR(EAGAIN)) {

                    // Try to decode
                    if(!DecodeVideoFrame()) {

                        // Failed to decode but the thing is full
                        LOG_ERROR("VideoPlayer: Video stream send error, full but can't "
                                  "decode, stopping playback");
                        StreamValid = false;

                        av_packet_unref(&packet);
                        return;
                    }

                    continue;
                }

                if(result < 0) {

                    LOG_ERROR("VideoPlayer: Video stream send error, stopping playback");
                    StreamValid = false;
                    return;
                }

                // Success
                break;
            }

        } else if(packet.stream_index == AudioIndex && AudioCodec) {
            // If audio codec is null audio playback is disabled (checked above) //


            // Send it to the decoder //
            while(true) {

                const auto result = avcodec_send_packet(AudioCodec, &packet);

                if(result == AVERROR(EAGAIN)) {

                    // Try to decode
                    if(!DecodeAudioFrame()) {

                        // Failed to decode but the thing is full
                        LOG_ERROR("VideoPlayer: Audio stream send error, full but can't "
                                  "decode, stopping playback");
                        StreamValid = false;

                        av_packet_unref(&packet);
                        return;
                    }

                    continue;
                }

                if(result < 0) {

                    LOG_ERROR("VideoPlayer: Audio stream send error, stopping playback");
                    StreamValid = false;
                    return;
                }

                // Success
                break;
            }

        } else {
            // Unknown stream, ignore
        }

        av_packet_unref(&packet);
    }

    // Lock guard(AudioMutex);
    // LOG_INFO("Ready frames: " + std::to_string(CountReadyFrames()) +
    //          ", ready audio count: " + std::to_string(DecodedAudioDataBuffer.size()));
}
// ------------------------------------ //
void VideoPlayer::UpdateTexture(DecodedVideoData& frametoshow)
{
    Ogre::PixelBox pixelView(
        FrameWidth, FrameHeight, 1, OGRE_IMAGE_FORMAT, frametoshow.FrameData.data());

    Ogre::v1::HardwarePixelBufferSharedPtr buffer = VideoOutputTexture->getBuffer();
    buffer->blitFromMemory(pixelView);

    frametoshow.Played = true;
}
// ------------------------------------ //
size_t VideoPlayer::ReadAudioData(uint8_t* output, size_t amount)
{
    if(amount < 1 || !StreamValid) {
        return 0;
    }

    size_t readAmount = 0;

    {
        Lock lock(AudioMutex);

        for(auto iter = DecodedAudioDataBuffer.begin(); iter != DecodedAudioDataBuffer.end();
            ++iter) {

            // Skip done buffers
            if(iter->Played)
                continue;

            const size_t dataAvailable = iter->DecodedData.size() - iter->CurrentReadOffset;

            if(dataAvailable <= amount) {

                // Can copy the whole thing
                std::memcpy(
                    output, iter->DecodedData.data() + iter->CurrentReadOffset, dataAvailable);

                // This buffer is now fully played
                iter->Played = true;

                readAmount += dataAvailable;

                // Reduce the amount left
                amount -= dataAvailable;

                // And adjust the output ptr
                output += dataAvailable;

            } else {

                // Can only play a part of it
                std::memcpy(
                    output, iter->DecodedData.data() + iter->CurrentReadOffset, amount);

                // Adjust the offset
                iter->CurrentReadOffset += amount;

                // We couldn't fit all the data so we can end now that the request amount of
                // bytes has been written
                break;
            }
        }
    }

    // Return the amount of bytes we managed to read, if any
    if(readAmount > 0)
        return readAmount;

    // Now we only return already decoded audio data. If there is none
    // then we have to play some silence
    // TODO: maybe it would be better to stop and restart the audio playback here
    LOG_ERROR("VideoPlayer: ReadAudioData: audio thread wanted more audio data before it was "
              "decoded!");

    const auto BytesPerSample = 2;
    // Maybe take SampleRate into account here?
    const auto silence = BytesPerSample * (1024 * ChannelCount);

    const auto size = std::min(amount, static_cast<size_t>(silence));

    std::memset(output, 0, size);
    return size;
}
// ------------------------------------ //
int VideoPlayer::CountReadyFrames() const
{
    int count = 0;

    for(const auto& buffer : DecodedVideoDataBuffer) {
        if(!buffer.Played)
            ++count;
    }

    return count;
}

int VideoPlayer::CountReadyAudioBuffers() const
{
    int count = 0;

    for(const auto& buffer : DecodedAudioDataBuffer) {
        if(!buffer.Played)
            ++count;
    }

    return count;
}
// ------------------------------------ //
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

        // If we are no longer playing unregister
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

        // Load more data if we need some
        FillPlaybackBuffers();

        // Play video buffers until we don't have any that have any with timestamp <=
        // PassedTimeSeconds
        while(true) {

            float lowestTimestamp = PassedTimeSeconds + 1;
            size_t index = 0;

            // Find the lowest timestamp frame to play
            for(size_t i = 0; i < DecodedVideoDataBuffer.size(); ++i) {

                const auto& item = DecodedVideoDataBuffer[i];

                if(!item.Played && item.Timestamp < lowestTimestamp) {
                    lowestTimestamp = item.Timestamp;
                    index = i;
                }
            }

            if(lowestTimestamp > PassedTimeSeconds) {
                // No new frames to show
                // And if the file has been read fully we have reached the end
                if(ReadReachedEnd) {
                    // There are no more frames, end the playback
                    OnStreamEndReached();
                    return 0;
                }

                break;
            }

            UpdateTexture(DecodedVideoDataBuffer[index]);

            // This break makes sure each frame is displayed. With the new timing this isn't
            // needed for smooth playback break;
        }

        // Start playing audio. Hopefully at the same time as the first frame of the
        // video is shown decoded
        // The audio is delayed a bit to make playing it less glitchy
        if(!IsPlayingAudio && AudioStream && AudioCodec // && PassedTimeSeconds >= 0.01f
        ) {

            LOG_INFO("VideoPlayer: Starting audio playback from the video...");

            AudioStream->Play2D();
            IsPlayingAudio = true;
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
