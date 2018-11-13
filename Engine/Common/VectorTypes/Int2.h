#pragma once

#include "Define.h"

namespace Leviathan {

struct Int2 {
public:
    DLLEXPORT constexpr Int2() noexcept = default;

    DLLEXPORT constexpr Int2(int x, int y) noexcept;
    DLLEXPORT constexpr explicit Int2(int data) noexcept;

    // access operator //
    DLLEXPORT constexpr int& operator[](int nindex);

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT constexpr Int2 operator+(const Int2& other) const noexcept;
    DLLEXPORT inline Int2& operator+=(const Int2& other) noexcept;

    // subtracts all elements //
    DLLEXPORT constexpr Int2 operator-(const Int2& other) const noexcept;
    DLLEXPORT inline Int2& operator-=(const Int2& other) noexcept;

    // negates all elements //
    DLLEXPORT constexpr Int2 operator-() const noexcept;

    // returns the vector //
    DLLEXPORT constexpr Int2 operator+() const noexcept;

    // multiplies elements together //
    DLLEXPORT constexpr Int2 operator*(const Int2& other) const noexcept;
    DLLEXPORT inline Int2& operator*=(const Int2& other) noexcept;

    // Divides all elements by int //
    DLLEXPORT constexpr Int2 operator/(int val) const;
    DLLEXPORT inline Int2& operator/=(int val);

    // multiply  by scalar f //
    DLLEXPORT constexpr Int2 operator*(int val) const noexcept;
    DLLEXPORT inline Int2& operator*=(int val) noexcept;

    // divides all elements //
    DLLEXPORT constexpr Int2 operator/(const Int2& other) const;
    DLLEXPORT inline Int2& operator/=(const Int2& other);

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Int2& other) const noexcept;
    DLLEXPORT constexpr bool operator<=(const Int2& other) const noexcept;
    DLLEXPORT constexpr bool operator>(const Int2& other) const noexcept;
    DLLEXPORT constexpr bool operator>=(const Int2& other) const noexcept;
    DLLEXPORT constexpr bool operator==(const Int2& other) const noexcept;
    DLLEXPORT constexpr bool operator!=(const Int2& other) const noexcept;

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr int GetX() const noexcept;
    DLLEXPORT constexpr int GetY() const noexcept;

    // setters //
    DLLEXPORT inline void SetX(int val);
    DLLEXPORT inline void SetY(int val);

    // add all elements together //
    DLLEXPORT constexpr int HAdd() const noexcept;

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline unsigned int HAddAbs() const noexcept;

    // getting min and max of objects //
    DLLEXPORT constexpr Int2 MinElements(const Int2& other) const noexcept;
    DLLEXPORT constexpr Int2 MaxElements(const Int2& other) const noexcept;

    // value clamping //
    DLLEXPORT constexpr Int2 Clamp(const Int2& min, const Int2& max) const noexcept;

	// Why these two?
    DLLEXPORT void SetData(const int& data);
    DLLEXPORT void SetData(const int& data1, const int& data2);

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT constexpr int Dot(const Int2& val) const noexcept;

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept;
    DLLEXPORT constexpr unsigned int LengthSquared() const noexcept;

    // ------------------------------------ //
    // static returns //
    // creates a Int2 with all zeros //
    DLLEXPORT constexpr static Int2 zero() noexcept;

    // creates a Int2 with all ones //
    DLLEXPORT constexpr static Int2 one() noexcept;

    // unitary vector x, to work with ozz declarations //
    DLLEXPORT constexpr static Int2 x_axis() noexcept;

    // unitary vector y //
    DLLEXPORT constexpr static Int2 y_axis() noexcept;

    VALUE_TYPE(Int2);

    // data //
    int X = 0;
    int Y = 0;
};

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Int2& value);

//// ------------------  IMPLEMENTATION ------------------  ////

DLLEXPORT constexpr Int2::Int2(int x, int y) noexcept : X(x), Y(y) {}

DLLEXPORT constexpr Int2::Int2(int data) noexcept : X(data), Y(data) {}

// ------------------------------------ //

