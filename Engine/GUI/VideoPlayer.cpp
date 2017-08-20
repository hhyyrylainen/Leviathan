// ------------------------------------ //
#include "VideoPlayer.h"

#include "Common/DataStoring/DataBlock.h"

#include "OgrePixelFormat.h"
#include "OgreTextureManager.h"
#include "OgrePixelBox.h"
#include "OgreHardwarePixelBuffer.h"

#include <boost/filesystem.hpp>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

//! Sizeof the buffer audio is read to and passed to SoundStream
constexpr auto WANTED_MAX_AUDIO = 4096;

constexpr Ogre::PixelFormat OGRE_IMAGE_FORMAT = Ogre::PF_R8G8B8A8;
// This must match the above definition
constexpr AVPixelFormat FFMPEG_DECODE_TARGET = AV_PIX_FMT_RGBA;


DLLEXPORT VideoPlayer::VideoPlayer(){
    
}

DLLEXPORT VideoPlayer::~VideoPlayer(){

    // Ensure all FFMPEG resources are closed
    Stop();
}

std::atomic<int> VideoPlayer::TextureSequenceNumber = {0};
// ------------------------------------ //
DLLEXPORT bool VideoPlayer::Play(const std::string &videofile){
    
    // Make sure nothing is playing currently //
    Stop();
    
    // Make sure ffmpeg is loaded //
    LoadFFMPEG();

    if(!boost::filesystem::exists(videofile)){

        LOG_ERROR("VideoPlayer: Play: file doesn't exist: " + videofile);
        return false;
    }

    VideoFile = videofile;

    // Parse stream data to know how big our textures need to be //
    if(!FFMPEGLoadFile()){

        LOG_ERROR("VideoPlayer: Play: ffmpeg failed to parse / setup playback for the file");
        Stop();
        return false;
    }

    // If Ogre isn't initialized we are going to pretend that we worked for testing purposes
    if(!Ogre::TextureManager::getSingletonPtr()){

        LOG_INFO("VideoPlayer: Ogre hasn't been initialized fully (no TextureManager), "
            "failing but pretending to have worked");
        return true;
    }

    if(!OnVideoDataLoaded()){

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

DLLEXPORT void VideoPlayer::Stop(){

    // Close all ffmpeg resources //
    StreamValid = false;

    // Stop audio playing first //
    if(IsPlayingAudio){
        IsPlayingAudio = false;
    }

    if(AudioStream){

        AudioStream->stop();
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

        //PlayingSource = nullptr;
        ReadAudioDataBuffer.clear();
    }

    // Video and Audio codecs are released by Context, but we still free them here?
    if(VideoCodec)
        avcodec_free_context(&VideoCodec);
    if(AudioCodec)
        avcodec_free_context(&AudioCodec);
        
    VideoCodec = nullptr;
    AudioCodec = nullptr;

    if(ImageConverter){

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

    if(ResourceReader){

        if(ResourceReader->buffer){

            av_free(ResourceReader->buffer);
            ResourceReader->buffer = nullptr;
        }

        av_free(ResourceReader);
        ResourceReader = nullptr;
    }

    if(FormatContext){
        // The doc says this is the right method to close it after
        // avformat_open_input has succeeded
        
        //avformat_free_context(FormatContext);
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
DLLEXPORT float VideoPlayer::GetDuration() const{

    if(!FormatContext)
        return 0;

    return static_cast<float>(FormatContext->duration) / AV_TIME_BASE;
}
// ------------------------------------ //
bool VideoPlayer::OnVideoDataLoaded(){

    const int number = TextureSequenceNumber++;

    TextureName = "Leviathan_VideoPlayer_" + std::to_string(number);

    VideoOutputTexture = Ogre::TextureManager::getSingleton().createManual(
        TextureName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        FrameWidth, FrameHeight,
        0,
        OGRE_IMAGE_FORMAT,
        Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

    if(VideoOutputTexture.isNull()){
        LOG_ERROR("VideoPlayer: Failed to create video output texture");
        return false;
    }

    LOG_INFO("VideoPlayer: Created video texture: " + std::to_string(FrameWidth) + "x" +
        std::to_string(FrameHeight));
    
    return true;
}
// ------------------------------------ //
bool VideoPlayer::FFMPEGLoadFile(){

    FormatContext = avformat_alloc_context();
    if(!FormatContext){
        LOG_ERROR("VideoPlayer: FFMPEG: avformat_alloc_context failed");
        return false;
    }

    // Not using custom reader
    // FormatContext->pb = ResourceReader;

    // We use a custom io object so we probably can pass null to the url parameter
    // instead of TCHAR_TO_ANSI(*VideoFile)
    if(avformat_open_input(&FormatContext, VideoFile.c_str(), nullptr, nullptr) < 0){

        // Context was freed automatically
        FormatContext = nullptr;
        LOG_ERROR("VideoPlayer: FFMPEG: FFMPEG failed to open video stream file resource");
        return false;
    }

    if(avformat_find_stream_info(FormatContext, nullptr) < 0){

        LOG_ERROR("VideoPlayer: FFMPEG: Failed to read video stream info");
        return false;
    }

    // Find audio and video streams //
    unsigned int foundVideoStreamIndex = std::numeric_limits<unsigned int>::max();
    unsigned int foundAudioStreamIndex = std::numeric_limits<unsigned int>::max();

    for(unsigned int i = 0; i < FormatContext->nb_streams; ++i){

        if(FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){

            foundVideoStreamIndex = i;
            continue;
        }

        if(FormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){

            foundAudioStreamIndex = i;
            continue;
        }
    }

    // Fail if didn't find a stream //
    if(foundVideoStreamIndex >= FormatContext->nb_streams){

        LOG_WARNING("VideoPlayer: FFMPEG: Video didn't have a video stream");
        return false;
    }


    if(foundVideoStreamIndex < FormatContext->nb_streams){

        // Found a video stream, play it
        if(!OpenStream(foundVideoStreamIndex, true)){

            LOG_ERROR("VideoPlayer: FFMPEG: Failed to open video stream");
            return false;
        }
    }

    if(foundAudioStreamIndex < FormatContext->nb_streams){

        // Found an audio stream, play it
        if(!OpenStream(foundAudioStreamIndex, false)){

            LOG_WARNING("VideoPlayer: FFMPEG: Failed to open audio stream, "
                "playing without audio");
        }
    }

    DecodedFrame = av_frame_alloc();
    ConvertedFrame = av_frame_alloc();
    DecodedAudio = av_frame_alloc();

    if(!DecodedFrame || !ConvertedFrame || !DecodedAudio){
        LOG_ERROR("VideoPlayer: FFMPEG: av_frame_alloc failed");
        return false;
    }

    FrameWidth = FormatContext->streams[foundVideoStreamIndex]->codecpar->width;
    FrameHeight = FormatContext->streams[foundVideoStreamIndex]->codecpar->height;

    // Calculate required size for the converted frame
    ConvertedBufferSize = av_image_get_buffer_size(FFMPEG_DECODE_TARGET,
        FrameWidth, FrameHeight, 1);

    ConvertedFrameBuffer = reinterpret_cast<uint8_t*>(av_malloc(
            ConvertedBufferSize * sizeof(uint8_t)));

    if(!ConvertedFrameBuffer){
        LOG_ERROR("VideoPlayer: FFMPEG: av_malloc failed for ConvertedFrameBuffer");
        return false;
    }

    if(ConvertedBufferSize != static_cast<size_t>(FrameWidth * FrameHeight * 4)){

        LOG_ERROR("VideoPlayer: FFMPEG: FFMPEG and Ogre image data sizes don't match! "
            "Check selected formats");
        return false;
    }

    if(av_image_fill_arrays(ConvertedFrame->data, ConvertedFrame->linesize,
            ConvertedFrameBuffer, FFMPEG_DECODE_TARGET,
            FrameWidth, FrameHeight, 1) < 0)
    {
        LOG_ERROR("VideoPlayer: FFMPEG: av_image_fill_arrays failed");
        return false;
    }

    // Converting images to be ue4 compatible is done by this
    // TODO: allow controlling how good conversion is done
    // SWS_FAST_BILINEAR is the fastest
    ImageConverter = sws_getContext(FrameWidth, FrameHeight,
        static_cast<AVPixelFormat>(FormatContext->streams[VideoIndex]->codecpar->format),
        FrameWidth, FrameHeight, FFMPEG_DECODE_TARGET, SWS_BICUBIC,
        nullptr, nullptr, nullptr);

    if(!ImageConverter){

        LOG_ERROR("VideoPlayer: FFMPEG: sws_getContext failed");
        return false;
    }

    if(AudioCodec){

        // Setup audio playing //
        SampleRate = AudioCodec->sample_rate;
        ChannelCount = AudioCodec->channels;

        // This may or may not be a limitation anymore
        // Especially not sure the channel count applies with SFML
        if(ChannelCount <= 0 || ChannelCount > 2){
            
            LOG_ERROR("VideoPlayer: FFMPEG: Unsupported audio channel count, "
                "only 1 or 2 are supported (todo: check does this apply with SFML)");
            return false;
        }

        // SFML expects AV_SAMPLE_FMT_S16
        //AudioCodec->sample_fmt;
        if(av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) != 2){

            LOG_FATAL("AV_SAMPLE_FMT_S16 size has changed");
            return false;
        }

        AudioConverter = swr_alloc();

        if(!AudioConverter){

            LOG_ERROR("VideoPlayer: FFMPEG: swr_alloc failed");
            return false;
        }

        const auto ChannelLayout = AudioCodec->channel_layout != 0 ?
            AudioCodec->channel_layout :
            // Guess
            av_get_default_channel_layout(AudioCodec->channels);

        // Check above for the note about the target audio format
        AudioConverter = swr_alloc_set_opts(AudioConverter, ChannelLayout,
            AV_SAMPLE_FMT_S16, AudioCodec->sample_rate,
            ChannelLayout, AudioCodec->sample_fmt, AudioCodec->sample_rate,
            0, nullptr);

        if(swr_init(AudioConverter) < 0){

            LOG_ERROR("VideoPlayer: FFMPEG: Failed to initialize audio converter for stream");
            return false;
        }

        // Create sound object //
        AudioStream = std::make_unique<SoundStream>(
            [=](std::vector<int16_t> &samplereceiver) -> bool{

                // 2 bytes per sample
                samplereceiver.resize(WANTED_MAX_AUDIO * 2 * ChannelCount);

                bool ended = false;
                const auto read = this->ReadAudioData(reinterpret_cast<uint8_t*>(
                        samplereceiver.data()), samplereceiver.size(), ended);

                samplereceiver.resize(read);
                return ended != true;
                
            }, ChannelCount, SampleRate);
    }

    DumpInfo();
    ResetClock();

    PassedTimeSeconds = 0.f;
    NextFrameReady = false;
    CurrentlyDecodedTimeStamp = 0.f;

    StreamValid = true;

    LOG_INFO("VideoPlayer: successfully opened all the ffmpeg streams for video file");

    return true;
}

bool VideoPlayer::OpenStream(unsigned int index, bool video){

    auto* thisStreamCodec = avcodec_find_decoder(
        FormatContext->streams[index]->codecpar->codec_id);

    if(!thisStreamCodec){

        LOG_ERROR("VideoPlayer: FFMPEG: unsupported codec used in video file");
        return false;
    }

    auto* thisCodecContext = avcodec_alloc_context3(thisStreamCodec);

    if(!thisCodecContext){

        LOG_ERROR("VideoPlayer: FFMPEG: failed to allocate codec context");
        return false;
    }

    // Try copying parameters //
    if(avcodec_parameters_to_context(thisCodecContext,
            FormatContext->streams[index]->codecpar) < 0)
    {
        avcodec_free_context(&thisCodecContext);
        LOG_ERROR("VideoPlayer: FFMPEG: failed to copy parameters to codec context");
        return false;
    }

    // Open the codec this is important to avoid segfaulting //
    // FFMPEG documentation warns that this is not thread safe
    const auto codecOpenResult = avcodec_open2(thisCodecContext, thisStreamCodec, nullptr);

    if(codecOpenResult < 0){

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
    if(video){

        //VideoParser = ThisCodecParser;
        VideoCodec = thisCodecContext;
        VideoIndex = static_cast<int>(index);
        VideoTimeBase = static_cast<float>(FormatContext->streams[index]->time_base.num) /
            static_cast<float>(FormatContext->streams[index]->time_base.den);
        // VideoTimeBase = static_cast<float>(VideoCodec->time_base.num) /
        //     static_cast<float>(VideoCodec->time_base.den);

    } else {

        //AudioParser = ThisCodecParser;
        AudioCodec = thisCodecContext;
        AudioIndex = static_cast<int>(index);
    }

    return true;
}
// ------------------------------------ //
bool VideoPlayer::DecodeVideoFrame(){

    const auto result = avcodec_receive_frame(VideoCodec, DecodedFrame);

    if(result >= 0){

        // Worked //
            
        // Convert the image from its native format to RGB
        if(sws_scale(ImageConverter, DecodedFrame->data, DecodedFrame->linesize,
                0, FrameHeight,
                ConvertedFrame->data, ConvertedFrame->linesize) < 0)
        {
            // Failed to convert frame //
            LOG_ERROR("Converting video frame failed");
            return false;
        }

        // Seems like DecodedFrame->pts contains garbage
        // and packet.pts is the timestamp in VideoCodec->time_base
        // so we access that through pkt_pts
        //CurrentlyDecodedTimeStamp = DecodedFrame->pkt_pts * VideoTimeBase;
        //VideoTimeBase = VideoCodec->time_base.num / VideoCodec->time_base.den;
        //CurrentlyDecodedTimeStamp = DecodedFrame->pkt_pts * VideoTimeBase;

        // Seems that the latest FFMPEG version has fixed this.
        // I would put this in a #IF macro bLock if ffmpeg provided a way to check the
        // version at compile time
        CurrentlyDecodedTimeStamp = DecodedFrame->pts * VideoTimeBase;
        return true;
    }

    if(result == AVERROR(EAGAIN)){

        // Waiting for data //
        return false;
    }

    LOG_ERROR("VideoPlayer: DecodeVideoFrame: frame receive failed, error: " +
        std::to_string(result));
    return false;
}

VideoPlayer::PacketReadResult VideoPlayer::ReadOnePacket(
    DecodePriority priority)
{
    if(!FormatContext || !StreamValid)
        return PacketReadResult::Ended;

    Lock lock(ReadPacketMutex);

    // Decode queued packets first
    if(priority == DecodePriority::Video && !WaitingVideoPackets.empty()){

        // Try to send it //
        const auto Result = avcodec_send_packet(VideoCodec,
            &WaitingVideoPackets.front()->packet);
            
        if(Result == AVERROR(EAGAIN)){

            // Still wailing to send //
            return PacketReadResult::QueueFull;
        }

        if(Result < 0){

            // An error occured //
            LOG_ERROR("Video stream send error from queue, stopping playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        // Successfully sent the first in the queue //
        WaitingVideoPackets.pop_front();
        return PacketReadResult::Ok;
    }
    
    if(priority == DecodePriority::Audio && !WaitingAudioPackets.empty()){

        // Try to send it //
        const auto Result = avcodec_send_packet(AudioCodec,
            &WaitingAudioPackets.front()->packet);
            
        if(Result == AVERROR(EAGAIN)){

            // Still wailing to send //
            return PacketReadResult::QueueFull;
        } 

        if(Result < 0){

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
    //av_init_packet(&packet);

    if(av_read_frame(FormatContext, &Packet) < 0){

        // Stream ended //
        //av_packet_unref(&packet);
        return PacketReadResult::Ended;
    }

    if(!StreamValid){

        av_packet_unref(&Packet);
        return PacketReadResult::Ended;
    }

    // Is this a packet from the video stream?
    if(Packet.stream_index == VideoIndex) {

        // If not wanting this stream don't send it //
        if(priority != DecodePriority::Video){

            WaitingVideoPackets.push_back(std::unique_ptr<ReadPacket>(
                    new ReadPacket(&Packet)));
            
            return PacketReadResult::Ok;
        }

        // Send it to the decoder //
        const auto Result = avcodec_send_packet(VideoCodec, &Packet);

        if(Result == AVERROR(EAGAIN)){

            // Add to queue //
            WaitingVideoPackets.push_back(std::unique_ptr<ReadPacket>(
                    new ReadPacket(&Packet)));
            return PacketReadResult::QueueFull;
        }

        av_packet_unref(&Packet);
        
        if(Result < 0){

            LOG_ERROR("VideoPlayer:Video stream send error, stopping playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        return PacketReadResult::Ok;

    } else if(Packet.stream_index == AudioIndex && AudioCodec){
            
        // If audio codec is null audio playback is disabled //
            
        // If not wanting this stream don't send it //
        if(priority != DecodePriority::Audio){

            WaitingAudioPackets.push_back(std::unique_ptr<ReadPacket>(
                    new ReadPacket(&Packet)));
            return PacketReadResult::Ok;
        }
        
        const auto Result = avcodec_send_packet(AudioCodec, &Packet);

        if(Result == AVERROR(EAGAIN)){

            // Add to queue //
            WaitingAudioPackets.push_back(std::unique_ptr<ReadPacket>(
                    new ReadPacket(&Packet)));
            return PacketReadResult::QueueFull;
        }

        av_packet_unref(&Packet);

        if(Result < 0){

            LOG_ERROR("Audio stream send error, stopping audio playback");
            StreamValid = false;
            return PacketReadResult::Ended;
        }

        // This is probably not needed? and was an error before
        //av_packet_unref(&Packet);
        return PacketReadResult::Ok;
    }

    // Unknown stream, ignore
    av_packet_unref(&Packet);
    return PacketReadResult::Ok;
}
// ------------------------------------ //
void VideoPlayer::UpdateTexture(){

    // Make sure if the 
    Ogre::PixelBox pixelView(FrameWidth, FrameHeight, 1,
        OGRE_IMAGE_FORMAT,
        // The data[0] buffer has some junk before the actual data so don't use that
        /*&ConvertedFrame->data[0]*/ ConvertedFrameBuffer);

    Ogre::v1::HardwarePixelBufferSharedPtr buffer = VideoOutputTexture->getBuffer();
    buffer->blitFromMemory(pixelView);
}
// ------------------------------------ //
size_t VideoPlayer::ReadAudioData(uint8_t* output, size_t amount, bool &ended){

    Lock lock(AudioMutex);
    
    if(amount < 1 || !AudioCodec || !StreamValid){
        ended = true;
        return 0;
    }

    ended = false;    

    // Receive audio packet //
    while(true){

        // First return from queue //
        if(!ReadAudioDataBuffer.empty()){

            // Try to read from the queue //
            const auto ReadAmount = ReadDataFromAudioQueue(lock, output, amount);

            if(ReadAmount == 0){

                // Queue is invalid... //
                LOG_ERROR("Invalid audio queue, emptying the queue");
                ReadAudioDataBuffer.clear();
                continue;
            }

            return ReadAmount;
        }

        const auto ReadResult = avcodec_receive_frame(AudioCodec, DecodedAudio);
            
        if(ReadResult == AVERROR(EAGAIN)){

            if(this->ReadOnePacket(DecodePriority::Audio) == PacketReadResult::Ended){

                // Stream ended //
                ended = true;
                return 0;
            }

            continue;
        }

        if(ReadResult < 0){

            // Some error //
            LOG_ERROR("Failed receiving audio packet, stopping audio playback");
            StreamValid = false;
            ended = true;
            return 0;
        }

        // Received audio data //

        // This is verified in open when setting up converting
        // av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) could also be used here
        const auto BytesPerSample = 2;

        const auto TotalSize = BytesPerSample * (DecodedAudio->nb_samples
            * ChannelCount);

        if(amount >= static_cast<size_t>(TotalSize)){
                
            // Lets try to directly feed the converted data to the requester //
            if(swr_convert(AudioConverter, &output, TotalSize,
                    const_cast<const uint8_t**>(DecodedAudio->data),
                    DecodedAudio->nb_samples) < 0)
            {
                LOG_ERROR("Invalid audio stream, converting to audio read buffer failed");
                StreamValid = false;
                ended = true;
                return 0;
            }

            return TotalSize;
        }
            
        // We need a temporary buffer //
        auto NewBuffer = std::unique_ptr<ReadAudioPacket>(new ReadAudioPacket());

        NewBuffer->DecodedData.resize(TotalSize);

        uint8_t* DecodeOutput = &NewBuffer->DecodedData[0];

        // Convert into the output data
        if(swr_convert(AudioConverter, &DecodeOutput, TotalSize,
                const_cast<const uint8_t**>(DecodedAudio->data),
                DecodedAudio->nb_samples) < 0)
        {
            LOG_ERROR("Invalid audio stream, converting failed");
            StreamValid = false;
            ended = true;            
            return 0;
        }
                    
        ReadAudioDataBuffer.push_back(std::move(NewBuffer));
        continue;
    }

    LEVIATHAN_ASSERT(false, "Execution never reaches here");
}

size_t VideoPlayer::ReadDataFromAudioQueue(Lock &audiolocked,
    uint8_t* output, size_t amount)
{
    if(ReadAudioDataBuffer.empty())
        return 0;
    
    auto& dataVector = ReadAudioDataBuffer.front()->DecodedData;

    if(amount >= dataVector.size()){

        // Can move an entire packet //
        const auto movedDataCount = dataVector.size();

        memcpy(output, &dataVector[0], movedDataCount);

        ReadAudioDataBuffer.pop_front();

        return movedDataCount;
    }

    // Need to return a partial packet //
    const auto movedDataCount = amount;
    const auto leftSize = dataVector.size() - movedDataCount;

    memcpy(output, &dataVector[0], movedDataCount);

    std::vector<uint8_t> newData;
    newData.resize(dataVector.size() - leftSize);

    LEVIATHAN_ASSERT((newData.size() == leftSize + movedDataCount), "Math assumption failed");

    std::copy(dataVector.begin() + leftSize, dataVector.end(),
        newData.begin());
    
    dataVector = newData;
    return movedDataCount;
}

// ------------------------------------ //
void VideoPlayer::ResetClock(){

    LastUpdateTime = ClockType::now();
}

void VideoPlayer::OnStreamEndReached(){

    auto vars = NamedVars::MakeShared(new NamedVars());
    
    vars->AddVar(std::make_shared<NamedVariableList>("oldvideo", new StringBlock(VideoFile)));
    
    Stop();
    OnPlayBackEnded.Call(vars);
}

void VideoPlayer::SeekVideo(float time){

    if(time < 0)
        time = 0;

    const auto seekPos = static_cast<uint64_t>(time * AV_TIME_BASE);

    const auto timeStamp = av_rescale_q(seekPos, 
    #ifdef _MSC_VER
        // Copy pasted from the definition of AV_TIME_BASE_Q
        // TODO: check is this still required
        AVRational{ 1, AV_TIME_BASE },
    #else
        AV_TIME_BASE_Q,
    #endif
        FormatContext->streams[VideoIndex]->time_base);

    av_seek_frame(FormatContext, VideoIndex, timeStamp, AVSEEK_FLAG_BACKWARD);

    LOG_WARNING("VideoPlayer: SeekVideo: audio seeking not implemented!");
}
// ------------------------------------ //
void VideoPlayer::DumpInfo() const{

    if(FormatContext){
        // Passing VideoFile here passes the name onto output, it's not needed
        // but it differentiates the output by file name
        av_dump_format(FormatContext, 0, VideoFile.c_str(), 0);
    }
}
// ------------------------------------ //
DLLEXPORT int VideoPlayer::OnEvent(Event** event){

    switch((*event)->GetType()){

    case EVENT_TYPE_FRAME_BEGIN:
    {

        // If we are no longer player unregister
        if(!IsPlaying)
            return -1;

        if(!IsStreamValid()){

            LOG_WARNING("VideoPlayer: Stream is invalid, closing playback");
            OnStreamEndReached();
            return -1;
        }

        const auto now = ClockType::now();

        const auto elapsed = now - LastUpdateTime;
        LastUpdateTime = now;

        PassedTimeSeconds += std::chrono::duration_cast<
            std::chrono::duration<float>>(elapsed).count();

        // Start playing audio. Hopefully at the same time as the first frame of the
        // video is decoded
        if(!IsPlayingAudio && AudioStream && AudioCodec){

            LOG_INFO("VideoPlayer: Starting audio playback from the video...");

            // auto err = Pa_StartStream(AudioStream);

            // if(err != paNoError){

            //     UE_LOG(ThriveLog, Error, TEXT("Error starting PortAudio audio stream: %s"),
            //         ANSI_TO_TCHAR(Pa_GetErrorText(err)));

            //     IsPlayingAudio = true;
            
            // } else {

            //     IsPlayingAudio = true;

            //     LOG_LOG("Audio playback started");
            // }
            IsPlayingAudio = true;
        }

        // This loops until we are displaying a frame we should be showing at this time // 
        while(PassedTimeSeconds >= CurrentlyDecodedTimeStamp){
    
            // Only decode if there isn't a frame ready
            while(!NextFrameReady){

                // Decode a packet if none are in queue
                if(ReadOnePacket(DecodePriority::Video) == PacketReadResult::Ended){

                    // There are no more frames, end the playback
                    OnStreamEndReached();
                    return -1;
                }

                NextFrameReady = DecodeVideoFrame();
            }

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

DLLEXPORT int VideoPlayer::OnGenericEvent(GenericEvent** event){

    return 0;
}

// ------------------------------------ //
static bool FFMPEGLoadedAlready = false;
static Mutex FFMPEGLoadMutex;

namespace Leviathan{

//! \brief Custom callback for ffmpeg to pipe output to our logger class
void FFMPEGCallback(void *, int level, const char * fmt, va_list varg){

    if(level > AV_LOG_INFO)
        return;

    // Format message //
    std::string formatedMessage;

    constexpr auto FORMAT_BUFFER_SIZE = 250;
    char strBuffer[FORMAT_BUFFER_SIZE];
    
    const int result = snprintf(strBuffer, FORMAT_BUFFER_SIZE, fmt, varg);

    if(result < 0 || result >= FORMAT_BUFFER_SIZE){

        LOG_WARNING("FFMPEG log message was too long and is truncated");
    }

    formatedMessage = strBuffer;
    
    if(level <= AV_LOG_FATAL){

        LOG_ERROR("[FFMPEG FATAL] " + formatedMessage);
        
    } else if(level <= AV_LOG_ERROR){

        LOG_ERROR("[FFMPEG] " + formatedMessage);

    } else if(level <= AV_LOG_WARNING){

        LOG_WARNING("[FFMPEG] " + formatedMessage);
        
    } else {

        LOG_INFO("[FFMPEG] " + formatedMessage);
    }
}

}

void VideoPlayer::LoadFFMPEG(){

    // Makes sure all threads can pass only when ffmpeg is loaded
    Lock lock(FFMPEGLoadMutex);

    if(FFMPEGLoadedAlready)
        return;

    FFMPEGLoadedAlready = true;

    av_log_set_callback(Leviathan::FFMPEGCallback);

    avcodec_register_all();
    av_register_all();
}
