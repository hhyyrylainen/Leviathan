#pragma once

#include "Define.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Int3 {
public:
    DLLEXPORT Int3() = default;

    DLLEXPORT Int3(int x, int y, int z);
    DLLEXPORT explicit Int3(int data);

    // ------------------------------------ //
    DLLEXPORT Int3 operator+(const Int3& val) const;

    DLLEXPORT Int3 operator*(int val) const;

    DLLEXPORT int operator[](const int nIndex) const;
    DLLEXPORT Int3 operator-(const Int3& other) const;

    DLLEXPORT inline Int3* operator*=(int f);

    DLLEXPORT int AddAllTogether() const;

    VALUE_TYPE(Int3);

    // ------------------------------------ //

    int X = 0;
    int Y = 0;
    int Z = 0;
};

}
