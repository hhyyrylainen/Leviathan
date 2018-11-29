// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Types.h"

#include <cmath>
#include <vector>

namespace Leviathan::MMath {

DLLEXPORT double AngleBetweenPoints(float x1, float x2, float y1, float y2) noexcept;
DLLEXPORT double AngleBetweenPoints(Float2 v1, Float2 v2) noexcept;

DLLEXPORT bool IsPointInsidePolygon(const std::vector<Float3>& polygon, const Float3& point);

// greatest common divisor, courtesy of Wikipedia
DLLEXPORT constexpr int GreatestCommonDivisor(int a, int b)
{
    return b == 0 ? a : GreatestCommonDivisor(b, a % b);
}

// calculates a normal for triangle and returns in normalized //
DLLEXPORT Float3 CalculateNormal(const Float3& p1, const Float3& p2, const Float3& p3);

/*--------------------------------------
Original Function written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
Code Edited by Henri Hyyryläinen

This function uses Fermat's Theorem 100 times to test the primeness of a
(large) positive integer.
----------------------------------------------------------------------------*/
// DLLEXPORT static bool IsPrime(const mpuint &p);

// float and double comparison //
DLLEXPORT inline bool IsEqual(double x, double y) noexcept
{
    return abs(x - y) <= EPSILON * abs(x);
}

DLLEXPORT inline bool IsEqual(float x, float y) noexcept
{
    return abs(x - y) <= EPSILON * abs(x);
}

} // namespace Leviathan::MMath
