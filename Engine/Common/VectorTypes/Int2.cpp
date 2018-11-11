#include "Int2.h"

using namespace Leviathan;

DLLEXPORT Int2::Int2(int x, int y)
{
    X = x;
    Y = y;
}

DLLEXPORT Int2::Int2(int data)
{
    X = data;
    Y = data;
}

// ------------------------------------ //
DLLEXPORT Int2 Int2::operator+(const Int2& val)
{
    return Int2(X + val.X, Y + val.Y);
}

DLLEXPORT int Int2::operator[](const int nIndex) const
{
    switch(nIndex) {
	    case 0: return X;
	    case 1: return Y;
    }

    LEVIATHAN_ASSERT(0, "invalid [] access");
    return 0;
}

// ------------------- Operators ----------------- //

DLLEXPORT inline Int2 Int2::operator+(const Int2& val) const
{
    return Int2(X + val.X, Y + val.Y);
}

DLLEXPORT inline Int2* Int2::operator+=(const Int2& val)
{
    X += val.X;
    Y += val.Y;
    return this;
}

DLLEXPORT inline Int2 Int2::operator-(const Int2& val) const
{
    return Int2(X - val.X, Y - val.Y);
}

DLLEXPORT inline Int2 Int2::operator-() const
{
    return Int2(-X, -Y);
}

DLLEXPORT inline Int2 Int2::operator*(const Int2& val) const
{
    return Int2(X * val.X, Y * val.Y);
}

DLLEXPORT inline Int2 Int2::operator*(int f) const
{
    return Int2(X * f, Y * f);
}

DLLEXPORT inline Int2* Int2::operator*=(int f)
{
    X *= f;
    Y *= f;
    return this;
}

DLLEXPORT inline Int2 Int2::operator/(const Int2& val) const
{
    return Int2(X / val.X, Y / val.Y);
}

DLLEXPORT inline Int2 Int2::operator/(int f) const
{
    return Int2(X / f, Y / f);
}

DLLEXPORT inline bool Int2::operator<(const Int2& other) const
{
    return X < other.X && Y < other.Y;
}

DLLEXPORT inline bool Int2::operator<=(const Int2& other) const
{
    return X <= other.X && Y <= other.Y;
}

DLLEXPORT inline bool Int2::operator>(const Int2& other) const
{
    return X > other.X && Y > other.Y;
}

DLLEXPORT inline bool Int2::operator>=(const Int2& other) const
{
    return X >= other.X && Y >= other.Y;
}

DLLEXPORT inline bool Int2::operator==(const Int2& other) const
{
    return X == other.X && Y == other.Y;
}

DLLEXPORT inline bool Int2::operator!=(const Int2& other) const
{
    return X != other.X && Y != other.Y;
}

// ------------------------------------ //

DLLEXPORT void Int2::SetData(const int& data)
{
    X = data;
    Y = data;
}

DLLEXPORT void Int2::SetData(const int& data1, const int& data2)
{
    X = data1;
    Y = data2;
}
