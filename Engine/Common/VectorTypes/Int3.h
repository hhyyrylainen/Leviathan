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
}
