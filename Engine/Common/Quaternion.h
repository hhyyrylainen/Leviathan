// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Types.h"

namespace Leviathan {

//! \brief A Quaternion type to make quaternion specific operations easier to access
//! \todo There are a ton of places that still use Float4 instead of Quaternion
struct Quaternion : public Float4 {
public:
    constexpr inline Quaternion() noexcept : Float4() {}
    constexpr inline Quaternion(const Quaternion& other) noexcept : Float4(other) {}
    constexpr inline Quaternion(const Float4& copy) noexcept : Float4(copy) {}

    constexpr inline Quaternion(float f1, float f2, float f3, float f4) noexcept :
        Float4(f1, f2, f3, f4)
    {}

    inline Quaternion(const bs::Quaternion& quat)
    {
        X = quat.x;
        Y = quat.y;
        Z = quat.z;
        W = quat.w;
    }

    //! \brief Makes a quaternion from axis angle
    //! \note axis must be normalized
    //!
    //! This resource is a life saver:
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/index.htm
    constexpr inline Quaternion(const Float3& axis, Radian angle) noexcept :
        Float4(axis * static_cast<float>(
                          std::sin(angle.ValueInRadians() / 2.0f) /* this is 's' */),
            static_cast<float>(std::cos(angle.ValueInRadians() / 2.0f) /* this is 'w' */))
    {}

    //! \brief Makes a quaternion from euler angles
    // MSVC can't deal with std::cos (or other math functions) in constexpr functions
    /*constexpr*/ inline Quaternion(const Float3& angles)
    {
        // multiplied by 0.5 to get double the value //
        const float cosx = std::cos(0.5f * angles.X);
        const float cosy = std::cos(0.5f * angles.Y);
        const float cosz = std::cos(0.5f * angles.Z);

        const float sinx = std::sin(0.5f * angles.X);
        const float siny = std::sin(0.5f * angles.Y);
        const float sinz = std::sin(0.5f * angles.Z);

        // compute quaternion //
        // X
        X = cosz * cosy * sinx - sinz * siny * cosx;
        // Y
        Y = cosz * siny * cosx + sinz * cosy * sinx;
        // Z
        Z = sinz * cosy * cosx - cosz * siny * sinx;
        // W
        W = cosz * cosy * cosx * sinz * siny * sinx;
    }


    // ------------------------------------ //
    // Operators disabled from Float4
    inline Float4 operator*(const Float4& other) const noexcept = delete;
    inline Float4& operator*=(const Float4& other) noexcept = delete;

    // ------------------------------------ //
    // general operators
    Quaternion& operator=(const Quaternion& other) noexcept
    {
        X = other.X;
        Y = other.Y;
        Z = other.Z;
        W = other.W;
        return *this;
    }

    constexpr bool operator==(const Quaternion& other) const noexcept
    {
        return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
    }

