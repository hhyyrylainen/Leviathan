// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Quaternion.h"
#include "Types.h"

namespace Leviathan {


// This file contains a lot of code from bs::framework with modifications, see License.txt for
// details. Original copyright notice:
//************************************ bs::framework - Copyright 2018 Marko Pintera
//**************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not
// to be removed. ***********//

/** Values that represent in which order are euler angles applied when used in transformations.
 */
enum class EulerAngleOrder { XYZ, XZY, YXZ, YZX, ZXY, ZYX };

Matrix3 operator*(float lhs, const Matrix3& rhs);

//! \brief A 3x3 rotation and scale matrix
struct Matrix3 {
    friend Matrix3 operator*(float lhs, const Matrix3& rhs);

    struct EulerAngleOrderData {
        int a, b, c;
        float sign;
    };

public:
    Matrix3() = default;
    constexpr Matrix3(const Matrix3&) = default;
    constexpr Matrix3& operator=(const Matrix3&) = default;

    constexpr Matrix3(float m00, float m01, float m02, float m10, float m11, float m12,
        float m20, float m21, float m22) :
        m{{m00, m01, m02}, {m10, m11, m12}, {m20, m21, m22}}
    {}

    /** Construct a matrix from a quaternion. */
    explicit Matrix3(const Quaternion& rotation)
    {
        FromQuaternion(rotation);
    }

    /** Construct a matrix that performs rotation and scale. */
    explicit Matrix3(const Quaternion& rotation, const Float3& scale)
    {
        FromQuaternion(rotation);

        for(int row = 0; row < 3; row++) {
            for(int col = 0; col < 3; col++)
                m[row][col] = scale[row] * m[row][col];
        }
    }

    /** Construct a matrix from an angle/axis pair. */
    explicit Matrix3(const Float3& axis, const Radian& angle)
    {
        FromAxisAngle(axis, angle);
    }

    /** Construct a matrix from 3 orthonormal local axes. */
    explicit Matrix3(const Float3& xaxis, const Float3& yaxis, const Float3& zaxis)
    {
        FromAxes(xaxis, yaxis, zaxis);
    }

    /**
     * Construct a matrix from euler angles, YXZ ordering.
     *
     * @see		Matrix3::fromEulerAngles
     */
    explicit Matrix3(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle)
    {
        FromEulerAngles(xAngle, yAngle, zAngle);
    }

    /**
     * Construct a matrix from euler angles, custom ordering.
     *
     * @see		Matrix3::fromEulerAngles
     */
    explicit Matrix3(const Radian& xAngle, const Radian& yAngle, const Radian& zAngle,
        EulerAngleOrder order)
    {
        FromEulerAngles(xAngle, yAngle, zAngle, order);
    }

    /** Swaps the contents of this matrix with another. */
    void swap(Matrix3& other)
    {
        std::swap(m[0][0], other.m[0][0]);
        std::swap(m[0][1], other.m[0][1]);
        std::swap(m[0][2], other.m[0][2]);
        std::swap(m[1][0], other.m[1][0]);
        std::swap(m[1][1], other.m[1][1]);
        std::swap(m[1][2], other.m[1][2]);
        std::swap(m[2][0], other.m[2][0]);
        std::swap(m[2][1], other.m[2][1]);
        std::swap(m[2][2], other.m[2][2]);
    }

    /** Returns a row of the matrix. */
    float* operator[](uint32_t row) const
    {
        // assert(row < 3);

        return (float*)m[row];
    }

    inline Float3 GetColumn(uint32_t col) const
    {
        LEVIATHAN_ASSERT(col < 3, "col out of range");

        return Float3(m[0][col], m[1][col], m[2][col]);
    }

    inline void SetColumn(uint32_t col, const Float3& vec)
    {
        LEVIATHAN_ASSERT(col < 3, "col out of range");

        m[0][col] = vec.X;
        m[1][col] = vec.Y;
        m[2][col] = vec.Z;
    }

    inline bool operator!=(const Matrix3& rhs) const
    {
        return !operator==(rhs);
    }

    inline bool operator==(const Matrix3& rhs) const
    {
        for(uint32_t row = 0; row < 3; row++) {
            for(uint32_t col = 0; col < 3; col++) {
                if(m[row][col] != rhs.m[row][col])
                    return false;
            }
        }

        return true;
    }

