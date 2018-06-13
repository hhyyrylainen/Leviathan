#pragma once
// ------------------------------------ //
#include "Include.h"
#include <math.h>


// this seems to only be half of the story, this might be double what it actually generates //
//#define RANDOM_MAX_POSSIBLE			(4294967296)
#define RANDOM_MAX_POSSIBLE (2147483648) // new hopefully corrected value //
#define RANDOM_STORAGE_SIZE 624

namespace Leviathan {

//! \brief Random number generator based on Mersenne Twister
class Random {
public:
    DLLEXPORT Random();
    DLLEXPORT Random(int seed);
    DLLEXPORT ~Random();

    DLLEXPORT static Random* Get();

    DLLEXPORT int GetSeed();

    DLLEXPORT int GetNumber();
    //! \brief Get a random in range [min, max]
    DLLEXPORT int GetNumber(int min, int max);
    DLLEXPORT float GetNumber(float min, float max);
    //! Basically the same as GetNumber but avoids requiring
    //! explicit casts to get the float variant
    DLLEXPORT inline float GetFloat(float min, float max)
    {

        return GetNumber(min, max);
    }

    DLLEXPORT float SymmetricRandom();

    // more advanced functions, should not be actually used //
    DLLEXPORT void SetSeed(int seed);
    DLLEXPORT void SetIndex(int index);
    DLLEXPORT int GetIndex();
    DLLEXPORT void SetAsMain();

private:
    // private functions that handle stuff //
    void _InitializeGenerator();
    void _GenerateNumbers();


    // number storage //
    int MT[RANDOM_STORAGE_SIZE];
    int Seed;
    int Index;

    // static access //
    static Random* staticaccess;
};

} // namespace Leviathan