    // add elements //
    DLLEXPORT inline Quaternion operator+(const Quaternion& other) const noexcept
    {
        return Quaternion(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
    }

    DLLEXPORT inline Quaternion& operator+=(const Quaternion& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
        W += other.W;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT inline Quaternion operator-(const Quaternion& other) const noexcept
    {
        return Quaternion(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
    }

    DLLEXPORT inline Quaternion& operator-=(const Quaternion& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;
        W -= other.W;
        return *this;
    }

    // negates all elements //
    DLLEXPORT inline Quaternion operator-() const noexcept
    {
        return Quaternion(-X, -Y, -Z, -W);
    }

    // returns the vector //
    DLLEXPORT inline Quaternion operator+() const noexcept
    {
        return Quaternion(*this);
    }


    // multiply  by scalar f //
    DLLEXPORT inline Quaternion operator*(float val) const noexcept
    {
        return Quaternion(X * val, Y * val, Z * val, W * val);
    }

    DLLEXPORT inline Quaternion& operator*=(float val) noexcept
    {
        X *= val;
        Y *= val;
        Z *= val;
        W *= val;
        return *this;
    }

    // ------------------------------------ //
    // Quaternion specific operations

    //! \returns A rotated quaternion
    constexpr inline Quaternion operator*(const Quaternion& other) const noexcept
    {
        return Quaternion(X * other.X + X * other.W + Y * other.Z - Z * other.Y,
            W * other.Y - X * other.Z + Y * other.W + Z * other.X,
            W * other.Z + X * other.Y - Y * other.X + Z * other.W,
            W * other.W - X * other.X - Y * other.Y - Z * other.Z);
    }

    //! Rotates a vector by this quaternion
    constexpr inline Float3 operator*(const Float3& vector) const noexcept
    {
        // // Alternative from
        // //
        // https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
        // const auto u = Float3(X, Y, Z);

        // // Complex math going on
        // return u * 2.0f * u.Dot(vector)
        //     + vector * (W*W - u.Dot(u))
        //     + u.Cross(vector) * 2.0f * W;

        // Math taken from Ogre::Quaternion
        Float3 qvec(X, Y, Z);
        const Float3 uv1 = qvec.Cross(vector);
        const Float3 uuv = qvec.Cross(uv1) * 2.0f;
        const Float3 uv2 = uv1 * 2.0f * W;

        return vector + uv2 + uuv;
    }

    //! Converts a quaternion to Axis (and skips outputting the angle)
    //! \note Must be normalized
    inline Float3 ToAxis() const
    {
        const auto s = std::sqrt(1 - std::pow(W, 2));
        // Avoid division by zero (this small axis it can be basically converted directly)
        if(s > 0) {
            return Float3(X / s, Y / s, Z / s);
        } else {
            return Float3(X, Y, Z);
        }
    }

    //! Converts a quaternion to angle (and skips outputting the Axis)
    //! \note Must be normalized
    DLLEXPORT inline float ToAngle() const noexcept
    {
        return 2 * std::acos(W);
    }

    //! Inverts a quaternion
    inline Quaternion Inverse() const noexcept
    {
        const auto length = Length();
        if(length > 0.0f) {
            const auto inverted = 1.0f / length;
            return Quaternion(-X * inverted, -Y * inverted, -Z * inverted, W * inverted);
        } else {
            // Invalid inversing
            return Quaternion(0.f, 0.f, 0.f, 0.f);
        }
    }

    //! Reverse quaternion. You probably actually want to use Inverse
    inline Quaternion Reverse() const noexcept
    {
        return Quaternion(-X, -Y, -Z, W);
    }

    DLLEXPORT Float3 XAxis() const;

    //! \brief Computes the y-axis of the quaternion
    DLLEXPORT Float3 YAxis() const;

    DLLEXPORT Float3 ZAxis() const;

    // ------------------------------------ //
    // Quaternion methods

    //! does SPHERICAL interpolation between quaternions
    inline Quaternion Slerp(const Quaternion& other, float f) const
    {
        // extra quaternion for calculations //
        Quaternion quaternion3;

        // dot product of both //
        float dot = this->Dot(other);

        if(dot < 0) {
            dot = -dot;
            quaternion3 = -other;
        } else {
            quaternion3 = other;
        }

        if(dot < 0.95f) {
            const float angle = std::acos(dot);
            return ((*this) * std::sin(angle * (1 - f)) + quaternion3 * std::sin(angle * f)) /
                   sinf(angle);

        } else {
            // small angle, linear interpolation will be fine //
            return this->Lerp(quaternion3, f);
        }
    }

    //! \note This quaternion has to be normalized
    //! \see
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
    DLLEXPORT inline Float3 ToEuler() const noexcept
    {
        const float test = X * Y + Z * W;

        if(test > 0.499) {
            // Singularity at north pole
            return Float3(2 * std::atan2(X, W), PI / 2, 0);
        }

        if(test < -0.499) {
            // Singularity at south pole
            return Float3(-2 * std::atan2(X, W), -PI / 2, 0);
        }

        const float sqx = X * X;
        const float sqy = Y * Y;
        const float sqz = Z * Z;

        return Float3(std::atan2(2 * Y * W - 2 * X * Z, 1 - 2 * sqy - 2 * sqz),
            std::asin(2 * test), std::atan2(2 * X * W - 2 * Y * Z, 1 - 2 * sqx - 2 * sqz));
    }

    // Math from here: https://stackoverflow.com/questions/12435671/quaternion-lookat-function
    static inline Quaternion LookAt(const Float3& sourcepoint, const Float3& target)
    {
        const auto forward = (target - sourcepoint).NormalizeSafe();
        const float dot = Float3::UnitVForward.Dot(forward);

        if(std::abs(dot - (-1.0f)) < 0.000001f) {
            // Assumes up is Float3(0, 1, 0)
            return Float4(Float3::UnitVUp, 3.1415926535897932f);
        }

        if(std::abs(dot - 1.0f) < 0.000001f) {
            return Quaternion::IDENTITY;
        }

        const auto rotAngle = Radian(std::acos(dot));
        const Float3 rotAxis = Float3::UnitVForward.Cross(forward).Normalize();
        return Quaternion(rotAxis, rotAngle);
    }

    VALUE_TYPE(Quaternion);

    static const Quaternion IDENTITY;
};

constexpr inline const Quaternion Quaternion::IDENTITY{0.f, 0.f, 0.f, 1.f};


constexpr inline Float3 operator*(const Float3& lhs, const Quaternion& rhs) noexcept
{
    return rhs * lhs;
}


} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Quaternion;
#endif
