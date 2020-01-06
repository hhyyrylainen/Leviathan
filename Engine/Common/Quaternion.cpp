// ------------------------------------ //
#include "Quaternion.h"

using namespace Leviathan;
// ------------------------------------ //
// See comment about IDENTITY
#ifdef _MSC_VER
DLLEXPORT const Quaternion Quaternion::IDENTITY{0.f, 0.f, 0.f, 1.f};
#endif
// ------------------------------------ //
// The _Axis methods are copied from bs::framework with modifications see License.txt for
// details
DLLEXPORT Float3 Quaternion::XAxis() const
{
    const float fTy = 2.0f * Y;
    const float fTz = 2.0f * Z;
    const float fTwy = fTy * W;
    const float fTwz = fTz * W;
    const float fTxy = fTy * X;
    const float fTxz = fTz * X;
    const float fTyy = fTy * Y;
    const float fTzz = fTz * Z;

    return Float3(1.0f - (fTyy + fTzz), fTxy + fTwz, fTxz - fTwy);
}

DLLEXPORT Float3 Quaternion::YAxis() const
{
    const float fTx = 2.0f * X;
    const float fTy = 2.0f * Y;
    const float fTz = 2.0f * Z;
    const float fTwx = fTx * W;
    const float fTwz = fTz * W;
    const float fTxx = fTx * X;
    const float fTxy = fTy * X;
    const float fTyz = fTz * Y;
    const float fTzz = fTz * Z;

    return Float3(fTxy - fTwz, 1.0f - (fTxx + fTzz), fTyz + fTwx);
}

DLLEXPORT Float3 Quaternion::ZAxis() const
{
    const float fTx = 2.0f * X;
    const float fTy = 2.0f * Y;
    const float fTz = 2.0f * Z;
    const float fTwx = fTx * W;
    const float fTwy = fTy * W;
    const float fTxx = fTx * X;
    const float fTxz = fTz * X;
    const float fTyy = fTy * Y;
    const float fTyz = fTz * Y;

    return Float3(fTxz + fTwy, fTyz - fTwx, 1.0f - (fTxx + fTyy));
}
