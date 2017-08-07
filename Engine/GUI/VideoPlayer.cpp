// ------------------------------------ //
#include "VideoPlayer.h"

#include "OgrePixelFormat.h"

#include <boost/filesystem.hpp>

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

constexpr auto DEFAULT_READ_BUFFER = 32000;

constexpr auto INPUT_BUFFER_SIZE = 4096;

constexpr Ogre::PixelFormat OGRE_IMAGE_FORMAT = Ogre::PF_R8G8B8A8;
// This must match the above definition
constexpr AVPixelFormat FFMPEG_DECODE_TARGET = AV_PIX_FMT_RGBA;

// Bits per pixel in the decode target format and OGRE_IMAGE_FORMAT
constexpr auto FORMAT_BPP = 32;


DLLEXPORT VideoPlayer::VideoPlayer(){
    
}

DLLEXPORT VideoPlayer::~VideoPlayer(){

    // Ensure all FFMPEG resources are closed
    Stop();
}
// ------------------------------------ //
DLLEXPORT bool VideoPlayer::Play(const std::string &targettexturename,
    const std::string &videofile)
{
    // Make sure nothing is playing currently //
    Stop();
    
    TextureName = targettexturename;

    if(TextureName.empty())
        return false;
    
    // Make sure ffmpeg is loaded //
    LoadFFMPEG();

    if(!boost::filesystem::exists(videofile)){

        LOG_ERROR("VideoPlayer: Play: file doesn't exist");
        return false;
    }

    VideoFile = videofile;

    // Parse stream data to know how big our textures need to be //
    if(!FFMPEGLoadFile()){

        LOG_ERROR("VideoPlayer ffmpeg failed to parse / setup playback for the file");
        Stop();
        return false;
    }

    if(!OnVideoDataLoaded()){

        VideoOutputTexture.reset();
        LOG_ERROR("VideoPlayer ue4 texture / material setup failed");
        return false;
    }

    // Make tick run
    IsPlaying = true;
    RegisterForEvent(EVENT_TYPE_FRAME_BEGIN);
    return true;
}

DLLEXPORT void VideoPlayer::Stop(){

    // Close all ffmpeg resources //
    bStreamValid = false;

    // Stop audio playing first //
    if(bIsPlayingAudio){
        bIsPlayingAudio = false;
    }

    if(AudioStream){

        

        // Immediately close the stream
        auto err = Pa_AbortStream(AudioStream);

        if(err != paNoError){

            UE_LOG(ThriveLog, Error, TEXT("Error aborting PortAudio stream: %s"),
                ANSI_TO_TCHAR(Pa_GetErrorText(err)));
        }
        
        err = Pa_CloseStream(AudioStream);

        if(err != paNoError){

            UE_LOG(ThriveLog, Error, TEXT("Error closing PortAudio stream: %s"),
                ANSI_TO_TCHAR(Pa_GetErrorText(err)));
        }
        
        AudioStream = nullptr;
    }
    
    // Dump remaining packet data frames //
    {
        std::lock_guard<std::mutex> Lock(ReadPacketMutex);

        WaitingVideoPackets.clear();
        WaitingAudioPackets.clear();
    }

    // Close down audio portion //
    {
        std::lock_guard<std::mutex> Lock(AudioMutex);

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
    VideoOutputTexture = nullptr;
    VideoOutput = nullptr;
    

    // Reset other resources //
    
    VideoFileReader.reset();
    bIsPlaying = false;
    
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