    inline Matrix3 operator+(const Matrix3& rhs) const
    {
        Matrix3 sum;
        for(uint32_t row = 0; row < 3; row++) {
            for(uint32_t col = 0; col < 3; col++) {
                sum.m[row][col] = m[row][col] + rhs.m[row][col];
            }
        }

        return sum;
    }

    inline Matrix3 operator-(const Matrix3& rhs) const
    {
        Matrix3 diff;
        for(uint32_t row = 0; row < 3; row++) {
            for(uint32_t col = 0; col < 3; col++) {
                diff.m[row][col] = m[row][col] - rhs.m[row][col];
            }
        }

        return diff;
    }

    inline Matrix3 operator*(const Matrix3& rhs) const
    {
        Matrix3 prod;
        for(uint32_t row = 0; row < 3; row++) {
            for(uint32_t col = 0; col < 3; col++) {
                prod.m[row][col] = m[row][0] * rhs.m[0][col] + m[row][1] * rhs.m[1][col] +
                                   m[row][2] * rhs.m[2][col];
            }
        }

        return prod;
    }

    inline Matrix3 operator-() const
    {
        Matrix3 neg;
        for(uint32_t row = 0; row < 3; row++) {
            for(uint32_t col = 0; col < 3; col++)
                neg[row][col] = -m[row][col];
        }

        return neg;
    }

    inline Matrix3 operator*(float rhs) const
    {
        Matrix3 prod;
        for(uint32_t row = 0; row < 3; row++) {
            for(uint32_t col = 0; col < 3; col++)
                prod[row][col] = rhs * m[row][col];
        }

        return prod;
    }

    /** Transforms the given vector by this matrix and returns the newly transformed vector. */
    DLLEXPORT Float3 Multiply(const Float3& vec) const;

    /** Returns a transpose of the matrix (switched columns and rows). */
    DLLEXPORT Matrix3 Transpose() const;

    /**
     * Calculates an inverse of the matrix if it exists.
     *
     * @param[out]	mat			Resulting matrix inverse.
     * @param[in]	fTolerance 	(optional) Tolerance to use when checking if determinant is
     * zero (or near zero in this case). Zero determinant means inverse doesn't exist.
     * @return					True if inverse exists, false otherwise.
     */
    DLLEXPORT bool Inverse(Matrix3& mat, float fTolerance = 1e-06f) const;

    /**
     * Calculates an inverse of the matrix if it exists.
     *
     * @param[in]	fTolerance 	(optional) Tolerance to use when checking if determinant is
     * zero (or near zero in this case). Zero determinant means inverse doesn't exist.
     *
     * @return					Resulting matrix inverse if it exists, otherwise a zero matrix.
     */
    DLLEXPORT Matrix3 Inverse(float fTolerance = 1e-06f) const;

    /** Calculates the matrix determinant. */
    DLLEXPORT float Determinant() const;

    /**
     * Decompose a Matrix3 to rotation and scale.
     *
     * @note
     * Matrix must consist only of rotation and uniform scale transformations, otherwise
     * accurate results are not guaranteed. Applying non-uniform scale guarantees rotation
     * portion will not be accurate.
     */
    DLLEXPORT void Decomposition(Quaternion& rotation, Float3& scale) const;

    /**
     * Decomposes the matrix into various useful values.
     *
     * @param[out]	matL	Unitary matrix. Columns form orthonormal bases. If your matrix is
     * affine and doesn't use non-uniform scaling this matrix will be a conjugate transpose of
     * the rotation part of the matrix.
     * @param[out]	matS	Singular values of the matrix. If your matrix is affine these will
     * be scaling factors of the matrix.
     * @param[out]	matR	Unitary matrix. Columns form orthonormal bases. If your matrix is
     * affine and doesn't use non-uniform scaling this matrix will be the rotation part of the
     * matrix.
     */
    DLLEXPORT void SingularValueDecomposition(
        Matrix3& matL, Float3& matS, Matrix3& matR) const;

