// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Events/CallableObject.h"
#include "Sound/SoundStream.h"

#include "OgreTexture.h"

#include <chrono>
#include <vector>

extern "C"{
// FFMPEG includes
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

namespace Leviathan{
namespace GUI{

//! \brief VideoPlayer that uses ffmpeg to play videos on Ogre
//!
//! textures and sound This is based on the VideoPlayer in Thrive
//! written by Henri Hyyryläinen which in turn was based on
//! ogre-ffmpeg-videoplayer, but all original ogre-ffmpeg-videoplayer
//! code has been removed in course of all the rewrites.
//! \todo Implement pausing and seeking
class VideoPlayer : public CallableObject{
protected:

    using ClockType = std::chrono::steady_clock;

    enum class PacketReadResult{

        Ended,
        Ok,
        QueueFull
    };

    enum class DecodePriority{

        Video,
        Audio
    };

    //! Holds converted audio data that could not be immediately returned by ReadAudioData
    struct ReadAudioPacket{

        std::vector<uint8_t> DecodedData;
    };

    //! Holds raw packets before sending
    struct ReadPacket{

        ReadPacket(AVPacket* src){

            av_packet_move_ref(&packet, src);
        }

        ~ReadPacket(){

            av_packet_unref(&packet);
        }

        AVPacket packet;
    };
    
public:

    DLLEXPORT VideoPlayer();
    DLLEXPORT ~VideoPlayer();

    VideoPlayer& operator=(const VideoPlayer&) = delete;

    //! \brief Starts playing the video file
    //! \returns True if successfully started
    //! \note Acquires the Ogre texture, a sound player and ffmpeg resources
    DLLEXPORT bool Play(const std::string &targetTextureName, const std::string &videofile);

    //! \brief Stops playing and unloads the current playback objects
    DLLEXPORT void Stop();

    // ------------------------------------ //
    // Stream info
    
    //! \returns True if currently loaded file has an audio stream
    DLLEXPORT bool HasAudio() const;

    //! \returns Current playback position, in seconds
    //! The return value is directly read from the last decoded frame timestamp
    DLLEXPORT float GetCurrentTime() const;

    //! \returns Width of the current video
    DLLEXPORT int32_t GetVideoWidth() const;

    //! \returns Height of the current video
    DLLEXPORT int32_t GetVideoHeight() const;


    //! \returns The number of audio channels
    DLLEXPORT int GetAudioChannelCount() const;

    //! \returns The number of samples per second of the audio stream
    //! or -1 if no audio streams exist
    DLLEXPORT int GetAudioSampleRate() const;

    //! \brief Dumps info about loaded ffmpeg streams
    DLLEXPORT void DumpInfo() const;

    //! \returns true if all the ffmpeg stream objects are valid for playback
    DLLEXPORT bool IsStreamValid() const;

    //! \brief Tries to call ffmpeg initialization once
    DLLEXPORT static void LoadFFMPEG();

public:

    //! \brief Reads audio data to the buffer
    //! \returns The number of bytes read
    //! \param amount The maximum number of bytes to read
    //! \protected
    size_t ReadAudioData(uint8_t* output, size_t amount);

protected:

    //! After loading the video this creates the output texture + material for it
    //! \returns false if the setup fails
    bool OnVideoDataLoaded();


    //! \brief Opens and parses the video info into ffmpeg streams and such
    //! \returns false if something fails
    bool FFMPEGLoadFile();

    //! helper for FFMPEGLoadFile
    //! \returns true on success
    bool OpenStream(unsigned int index, bool video);

    //! \brief Decodes one video frame. Returns false if more data is required
    //! by the decoder
    bool DecodeVideoFrame();

    //! \brief Reads a single packet from the stream that matches Priority
    PacketReadResult ReadOnePacket(DecodePriority priority);

    //! \brief Updates the texture
    void UpdateTexture();

    //! \brief Reads already decoded audio data. The audio data vector must be locked
    //! before calling this
    size_t ReadDataFromAudioQueue(Lock &audiolocked, uint8_t* output, size_t amount);

    //! \brief Resets timers. Call when playback start or resumes
    void ResetClock();

    //! \brief Called when end of playback has been reached
    //!
    //! Closes the playback and invokes the delegates
    void OnStreamEndReached();

    //! Video streem seaking. Don't use as the audio will get out of sync
    DLLEXPORT void SeekVideo(float time);

public:

    // CallableObject
    DLLEXPORT int OnEvent(Event** event) override;
    DLLEXPORT int OnGenericEvent(GenericEvent** event) override;
    
    
protected:
    
    std::string VideoFile;

    // The target texture
    std::string TextureName;
    Ogre::TexturePtr VideoOutputTexture;

    //! True when playing back something and frame start events do something
    bool IsPlaying = false;

    AVIOContext* ResourceReader = nullptr;

    // This seems to be not be used anymore
    //AVCodecContext* Context = nullptr;
    AVFormatContext* FormatContext = nullptr;


    //AVCodecParserContext* VideoParser = nullptr;
    AVCodecContext* VideoCodec = nullptr;
    int VideoIndex = 0;


    //! How many timestamp units are in a second in the video stream
    float VideoTimeBase = 1.f;

    //AVCodecParserContext* AudioParser = nullptr;
    AVCodecContext* AudioCodec = nullptr;
    int AudioIndex = 0;

    AVFrame* DecodedFrame = nullptr;
    AVFrame* DecodedAudio = nullptr;

    //! Once a frame has been loaded to DecodedFrame it is converted
    //! to a format that Ogre texture can accept
    AVFrame* ConvertedFrame = nullptr;

    uint8_t* ConvertedFrameBuffer = nullptr;

    // Required size for a single converted frame
    size_t ConvertedBufferSize = 0;

    int32_t FrameWidth = 0;
    int32_t FrameHeight = 0;

    SwsContext* ImageConverter = nullptr;

    SwrContext* AudioConverter = nullptr;

    //! Audio sample rate
    int SampleRate = 0; 
    int ChannelCount = 0;

    std::list<std::unique_ptr<ReadAudioPacket>> ReadAudioDataBuffer;
    Mutex AudioMutex;

    //! Used to start the audio playback once
    bool IsPlayingAudio = false;

    //! Audio output
    SoundStream* AudioStream = nullptr;

    // Timing control
    float PassedTimeSeconds = 0.f;
    float CurrentlyDecodedTimeStamp = 0.f;

    bool NextFrameReady = false;

    //! Set to false if an error occurs and playback should stop
    bool StreamValid = false;

    ClockType::time_point LastUpdateTime;

    Mutex ReadPacketMutex;
    std::list<std::unique_ptr<ReadPacket>> WaitingVideoPackets;
    std::list<std::unique_ptr<ReadPacket>> WaitingAudioPackets;
};

}
}
