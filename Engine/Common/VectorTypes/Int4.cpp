#include "Int4.h"

using namespace Leviathan;

DLLEXPORT Int4::Int4(int x, int y, int z, int w) : X(x), Y(y), Z(z), W(w) {}

DLLEXPORT Int4::Int4(int data)
{
    X = Y = Z = W = data;
}

// ------------------------------------ //
DLLEXPORT Int4& Int4::operator+(const Int4& val)
{
    X += val.X;
    Y += val.Y;
    Z += val.Z;
    W += val.W;
    return *this;
}
DLLEXPORT int Int4::operator[](const int nIndex) const
{
    switch(nIndex) {
    case 0: return X;
    case 1: return Y;
    case 2: return Z;
    case 3: return W;
    }

    LEVIATHAN_ASSERT(0, "invalid Int4[] access");
    return 0;
}
DLLEXPORT Int4& Int4::operator-(const Int4& val)
{
    X -= val.X;
    Y -= val.Y;
    Z -= val.Z;
    W -= val.W;
    return *this;
}
DLLEXPORT int Int4::AddAllTogether() const
{
    return X + Y + Z + W;
}