    /**
     * Decomposes the matrix into a set of values.
     *
     * @param[out]	matQ	Columns form orthonormal bases. If your matrix is affine and
     * 						doesn't use non-uniform scaling this matrix will be the rotation
     * part of the matrix.
     * @param[out]	vecD	If the matrix is affine these will be scaling factors of the
     * matrix.
     * @param[out]	vecU	If the matrix is affine these will be shear factors of the matrix.
     */
    DLLEXPORT void QDUDecomposition(Matrix3& matQ, Float3& vecD, Float3& vecU) const;

    /** Gram-Schmidt orthonormalization (applied to columns of rotation matrix) */
    DLLEXPORT void Orthonormalize();

    /**
     * Converts an orthonormal matrix to axis angle representation.
     *
     * @note	Matrix must be orthonormal.
     */
    DLLEXPORT void ToAxisAngle(Float3& axis, Radian& angle) const;

    /** Creates a rotation matrix from an axis angle representation. */
    DLLEXPORT void FromAxisAngle(const Float3& axis, const Radian& angle);

    /**
     * Converts an orthonormal matrix to quaternion representation.
     *
     * @note	Matrix must be orthonormal.
     */
    DLLEXPORT void ToQuaternion(Quaternion& quat) const;

    /** Creates a rotation matrix from a quaternion representation. */
    DLLEXPORT void FromQuaternion(const Quaternion& quat);

    /** Creates a matrix from a three axes. */
    DLLEXPORT void FromAxes(const Float3& xAxis, const Float3& yAxis, const Float3& zAxis);

    /**
     * Converts an orthonormal matrix to euler angle (pitch/yaw/roll) representation.
     *
     * @param[in,out]	xAngle	Rotation about x axis. (AKA Pitch)
     * @param[in,out]	yAngle  Rotation about y axis. (AKA Yaw)
     * @param[in,out]	zAngle 	Rotation about z axis. (AKA Roll)
     * @return					True if unique solution was found, false otherwise.
     *
     * @note	Matrix must be orthonormal.
     */
    DLLEXPORT bool ToEulerAngles(Radian& xAngle, Radian& yAngle, Radian& zAngle) const;

    /**
     * Creates a rotation matrix from the provided Pitch/Yaw/Roll angles.
     *
     * @param[in]	xAngle	Rotation about x axis. (AKA Pitch)
     * @param[in]	yAngle	Rotation about y axis. (AKA Yaw)
     * @param[in]	zAngle	Rotation about z axis. (AKA Roll)
     *
     * @note	Matrix must be orthonormal.
     * 			Since different values will be produced depending in which order are the
     * rotations applied, this method assumes they are applied in YXZ order. If you need a
     * specific order, use the overloaded "fromEulerAngles" method instead.
     */
    DLLEXPORT void FromEulerAngles(
        const Radian& xAngle, const Radian& yAngle, const Radian& zAngle);

    /**
     * Creates a rotation matrix from the provided Pitch/Yaw/Roll angles.
     *
     * @param[in]	xAngle	Rotation about x axis. (AKA Pitch)
     * @param[in]	yAngle	Rotation about y axis. (AKA Yaw)
     * @param[in]	zAngle	Rotation about z axis. (AKA Roll)
     * @param[in]	order 	The order in which rotations will be applied.
     *						Different rotations can be created depending on the order.
     *
     * @note	Matrix must be orthonormal.
     */
    DLLEXPORT void FromEulerAngles(const Radian& xAngle, const Radian& yAngle,
        const Radian& zAngle, EulerAngleOrder order);

    /**
     * Eigensolver, matrix must be symmetric.
     *
     * @note
     * Eigenvectors are vectors which when transformed by the matrix, only change in magnitude,
     * but not in direction. Eigenvalue is that magnitude. In other words you will get the same
     * result whether you multiply the vector by the matrix or by its eigenvalue.
     */
    DLLEXPORT void EigenSolveSymmetric(float eigenValues[3], Float3 eigenVectors[3]) const;

    VALUE_TYPE(Matrix3);

protected:
    friend struct Matrix4;

    // Support for eigensolver
    void tridiagonal(float diag[3], float subDiag[3]);
    bool QLAlgorithm(float diag[3], float subDiag[3]);

    // Support for singular value decomposition
    static constexpr const float SVD_EPSILON = 1e-04f;

    static constexpr const unsigned int SVD_MAX_ITERS = 32;
    static void bidiagonalize(Matrix3& matA, Matrix3& matL, Matrix3& matR);
    static void golubKahanStep(Matrix3& matA, Matrix3& matL, Matrix3& matR);

public:
    float m[3][3];


