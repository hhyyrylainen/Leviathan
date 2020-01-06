// ------------------------------------ //
#include "Matrix.h"

using namespace Leviathan;
// ------------------------------------ //
// This file contains a lot of code from bs::framework with modifications, see License.txt for
// details. Original copyright notice:
//************************************ bs::framework - Copyright 2018 Marko Pintera
//**************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not
// to be removed. ***********//
// ------------------------------------ //
// Matrix3
DLLEXPORT const Matrix3 Matrix3::IDENTITY = {
    1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
DLLEXPORT const Matrix3 Matrix3::ZERO = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
// ------------------------------------ //
DLLEXPORT void Matrix3::FromAxes(const Float3& xAxis, const Float3& yAxis, const Float3& zAxis)
{
    SetColumn(0, xAxis);
    SetColumn(1, yAxis);
    SetColumn(2, zAxis);
}
// ------------------------------------ //
DLLEXPORT Float3 Matrix3::Multiply(const Float3& vec) const
{
    Float3 prod;
    for(uint32_t row = 0; row < 3; row++) {
        prod[row] = m[row][0] * vec[0] + m[row][1] * vec[1] + m[row][2] * vec[2];
    }

    return prod;
}

DLLEXPORT Matrix3 Matrix3::Transpose() const
{
    Matrix3 matTranspose;
    for(uint32_t row = 0; row < 3; row++) {
        for(uint32_t col = 0; col < 3; col++)
            matTranspose[row][col] = m[col][row];
    }

    return matTranspose;
}

DLLEXPORT bool Matrix3::Inverse(Matrix3& matInv, float tolerance) const
{
    matInv[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    matInv[0][1] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
    matInv[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];
    matInv[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    matInv[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    matInv[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];
    matInv[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    matInv[2][1] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
    matInv[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];

    float det = m[0][0] * matInv[0][0] + m[0][1] * matInv[1][0] + m[0][2] * matInv[2][0];

    if(std::abs(det) <= tolerance)
        return false;

    float invDet = 1.0f / det;
    for(uint32_t row = 0; row < 3; row++) {
        for(uint32_t col = 0; col < 3; col++)
            matInv[row][col] *= invDet;
    }

    return true;
}

DLLEXPORT Matrix3 Matrix3::Inverse(float tolerance) const
{
    Matrix3 matInv = Matrix3::ZERO;
    Inverse(matInv, tolerance);
    return matInv;
}

DLLEXPORT float Matrix3::Determinant() const
{
    float cofactor00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    float cofactor10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    float cofactor20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    float det = m[0][0] * cofactor00 + m[0][1] * cofactor10 + m[0][2] * cofactor20;

    return det;
}
// ------------------------------------ //
void Matrix3::bidiagonalize(Matrix3& matA, Matrix3& matL, Matrix3& matR)
{
    float v[3], w[3];
    float length, sign, t1, invT1, t2;
    bool bIdentity;

    // Map first column to (*,0,0)
    length =
        std::sqrt(matA[0][0] * matA[0][0] + matA[1][0] * matA[1][0] + matA[2][0] * matA[2][0]);
    if(length > 0.0f) {
        sign = (matA[0][0] > 0.0f ? 1.0f : -1.0f);
        t1 = matA[0][0] + sign * length;
        invT1 = 1.0f / t1;
        v[1] = matA[1][0] * invT1;
        v[2] = matA[2][0] * invT1;

        t2 = -2.0f / (1.0f + v[1] * v[1] + v[2] * v[2]);
        w[0] = t2 * (matA[0][0] + matA[1][0] * v[1] + matA[2][0] * v[2]);
        w[1] = t2 * (matA[0][1] + matA[1][1] * v[1] + matA[2][1] * v[2]);
        w[2] = t2 * (matA[0][2] + matA[1][2] * v[1] + matA[2][2] * v[2]);
        matA[0][0] += w[0];
        matA[0][1] += w[1];
        matA[0][2] += w[2];
        matA[1][1] += v[1] * w[1];
        matA[1][2] += v[1] * w[2];
        matA[2][1] += v[2] * w[1];
        matA[2][2] += v[2] * w[2];

        matL[0][0] = 1.0f + t2;
        matL[0][1] = matL[1][0] = t2 * v[1];
        matL[0][2] = matL[2][0] = t2 * v[2];
        matL[1][1] = 1.0f + t2 * v[1] * v[1];
        matL[1][2] = matL[2][1] = t2 * v[1] * v[2];
        matL[2][2] = 1.0f + t2 * v[2] * v[2];
        bIdentity = false;
    } else {
        matL = Matrix3::IDENTITY;
        bIdentity = true;
    }

    // Map first row to (*,*,0)
    length = std::sqrt(matA[0][1] * matA[0][1] + matA[0][2] * matA[0][2]);
    if(length > 0.0) {
        sign = (matA[0][1] > 0.0f ? 1.0f : -1.0f);
        t1 = matA[0][1] + sign * length;
        v[2] = matA[0][2] / t1;

        t2 = -2.0f / (1.0f + v[2] * v[2]);
        w[0] = t2 * (matA[0][1] + matA[0][2] * v[2]);
        w[1] = t2 * (matA[1][1] + matA[1][2] * v[2]);
        w[2] = t2 * (matA[2][1] + matA[2][2] * v[2]);
        matA[0][1] += w[0];
        matA[1][1] += w[1];
        matA[1][2] += w[1] * v[2];
        matA[2][1] += w[2];
        matA[2][2] += w[2] * v[2];

        matR[0][0] = 1.0;
        matR[0][1] = matR[1][0] = 0.0;
        matR[0][2] = matR[2][0] = 0.0;
        matR[1][1] = 1.0f + t2;
        matR[1][2] = matR[2][1] = t2 * v[2];
        matR[2][2] = 1.0f + t2 * v[2] * v[2];
    } else {
        matR = Matrix3::IDENTITY;
    }

    // Map second column to (*,*,0)
    length = std::sqrt(matA[1][1] * matA[1][1] + matA[2][1] * matA[2][1]);
    if(length > 0.0) {
        sign = (matA[1][1] > 0.0f ? 1.0f : -1.0f);
        t1 = matA[1][1] + sign * length;
        v[2] = matA[2][1] / t1;

        t2 = -2.0f / (1.0f + v[2] * v[2]);
        w[1] = t2 * (matA[1][1] + matA[2][1] * v[2]);
        w[2] = t2 * (matA[1][2] + matA[2][2] * v[2]);
        matA[1][1] += w[1];
        matA[1][2] += w[2];
        matA[2][2] += v[2] * w[2];

        float a = 1.0f + t2;
        float b = t2 * v[2];
        float c = 1.0f + b * v[2];

        if(bIdentity) {
            matL[0][0] = 1.0;
            matL[0][1] = matL[1][0] = 0.0;
            matL[0][2] = matL[2][0] = 0.0;
            matL[1][1] = a;
            matL[1][2] = matL[2][1] = b;
            matL[2][2] = c;
        } else {
            for(int row = 0; row < 3; row++) {
                float tmp0 = matL[row][1];
                float tmp1 = matL[row][2];
                matL[row][1] = a * tmp0 + b * tmp1;
                matL[row][2] = b * tmp0 + c * tmp1;
            }
        }
    }
}
// ------------------------------------ //
void Matrix3::golubKahanStep(Matrix3& matA, Matrix3& matL, Matrix3& matR)
{
    float f11 = matA[0][1] * matA[0][1] + matA[1][1] * matA[1][1];
    float t22 = matA[1][2] * matA[1][2] + matA[2][2] * matA[2][2];
    float t12 = matA[1][1] * matA[1][2];
    float trace = f11 + t22;
    float diff = f11 - t22;
    float discr = std::sqrt(diff * diff + 4.0f * t12 * t12);
    float root1 = 0.5f * (trace + discr);
    float root2 = 0.5f * (trace - discr);

    // Adjust right
    float y = matA[0][0] - (std::abs(root1 - t22) <= std::abs(root2 - t22) ? root1 : root2);
    float z = matA[0][1];
    float invLength = 1.f / std::sqrt(y * y + z * z);
    float sin = z * invLength;
    float cos = -y * invLength;

    float tmp0 = matA[0][0];
    float tmp1 = matA[0][1];
    matA[0][0] = cos * tmp0 - sin * tmp1;
    matA[0][1] = sin * tmp0 + cos * tmp1;
    matA[1][0] = -sin * matA[1][1];
    matA[1][1] *= cos;

    uint32_t row;
    for(row = 0; row < 3; row++) {
        tmp0 = matR[0][row];
        tmp1 = matR[1][row];
        matR[0][row] = cos * tmp0 - sin * tmp1;
        matR[1][row] = sin * tmp0 + cos * tmp1;
    }

    // Adjust left
    y = matA[0][0];
    z = matA[1][0];
    invLength = 1.f / std::sqrt(y * y + z * z);
    sin = z * invLength;
    cos = -y * invLength;

    matA[0][0] = cos * matA[0][0] - sin * matA[1][0];
    tmp0 = matA[0][1];
    tmp1 = matA[1][1];
    matA[0][1] = cos * tmp0 - sin * tmp1;
    matA[1][1] = sin * tmp0 + cos * tmp1;
    matA[0][2] = -sin * matA[1][2];
    matA[1][2] *= cos;

    uint32_t col;
    for(col = 0; col < 3; col++) {
        tmp0 = matL[col][0];
        tmp1 = matL[col][1];
        matL[col][0] = cos * tmp0 - sin * tmp1;
        matL[col][1] = sin * tmp0 + cos * tmp1;
    }

    // Adjust right
    y = matA[0][1];
    z = matA[0][2];
    invLength = 1.f / std::sqrt(y * y + z * z);
    sin = z * invLength;
    cos = -y * invLength;

    matA[0][1] = cos * matA[0][1] - sin * matA[0][2];
    tmp0 = matA[1][1];
    tmp1 = matA[1][2];
    matA[1][1] = cos * tmp0 - sin * tmp1;
    matA[1][2] = sin * tmp0 + cos * tmp1;
    matA[2][1] = -sin * matA[2][2];
    matA[2][2] *= cos;

    for(row = 0; row < 3; row++) {
        tmp0 = matR[1][row];
        tmp1 = matR[2][row];
        matR[1][row] = cos * tmp0 - sin * tmp1;
        matR[2][row] = sin * tmp0 + cos * tmp1;
    }

    // Adjust left
    y = matA[1][1];
    z = matA[2][1];
    invLength = 1.f / std::sqrt(y * y + z * z);
    sin = z * invLength;
    cos = -y * invLength;

    matA[1][1] = cos * matA[1][1] - sin * matA[2][1];
    tmp0 = matA[1][2];
    tmp1 = matA[2][2];
    matA[1][2] = cos * tmp0 - sin * tmp1;
    matA[2][2] = sin * tmp0 + cos * tmp1;

    for(col = 0; col < 3; col++) {
        tmp0 = matL[col][1];
        tmp1 = matL[col][2];
        matL[col][1] = cos * tmp0 - sin * tmp1;
        matL[col][2] = sin * tmp0 + cos * tmp1;
    }
}

DLLEXPORT void Matrix3::SingularValueDecomposition(
    Matrix3& matL, Float3& matS, Matrix3& matR) const
{
    uint32_t row, col;

    Matrix3 mat = *this;
    bidiagonalize(mat, matL, matR);

    for(unsigned int i = 0; i < SVD_MAX_ITERS; i++) {
        float tmp, tmp0, tmp1;
        float sin0, cos0, tan0;
        float sin1, cos1, tan1;

        bool test1 =
            (std::abs(mat[0][1]) <= SVD_EPSILON * (std::abs(mat[0][0]) + std::abs(mat[1][1])));
        bool test2 =
            (std::abs(mat[1][2]) <= SVD_EPSILON * (std::abs(mat[1][1]) + std::abs(mat[2][2])));

        if(test1) {
            if(test2) {
                matS[0] = mat[0][0];
                matS[1] = mat[1][1];
                matS[2] = mat[2][2];
                break;
            } else {
                // 2x2 closed form factorization
                tmp = (mat[1][1] * mat[1][1] - mat[2][2] * mat[2][2] + mat[1][2] * mat[1][2]) /
                      (mat[1][2] * mat[2][2]);
                tan0 = 0.5f * (tmp + std::sqrt(tmp * tmp + 4.0f));
                cos0 = 1.f / std::sqrt(1.0f + tan0 * tan0);
                sin0 = tan0 * cos0;

                for(col = 0; col < 3; col++) {
                    tmp0 = matL[col][1];
                    tmp1 = matL[col][2];
                    matL[col][1] = cos0 * tmp0 - sin0 * tmp1;
                    matL[col][2] = sin0 * tmp0 + cos0 * tmp1;
                }

                tan1 = (mat[1][2] - mat[2][2] * tan0) / mat[1][1];
                cos1 = 1.f / std::sqrt(1.0f + tan1 * tan1);
                sin1 = -tan1 * cos1;

                for(row = 0; row < 3; row++) {
                    tmp0 = matR[1][row];
                    tmp1 = matR[2][row];
                    matR[1][row] = cos1 * tmp0 - sin1 * tmp1;
                    matR[2][row] = sin1 * tmp0 + cos1 * tmp1;
                }

                matS[0] = mat[0][0];
                matS[1] =
                    cos0 * cos1 * mat[1][1] - sin1 * (cos0 * mat[1][2] - sin0 * mat[2][2]);
                matS[2] =
                    sin0 * sin1 * mat[1][1] + cos1 * (sin0 * mat[1][2] + cos0 * mat[2][2]);
                break;
            }
        } else {
            if(test2) {
                // 2x2 closed form factorization
                tmp = (mat[0][0] * mat[0][0] + mat[1][1] * mat[1][1] - mat[0][1] * mat[0][1]) /
                      (mat[0][1] * mat[1][1]);
                tan0 = 0.5f * (-tmp + std::sqrt(tmp * tmp + 4.0f));
                cos0 = 1.f / std::sqrt(1.0f + tan0 * tan0);
                sin0 = tan0 * cos0;

                for(col = 0; col < 3; col++) {
                    tmp0 = matL[col][0];
                    tmp1 = matL[col][1];
                    matL[col][0] = cos0 * tmp0 - sin0 * tmp1;
                    matL[col][1] = sin0 * tmp0 + cos0 * tmp1;
                }

                tan1 = (mat[0][1] - mat[1][1] * tan0) / mat[0][0];
                cos1 = 1.f / std::sqrt(1.0f + tan1 * tan1);
                sin1 = -tan1 * cos1;

                for(row = 0; row < 3; row++) {
                    tmp0 = matR[0][row];
                    tmp1 = matR[1][row];
                    matR[0][row] = cos1 * tmp0 - sin1 * tmp1;
                    matR[1][row] = sin1 * tmp0 + cos1 * tmp1;
                }

                matS[0] =
                    cos0 * cos1 * mat[0][0] - sin1 * (cos0 * mat[0][1] - sin0 * mat[1][1]);
                matS[1] =
                    sin0 * sin1 * mat[0][0] + cos1 * (sin0 * mat[0][1] + cos0 * mat[1][1]);
                matS[2] = mat[2][2];
                break;
            } else {
                golubKahanStep(mat, matL, matR);
            }
        }
    }

    // Positize diagonal
    for(row = 0; row < 3; row++) {
        if(matS[row] < 0.0) {
            matS[row] = -matS[row];
            for(col = 0; col < 3; col++)
                matR[row][col] = -matR[row][col];
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Matrix3::Orthonormalize()
{
    // Compute q0
    float invLength =
        1.f / std::sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]);

    m[0][0] *= invLength;
    m[1][0] *= invLength;
    m[2][0] *= invLength;

    // Compute q1
    float dot0 = m[0][0] * m[0][1] + m[1][0] * m[1][1] + m[2][0] * m[2][1];

    m[0][1] -= dot0 * m[0][0];
    m[1][1] -= dot0 * m[1][0];
    m[2][1] -= dot0 * m[2][0];

    invLength = 1.f / std::sqrt(m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1]);

    m[0][1] *= invLength;
    m[1][1] *= invLength;
    m[2][1] *= invLength;

    // Compute q2
    float dot1 = m[0][1] * m[0][2] + m[1][1] * m[1][2] + m[2][1] * m[2][2];
    dot0 = m[0][0] * m[0][2] + m[1][0] * m[1][2] + m[2][0] * m[2][2];

    m[0][2] -= dot0 * m[0][0] + dot1 * m[0][1];
    m[1][2] -= dot0 * m[1][0] + dot1 * m[1][1];
    m[2][2] -= dot0 * m[2][0] + dot1 * m[2][1];

    invLength = 1.f / std::sqrt(m[0][2] * m[0][2] + m[1][2] * m[1][2] + m[2][2] * m[2][2]);

    m[0][2] *= invLength;
    m[1][2] *= invLength;
    m[2][2] *= invLength;
}
// ------------------------------------ //
DLLEXPORT void Matrix3::Decomposition(Quaternion& rotation, Float3& scale) const
{
    Matrix3 matQ;
    Float3 vecU;
    QDUDecomposition(matQ, scale, vecU);

    rotation = Quaternion(matQ);
}

DLLEXPORT void Matrix3::QDUDecomposition(Matrix3& matQ, Float3& vecD, Float3& vecU) const
{
    // Build orthogonal matrix Q
    float invLength =
        1.f / std::sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]);
    matQ[0][0] = m[0][0] * invLength;
    matQ[1][0] = m[1][0] * invLength;
    matQ[2][0] = m[2][0] * invLength;

    float dot = matQ[0][0] * m[0][1] + matQ[1][0] * m[1][1] + matQ[2][0] * m[2][1];
    matQ[0][1] = m[0][1] - dot * matQ[0][0];
    matQ[1][1] = m[1][1] - dot * matQ[1][0];
    matQ[2][1] = m[2][1] - dot * matQ[2][0];

    invLength = 1.f / std::sqrt(matQ[0][1] * matQ[0][1] + matQ[1][1] * matQ[1][1] +
                                matQ[2][1] * matQ[2][1]);
    matQ[0][1] *= invLength;
    matQ[1][1] *= invLength;
    matQ[2][1] *= invLength;

    dot = matQ[0][0] * m[0][2] + matQ[1][0] * m[1][2] + matQ[2][0] * m[2][2];
    matQ[0][2] = m[0][2] - dot * matQ[0][0];
    matQ[1][2] = m[1][2] - dot * matQ[1][0];
    matQ[2][2] = m[2][2] - dot * matQ[2][0];

    dot = matQ[0][1] * m[0][2] + matQ[1][1] * m[1][2] + matQ[2][1] * m[2][2];
    matQ[0][2] -= dot * matQ[0][1];
    matQ[1][2] -= dot * matQ[1][1];
    matQ[2][2] -= dot * matQ[2][1];

    invLength = 1.f / std::sqrt(matQ[0][2] * matQ[0][2] + matQ[1][2] * matQ[1][2] +
                                matQ[2][2] * matQ[2][2]);
    matQ[0][2] *= invLength;
    matQ[1][2] *= invLength;
    matQ[2][2] *= invLength;

    // Guarantee that orthogonal matrix has determinant 1 (no reflections)
    float fDet = matQ[0][0] * matQ[1][1] * matQ[2][2] + matQ[0][1] * matQ[1][2] * matQ[2][0] +
                 matQ[0][2] * matQ[1][0] * matQ[2][1] - matQ[0][2] * matQ[1][1] * matQ[2][0] -
                 matQ[0][1] * matQ[1][0] * matQ[2][2] - matQ[0][0] * matQ[1][2] * matQ[2][1];

    if(fDet < 0.0f) {
        for(uint32_t row = 0; row < 3; row++)
            for(uint32_t col = 0; col < 3; col++)
                matQ[row][col] = -matQ[row][col];
    }

    // Build "right" matrix R
    Matrix3 matRight;
    matRight[0][0] = matQ[0][0] * m[0][0] + matQ[1][0] * m[1][0] + matQ[2][0] * m[2][0];
    matRight[0][1] = matQ[0][0] * m[0][1] + matQ[1][0] * m[1][1] + matQ[2][0] * m[2][1];
    matRight[1][1] = matQ[0][1] * m[0][1] + matQ[1][1] * m[1][1] + matQ[2][1] * m[2][1];
    matRight[0][2] = matQ[0][0] * m[0][2] + matQ[1][0] * m[1][2] + matQ[2][0] * m[2][2];
    matRight[1][2] = matQ[0][1] * m[0][2] + matQ[1][1] * m[1][2] + matQ[2][1] * m[2][2];
    matRight[2][2] = matQ[0][2] * m[0][2] + matQ[1][2] * m[1][2] + matQ[2][2] * m[2][2];

    // The scaling component
    vecD[0] = matRight[0][0];
    vecD[1] = matRight[1][1];
    vecD[2] = matRight[2][2];

    // The shear component
    float invD0 = 1.0f / vecD[0];
    vecU[0] = matRight[0][1] * invD0;
    vecU[1] = matRight[0][2] * invD0;
    vecU[2] = matRight[1][2] / vecD[1];
}
// ------------------------------------ //
DLLEXPORT void Matrix3::ToAxisAngle(Float3& axis, Radian& radians) const
{
    float trace = m[0][0] + m[1][1] + m[2][2];
    float cos = 0.5f * (trace - 1.0f);
    radians = Radian(std::acos(cos)); // In [0, PI]

    if(radians > Radian(0.0f)) {
        if(radians < Radian(PI)) {
            axis.X = m[2][1] - m[1][2];
            axis.Y = m[0][2] - m[2][0];
            axis.Z = m[1][0] - m[0][1];
            axis.Normalize();
        } else {
            // Angle is PI
            float fHalfInverse;
            if(m[0][0] >= m[1][1]) {
                // r00 >= r11
                if(m[0][0] >= m[2][2]) {
                    // r00 is maximum diagonal term
                    axis.X = 0.5f * std::sqrt(m[0][0] - m[1][1] - m[2][2] + 1.0f);
                    fHalfInverse = 0.5f / axis.X;
                    axis.Y = fHalfInverse * m[0][1];
                    axis.Z = fHalfInverse * m[0][2];
                } else {
                    // r22 is maximum diagonal term
                    axis.Z = 0.5f * std::sqrt(m[2][2] - m[0][0] - m[1][1] + 1.0f);
                    fHalfInverse = 0.5f / axis.Z;
                    axis.X = fHalfInverse * m[0][2];
                    axis.Y = fHalfInverse * m[1][2];
                }
            } else {
                // r11 > r00
                if(m[1][1] >= m[2][2]) {
                    // r11 is maximum diagonal term
                    axis.Y = 0.5f * std::sqrt(m[1][1] - m[0][0] - m[2][2] + 1.0f);
                    fHalfInverse = 0.5f / axis.Y;
                    axis.X = fHalfInverse * m[0][1];
                    axis.Z = fHalfInverse * m[1][2];
                } else {
                    // r22 is maximum diagonal term
                    axis.Z = 0.5f * std::sqrt(m[2][2] - m[0][0] - m[1][1] + 1.0f);
                    fHalfInverse = 0.5f / axis.Z;
                    axis.X = fHalfInverse * m[0][2];
                    axis.Y = fHalfInverse * m[1][2];
                }
            }
        }
    } else {
        // The angle is 0 and the matrix is the identity.  Any axis will
        // work, so just use the x-axis.
        axis.X = 1.0f;
        axis.Y = 0.0f;
        axis.Z = 0.0f;
    }
}

DLLEXPORT void Matrix3::FromAxisAngle(const Float3& axis, const Radian& angle)
{
    float cos = std::cos(angle.ValueInRadians());
    float sin = std::sin(angle.ValueInRadians());
    float oneMinusCos = 1.0f - cos;
    float x2 = axis.X * axis.X;
    float y2 = axis.Y * axis.Y;
    float z2 = axis.Z * axis.Z;
    float xym = axis.X * axis.Y * oneMinusCos;
    float xzm = axis.X * axis.Z * oneMinusCos;
    float yzm = axis.Y * axis.Z * oneMinusCos;
    float xSin = axis.X * sin;
    float ySin = axis.Y * sin;
    float zSin = axis.Z * sin;

    m[0][0] = x2 * oneMinusCos + cos;
    m[0][1] = xym - zSin;
    m[0][2] = xzm + ySin;
    m[1][0] = xym + zSin;
    m[1][1] = y2 * oneMinusCos + cos;
    m[1][2] = yzm - xSin;
    m[2][0] = xzm - ySin;
    m[2][1] = yzm + xSin;
    m[2][2] = z2 * oneMinusCos + cos;
}
// ------------------------------------ //
DLLEXPORT void Matrix3::ToQuaternion(Quaternion& quat) const
{
    quat.FromRotationMatrix(*this);
}

DLLEXPORT void Matrix3::FromQuaternion(const Quaternion& quat)
{
    quat.ToRotationMatrix(*this);
}
// ------------------------------------ //
DLLEXPORT bool Matrix3::ToEulerAngles(Radian& xAngle, Radian& yAngle, Radian& zAngle) const
{
    float m21 = m[2][1];
    if(m21 < 1) {
        if(m21 > -1) {
            xAngle = Radian(std::asin(m21));
            yAngle = Radian(std::atan2(-m[2][0], m[2][2]));
            zAngle = Radian(std::atan2(-m[0][1], m[1][1]));

            return true;
        } else {
            // Note: Not an unique solution.
            xAngle = Radian(-HALF_PI);
            yAngle = Radian(0.0f);
            zAngle = Radian(-std::atan2(m[0][2], m[0][0]));

            return false;
        }
    } else {
        // Note: Not an unique solution.
        xAngle = Radian(HALF_PI);
        yAngle = Radian(0.0f);
        zAngle = Radian(std::atan2(m[0][2], m[0][0]));

        return false;
    }
}

DLLEXPORT void Matrix3::FromEulerAngles(
    const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
{
    float cx = std::cos(xAngle.ValueInRadians());
    float sx = std::sin(xAngle.ValueInRadians());

    float cy = std::cos(yAngle.ValueInRadians());
    float sy = std::sin(yAngle.ValueInRadians());

    float cz = std::cos(zAngle.ValueInRadians());
    float sz = std::sin(zAngle.ValueInRadians());

    m[0][0] = cy * cz - sx * sy * sz;
    m[0][1] = -cx * sz;
    m[0][2] = cz * sy + cy * sx * sz;

    m[1][0] = cz * sx * sy + cy * sz;
    m[1][1] = cx * cz;
    m[1][2] = -cy * cz * sx + sy * sz;

    m[2][0] = -cx * sy;
    m[2][1] = sx;
    m[2][2] = cx * cy;
}

DLLEXPORT void Matrix3::FromEulerAngles(
    const Radian& xAngle, const Radian& yAngle, const Radian& zAngle, EulerAngleOrder order)
{
    // Euler angle conversions
    static constexpr const EulerAngleOrderData EA_LOOKUP[6] = {{0, 1, 2, 1.0f},
        {0, 2, 1, -1.0f}, {1, 0, 2, -1.0f}, {1, 2, 0, 1.0f}, {2, 0, 1, 1.0f},
        {2, 1, 0, -1.0f}};

    const EulerAngleOrderData& l = EA_LOOKUP[(int)order];

    Matrix3 mats[3];
    float cx = std::cos(xAngle.ValueInRadians());
    float sx = std::sin(xAngle.ValueInRadians());
    mats[0] = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, cx, -sx, 0.0f, sx, cx);

    float cy = std::cos(yAngle.ValueInRadians());
    float sy = std::sin(yAngle.ValueInRadians());
    mats[1] = Matrix3(cy, 0.0f, sy, 0.0f, 1.0f, 0.0f, -sy, 0.0f, cy);

    float cz = std::cos(zAngle.ValueInRadians());
    float sz = std::sin(zAngle.ValueInRadians());
    mats[2] = Matrix3(cz, -sz, 0.0f, sz, cz, 0.0f, 0.0f, 0.0f, 1.0f);

    *this = mats[l.c] * (mats[l.b] * mats[l.a]);
}
// ------------------------------------ //
void Matrix3::tridiagonal(float diag[3], float subDiag[3])
{
    // Householder reduction T = Q^t M Q
    //   Input:
    //     mat, symmetric 3x3 matrix M
    //   Output:
    //     mat, orthogonal matrix Q
    //     diag, diagonal entries of T
    //     subd, subdiagonal entries of T (T is symmetric)

    float fA = m[0][0];
    float fB = m[0][1];
    float fC = m[0][2];
    float fD = m[1][1];
    float fE = m[1][2];
    float fF = m[2][2];

    diag[0] = fA;
    subDiag[2] = 0.0;
    if(std::abs(fC) >= EPSILON) {
        float length = std::sqrt(fB * fB + fC * fC);
        float invLength = 1.0f / length;
        fB *= invLength;
        fC *= invLength;
        float fQ = 2.0f * fB * fE + fC * (fF - fD);
        diag[1] = fD + fC * fQ;
        diag[2] = fF - fC * fQ;
        subDiag[0] = length;
        subDiag[1] = fE - fB * fQ;
        m[0][0] = 1.0;
        m[0][1] = 0.0;
        m[0][2] = 0.0;
        m[1][0] = 0.0;
        m[1][1] = fB;
        m[1][2] = fC;
        m[2][0] = 0.0;
        m[2][1] = fC;
        m[2][2] = -fB;
    } else {
        diag[1] = fD;
        diag[2] = fF;
        subDiag[0] = fB;
        subDiag[1] = fE;
        m[0][0] = 1.0;
        m[0][1] = 0.0;
        m[0][2] = 0.0;
        m[1][0] = 0.0;
        m[1][1] = 1.0;
        m[1][2] = 0.0;
        m[2][0] = 0.0;
        m[2][1] = 0.0;
        m[2][2] = 1.0;
    }
}

bool Matrix3::QLAlgorithm(float diag[3], float subDiag[3])
{
    // QL iteration with implicit shifting to reduce matrix from tridiagonal to diagonal

    for(int i = 0; i < 3; i++) {
        const unsigned int maxIter = 32;
        unsigned int iter;
        for(iter = 0; iter < maxIter; iter++) {
            int j;
            for(j = i; j <= 1; j++) {
                float sum = std::abs(diag[j]) + std::abs(diag[j + 1]);

                if(std::abs(subDiag[j]) + sum == sum)
                    break;
            }

            if(j == i)
                break;

            float tmp0 = (diag[i + 1] - diag[i]) / (2.0f * subDiag[i]);
            float tmp1 = std::sqrt(tmp0 * tmp0 + 1.0f);

            if(tmp0 < 0.0f)
                tmp0 = diag[j] - diag[i] + subDiag[i] / (tmp0 - tmp1);
            else
                tmp0 = diag[j] - diag[i] + subDiag[i] / (tmp0 + tmp1);

            float sin = 1.0f;
            float cos = 1.0f;
            float tmp2 = 0.0f;
            for(int k = j - 1; k >= i; k--) {
                float tmp3 = sin * subDiag[k];
                float tmp4 = cos * subDiag[k];

                if(std::abs(tmp3) >= std::abs(tmp0)) {
                    cos = tmp0 / tmp3;
                    tmp1 = std::sqrt(cos * cos + 1.0f);
                    subDiag[k + 1] = tmp3 * tmp1;
                    sin = 1.0f / tmp1;
                    cos *= sin;
                } else {
                    sin = tmp3 / tmp0;
                    tmp1 = std::sqrt(sin * sin + 1.0f);
                    subDiag[k + 1] = tmp0 * tmp1;
                    cos = 1.0f / tmp1;
                    sin *= cos;
                }

                tmp0 = diag[k + 1] - tmp2;
                tmp1 = (diag[k] - tmp0) * sin + 2.0f * tmp4 * cos;
                tmp2 = sin * tmp1;
                diag[k + 1] = tmp0 + tmp2;
                tmp0 = cos * tmp1 - tmp4;

                for(int row = 0; row < 3; row++) {
                    tmp3 = m[row][k + 1];
                    m[row][k + 1] = sin * m[row][k] + cos * tmp3;
                    m[row][k] = cos * m[row][k] - sin * tmp3;
                }
            }

            diag[i] -= tmp2;
            subDiag[i] = tmp0;
            subDiag[j] = 0.0;
        }

        if(iter == maxIter) {
            // Should not get here under normal circumstances
            return false;
        }
    }

    return true;
}

DLLEXPORT void Matrix3::EigenSolveSymmetric(float eigenValues[3], Float3 eigenVectors[3]) const
{
    Matrix3 mat = *this;
    float subDiag[3];
    mat.tridiagonal(eigenValues, subDiag);
    mat.QLAlgorithm(eigenValues, subDiag);

    for(uint32_t i = 0; i < 3; i++) {
        eigenVectors[i][0] = mat[0][i];
        eigenVectors[i][1] = mat[1][i];
        eigenVectors[i][2] = mat[2][i];
    }

    // Make eigenvectors form a right--handed system
    Float3 cross = eigenVectors[1].Cross(eigenVectors[2]);
    float det = eigenVectors[0].Dot(cross);
    if(det < 0.0f) {
        eigenVectors[2][0] = -eigenVectors[2][0];
        eigenVectors[2][1] = -eigenVectors[2][1];
        eigenVectors[2][2] = -eigenVectors[2][2];
    }
}
// ------------------------------------ //
Matrix3 Leviathan::operator*(float lhs, const Matrix3& rhs)
{
    Matrix3 prod;
    for(uint32_t row = 0; row < 3; row++) {
        for(uint32_t col = 0; col < 3; col++)
            prod[row][col] = lhs * rhs.m[row][col];
    }

    return prod;
}

// ------------------------------------ //
// Matrix 4
DLLEXPORT const Matrix4 Matrix4::IDENTITY = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
DLLEXPORT const Matrix4 Matrix4::ZERO = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
// ------------------------------------ //
static float MINOR(const Matrix4& m, const uint32_t r0, const uint32_t r1, const uint32_t r2,
    const uint32_t c0, const uint32_t c1, const uint32_t c2)
{
    return m[r0][c0] * (m[r1][c1] * m[r2][c2] - m[r2][c1] * m[r1][c2]) -
           m[r0][c1] * (m[r1][c0] * m[r2][c2] - m[r2][c0] * m[r1][c2]) +
           m[r0][c2] * (m[r1][c0] * m[r2][c1] - m[r2][c0] * m[r1][c1]);
}

DLLEXPORT Matrix4 Matrix4::Adjoint() const
{
    return Matrix4(MINOR(*this, 1, 2, 3, 1, 2, 3), -MINOR(*this, 0, 2, 3, 1, 2, 3),
        MINOR(*this, 0, 1, 3, 1, 2, 3), -MINOR(*this, 0, 1, 2, 1, 2, 3),

        -MINOR(*this, 1, 2, 3, 0, 2, 3), MINOR(*this, 0, 2, 3, 0, 2, 3),
        -MINOR(*this, 0, 1, 3, 0, 2, 3), MINOR(*this, 0, 1, 2, 0, 2, 3),

        MINOR(*this, 1, 2, 3, 0, 1, 3), -MINOR(*this, 0, 2, 3, 0, 1, 3),
        MINOR(*this, 0, 1, 3, 0, 1, 3), -MINOR(*this, 0, 1, 2, 0, 1, 3),

        -MINOR(*this, 1, 2, 3, 0, 1, 2), MINOR(*this, 0, 2, 3, 0, 1, 2),
        -MINOR(*this, 0, 1, 3, 0, 1, 2), MINOR(*this, 0, 1, 2, 0, 1, 2));
}

DLLEXPORT float Matrix4::Determinant() const
{
    return m[0][0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
           m[0][1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
           m[0][2] * MINOR(*this, 1, 2, 3, 0, 1, 3) - m[0][3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
}

DLLEXPORT float Matrix4::Determinant3x3() const
{
    float cofactor00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    float cofactor10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    float cofactor20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    float det = m[0][0] * cofactor00 + m[0][1] * cofactor10 + m[0][2] * cofactor20;

    return det;
}

DLLEXPORT Matrix4 Matrix4::Inverse() const
{
    float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
    float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
    float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
    float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

    float v0 = m20 * m31 - m21 * m30;
    float v1 = m20 * m32 - m22 * m30;
    float v2 = m20 * m33 - m23 * m30;
    float v3 = m21 * m32 - m22 * m31;
    float v4 = m21 * m33 - m23 * m31;
    float v5 = m22 * m33 - m23 * m32;

    float t00 = +(v5 * m11 - v4 * m12 + v3 * m13);
    float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
    float t20 = +(v4 * m10 - v2 * m11 + v0 * m13);
    float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

    float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

    float d00 = t00 * invDet;
    float d10 = t10 * invDet;
    float d20 = t20 * invDet;
    float d30 = t30 * invDet;

    float d01 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d11 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d21 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d31 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    float d02 = +(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d12 = -(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d22 = +(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d32 = -(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    float d03 = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d13 = +(v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d23 = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d33 = +(v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    return Matrix4(
        d00, d01, d02, d03, d10, d11, d12, d13, d20, d21, d22, d23, d30, d31, d32, d33);
}

DLLEXPORT Matrix4 Matrix4::InverseAffine() const
{
    float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
    float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

    float t00 = m22 * m11 - m21 * m12;
    float t10 = m20 * m12 - m22 * m10;
    float t20 = m21 * m10 - m20 * m11;

    float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];

    float invDet = 1 / (m00 * t00 + m01 * t10 + m02 * t20);

    t00 *= invDet;
    t10 *= invDet;
    t20 *= invDet;

    m00 *= invDet;
    m01 *= invDet;
    m02 *= invDet;

    float r00 = t00;
    float r01 = m02 * m21 - m01 * m22;
    float r02 = m01 * m12 - m02 * m11;

    float r10 = t10;
    float r11 = m00 * m22 - m02 * m20;
    float r12 = m02 * m10 - m00 * m12;

    float r20 = t20;
    float r21 = m01 * m20 - m00 * m21;
    float r22 = m00 * m11 - m01 * m10;

    float m03 = m[0][3], m13 = m[1][3], m23 = m[2][3];

    float r03 = -(r00 * m03 + r01 * m13 + r02 * m23);
    float r13 = -(r10 * m03 + r11 * m13 + r12 * m23);
    float r23 = -(r20 * m03 + r21 * m13 + r22 * m23);

    return Matrix4(r00, r01, r02, r03, r10, r11, r12, r13, r20, r21, r22, r23, 0, 0, 0, 1);
}

DLLEXPORT void Matrix4::SetTRS(
    const Float3& translation, const Quaternion& rotation, const Float3& scale)
{
    Matrix3 rot3x3;
    rotation.ToRotationMatrix(rot3x3);

    m[0][0] = scale.X * rot3x3[0][0];
    m[0][1] = scale.Y * rot3x3[0][1];
    m[0][2] = scale.Z * rot3x3[0][2];
    m[0][3] = translation.X;
    m[1][0] = scale.X * rot3x3[1][0];
    m[1][1] = scale.Y * rot3x3[1][1];
    m[1][2] = scale.Z * rot3x3[1][2];
    m[1][3] = translation.Y;
    m[2][0] = scale.X * rot3x3[2][0];
    m[2][1] = scale.Y * rot3x3[2][1];
    m[2][2] = scale.Z * rot3x3[2][2];
    m[2][3] = translation.Z;

    // No projection term
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}

DLLEXPORT void Matrix4::SetInverseTRS(
    const Float3& translation, const Quaternion& rotation, const Float3& scale)
{
    // Invert the parameters
    Float3 invTranslate = -translation;
    Float3 invScale(1 / scale.X, 1 / scale.Y, 1 / scale.Z);
    Quaternion invRot = rotation.Inverse();

    // Because we're inverting, order is translation, rotation, scale
    // So make translation relative to scale & rotation
    invTranslate = invRot * invTranslate;
    invTranslate *= invScale;

    // Next, make a 3x3 rotation matrix
    Matrix3 rot3x3;
    invRot.ToRotationMatrix(rot3x3);

    // Set up final matrix with scale, rotation and translation
    m[0][0] = invScale.X * rot3x3[0][0];
    m[0][1] = invScale.X * rot3x3[0][1];
    m[0][2] = invScale.X * rot3x3[0][2];
    m[0][3] = invTranslate.X;
    m[1][0] = invScale.Y * rot3x3[1][0];
    m[1][1] = invScale.Y * rot3x3[1][1];
    m[1][2] = invScale.Y * rot3x3[1][2];
    m[1][3] = invTranslate.Y;
    m[2][0] = invScale.Z * rot3x3[2][0];
    m[2][1] = invScale.Z * rot3x3[2][1];
    m[2][2] = invScale.Z * rot3x3[2][2];
    m[2][3] = invTranslate.Z;

    // No projection term
    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}

DLLEXPORT void Matrix4::Decomposition(
    Float3& position, Quaternion& rotation, Float3& scale) const
{
    Matrix3 m3x3 = get3x3();

    Matrix3 matQ;
    Float3 vecU;
    m3x3.QDUDecomposition(matQ, scale, vecU);

    rotation = Quaternion(matQ);
    position = Float3(m[0][3], m[1][3], m[2][3]);
}

DLLEXPORT void Matrix4::MakeView(const Float3& position, const Quaternion& orientation)
{
    // View matrix is:
    //
    //  [ Lx  Uy  Dz  Tx  ]
    //  [ Lx  Uy  Dz  Ty  ]
    //  [ Lx  Uy  Dz  Tz  ]
    //  [ 0   0   0   1   ]
    //
    // Where T = -(Transposed(Rot) * Pos)

    // This is most efficiently done using 3x3 Matrices
    Matrix3 rot;
    orientation.ToRotationMatrix(rot);

    // Make the translation relative to new axes
    Matrix3 rotT = rot.Transpose();
    Float3 trans = (-rotT).Multiply(position);

    // Make final matrix
    *this = Matrix4(rotT);
    m[0][3] = trans.X;
    m[1][3] = trans.Y;
    m[2][3] = trans.Z;
}

DLLEXPORT void Matrix4::MakeProjectionOrtho(
    float left, float right, float top, float bottom, float near, float far)
{
    // Create a matrix that transforms coordinate to normalized device coordinate in range:
    // Left -1 - Right 1
    // Bottom -1 - Top 1
    // Near -1 - Far 1

    float deltaX = right - left;
    float deltaY = bottom - top;
    float deltaZ = far - near;

    m[0][0] = 2.0F / deltaX;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = -(right + left) / deltaX;

    m[1][0] = 0.0f;
    m[1][1] = -2.0F / deltaY;
    m[1][2] = 0.0f;
    m[1][3] = (top + bottom) / deltaY;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;

    if(far == 0.0f) {
        m[2][2] = 1.0f;
        m[2][3] = 0.0f;
    } else {
        m[2][2] = -2.0F / deltaZ;
        m[2][3] = -(far + near) / deltaZ;
    }

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}
// ------------------------------------ //
DLLEXPORT Matrix4 Matrix4::ProjectionPerspective(
    const Degree& horzFOV, float aspect, float near, float far, bool positiveZ)
{
    // Note: Duplicate code in Camera, bring it all here eventually
    static constexpr float INFINITE_FAR_PLANE_ADJUST = 0.00001f;

    Radian thetaX(horzFOV.ValueInRadians() * 0.5f);
    float tanThetaX = std::tan(thetaX.ValueInRadians());
    float tanThetaY = tanThetaX / aspect;

    float half_w = tanThetaX * near;
    float half_h = tanThetaY * near;

    float left = -half_w;
    float right = half_w;
    float bottom = -half_h;
    float top = half_h;

    float inv_w = 1 / (right - left);
    float inv_h = 1 / (top - bottom);
    float inv_d = 1 / (far - near);

    float A = 2 * near * inv_w;
    float B = 2 * near * inv_h;
    float C = (right + left) * inv_w;
    float D = (top + bottom) * inv_h;
    float q, qn;
    float sign = positiveZ ? 1.0f : -1.0f;

    if(far == 0) {
        // Infinite far plane
        q = INFINITE_FAR_PLANE_ADJUST - 1;
        qn = near * (INFINITE_FAR_PLANE_ADJUST - 2);
    } else {
        q = sign * (far + near) * inv_d;
        qn = -2.0f * (far * near) * inv_d;
    }

    Matrix4 mat;
    mat[0][0] = A;
    mat[0][1] = 0.0f;
    mat[0][2] = C;
    mat[0][3] = 0.0f;
    mat[1][0] = 0.0f;
    mat[1][1] = B;
    mat[1][2] = D;
    mat[1][3] = 0.0f;
    mat[2][0] = 0.0f;
    mat[2][1] = 0.0f;
    mat[2][2] = q;
    mat[2][3] = qn;
    mat[3][0] = 0.0f;
    mat[3][1] = 0.0f;
    mat[3][2] = sign;
    mat[3][3] = 0.0f;

    return mat;
}

DLLEXPORT Matrix4 Matrix4::ProjectionOrthographic(
    float left, float right, float top, float bottom, float near, float far)
{
    Matrix4 output;
    output.MakeProjectionOrtho(left, right, top, bottom, near, far);

    return output;
}

DLLEXPORT Matrix4 Matrix4::View(const Float3& position, const Quaternion& orientation)
{
    Matrix4 mat;
    mat.MakeView(position, orientation);

    return mat;
}

DLLEXPORT Matrix4 Matrix4::FromTRS(
    const Float3& translation, const Quaternion& rotation, const Float3& scale)
{
    Matrix4 mat;
    mat.SetTRS(translation, rotation, scale);

    return mat;
}

DLLEXPORT Matrix4 Matrix4::FromInverseTRS(
    const Float3& translation, const Quaternion& rotation, const Float3& scale)
{
    Matrix4 mat;
    mat.SetInverseTRS(translation, rotation, scale);

    return mat;
}