DLLEXPORT constexpr int& Int2::operator[](int nindex)
{
    switch(nindex) {
    case 0: return X;
    case 1: return Y;
    default: break;
    }

    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

// ------------------- Operators ----------------- //

DLLEXPORT constexpr Int2 Int2::operator+(const Int2& other) const noexcept
{
    return Int2(X + other.X, Y + other.Y);
}

DLLEXPORT inline Int2& Int2::operator+=(const Int2& other) noexcept
{
    X += other.X;
    Y += other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator-(const Int2& other) const noexcept
{
    return Int2(X - other.X, Y - other.Y);
}

DLLEXPORT constexpr Int2 Int2::operator-() const noexcept
{
    return Int2(-X, -Y);
}

DLLEXPORT inline Int2& Int2::operator-=(const Int2& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator+() const noexcept
{
    return Int2(*this);
}

DLLEXPORT constexpr Int2 Int2::operator*(const Int2& other) const noexcept
{
    return Int2(X * other.X, Y * other.Y);
}

DLLEXPORT inline Int2& Int2::operator*=(const Int2& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator*(int val) const noexcept
{
    return Int2(X * val, Y * val);
}

DLLEXPORT inline Int2& Int2::operator*=(int val) noexcept
{
    X *= val;
    Y *= val;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator/(const Int2& other) const
{
    return Int2(X / other.X, Y / other.Y);
}

DLLEXPORT inline Int2& Int2::operator/=(const Int2& other)
{
    X /= other.X;
    Y /= other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator/(int val) const
{
    return Int2(X / val, Y / val);
}

DLLEXPORT inline Int2& Int2::operator/=(int val)
{
    X /= val;
    Y /= val;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Int2::operator<(const Int2& other) const noexcept
{
    return std::tie(X, Y) < std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Int2::operator<=(const Int2& other) const noexcept
{
    return std::tie(X, Y) <= std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Int2::operator>(const Int2& other) const noexcept
{
    return std::tie(X, Y) > std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Int2::operator>=(const Int2& other) const noexcept
{
    return std::tie(X, Y) >= std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Int2::operator==(const Int2& other) const noexcept
{
    return X == other.X && Y == other.Y;
}

DLLEXPORT constexpr bool Int2::operator!=(const Int2& other) const noexcept
{
    return X != other.X || Y != other.Y;
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr int Int2::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr int Int2::GetY() const noexcept
{
    return Y;
}

DLLEXPORT inline void Int2::SetX(int val)
{
    X = val;
}

DLLEXPORT inline void Int2::SetY(int val)
{
    Y = val;
}

DLLEXPORT constexpr int Int2::HAdd() const noexcept
{
    return X + Y;
}

DLLEXPORT inline unsigned int Int2::HAddAbs() const noexcept
{
    return std::abs(X) + std::abs(Y);
}

DLLEXPORT constexpr Int2 Int2::MinElements(const Int2& other) const noexcept
{
    return Int2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
}

DLLEXPORT constexpr Int2 Int2::MaxElements(const Int2& other) const noexcept
{
    return Int2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
}

DLLEXPORT constexpr Int2 Int2::Clamp(const Int2& min, const Int2& max) const noexcept
{
    const Int2 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr int Int2::Dot(const Int2& val) const noexcept
{
    return X * val.X + Y * val.Y;
}

DLLEXPORT inline float Int2::Length() const noexcept
{
    return sqrt(X * X + Y * Y);
}

DLLEXPORT constexpr unsigned int Int2::LengthSquared() const noexcept
{
    return X * X + Y * Y;
}

// ------------------------------------ //

DLLEXPORT constexpr Int2 Int2::zero() noexcept
{
    return Int2(0);
}

DLLEXPORT constexpr Int2 Int2::one() noexcept
{
    return Int2(1);
}

DLLEXPORT constexpr Int2 Int2::x_axis() noexcept
{
    return Int2(1, 0);
}

DLLEXPORT constexpr Int2 Int2::y_axis() noexcept
{
    return Int2(0, 1);
}

}
