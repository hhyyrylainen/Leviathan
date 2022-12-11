/********************************************************************
    created:	2012/11/21
    created:	21:11:2012   16:22
    filename: 	Random.cpp
    file path:	Engine
    file base:	Random
    file ext:	cpp
    author:		Henri Hyyryläinen

    purpose:	Platform independent random number generator, based on implementation of
Mersenne twister. Code written based on pseudocode on
                http://en.wikipedia.org/wiki/Mersenne_twister MT19937 algorithm.
*********************************************************************/
// ------------------------------------ //
#include "Random.h"

#include "../Logger.h"
#ifndef _WIN32
#include <ctime>
#include <sys/time.h>
#else
#include "WindowsInclude.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Leviathan::Random::Random()
{
    Index = 0;

    // no seed provided, so we must use current system time as seed //
#ifdef _WIN32
    // structures to get time //
    SYSTEMTIME time;
    GetSystemTime(&time);
    // just mash something together from current time //
    Seed =
        2500 + time.wDay * 25 + time.wHour * 15 + time.wMilliseconds * 50 + time.wSecond * 2;
#else
    timespec time;

    clock_gettime(CLOCK_REALTIME, &time);

    Seed = time.tv_nsec + time.tv_sec * 2;

#endif

    // initialize numbers //
    _InitializeGenerator();
}

Leviathan::Random::Random(int seed)
{
    // just save values and initialize it //
    Index = 0;
    Seed = seed;

    _InitializeGenerator();
}

Leviathan::Random::~Random()
{
    // we don't really need to delete anything //

    // check is static access this and set to null if it is //
    if(staticaccess == this)
        staticaccess = NULL;
}

Random* Leviathan::Random::Get()
{
    return staticaccess;
}

Random* Leviathan::Random::staticaccess = NULL;
// ------------------------------------ //
int Leviathan::Random::GetSeed()
{
    return Seed;
}
// ------------------------------------ //
int Leviathan::Random::GetNumber()
{
    // gets the number from index //

    if(Index == 0) {
        // needs new set of numbers //
        _GenerateNumbers();
    }

    int y = MT[Index];

    // randomize the result more here //

    y = y ^ (y >> 11);
    y = y ^ ((y << 7) & (0x9d2c5680));
    y = y ^ ((y << 15) & (0xefc60000));
    y = y ^ (y >> 18);

    // increase index //
    Index = (Index + 1) % RANDOM_STORAGE_SIZE;
    return y;
}

int Leviathan::Random::GetNumber(int min, int max)
{
    // New, hopefully better approach for smaller ranges
    // See stuff here:
    // https://stackoverflow.com/questions/1202687/how-do-i-get-a-specific-range-of-numbers-from-rand
    return static_cast<int>(
        min + (GetNumber() / (RANDOM_MAX_POSSIBLE / (max - min + 1) + 1.0f)));
}

float Leviathan::Random::GetNumber(float min, float max)
{
    // same as above but with floats //

    // get number and sample it to be between given values //
    // basically get percentage of max value and then get difference of min and max and
    // multiply that by percents and add it to min to get value between min and max
    return min + (((float)GetNumber() / RANDOM_MAX_POSSIBLE) * (max - min));
}

DLLEXPORT float Leviathan::Random::SymmetricRandom()
{
    return 2.f * GetNumber(0.f, 1.f) - 1.f;
}
// ------------------------------------ //
void Leviathan::Random::SetSeed(int seed)
{
    // this acts the same as generating new object with the provided seed //
    Seed = seed;
    Index = 0;
    _InitializeGenerator();
}

void Leviathan::Random::SetIndex(int index)
{
    Index = index % RANDOM_STORAGE_SIZE;
}

int Leviathan::Random::GetIndex()
{
    return Index;
}

void Leviathan::Random::SetAsMain()
{
    if(staticaccess != NULL)
        Logger::Get()->Info("Random: old static access replaced by new");
    staticaccess = this;
}
// ------------------------------------ //
void Leviathan::Random::_InitializeGenerator()
{
    MT[0] = Seed;

    for(int i = 1; i < RANDOM_STORAGE_SIZE; i++) {
        MT[i] = (1812433253 * (MT[i - 1] ^ (MT[i - 1]) >> 30) + i);
    }
}

void Leviathan::Random::_GenerateNumbers()
{
    // generates new set of numbers //

    for(int i = 0; i < RANDOM_STORAGE_SIZE; i++) {
        int y = MT[i] =
            (MT[i] & 0x80000000) + (MT[(i + i) % RANDOM_STORAGE_SIZE] & 0x7fffffff);

        MT[i] = MT[(i + 397) % RANDOM_STORAGE_SIZE] ^ (y >> 1);
        if((y % 2) != 0) {
            // odd number //
            MT[i] = MT[i] ^ (0x9908b0df);
        }
    }
}
