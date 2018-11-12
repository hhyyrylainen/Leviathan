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
    DLLEXPORT constexpr float& operator[](const int& nindex);

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
    DLLEXPORT inline void SetX(const float& val);
    DLLEXPORT inline void SetY(const float& val);
    DLLEXPORT inline void SetZ(const float& val);

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
    DLLEXPORT Float3(const Ogre::Vector3& vec);
    DLLEXPORT inline operator Ogre::Vector3() const;

    DLLEXPORT Float3(const btVector3& vec);
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

}
