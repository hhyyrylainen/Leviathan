#pragma once

#include "Define.h"

namespace Leviathan {

struct Int2 {
public:
    DLLEXPORT Int2() = default;

    DLLEXPORT Int2(int x, int y);
    DLLEXPORT explicit Int2(int data);

    // ------------------------------------ //
    DLLEXPORT Int2 operator+(const Int2& val);
    DLLEXPORT int operator[](const int nIndex) const;

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Int2 operator+(const Int2& val) const;
    DLLEXPORT inline Int2* operator+=(const Int2& val);

    // subtracts all elements //
    DLLEXPORT inline Int2 operator-(const Int2& val) const;

    // negates all elements //
    DLLEXPORT inline Int2 operator-() const;

    // multiplies elements together //
    DLLEXPORT inline Int2 operator*(const Int2& val) const;

    // multiply  by scalar f //
    DLLEXPORT inline Int2 operator*(int f) const;
    DLLEXPORT inline Int2* operator*=(int f);

    // divides all elements //
    DLLEXPORT inline Int2 operator/(const Int2& val) const;

    // divides by int //
    DLLEXPORT inline Int2 operator/(int f) const;

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Int2& other) const;
    DLLEXPORT inline bool operator<=(const Int2& other) const;
    DLLEXPORT inline bool operator>(const Int2& other) const;
    DLLEXPORT inline bool operator>=(const Int2& other) const;
    DLLEXPORT inline bool operator==(const Int2& other) const;
    DLLEXPORT inline bool operator!=(const Int2& other) const;

    // ------------------------------------ //

    DLLEXPORT void SetData(const int& data);
    DLLEXPORT void SetData(const int& data1, const int& data2);

    VALUE_TYPE(Int2);

    int X = 0;
    int Y = 0;
};

}
