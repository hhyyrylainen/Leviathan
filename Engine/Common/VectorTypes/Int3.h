#pragma once

#include "Define.h"
#include "Int2.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Int3 {
public:
    DLLEXPORT constexpr Int3() noexcept = default;

    DLLEXPORT constexpr Int3(int x, int y, int z);
    DLLEXPORT constexpr Int3(Int2 ints, int z);
    DLLEXPORT constexpr explicit Int3(int data);

    // access operator //
    DLLEXPORT constexpr int& operator[](int nindex);

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT constexpr Int3 operator+(const Int3& other) const noexcept;
    DLLEXPORT inline Int3& operator+=(const Int3& other) noexcept;

    // subtracts all elements //
    DLLEXPORT constexpr Int3 operator-(const Int3& other) const noexcept;
    DLLEXPORT inline Int3& operator-=(const Int3& other) noexcept;

    // negates all elements //
    DLLEXPORT constexpr Int3 operator-() const noexcept;

    // returns the vector //
    DLLEXPORT constexpr Int3 operator+() const noexcept;

    // multiplies elements together //
    DLLEXPORT constexpr Int3 operator*(const Int3& other) const noexcept;
    DLLEXPORT inline Int3& operator*=(const Int3& other) noexcept;

    // Divides all elements by int //
    DLLEXPORT constexpr Int3 operator/(int val) const;
    DLLEXPORT inline Int3& operator/=(int val);

    // multiply  by scalar f //
    DLLEXPORT constexpr Int3 operator*(int val) const noexcept;
    DLLEXPORT inline Int3& operator*=(int val) noexcept;

    // divides all elements //
    DLLEXPORT constexpr Int3 operator/(const Int3& other) const;
    DLLEXPORT inline Int3& operator/=(const Int3& other);

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Int3& other) const noexcept;
    DLLEXPORT constexpr bool operator<=(const Int3& other) const noexcept;
    DLLEXPORT constexpr bool operator>(const Int3& other) const noexcept;
    DLLEXPORT constexpr bool operator>=(const Int3& other) const noexcept;
    DLLEXPORT constexpr bool operator==(const Int3& other) const noexcept;
    DLLEXPORT constexpr bool operator!=(const Int3& other) const noexcept;

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr int GetX() const noexcept;
    DLLEXPORT constexpr int GetY() const noexcept;
    DLLEXPORT constexpr int GetZ() const noexcept;

    // setters //
    DLLEXPORT inline void SetX(int val);
    DLLEXPORT inline void SetY(int val);
    DLLEXPORT inline void SetZ(int val);

    // add all elements together //
    DLLEXPORT constexpr int HAdd() const noexcept;

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline unsigned int HAddAbs() const noexcept;

    // getting min and max of objects //
    DLLEXPORT constexpr Int3 MinElements(const Int3& other) const noexcept;
    DLLEXPORT constexpr Int3 MaxElements(const Int3& other) const noexcept;

    // value clamping //
    DLLEXPORT constexpr Int3 Clamp(const Int3& min, const Int3& max) const noexcept;

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT constexpr int Dot(const Int3& val) const noexcept;

    DLLEXPORT constexpr Int3 Cross(const Int3& val) const;

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept;
    DLLEXPORT constexpr unsigned int LengthSquared() const noexcept;

    // ------------------------------------ //
    // functions to be compatible with ozz functions //
    // all zero values object //
    DLLEXPORT constexpr static Int3 zero() noexcept;

    // all ones //
    DLLEXPORT constexpr static Int3 one() noexcept;

    // unitary vectors //
    // x axis
    DLLEXPORT constexpr static Int3 x_axis() noexcept;

    // y axis
    DLLEXPORT constexpr static Int3 y_axis() noexcept;

    // z axis
    DLLEXPORT constexpr static Int3 z_axis() noexcept;
    // ------------------------------------ //

    VALUE_TYPE(Int3);

    int X = 0;
    int Y = 0;
    int Z = 0;
};

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Int3& value);

//// ------------------  IMPLEMENTATION ------------------  ////

DLLEXPORT constexpr Int3::Int3(int x, int y, int z) : X(x), Y(y), Z(z) {}

DLLEXPORT constexpr Int3::Int3(Int2 ints, int z) : X(ints.X), Y(ints.Y), Z(z) {}

DLLEXPORT constexpr Int3::Int3(int data) : X(data), Y(data), Z(data) {}

DLLEXPORT constexpr int& Int3::operator[](int nindex)
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

DLLEXPORT constexpr Int3 Int3::operator+(const Int3& other) const noexcept
{
    return Int3(X + other.X, Y + other.Y, Z + other.Z);
}

