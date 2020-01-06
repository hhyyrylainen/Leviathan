// ------------------------------------ //
#include "Quaternion.h"

#include "Matrix.h"

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
// ------------------------------------ //
DLLEXPORT void Quaternion::FromRotationMatrix(const Matrix3& matrix)
{
    // Code taken from bs::framework with modifications, see License.txt for details

    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    float trace = matrix[0][0] + matrix[1][1] + matrix[2][2];
    float root;

    if(trace > 0.0f) {
        // |w| > 1/2, may as well choose w > 1/2
        root = std::sqrt(trace + 1.0f); // 2w
        W = 0.5f * root;
        root = 0.5f / root; // 1/(4w)
        X = (matrix[2][1] - matrix[1][2]) * root;
        Y = (matrix[0][2] - matrix[2][0]) * root;
        Z = (matrix[1][0] - matrix[0][1]) * root;
    } else {
        // |w| <= 1/2
        static uint32_t nextLookup[3] = {1, 2, 0};
        uint32_t i = 0;

        if(matrix[1][1] > matrix[0][0])
            i = 1;

        if(matrix[2][2] > matrix[i][i])
            i = 2;

        uint32_t j = nextLookup[i];
        uint32_t k = nextLookup[j];

        root = std::sqrt(matrix[i][i] - matrix[j][j] - matrix[k][k] + 1.0f);

        float* cmpntLookup[3] = {&X, &Y, &Z};
        *cmpntLookup[i] = 0.5f * root;
        root = 0.5f / root;

        W = (matrix[k][j] - matrix[j][k]) * root;
        *cmpntLookup[j] = (matrix[j][i] + matrix[i][j]) * root;
        *cmpntLookup[k] = (matrix[k][i] + matrix[i][k]) * root;
    }

    Normalize();
}

DLLEXPORT void Quaternion::ToRotationMatrix(Matrix3& matrix) const
{
    // Math from bs::framework
    const float tx = X + X;
    const float ty = Y + Y;
    const float tz = Z + Z;
    const float twx = tx * W;
    const float twy = ty * W;
    const float twz = tz * W;
    const float txx = tx * X;
    const float txy = ty * X;
    const float txz = tz * X;
    const float tyy = ty * Y;
    const float tyz = tz * Y;
    const float tzz = tz * Z;

    matrix[0][0] = 1.0f - (tyy + tzz);
    matrix[0][1] = txy - twz;
    matrix[0][2] = txz + twy;
    matrix[1][0] = txy + twz;
    matrix[1][1] = 1.0f - (txx + tzz);
    matrix[1][2] = tyz - twx;
    matrix[2][0] = txz - twy;
    matrix[2][1] = tyz + twx;
    matrix[2][2] = 1.0f - (txx + tyy);
}
