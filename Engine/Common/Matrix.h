// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Types.h"

namespace Leviathan {

//! \brief A 4x4 matrix type
//!
//! This contains a lot of adapted code from bs::framework, see License.txt for more details
struct Matrix4 {
public:
    inline Matrix4() {}
    inline Matrix4(const Matrix4& other)
    {
        Values[0] = other.Values[0];
        Values[1] = other.Values[1];
        Values[2] = other.Values[2];
        Values[3] = other.Values[3];
        Values[4] = other.Values[4];
        Values[5] = other.Values[5];
        Values[6] = other.Values[6];
        Values[7] = other.Values[7];
        Values[8] = other.Values[8];
        Values[9] = other.Values[9];
        Values[10] = other.Values[10];
        Values[11] = other.Values[11];
        Values[12] = other.Values[12];
        Values[13] = other.Values[13];
        Values[14] = other.Values[14];
        Values[15] = other.Values[15];
    }

    Matrix4& operator=(const Matrix4& other)
    {
        Values[0] = other.Values[0];
        Values[1] = other.Values[1];
        Values[2] = other.Values[2];
        Values[3] = other.Values[3];
        Values[4] = other.Values[4];
        Values[5] = other.Values[5];
        Values[6] = other.Values[6];
        Values[7] = other.Values[7];
        Values[8] = other.Values[8];
        Values[9] = other.Values[9];
        Values[10] = other.Values[10];
        Values[11] = other.Values[11];
        Values[12] = other.Values[12];
        Values[13] = other.Values[13];
        Values[14] = other.Values[14];
        Values[15] = other.Values[15];
        return *this;
    }


    float Values[16];
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Matrix4;
#endif
