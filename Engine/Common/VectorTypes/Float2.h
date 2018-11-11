#pragma once

#include "Define.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float2 {
public:
    DLLEXPORT inline Float2() = default;

    DLLEXPORT inline Float2(float x, float y);
    DLLEXPORT inline explicit Float2(float both);

    DLLEXPORT inline bool HasInvalidValues() const;
    DLLEXPORT inline void CheckForNans();

    // access operator //
    DLLEXPORT inline float& operator[](const int& nindex);

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Float2 operator+(const Float2& val) const;
    DLLEXPORT inline Float2* operator+=(const Float2& val);

    // subtracts all elements //
    DLLEXPORT inline Float2 operator-(const Float2& val) const;

    // negates all elements //
    DLLEXPORT inline Float2 operator-() const;

    // multiplies elements together //
    DLLEXPORT inline Float2 operator*(const Float2& val) const;

    // multiply  by scalar f //
    DLLEXPORT inline Float2 operator*(float f) const;
    DLLEXPORT inline Float2& operator*=(float f);

    // divides all elements //
    DLLEXPORT inline Float2 operator/(const Float2& val) const;

    // divides by float //
    DLLEXPORT inline Float2 operator/(float f) const;

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float2& other) const;
    DLLEXPORT inline bool operator<=(const Float2& other) const;
    DLLEXPORT inline bool operator>(const Float2& other) const;
    DLLEXPORT inline bool operator>=(const Float2& other) const;
    DLLEXPORT inline bool operator==(const Float2& other) const;
    DLLEXPORT inline bool operator!=(const Float2& other) const;

    // ------------------ Functions ------------------ //
    DLLEXPORT inline float GetX() const;
    DLLEXPORT inline float GetY() const;
    DLLEXPORT inline void SetX(const float& val);
    DLLEXPORT inline void SetY(const float& val);

    // add all elements together //
    DLLEXPORT inline float HAdd() const;

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const;

    // getting min and max of objects //
    DLLEXPORT inline Float2 MinElements(const Float2& other) const;
    DLLEXPORT inline Float2 MaxElements(const Float2& other) const;

    // value clamping //
    DLLEXPORT inline Float2 Clamp(const Float2& min, const Float2& max);

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float2& val) const;

    // length of the vector //
    DLLEXPORT inline float Length() const;

    // normalizes the vector //
    DLLEXPORT inline Float2 Normalize() const;

    // safe version of normalization //
    DLLEXPORT inline Float2 NormalizeSafe(const Float2& safer) const;

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const;

    // does linear interpolation between vectors and coefficient f, not limited to range [0,1],
    // courtesy of ozz-animation //
    DLLEXPORT inline Float2 Lerp(const Float2& other, float f) const;

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float2& other, float tolerance) const;

    // ------------------------------------ //
    // static returns //
    // creates a Float2 with all zeros //
    DLLEXPORT inline static Float2 zero();

    // creates a Float2 with all ones //
    DLLEXPORT inline static Float2 one();

    // unitary vector x, to work with ozz declarations //
    DLLEXPORT inline static Float2 x_asix();

    // unitary vector y //
    DLLEXPORT inline static Float2 y_axis();

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
