#pragma once

#include "Define.h"
#include "Float2.h"
#include "Int3.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float3 {
public:
    DLLEXPORT constexpr Float3() noexcept = default;

    DLLEXPORT constexpr Float3(float x, float y, float z);
    DLLEXPORT constexpr Float3(Float2 floats, float z);
    DLLEXPORT constexpr explicit Float3(float data);
    DLLEXPORT constexpr Float3(const Int3& values);

    DLLEXPORT inline bool HasInvalidValues() const noexcept;
    DLLEXPORT inline void CheckForNans() const;

    // access operator //
    DLLEXPORT constexpr float& operator[](int nindex);

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT constexpr Float3 operator+(const Float3& other) const noexcept;
    DLLEXPORT inline Float3& operator+=(const Float3& other) noexcept;

    // subtracts all elements //
    DLLEXPORT constexpr Float3 operator-(const Float3& other) const noexcept;
    DLLEXPORT inline Float3& operator-=(const Float3& other) noexcept;

    // negates all elements //
    DLLEXPORT constexpr Float3 operator-() const noexcept;

    // returns the vector //
    DLLEXPORT constexpr Float3 operator+() const noexcept;

    // multiplies elements together //
    DLLEXPORT constexpr Float3 operator*(const Float3& other) const noexcept;
    DLLEXPORT inline Float3& operator*=(const Float3& other) noexcept;

    // Divides all elements by float //
    DLLEXPORT constexpr Float3 operator/(float val) const;
    DLLEXPORT inline Float3& operator/=(float val);

    // multiply  by scalar f //
    DLLEXPORT constexpr Float3 operator*(float val) const noexcept;
    DLLEXPORT inline Float3& operator*=(float val) noexcept;

    // divides all elements //
    DLLEXPORT constexpr Float3 operator/(const Float3& other) const;
    DLLEXPORT inline Float3& operator/=(const Float3& other);

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Float3& other) const noexcept;
    DLLEXPORT constexpr bool operator<=(const Float3& other) const noexcept;
    DLLEXPORT constexpr bool operator>(const Float3& other) const noexcept;
    DLLEXPORT constexpr bool operator>=(const Float3& other) const noexcept;
    DLLEXPORT constexpr bool operator==(const Float3& other) const noexcept;
    DLLEXPORT constexpr bool operator!=(const Float3& other) const noexcept;

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr float GetX() const noexcept;
    DLLEXPORT constexpr float GetY() const noexcept;
    DLLEXPORT constexpr float GetZ() const noexcept;

    // setters //
    DLLEXPORT inline void SetX(float val);
    DLLEXPORT inline void SetY(float val);
    DLLEXPORT inline void SetZ(float val);

    // add all elements together //
    DLLEXPORT constexpr float HAdd() const noexcept;

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline float HAddAbs() const noexcept;

    // getting min and max of objects //
    DLLEXPORT constexpr Float3 MinElements(const Float3& other) const noexcept;
    DLLEXPORT constexpr Float3 MaxElements(const Float3& other) const noexcept;

    // value clamping //
    DLLEXPORT constexpr Float3 Clamp(const Float3& min, const Float3& max) const noexcept;

    DLLEXPORT constexpr Float3 DegreesToRadians() const noexcept;

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT constexpr float Dot(const Float3& val) const noexcept;

    DLLEXPORT constexpr Float3 Cross(const Float3& val) const;

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept;
    DLLEXPORT constexpr float LengthSquared() const noexcept;

    // normalizes the vector //
    DLLEXPORT inline Float3 Normalize() const;

    // safe version of normalization //
    DLLEXPORT inline Float3 NormalizeSafe(const Float3& safer = Float3(1, 0, 0)) const noexcept;

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept;

    // does linear interpolation between vectors and coefficient f, not limited to range
    // [0,1], courtesy of ozz-animation //
    DLLEXPORT constexpr Float3 Lerp(const Float3& other, float f) const noexcept;

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT constexpr bool Compare(const Float3& other, float tolerance) const noexcept;

    DLLEXPORT static inline Float3 CreateVectorFromAngles(const float& yaw, const float& pitch) noexcept;

    // ------------------------------------ //
    // functions to be compatible with ozz functions //
    // all zero values object //
    DLLEXPORT constexpr static Float3 zero() noexcept;

    // all ones //
    DLLEXPORT constexpr static Float3 one() noexcept;

    // unitary vectors //
    // x axis
    DLLEXPORT constexpr static Float3 x_axis() noexcept;

    // y axis
    DLLEXPORT constexpr static Float3 y_axis() noexcept;

    // z axis
    DLLEXPORT constexpr static Float3 z_axis() noexcept;

    // ----------------- casts ------------------- //
	// Should this macro be replaced by a constexpr if in the cpp file?