DLLEXPORT inline Int3& Int3::operator+=(const Int3& other) noexcept
{
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    return *this;
}

DLLEXPORT inline Int3& Int3::operator-=(const Int3& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator-(const Int3& other) const noexcept
{
    return Int3(X - other.X, Y - other.Y, Z - other.Z);
}

DLLEXPORT constexpr Int3 Int3::operator-() const noexcept
{
    return Int3(-X, -Y, -Z);
}

DLLEXPORT constexpr Int3 Int3::operator+() const noexcept
{
    return Int3(*this);
}

DLLEXPORT constexpr Int3 Int3::operator*(const Int3& other) const noexcept
{
    return Int3(X * other.X, Y * other.Y, Z * other.Z);
}

DLLEXPORT inline Int3& Int3::operator*=(const Int3& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    Z *= other.Z;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator/(int val) const
{
    return Int3(X / val, Y / val, Z / val);
}

DLLEXPORT inline Int3& Int3::operator/=(int val)
{
    X /= val;
    Y /= val;
    Z /= val;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator*(int val) const noexcept
{
    return Int3(X * val, Y * val, Z * val);
}

DLLEXPORT inline Int3& Int3::operator*=(int val) noexcept
{
    X *= val;
    Y *= val;
    Z *= val;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator/(const Int3& other) const
{
    return Int3(X / other.X, Y / other.Y, Z / other.Z);
}

DLLEXPORT inline Int3& Int3::operator/=(const Int3& other)
{
    X /= other.X;
    Y /= other.Y;
    Z /= other.Z;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Int3::operator<(const Int3& other) const noexcept
{
    return std::tie(X, Y, Z) < std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Int3::operator<=(const Int3& other) const noexcept
{
    return std::tie(X, Y, Z) <= std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Int3::operator>(const Int3& other) const noexcept
{
    return std::tie(X, Y, Z) > std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Int3::operator>=(const Int3& other) const noexcept
{
    return std::tie(X, Y, Z) >= std::tie(other.X, other.Y, other.Z);
}

DLLEXPORT constexpr bool Int3::operator==(const Int3& other) const noexcept
{
    return X == other.X && Y == other.Y && Z == other.Z;
}

DLLEXPORT constexpr bool Int3::operator!=(const Int3& other) const noexcept
{
    return X != other.X || Y != other.Y || Z != other.Z;
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr int Int3::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr int Int3::GetY() const noexcept
{
    return Y;
}

DLLEXPORT constexpr int Int3::GetZ() const noexcept
{
    return Z;
}

DLLEXPORT inline void Int3::SetX(int val)
{
    X = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Int3::SetY(int val)
{
    Y = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Int3::SetZ(int val)
{
    Z = val;
    DO_NAN_CHECK;
}

DLLEXPORT constexpr int Int3::HAdd() const noexcept
{
    return X + Y + Z;
}

DLLEXPORT inline unsigned int Int3::HAddAbs() const noexcept
{
    return std::abs(X) + std::abs(Y) + std::abs(Z);
}

DLLEXPORT constexpr Int3 Int3::MinElements(const Int3& other) const noexcept
{
    return Int3(
        X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
}

DLLEXPORT constexpr Int3 Int3::MaxElements(const Int3& other) const noexcept
{
    return Int3(
        X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
}

DLLEXPORT constexpr Int3 Int3::Clamp(const Int3& min, const Int3& max) const noexcept
{
    const Int3 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr int Int3::Dot(const Int3& val) const noexcept
{
    return X * val.X + Y * val.Y + Z * val.Z;
}

DLLEXPORT constexpr Int3 Int3::Cross(const Int3& val) const
{
    return Int3(Y * val.Z - val.Y * Z, Z * val.X - val.Z * X, X * val.Y - val.X * Y);
}

DLLEXPORT inline float Int3::Length() const noexcept
{
    return sqrt(X * X + Y * Y + Z * Z);
}

DLLEXPORT constexpr unsigned int Int3::LengthSquared() const noexcept
{
    return X * X + Y * Y + Z * Z;
}

// ------------------------------------ //

DLLEXPORT constexpr Int3 Int3::zero() noexcept
{
    return Int3(0);
}

DLLEXPORT constexpr Int3 Int3::one() noexcept
{
    return Int3(1);
}

DLLEXPORT constexpr Int3 Int3::x_axis() noexcept
{
    return Int3(1, 0, 0);
}

DLLEXPORT constexpr Int3 Int3::y_axis() noexcept
{
    return Int3(0, 1, 0);
}

DLLEXPORT constexpr Int3 Int3::z_axis() noexcept
{
    return Int3(0, 0, 1);
}

}