    // static constexpr const float EPSILON = 1e-06f;
    DLLEXPORT static const Matrix3 IDENTITY;
    DLLEXPORT static const Matrix3 ZERO;
};

//! \brief A 4x4 matrix type
struct Matrix4 {
public:
    inline Matrix4() = default;
    constexpr inline Matrix4(const Matrix4& other) = default;

    inline Matrix4(const Float3& translation, const Quaternion& rotation, const Float3& scale)
    {
        SetTRS(translation, rotation, scale);
    }

    constexpr Matrix4(float m00, float m01, float m02, float m03, float m10, float m11,
        float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31,
        float m32, float m33) :
        m{{m00, m01, m02, m03}, {m10, m11, m12, m13}, {m20, m21, m22, m23},
            {m30, m31, m32, m33}}
    {}

    /** Creates a 4x4 transformation matrix with a zero translation part from a
     * rotation/scaling 3x3 matrix. */
    constexpr explicit Matrix4(const Matrix3& mat3) :
        m{{mat3.m[0][0], mat3.m[0][1], mat3.m[0][2], 0.0f},
            {mat3.m[1][0], mat3.m[1][1], mat3.m[1][2], 0.0f},
            {mat3.m[2][0], mat3.m[2][1], mat3.m[2][2], 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}}
    {}

    /** Swaps the contents of this matrix with another. */
    void swap(Matrix4& other)
    {
        std::swap(m[0][0], other.m[0][0]);
        std::swap(m[0][1], other.m[0][1]);
        std::swap(m[0][2], other.m[0][2]);
        std::swap(m[0][3], other.m[0][3]);
        std::swap(m[1][0], other.m[1][0]);
        std::swap(m[1][1], other.m[1][1]);
        std::swap(m[1][2], other.m[1][2]);
        std::swap(m[1][3], other.m[1][3]);
        std::swap(m[2][0], other.m[2][0]);
        std::swap(m[2][1], other.m[2][1]);
        std::swap(m[2][2], other.m[2][2]);
        std::swap(m[2][3], other.m[2][3]);
        std::swap(m[3][0], other.m[3][0]);
        std::swap(m[3][1], other.m[3][1]);
        std::swap(m[3][2], other.m[3][2]);
        std::swap(m[3][3], other.m[3][3]);
    }

    constexpr Matrix4& operator=(const Matrix4& other)
    {
        m[0][0] = other.m[0][0];
        m[0][1] = other.m[0][1];
        m[0][2] = other.m[0][2];
        m[0][3] = other.m[0][3];
        m[1][0] = other.m[1][0];
        m[1][1] = other.m[1][1];
        m[1][2] = other.m[1][2];
        m[1][3] = other.m[1][3];
        m[2][0] = other.m[2][0];
        m[2][1] = other.m[2][1];
        m[2][2] = other.m[2][2];
        m[2][3] = other.m[2][3];
        m[3][0] = other.m[3][0];
        m[3][1] = other.m[3][1];
        m[3][2] = other.m[3][2];
        m[3][3] = other.m[3][3];
        return *this;
    }

    /** Returns a row of the matrix. */
    Float4& operator[](uint32_t row)
    {
        assert(row < 4);

        return *reinterpret_cast<Float4*>(m[row]);
    }

    /** Returns a row of the matrix. */
    const Float4& operator[](uint32_t row) const
    {
        assert(row < 4);

        return *reinterpret_cast<const Float4*>(m[row]);
    }

