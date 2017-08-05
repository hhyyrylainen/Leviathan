// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"

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
class VideoPlayer{
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

    VideoPlayer();
    ~VideoPlayer();

    VideoPlayer& operator=(const VideoPlayer&) = delete;

    //! \brief Acquires the Ogre texture, a sound player and ffmpeg resources
    //! \returns True if succeeded. False if something went wrong
    bool Init(const std::string &targetTextureName);

    // When playing should listen for frame start events

    
    //! \brief Shuts down playback and releases all objects
    void Release();
    
    
protected:
    
    
};

}
}
