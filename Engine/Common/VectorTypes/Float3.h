#pragma once

#include "Define.h"
#include "Float2.h"
#include "Int3.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float3 {
public:
    DLLEXPORT Float3() = default;
    DLLEXPORT Float3::Float3(const Float3& other);

    DLLEXPORT Float3(float x, float y, float z);
    DLLEXPORT Float3(Float2 floats, float z);
    DLLEXPORT explicit Float3(float data);
    DLLEXPORT Float3(const Int3& values);

    DLLEXPORT inline bool HasInvalidValues() const;

    DLLEXPORT inline void CheckForNans();

    // access operator //
    DLLEXPORT inline float& operator[](const int& nindex);

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Float3 operator+(const Float3& val) const;
    DLLEXPORT inline Float3& operator+=(const Float3& val);


    // subtracts all elements //
    DLLEXPORT inline Float3 operator-(const Float3& val) const;
    DLLEXPORT inline Float3& operator-=(const Float3& val);

    // negates all elements //
    DLLEXPORT inline Float3 operator-() const;

    // multiplies elements together //
    DLLEXPORT inline Float3 operator*(const Float3& val) const;

    // Divides all elements by float //
    DLLEXPORT inline Float3 operator/(const float& val) const;
    DLLEXPORT inline Float3& operator/=(const float& val);

    // multiply  by scalar f //
    DLLEXPORT inline Float3 operator*(float f) const;
    DLLEXPORT inline Float3& operator*=(float f);

    // divides all elements //
    DLLEXPORT inline Float3 operator/(const Float3& val) const;

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float3& other) const;
    DLLEXPORT inline bool operator<=(const Float3& other) const;
    DLLEXPORT inline bool operator>(const Float3& other) const;
    DLLEXPORT inline bool operator>=(const Float3& other) const;
    DLLEXPORT inline bool operator==(const Float3& other) const;
    DLLEXPORT inline bool operator!=(const Float3& other) const;

    // ------------------ Functions ------------------ //
    DLLEXPORT inline float GetX() const;
    DLLEXPORT inline float GetY() const;
    DLLEXPORT inline float GetZ() const;
    DLLEXPORT inline void SetX(const float& val);
    DLLEXPORT inline void SetY(const float& val);
    DLLEXPORT inline void SetZ(const float& val);

    // add all elements together //
    DLLEXPORT inline float HAdd() const;

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline float HAddAbs() const;

    // getting min and max of objects //
    DLLEXPORT inline Float3 MinElements(const Float3& other) const;
    DLLEXPORT inline Float3 MaxElements(const Float3& other) const;

    // value clamping //
    DLLEXPORT inline Float3 Clamp(const Float3& min, const Float3& max);

    DLLEXPORT inline Float3 DegreesToRadians();

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float3& val) const;

    DLLEXPORT inline Float3 Cross(const Float3& val) const;

    // length of the vector //
    DLLEXPORT inline float Length() const;
    DLLEXPORT inline float LengthSquared() const;

    // normalizes the vector //
    DLLEXPORT inline Float3 Normalize() const;

    // safe version of normalization //
    DLLEXPORT inline Float3 NormalizeSafe(const Float3& safer = Float3(1, 0, 0)) const;

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const;

    // does linear interpolation between vectors and coefficient f, not limited to range
    // [0,1], courtesy of ozz-animation //
    DLLEXPORT inline Float3 Lerp(const Float3& other, float f) const;

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float3& other, float tolerance) const;

    DLLEXPORT static inline Float3 CreateVectorFromAngles(const float& yaw, const float& pitch);

    // ------------------------------------ //
    // functions to be compatible with ozz functions //
    // all zero values object //
    DLLEXPORT inline static Float3 zero();

    // all ones //
    DLLEXPORT inline static Float3 one();

    // unitary vectors //
    // x axis
    DLLEXPORT inline static Float3 x_axis();

    // y axis
    DLLEXPORT inline static Float3 y_axis();

    // z axis
    DLLEXPORT inline static Float3 z_axis();

    // ----------------- casts ------------------- //
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

    static const Float3 UnitVForward; // What
    static const Float3 UnitVUp;      // are
    static const Float3 Zeroed;       // these?
};

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Float3& value);

}