    Matrix4 operator*(const Matrix4& rhs) const
    {
        Matrix4 r;

        r.m[0][0] = m[0][0] * rhs.m[0][0] + m[0][1] * rhs.m[1][0] + m[0][2] * rhs.m[2][0] +
                    m[0][3] * rhs.m[3][0];
        r.m[0][1] = m[0][0] * rhs.m[0][1] + m[0][1] * rhs.m[1][1] + m[0][2] * rhs.m[2][1] +
                    m[0][3] * rhs.m[3][1];
        r.m[0][2] = m[0][0] * rhs.m[0][2] + m[0][1] * rhs.m[1][2] + m[0][2] * rhs.m[2][2] +
                    m[0][3] * rhs.m[3][2];
        r.m[0][3] = m[0][0] * rhs.m[0][3] + m[0][1] * rhs.m[1][3] + m[0][2] * rhs.m[2][3] +
                    m[0][3] * rhs.m[3][3];

        r.m[1][0] = m[1][0] * rhs.m[0][0] + m[1][1] * rhs.m[1][0] + m[1][2] * rhs.m[2][0] +
                    m[1][3] * rhs.m[3][0];
        r.m[1][1] = m[1][0] * rhs.m[0][1] + m[1][1] * rhs.m[1][1] + m[1][2] * rhs.m[2][1] +
                    m[1][3] * rhs.m[3][1];
        r.m[1][2] = m[1][0] * rhs.m[0][2] + m[1][1] * rhs.m[1][2] + m[1][2] * rhs.m[2][2] +
                    m[1][3] * rhs.m[3][2];
        r.m[1][3] = m[1][0] * rhs.m[0][3] + m[1][1] * rhs.m[1][3] + m[1][2] * rhs.m[2][3] +
                    m[1][3] * rhs.m[3][3];

        r.m[2][0] = m[2][0] * rhs.m[0][0] + m[2][1] * rhs.m[1][0] + m[2][2] * rhs.m[2][0] +
                    m[2][3] * rhs.m[3][0];
        r.m[2][1] = m[2][0] * rhs.m[0][1] + m[2][1] * rhs.m[1][1] + m[2][2] * rhs.m[2][1] +
                    m[2][3] * rhs.m[3][1];
        r.m[2][2] = m[2][0] * rhs.m[0][2] + m[2][1] * rhs.m[1][2] + m[2][2] * rhs.m[2][2] +
                    m[2][3] * rhs.m[3][2];
        r.m[2][3] = m[2][0] * rhs.m[0][3] + m[2][1] * rhs.m[1][3] + m[2][2] * rhs.m[2][3] +
                    m[2][3] * rhs.m[3][3];

        r.m[3][0] = m[3][0] * rhs.m[0][0] + m[3][1] * rhs.m[1][0] + m[3][2] * rhs.m[2][0] +
                    m[3][3] * rhs.m[3][0];
        r.m[3][1] = m[3][0] * rhs.m[0][1] + m[3][1] * rhs.m[1][1] + m[3][2] * rhs.m[2][1] +
                    m[3][3] * rhs.m[3][1];
        r.m[3][2] = m[3][0] * rhs.m[0][2] + m[3][1] * rhs.m[1][2] + m[3][2] * rhs.m[2][2] +
                    m[3][3] * rhs.m[3][2];
        r.m[3][3] = m[3][0] * rhs.m[0][3] + m[3][1] * rhs.m[1][3] + m[3][2] * rhs.m[2][3] +
                    m[3][3] * rhs.m[3][3];

        return r;
    }

    Matrix4 operator+(const Matrix4& rhs) const
    {
        Matrix4 r;

        r.m[0][0] = m[0][0] + rhs.m[0][0];
        r.m[0][1] = m[0][1] + rhs.m[0][1];
        r.m[0][2] = m[0][2] + rhs.m[0][2];
        r.m[0][3] = m[0][3] + rhs.m[0][3];

        r.m[1][0] = m[1][0] + rhs.m[1][0];
        r.m[1][1] = m[1][1] + rhs.m[1][1];
        r.m[1][2] = m[1][2] + rhs.m[1][2];
        r.m[1][3] = m[1][3] + rhs.m[1][3];

        r.m[2][0] = m[2][0] + rhs.m[2][0];
        r.m[2][1] = m[2][1] + rhs.m[2][1];
        r.m[2][2] = m[2][2] + rhs.m[2][2];
        r.m[2][3] = m[2][3] + rhs.m[2][3];

        r.m[3][0] = m[3][0] + rhs.m[3][0];
        r.m[3][1] = m[3][1] + rhs.m[3][1];
        r.m[3][2] = m[3][2] + rhs.m[3][2];
        r.m[3][3] = m[3][3] + rhs.m[3][3];

        return r;
    }

