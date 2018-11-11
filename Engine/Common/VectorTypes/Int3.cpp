#include "Int3.h"

using namespace Leviathan;

DLLEXPORT Int3::Int3(int x, int y, int z)
{
    X = x;
    Y = y;
    Z = z;
}
DLLEXPORT Int3::Int3(int data)
{
    // save a bit of space //
    X = Y = Z = data;
}

// ------------------------------------ //
DLLEXPORT Int3 Int3::operator+(const Int3& val) const
{
    return Int3(X + val.X, Y + val.Y, Z + val.Z);
}

DLLEXPORT Int3 Int3::operator*(int val) const
{
    return Int3(X * val, Y * val, Z * val);
}

DLLEXPORT int Int3::operator[](const int nIndex) const
{
    switch(nIndex) {
    case 0: return X;
    case 1: return Y;
    case 2: return Z;
    }

    LEVIATHAN_ASSERT(0, "invalid Int3[] access");
    return 0;
}
DLLEXPORT Int3 Int3::operator-(const Int3& other) const
{
    return Int3(X - other.X, Y - other.Y, Z - other.Z);
}

DLLEXPORT inline Int3* Int3::operator*=(int f)
{
    X *= f;
    Y *= f;
    Z *= f;
    return this;
}

DLLEXPORT int Int3::AddAllTogether() const
{
    return X + Y + Z;
}
