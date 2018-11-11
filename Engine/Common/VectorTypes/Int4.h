#pragma once

#include "Define.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Int4 {
public:
    DLLEXPORT Int4() = default;

    DLLEXPORT Int4(int x, int y, int z, int w);
    DLLEXPORT explicit Int4(int data);

    // ------------------------------------ //
    DLLEXPORT Int4& operator+(const Int4& val);
    DLLEXPORT int operator[](const int nIndex) const;
    DLLEXPORT Int4& operator-(const Int4& val);
    DLLEXPORT int AddAllTogether() const;
    // ------------------------------------ //

    int X = 0;
    int Y = 0;
    int Z = 0;
    int W = 0;
};

}