#ifdef LEVIATHAN_FULL
    DLLEXPORT inline Float3(const Ogre::Vector3& vec);
    DLLEXPORT inline operator Ogre::Vector3() const;

    DLLEXPORT inline Float3(const btVector3& vec);
    DLLEXPORT inline operator btVector3() const;
#endif // LEVIATHAN_USING_OGRE
    // ------------------------------------ //

    VALUE_TYPE(Float3);

    float X = 0;
    float Y = 0;
    float Z = 0;

    static const Float3 UnitVForward;
    static const Float3 UnitVUp;
    static const Float3 Zeroed;
};

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Float3& value);

//// ------------------  IMPLEMENTATION ------------------  ////

DLLEXPORT constexpr Float3::Float3(float x, float y, float z) : X(x), Y(y), Z(z)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float3::Float3(Float2 floats, float z) : X(floats.X), Y(floats.Y), Z(z)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float3::Float3(float data) : X(data), Y(data), Z(data)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float3::Float3(const Int3& values) :
    X(static_cast<float>(values.X)), Y(static_cast<float>(values.Y)),
    Z(static_cast<float>(values.Z))
{
    DO_NAN_CHECK;
}

DLLEXPORT inline bool Float3::HasInvalidValues() const noexcept
{
    return !std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z);
}

DLLEXPORT inline void Float3::CheckForNans() const
{
    if(HasInvalidValues()) {
        DEBUG_BREAK;
        throw std::runtime_error("Float3 has NaNs (or infinites in it) in it!");
    }
}

