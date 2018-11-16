#pragma once

#include "Define.h"
#include "Int3.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float2 {
public:
    DLLEXPORT constexpr Float2() noexcept = default;

    DLLEXPORT constexpr Float2(float x, float y) : X(x), Y(y)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT constexpr Float2(const Int2& values) noexcept :
        X(static_cast<float>(values.X)), Y(static_cast<float>(values.Y))
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT constexpr explicit Float2(float data) : X(data), Y(data)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const noexcept
    {
        return !std::isfinite(X) || !std::isfinite(Y);
    }

    DLLEXPORT inline void CheckForNans() const
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float2 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](int nindex)
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
    // add elements //
    DLLEXPORT constexpr Float2 operator+(const Float2& other) const noexcept
    {
        return Float2(X + other.X, Y + other.Y);
    }

    DLLEXPORT inline Float2& operator+=(const Float2& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT constexpr Float2 operator-(const Float2& other) const noexcept
    {
        return Float2(X - other.X, Y - other.Y);
    }

    DLLEXPORT inline Float2& operator-=(const Float2& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        return *this;
    }

    // negates all elements //
    DLLEXPORT constexpr Float2 operator-() const noexcept
    {
        return Float2(-X, -Y);
    }

    // returns the vector //
    DLLEXPORT constexpr Float2 operator+() const noexcept
    {
        return Float2(*this);
    }

    // multiplies elements together //
    DLLEXPORT constexpr Float2 operator*(const Float2& other) const noexcept
    {
        return Float2(X * other.X, Y * other.Y);
    }

    DLLEXPORT inline Float2& operator*=(const Float2& other) noexcept
    {
        X *= other.X;
        Y *= other.Y;
        DO_NAN_CHECK;
        return *this;
    }

    // Divides all elements by float //
    DLLEXPORT constexpr Float2 operator/(float val) const
    {
        return Float2(X / val, Y / val);
    }

    DLLEXPORT inline Float2& operator/=(float val)
    {
        X /= val;
        Y /= val;
        DO_NAN_CHECK;
        return *this;
    }

    // multiply  by scalar f //
    DLLEXPORT constexpr Float2 operator*(float val) const noexcept
    {
        return Float2(X * val, Y * val);
    }

    DLLEXPORT inline Float2& operator*=(float val) noexcept
    {
        X *= val;
        Y *= val;
        DO_NAN_CHECK;
        return *this;
    }

    // divides all elements //
    DLLEXPORT constexpr Float2 operator/(const Float2& other) const
    {
        return Float2(X / other.X, Y / other.Y);
    }

    DLLEXPORT inline Float2& operator/=(const Float2& other)
    {
        X /= other.X;
        Y /= other.Y;
        DO_NAN_CHECK;
        return *this;
    }

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Float2& other) const noexcept
    {
        return std::tie(X, Y) < std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator<=(const Float2& other) const noexcept
    {
        return std::tie(X, Y) <= std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator>(const Float2& other) const noexcept
    {
        return std::tie(X, Y) > std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator>=(const Float2& other) const noexcept
    {
        return std::tie(X, Y) >= std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator==(const Float2& other) const noexcept
    {
        return X == other.X && Y == other.Y;
    }

    DLLEXPORT constexpr bool operator!=(const Float2& other) const noexcept
    {
        return X != other.X || Y != other.Y;
    }

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr float GetX() const noexcept
    {
        return X;
    }

    DLLEXPORT constexpr float GetY() const noexcept
    {
        return Y;
    }

    // setters //
    DLLEXPORT inline void SetX(float val)
    {
        X = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetY(float val)
    {
        Y = val;
        DO_NAN_CHECK;
    }

    // add all elements together //
    DLLEXPORT constexpr float HAdd() const noexcept
    {
        return X + Y;
    }

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const noexcept
    {
        return std::fabs(X) + std::fabs(Y);
    }

    // getting min and max of objects //
    DLLEXPORT constexpr Float2 MinElements(const Float2& other) const noexcept
    {
        return Float2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
    }

    DLLEXPORT constexpr Float2 MaxElements(const Float2& other) const noexcept
    {
        return Float2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
    }

    // value clamping //
    DLLEXPORT constexpr Float2 Clamp(const Float2& min, const Float2& max) const noexcept
    {
        const Float2 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT constexpr float Dot(const Float2& val) const noexcept
    {
        return X * val.X + Y * val.Y;
    }

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept
    {
        return static_cast<float>(sqrt(X * X + Y * Y));
    }

    DLLEXPORT constexpr float LengthSquared() const noexcept
    {
        return X * X + Y * Y;
    }

    // normalizes the vector //
    DLLEXPORT inline Float2 Normalize() const
    {
        const float length = Length();
        if(length == 0)
            return Float2(0, 0);
        return (*this) / length;
    }

    // safe version of normalization //
    DLLEXPORT inline Float2 NormalizeSafe(const Float2& safer) const noexcept
    {
        // security //
        LEVIATHAN_ASSERT(safer.IsNormalized(), "safer not normalized");
        if(LengthSquared() == 0)
            return safer;
        const float length = Length();
        return (*this) / length;
    }

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y - 1.0f) < NORMALIZATION_TOLERANCE;
    }

    // does linear interpolation between vectors and coefficient f, not limited to range [0,1],
    // courtesy of ozz-animation //
    DLLEXPORT constexpr Float2 Lerp(const Float2& other, float f) const noexcept
    {
        return Float2((other.X - X) * f + X, (other.Y - Y) * f + Y);
    }

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT constexpr bool Compare(const Float2& other, float tolerance) const noexcept
    {
        const Float2 difference = (*this) - other;
        return difference.Dot(difference) < tolerance * tolerance;
    }

    // ------------------------------------ //
    // static returns //
    // creates a Float2 with all zeros //
    DLLEXPORT constexpr static Float2 zero() noexcept
    {
        return Float2(0.f);
    }

    // creates a Float2 with all ones //
    DLLEXPORT constexpr static Float2 one() noexcept
    {
        return Float2(1.f);
    }

    // unitary vector x, to work with ozz declarations //
    DLLEXPORT constexpr static Float2 x_axis() noexcept
    {
        return Float2(1.f, 0.f);
    }

    // unitary vector y //
    DLLEXPORT constexpr static Float2 y_axis() noexcept
    {
        return Float2(0.f, 1.f);
    }

    // ----------------- casts ------------------- //
    // waiting for Microsoft's compilers to add support for "explicit" here //
    // DLLEXPORT inline operator D3DXVECTOR2(){
    //	return D3DXVECTOR2(X, Y);
    //}

    VALUE_TYPE(Float2);

    // data //
    float X = 0;
    float Y = 0;
};

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Float2& value);

}