    Matrix4 operator-(const Matrix4& rhs) const
    {
        Matrix4 r;
        r.m[0][0] = m[0][0] - rhs.m[0][0];
        r.m[0][1] = m[0][1] - rhs.m[0][1];
        r.m[0][2] = m[0][2] - rhs.m[0][2];
        r.m[0][3] = m[0][3] - rhs.m[0][3];

        r.m[1][0] = m[1][0] - rhs.m[1][0];
        r.m[1][1] = m[1][1] - rhs.m[1][1];
        r.m[1][2] = m[1][2] - rhs.m[1][2];
        r.m[1][3] = m[1][3] - rhs.m[1][3];

        r.m[2][0] = m[2][0] - rhs.m[2][0];
        r.m[2][1] = m[2][1] - rhs.m[2][1];
        r.m[2][2] = m[2][2] - rhs.m[2][2];
        r.m[2][3] = m[2][3] - rhs.m[2][3];

        r.m[3][0] = m[3][0] - rhs.m[3][0];
        r.m[3][1] = m[3][1] - rhs.m[3][1];
        r.m[3][2] = m[3][2] - rhs.m[3][2];
        r.m[3][3] = m[3][3] - rhs.m[3][3];

        return r;
    }

    inline bool operator==(const Matrix4& rhs) const
    {
        if(m[0][0] != rhs.m[0][0] || m[0][1] != rhs.m[0][1] || m[0][2] != rhs.m[0][2] ||
            m[0][3] != rhs.m[0][3] || m[1][0] != rhs.m[1][0] || m[1][1] != rhs.m[1][1] ||
            m[1][2] != rhs.m[1][2] || m[1][3] != rhs.m[1][3] || m[2][0] != rhs.m[2][0] ||
            m[2][1] != rhs.m[2][1] || m[2][2] != rhs.m[2][2] || m[2][3] != rhs.m[2][3] ||
            m[3][0] != rhs.m[3][0] || m[3][1] != rhs.m[3][1] || m[3][2] != rhs.m[3][2] ||
            m[3][3] != rhs.m[3][3]) {
            return false;
        }

        return true;
    }

    inline bool operator!=(const Matrix4& rhs) const
    {
        return !operator==(rhs);
    }

    Matrix4 operator*(float rhs) const
    {
        return Matrix4(rhs * m[0][0], rhs * m[0][1], rhs * m[0][2], rhs * m[0][3],
            rhs * m[1][0], rhs * m[1][1], rhs * m[1][2], rhs * m[1][3], rhs * m[2][0],
            rhs * m[2][1], rhs * m[2][2], rhs * m[2][3], rhs * m[3][0], rhs * m[3][1],
            rhs * m[3][2], rhs * m[3][3]);
    }

