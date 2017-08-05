// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

namespace Leviathan{
namespace GUI{

//! \brief VideoPlayer that uses ffmpeg to play videos on Ogre
//!
//! textures and sound This is based on the VideoPlayer in Thrive
//! written by Henri Hyyryläinen which in turn was based on
//! ogre-ffmpeg-videoplayer, but all original ogre-ffmpeg-videoplayer
//! code has been removed in course of all the rewrites.
class VideoPlayer{
public:

    VideoPlayer();
    ~VideoPlayer();

    VideoPlayer& operator=(const VideoPlayer&) = delete;

    //! \brief Acquires the Ogre texture, a sound player and ffmpeg resources
    //! \returns True if succeeded. False if something went wrong
    bool Init(const std::string &targetTextureName);

    
    //! \brief Shuts down playback and releases all objects
    void Release();
    
    
protected:
    
    
};

}
}