DLLEXPORT constexpr float& Float3::operator[](int nindex)
{
    switch(nindex) {
    case 0: return X;
    case 1: return Y;
    case 2: return Z;
    default: break;
    }
    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

// ------------------- Operators ----------------- //

DLLEXPORT constexpr Float3 Float3::operator+(const Float3& other) const noexcept
{
    return Float3(X + other.X, Y + other.Y, Z + other.Z);
}

DLLEXPORT inline Float3& Float3::operator+=(const Float3& other) noexcept
{
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    return *this;
}

DLLEXPORT inline Float3& Float3::operator-=(const Float3& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    return *this;
}

DLLEXPORT constexpr Float3 Float3::operator-(const Float3& other) const noexcept
{
    return Float3(X - other.X, Y - other.Y, Z - other.Z);
}

DLLEXPORT constexpr Float3 Float3::operator-() const noexcept
{
    return Float3(-X, -Y, -Z);
}

DLLEXPORT constexpr Float3 Float3::operator+() const noexcept
{
    return Float3(*this);
}

DLLEXPORT constexpr Float3 Float3::operator*(const Float3& other) const noexcept
{
    return Float3(X * other.X, Y * other.Y, Z * other.Z);
}

DLLEXPORT inline Float3& Float3::operator*=(const Float3& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    Z *= other.Z;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float3 Float3::operator/(float val) const
{
    return Float3(X / val, Y / val, Z / val);
}

DLLEXPORT inline Float3& Float3::operator/=(float val)
{
    X /= val;
    Y /= val;
    Z /= val;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float3 Float3::operator*(float val) const noexcept
{
    return Float3(X * val, Y * val, Z * val);
}

DLLEXPORT inline Float3& Float3::operator*=(float val) noexcept
{
    X *= val;
    Y *= val;
    Z *= val;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float3 Float3::operator/(const Float3& other) const
{
    return Float3(X / other.X, Y / other.Y, Z / other.Z);
}

DLLEXPORT inline Float3& Float3::operator/=(const Float3& other)
{
    X /= other.X;
    Y /= other.Y;
    Z /= other.Z;
    DO_NAN_CHECK;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Float3::operator<(const Float3& other) const noexcept
{
    return std::tie(X, Y, Z) < std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Float3::operator<=(const Float3& other) const noexcept
{
    return std::tie(X, Y, Z) <= std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Float3::operator>(const Float3& other) const noexcept
{
    return std::tie(X, Y, Z) > std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Float3::operator>=(const Float3& other) const noexcept
{
    return std::tie(X, Y, Z) >= std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Float3::operator==(const Float3& other) const noexcept
{
    return X == other.X && Y == other.Y && Z == other.Z;
}

DLLEXPORT constexpr bool Float3::operator!=(const Float3& other) const noexcept
{
    return X != other.X || Y != other.Y || Z != other.Z;
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr float Float3::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr float Float3::GetY() const noexcept
{
    return Y;
}

DLLEXPORT constexpr float Float3::GetZ() const noexcept
{
    return Z;
}

DLLEXPORT inline void Float3::SetX(float val)
{
    X = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Float3::SetY(float val)
{
    Y = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Float3::SetZ(float val)
{
    Z = val;
    DO_NAN_CHECK;
}

DLLEXPORT constexpr float Float3::HAdd() const noexcept
{
    return X + Y + Z;
}

DLLEXPORT inline float Float3::HAddAbs() const noexcept
{
    return std::abs(X) + std::abs(Y) + std::abs(Z);
}

DLLEXPORT constexpr Float3 Float3::MinElements(const Float3& other) const noexcept
{
    return Float3(
        X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
}

DLLEXPORT constexpr Float3 Float3::MaxElements(const Float3& other) const noexcept
{
    return Float3(
        X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
}

DLLEXPORT constexpr Float3 Float3::Clamp(const Float3& min, const Float3& max) const noexcept
{
    const Float3 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

DLLEXPORT constexpr Float3 Float3::DegreesToRadians() const noexcept
{
    return Float3(X * DEGREES_TO_RADIANS, Y * DEGREES_TO_RADIANS, Z * DEGREES_TO_RADIANS);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr float Float3::Dot(const Float3& val) const noexcept
{
    return X * val.X + Y * val.Y + Z * val.Z;
}

DLLEXPORT constexpr Float3 Float3::Cross(const Float3& val) const
{
    return Float3(Y * val.Z - val.Y * Z, Z * val.X - val.Z * X, X * val.Y - val.X * Y);
}

DLLEXPORT inline float Float3::Length() const noexcept
{
    return static_cast<float>(sqrt(X * X + Y * Y + Z * Z));
}

DLLEXPORT constexpr float Float3::LengthSquared() const noexcept
{
    return X * X + Y * Y + Z * Z;
}

DLLEXPORT inline Float3 Float3::Normalize() const
{
    const float length = Length();
    if(length == 0)
        return Float3(0, 0, 0);
    return (*this) / length;
}

DLLEXPORT inline Float3 Float3::NormalizeSafe(const Float3& safer) const noexcept
{
    // security //
    LEVIATHAN_ASSERT(safer.IsNormalized(), "safer not normalized");
    if(LengthSquared() == 0) return safer;
    const float length = Length();
    return (*this) / length;
}

DLLEXPORT inline bool Float3::IsNormalized() const noexcept
{
    // is absolute -1.f under normalization tolerance //
    return fabs(X * X + Y * Y + Z * Z - 1.0f) < NORMALIZATION_TOLERANCE;
}

DLLEXPORT constexpr Float3 Float3::Lerp(const Float3& other, float f) const noexcept
{
    return Float3((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z);
}

DLLEXPORT constexpr bool Float3::Compare(const Float3& other, float tolerance) const noexcept
{
    const Float3 difference = (*this) - other;
    return difference.Dot(difference) < tolerance * tolerance;
}

DLLEXPORT inline Float3 Float3::CreateVectorFromAngles(
    const float& yaw, const float& pitch) noexcept
{
    return Float3(-sin(yaw * DEGREES_TO_RADIANS), sin(pitch * DEGREES_TO_RADIANS),
        -cos(yaw * DEGREES_TO_RADIANS))
        .NormalizeSafe(Zeroed);
}

// ------------------------------------ //

DLLEXPORT constexpr Float3 Float3::zero() noexcept
{
    return Float3(0.f);
}

DLLEXPORT constexpr Float3 Float3::one() noexcept
{
    return Float3(1.f);
}

DLLEXPORT constexpr Float3 Float3::x_axis() noexcept
{
    return Float3(1.f, 0.f, 0.f);
}

DLLEXPORT constexpr Float3 Float3::y_axis() noexcept
{
    return Float3(0.f, 1.f, 0.f);
}

DLLEXPORT constexpr Float3 Float3::z_axis() noexcept
{
    return Float3(0.f, 0.f, 1.f);
}

// ----------------- casts ------------------- //

#ifdef LEVIATHAN_FULL

DLLEXPORT inline Float3::Float3(const Ogre::Vector3& vec) :
    // copy values //
    X(vec.x), Y(vec.y), Z(vec.z)
{
    DO_NAN_CHECK;
}

DLLEXPORT inline Float3::operator Ogre::Vector3() const
{
    return Ogre::Vector3(X, Y, Z);
}

DLLEXPORT inline Float3::Float3(const btVector3& vec) :
    // copy values //
    X(vec.x()), Y(vec.y()), Z(vec.z())
{
    DO_NAN_CHECK;
}

DLLEXPORT inline Float3::operator btVector3() const
{
    return btVector3(X, Y, Z);
}

#endif // LEVIATHAN_USING_OGRE

}
