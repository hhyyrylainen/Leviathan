// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <array>
#include <tuple>

namespace Leviathan {

enum class YUV_COLOUR_SPACE { BT_601 = 0 };

//! \brief Converts a few kinds of supported YUV formats to RGBA
//!
//! This assumes that the target buffer is big enough
//! \returns True on success
DLLEXPORT bool YUVToRGBA(const std::array<const uint8_t*, 3>& planes,
    const std::array<int, 3>& strides, const std::array<std::tuple<int, int>, 3>& planesizes,
    bool bytesare16bit, uint8_t* target, size_t width, size_t height,
    YUV_COLOUR_SPACE colourtype = YUV_COLOUR_SPACE::BT_601);

} // namespace Leviathan
