// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Common/ReferenceCounted.h"

#include <functional>
#include <vector>

#include <SFML/Audio/SoundStream.hpp>
        
namespace Leviathan{

//! \brief Streaming sound that uses a std::function object to receive
//! new audio data to play
class SoundStream : public ThreadSafe, public sf::SoundStream{
public:
    
    SoundStream(std::function<bool (std::vector<int16_t>&)> datacallback,
        unsigned int channelcount, unsigned int samplerate);

protected:
    
    // sf::SoundStream
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override{}

protected:

    std::vector<int16_t> PlayingAudioData;

    //! Callback for filling in PlayingAudioData
    //! Return value of true keeps playing. Returning false stops the playback
    std::function<bool (std::vector<int16_t>&)> MoreDataCallback;
};


}