    /** Returns a transpose of the matrix (switched columns and rows). */
    Matrix4 Transpose() const
    {
        return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3]);
    }

    /** Returns the specified column of the matrix, ignoring the last row. */
    Float3 GetColumn(uint32_t col) const
    {
        assert(col < 4);

        return Float3(m[0][col], m[1][col], m[2][col]);
    }

    /** Returns the specified column of the matrix. */
    Float4 GetColumn4D(uint32_t col) const
    {
        assert(col < 4);

        return Float4(m[0][col], m[1][col], m[2][col], m[3][col]);
    }

    /** Assigns the vector to a column of the matrix. */
    void SetColumn(uint32_t idx, const Float4& column)
    {
        m[0][idx] = column.X;
        m[1][idx] = column.Y;
        m[2][idx] = column.Z;
        m[3][idx] = column.W;
    }

    /** Assigns the vector to a row of the matrix. */
    void SetRow(uint32_t idx, const Float4& column)
    {
        m[idx][0] = column.X;
        m[idx][1] = column.Y;
        m[idx][2] = column.Z;
        m[idx][3] = column.W;
    }

    /** Returns the rotation/scaling part of the matrix as a 3x3 matrix. */
    Matrix3 get3x3() const
    {
        Matrix3 m3x3;
        m3x3.m[0][0] = m[0][0];
        m3x3.m[0][1] = m[0][1];
        m3x3.m[0][2] = m[0][2];
        m3x3.m[1][0] = m[1][0];
        m3x3.m[1][1] = m[1][1];
        m3x3.m[1][2] = m[1][2];
        m3x3.m[2][0] = m[2][0];
        m3x3.m[2][1] = m[2][1];
        m3x3.m[2][2] = m[2][2];

        return m3x3;
    }

    /** Calculates the adjoint of the matrix. */
    DLLEXPORT Matrix4 Adjoint() const;

    /** Calculates the determinant of the matrix. */
    DLLEXPORT float Determinant() const;

    /** Calculates the determinant of the 3x3 sub-matrix. */
    DLLEXPORT float Determinant3x3() const;

    /** Calculates the inverse of the matrix. */
    DLLEXPORT Matrix4 Inverse() const;

    /**
     * Creates a matrix from translation, rotation and scale.
     *
     * @note	The transformation are applied in scale->rotation->translation order.
     */
    DLLEXPORT void SetTRS(
        const Float3& translation, const Quaternion& rotation, const Float3& scale);

    /**
     * Creates a matrix from inverse translation, rotation and scale.
     *
     * @note	This is cheaper than setTRS() and then performing inverse().
     */
    DLLEXPORT void SetInverseTRS(
        const Float3& translation, const Quaternion& rotation, const Float3& scale);

    /**
     * Decompose a Matrix4 to translation, rotation and scale.
     *
     * @note
     * This method is unable to decompose all types of matrices, in particular these are the
     * limitations:
     *  - Only translation, rotation and scale transforms are supported
     *  - Plain TRS matrices (that aren't composed with other matrices) can always be
     * decomposed
     *  - Composed TRS matrices can be decomposed ONLY if the scaling factor is uniform
     */
    DLLEXPORT void Decomposition(Float3& position, Quaternion& rotation, Float3& scale) const;

    /** Extracts the translation (position) part of the matrix. */
    inline Float3 GetTranslation() const
    {
        return Float3(m[0][3], m[1][3], m[2][3]);
    }

    /**
     * Check whether or not the matrix is affine matrix.
     *
     * @note	An affine matrix is a 4x4 matrix with row 3 equal to (0, 0, 0, 1), meaning no
     * projective coefficients.
     */
    inline bool IsAffine() const
    {
        return m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0 && m[3][3] == 1;
    }

    /**
     * Returns the inverse of the affine matrix.
     *
     * @note	Matrix must be affine.
     */
    DLLEXPORT Matrix4 InverseAffine() const;

    /**
     * Concatenate two affine matrices.
     *
     * @note	Both matrices must be affine.
     */
    Matrix4 ConcatenateAffine(const Matrix4& other) const
    {
        return Matrix4(
            m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0],
            m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1],
            m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2],
            m[0][0] * other.m[0][3] + m[0][1] * other.m[1][3] + m[0][2] * other.m[2][3] +
                m[0][3],

            m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0],
            m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1],
            m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2],
            m[1][0] * other.m[0][3] + m[1][1] * other.m[1][3] + m[1][2] * other.m[2][3] +
                m[1][3],

            m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0],
            m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1],
            m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2],
            m[2][0] * other.m[0][3] + m[2][1] * other.m[1][3] + m[2][2] * other.m[2][3] +
                m[2][3],

            0, 0, 0, 1);
    }

    // /**
    //  * Transform a plane by this matrix.
    //  *
    //  * @note	Matrix must be affine.
    //  */
    // Plane multiplyAffine(const Plane& p) const
    // {
    //     Float4 localNormal(p.normal.x, p.normal.y, p.normal.z, 0.0f);
    //     Float4 localPoint = localNormal * p.d;
    //     localPoint.w = 1.0f;

    //     Matrix4 itMat = inverse().transpose();
    //     Float4 worldNormal = itMat.multiplyAffine(localNormal);
    //     Float4 worldPoint = multiplyAffine(localPoint);

    //     float d = worldNormal.dot(worldPoint);

    //     return Plane(worldNormal.x, worldNormal.y, worldNormal.z, d);
    // }

    /**
     * Transform a 3D point by this matrix.
     *
     * @note	Matrix must be affine, if it is not use multiply() method.
     */
    inline Float3 MultiplyAffine(const Float3& v) const
    {
        return Float3(m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3],
            m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3],
            m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3]);
    }

    /**
     * Transform a 4D vector by this matrix.
     *
     * @note	Matrix must be affine, if it is not use multiply() method.
     */
    inline Float4 MultiplyAffine(const Float4& v) const
    {
        return Float4(m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3] * v.W,
            m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3] * v.W,
            m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3] * v.W, v.W);
    }

    /** Transform a 3D direction by this matrix. */
    inline Float3 MultiplyDirection(const Float3& v) const
    {
        return Float3(m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z,
            m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z,
            m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z);
    }

    /**
     * Transform a 3D point by this matrix.
     *
     * @note
     * w component of the vector is assumed to be 1. After transformation all components
     * are projected back so that w remains 1.
     * @note
     * If your matrix doesn't contain projection components use multiplyAffine() method as it
     * is faster.
     */
    inline Float3 Multiply(const Float3& v) const
    {
        Float3 r;

        float fInvW = 1.0f / (m[3][0] * v.X + m[3][1] * v.Y + m[3][2] * v.Z + m[3][3]);

        r.X = (m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3]) * fInvW;
        r.Y = (m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3]) * fInvW;
        r.Z = (m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3]) * fInvW;

        return r;
    }

    /**
     * Transform a 4D vector by this matrix.
     *
     * @note	If your matrix doesn't contain projection components use multiplyAffine()
     * method as it is faster.
     */
    inline Float4 Multiply(const Float4& v) const
    {
        return Float4(m[0][0] * v.X + m[0][1] * v.Y + m[0][2] * v.Z + m[0][3] * v.W,
            m[1][0] * v.X + m[1][1] * v.Y + m[1][2] * v.Z + m[1][3] * v.W,
            m[2][0] * v.X + m[2][1] * v.Y + m[2][2] * v.Z + m[2][3] * v.W,
            m[3][0] * v.X + m[3][1] * v.Y + m[3][2] * v.Z + m[3][3] * v.W);
    }

    /** Creates a view matrix and applies optional reflection. */
    DLLEXPORT void MakeView(const Float3& position, const Quaternion& orientation);

    /**
     * Creates an ortographic projection matrix that scales the part of the view bounded by @p
     * left, @p right,
     * @p top and @p bottom into [-1, 1] range. If @p far is non-zero the matrix will also
     * transform the depth into
     * [-1, 1] range, otherwise it will leave it as-is.
     */
    DLLEXPORT void MakeProjectionOrtho(
        float left, float right, float top, float bottom, float near, float far);

    /**
     * Creates a 4x4 perspective projection matrix.
     *
     * @param[in]	horzFOV		Horizontal field of view.
     * @param[in]	aspect		Aspect ratio. Determines the vertical field of view.
     * @param[in]	near		Distance to the near plane.
     * @param[in]	far			Distance to the far plane.
     * @param[in]	positiveZ	If true the matrix will project geometry as if its looking
     *along the positive Z axis. Otherwise it projects along the negative Z axis (default).
     */
    DLLEXPORT static Matrix4 ProjectionPerspective(
        const Degree& horzFOV, float aspect, float near, float far, bool isopengl);

    /** @copydoc makeProjectionOrtho() */
    DLLEXPORT static Matrix4 ProjectionOrthographic(
        float left, float right, float top, float bottom, float near, float far);

    /** Creates a view matrix. */
    DLLEXPORT static Matrix4 View(const Float3& position, const Quaternion& orientation);

    /**
     * Creates a matrix from translation, rotation and scale.
     *
     * @note	The transformation are applied in scale->rotation->translation order.
     */
    DLLEXPORT static Matrix4 FromTRS(
        const Float3& translation, const Quaternion& rotation, const Float3& scale);

    /**
     * Creates a matrix from inverse translation, rotation and scale.
     *
     * @note	This is cheaper than setTRS() and then performing inverse().
     */
    DLLEXPORT static Matrix4 FromInverseTRS(
        const Float3& translation, const Quaternion& rotation, const Float3& scale);


    VALUE_TYPE(Matrix4);

public:
    union {
        struct {
            float m00;
            float m01;
            float m02;
            float m03;
            float m10;
            float m11;
            float m12;
            float m13;
            float m20;
            float m21;
            float m22;
            float m23;
            float m30;
            float m31;
            float m32;
            float m33;
        };
        float m[4][4];
    };

    DLLEXPORT static const Matrix4 IDENTITY;
    DLLEXPORT static const Matrix4 ZERO;
};

// Stream operators //
DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Leviathan::Matrix3& value);
DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Leviathan::Matrix4& value);

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Matrix3;
using Leviathan::Matrix4;
#endif
