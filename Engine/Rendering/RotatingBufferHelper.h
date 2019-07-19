// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "bsfCore/Image/BsPixelData.h"

#include <array>

namespace Leviathan {

//! \brief Helper for having a few BSF buffers for writing in turn
//!
//! Due to the asynchronous data upload to the GPU multiple buffers may be needed in some
//! places.
template<class BufferT, size_t MaxBuffers>
class RotatingBufferHelper {
protected:
    //! \returns The next available data buffer or nullptr if all up to MaxBuffers are locked
    BufferT* GetNextDataBuffer()
    {
        for(size_t attempt = 0; attempt < _CurrentDataBufferCount; ++attempt) {

            auto* buffer = &DataBuffers[(_BufferReadCount++) % _CurrentDataBufferCount];

            if(!(*buffer)->isLocked())
                return buffer;
        }

        // No free buffer
        if(_CurrentDataBufferCount < MaxBuffers) {
            // Can create new one
            DataBuffers[_CurrentDataBufferCount++] = _OnNewBufferNeeded();
            return &DataBuffers[_CurrentDataBufferCount - 1];
        }

        // No buffers available
        return nullptr;
    }

    void ClearDataBuffers()
    {
        for(size_t i = 0; i < _CurrentDataBufferCount; ++i) {
            DataBuffers[i] = nullptr;
        }

        _CurrentDataBufferCount = 0;
        _BufferReadCount = 0;
    }

    virtual BufferT _OnNewBufferNeeded() = 0;

protected:
    std::array<BufferT, MaxBuffers> DataBuffers;

private:
    unsigned _BufferReadCount = 0;
    size_t _CurrentDataBufferCount = 0;
};

} // namespace Leviathan
