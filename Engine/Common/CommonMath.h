// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Types.h"

#include <cmath>
#include <vector>

namespace Leviathan { namespace MMath {

DLLEXPORT float CoordinateDistance(float x1, float x2, float y1, float y2);
DLLEXPORT double AngleBetweenPoints(float x1, float x2, float y1, float y2);

DLLEXPORT bool IsPointInsidePolygon(const std::vector<Float3>& polygon, const Float3& point);

//! greatest common divisor, courtesy of Wikipedia
DLLEXPORT int GreatestCommonDivisor(int a, int b);

//! calculates a normal for triangle and returns in normalized //
DLLEXPORT Float3 CalculateNormal(const Float3& p1, const Float3& p2, const Float3& p3);

/*--------------------------------------
Original Function written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
Code Edited by Henri Hyyryläinen

This function uses Fermat's Theorem 100 times to test the primeness of a
(large) positive integer.
----------------------------------------------------------------------------*/
// DLLEXPORT bool IsPrime(const mpuint &p);

// float and double comparison (with EPSILON)
DLLEXPORT bool IsEqual(double x, double y);
DLLEXPORT bool IsEqual(float x, float y);

}} // namespace Leviathan::MMath
