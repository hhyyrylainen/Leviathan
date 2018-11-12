#pragma once

#include "Define.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float2 {
public:
    DLLEXPORT constexpr Float2() noexcept = default;

    DLLEXPORT constexpr Float2(float x, float y);
    DLLEXPORT constexpr explicit Float2(float both);

    DLLEXPORT inline bool HasInvalidValues() const noexcept;
    DLLEXPORT inline void CheckForNans() const;

    // access operator //
    DLLEXPORT constexpr float& operator[](const int& nindex);

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT constexpr Float2 operator+(const Float2& other) const noexcept;
    DLLEXPORT inline Float2& operator+=(const Float2& other) noexcept;

    // subtracts all elements //
    DLLEXPORT constexpr Float2 operator-(const Float2& other) const noexcept;
    DLLEXPORT inline Float2& operator-=(const Float2& other) noexcept;

    // negates all elements //
    DLLEXPORT constexpr Float2 operator-() const noexcept;

    // returns the vector //
    DLLEXPORT constexpr Float2 operator+() const noexcept;

    // multiplies elements together //
    DLLEXPORT constexpr Float2 operator*(const Float2& other) const noexcept;
    DLLEXPORT inline Float2& operator*=(const Float2& other) noexcept;

    // Divides all elements by float //
    DLLEXPORT constexpr Float2 operator/(float val) const;
    DLLEXPORT inline Float2& operator/=(float val);

    // multiply  by scalar f //
    DLLEXPORT constexpr Float2 operator*(float val) const noexcept;
    DLLEXPORT inline Float2& operator*=(float val) noexcept;

    // divides all elements //
    DLLEXPORT constexpr Float2 operator/(const Float2& other) const;
    DLLEXPORT inline Float2& operator/=(const Float2& other);

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Float2& other) const noexcept;
    DLLEXPORT constexpr bool operator<=(const Float2& other) const noexcept;
    DLLEXPORT constexpr bool operator>(const Float2& other) const noexcept;
    DLLEXPORT constexpr bool operator>=(const Float2& other) const noexcept;
    DLLEXPORT constexpr bool operator==(const Float2& other) const noexcept;
    DLLEXPORT constexpr bool operator!=(const Float2& other) const noexcept;

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr float GetX() const noexcept;
    DLLEXPORT constexpr float GetY() const noexcept;

    // setters //
    DLLEXPORT inline void SetX(const float& val);
    DLLEXPORT inline void SetY(const float& val);

    // add all elements together //
    DLLEXPORT constexpr float HAdd() const noexcept;

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const noexcept;

    // getting min and max of objects //
    DLLEXPORT constexpr Float2 MinElements(const Float2& other) const noexcept;
    DLLEXPORT constexpr Float2 MaxElements(const Float2& other) const noexcept;

    // value clamping //
    DLLEXPORT constexpr Float2 Clamp(const Float2& min, const Float2& max) const noexcept;

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT constexpr float Dot(const Float2& val) const noexcept;

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept;
    DLLEXPORT constexpr float LengthSquared() const noexcept;

    // normalizes the vector //
    DLLEXPORT inline Float2 Normalize() const;

    // safe version of normalization //
    DLLEXPORT inline Float2 NormalizeSafe(const Float2& safer) const noexcept;

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept;

    // does linear interpolation between vectors and coefficient f, not limited to range [0,1],
    // courtesy of ozz-animation //
    DLLEXPORT constexpr Float2 Lerp(const Float2& other, float f) const noexcept;

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT constexpr bool Compare(const Float2& other, float tolerance) const noexcept;

    // ------------------------------------ //
    // static returns //
    // creates a Float2 with all zeros //
    DLLEXPORT constexpr static Float2 zero() noexcept;

    // creates a Float2 with all ones //
    DLLEXPORT constexpr static Float2 one() noexcept;

    // unitary vector x, to work with ozz declarations //
    DLLEXPORT constexpr static Float2 x_asix() noexcept;

    // unitary vector y //
    DLLEXPORT constexpr static Float2 y_axis() noexcept;

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
