// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "../Common/Types.h"
#include <cmath>
#include <vector>


namespace Leviathan{

template<class T, class Compare>
    constexpr const T& clamp( const T& v, const T& lo, const T& hi, Compare comp )
{
    return assert( !comp(hi, lo) ),
        comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template<class T>
    constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    return clamp( v, lo, hi, std::less<>() );
}

class mpuint;

class MMath{
public:
    static DLLEXPORT float CoordinateDistance(float x1, float x2,float y1, float y2);
    static DLLEXPORT double AngleBetweenPoints(float x1, float x2,float y1, float y2);

    DLLEXPORT static bool IsPointInsidePolygon(const std::vector<Float3>& polygon,
        const Float3& point);

    // greatest common divisor, courtesy of Wikipedia
    DLLEXPORT static int GreatestCommonDivisor(int a, int b);

    // calculates a normal for triangle and returns in normalized //
    DLLEXPORT static Float3 CalculateNormal(const Float3 &p1, const Float3 &p2,
        const Float3 &p3);

    /*--------------------------------------
    Original Function written by Philip J. Erdelsky October 25, 2001 (revised August 22, 2002)
    Code Edited by Henri Hyyryläinen

    This function uses Fermat's Theorem 100 times to test the primeness of a
    (large) positive integer.
    ----------------------------------------------------------------------------*/
    //DLLEXPORT static bool IsPrime(const mpuint &p);

    // float and double comparison //
    DLLEXPORT static bool IsEqual(double x, double y);
    DLLEXPORT static bool IsEqual(float x, float y);
};







}


