// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Matrix.h"

#include "DiligentCore/Common/interface/BasicMath.hpp"

namespace Leviathan {

inline Diligent::float4x4 MatrixToDiligent(const Matrix4& matrix)
{
    // Diligent::float4x4 temp;
    // std::memcpy(&temp, &matrix, sizeof(Matrix4));
    // return temp;

    return Diligent::float4x4(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3],
        matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3], matrix.m[2][0],
        matrix.m[2][1], matrix.m[2][2], matrix.m[2][3], matrix.m[3][0], matrix.m[3][1],
        matrix.m[3][2], matrix.m[3][3]);
}

inline Matrix4 MatrixFromDiligent(const Diligent::float4x4& matrix)
{
    return Matrix4(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3],
        matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3], matrix.m[2][0],
        matrix.m[2][1], matrix.m[2][2], matrix.m[2][3], matrix.m[3][0], matrix.m[3][1],
        matrix.m[3][2], matrix.m[3][3]);
}

inline Diligent::float4 Float4ToDiligent(const Float4& vec)
{
    return Diligent::float4(vec.X, vec.Y, vec.Z, vec.W);
}

} // namespace Leviathan
