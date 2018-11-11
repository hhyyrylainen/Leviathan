#pragma once

#include "Define.h"
#include "Float2.h"
#include "Float3.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float4 {
public:
    DLLEXPORT Float4() = default;

    DLLEXPORT Float4(float f1, float f2, float f3, float f4);
    DLLEXPORT Float4(Float2 floats, float f3, float f4);
    DLLEXPORT Float4(Float3 floats, float f4);
    DLLEXPORT explicit Float4(float val);

    DLLEXPORT inline bool HasInvalidValues() const;
    DLLEXPORT inline void CheckForNans();

    // access operator //
    DLLEXPORT inline float& operator[](const int& nindex);

    //! return first value of {X, Y, Z, W} as a pointer
    DLLEXPORT inline operator float*();

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Float4 operator+(const Float4& val) const;

    // subtracts all elements //
    DLLEXPORT inline Float4 operator-(const Float4& val) const;

    // negates all elements //
    DLLEXPORT inline Float4 operator-() const;

    // multiplies elements together //
    DLLEXPORT inline Float4 operator*(const Float4& val) const;

    // multiply  by scalar f //
    DLLEXPORT inline Float4 operator*(float f) const;

    // divides all elements //
    DLLEXPORT inline Float4 operator/(const Float4& val) const;

    // divides by float //
    DLLEXPORT inline Float4 operator/(float f) const;

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float4& other) const;
    DLLEXPORT inline bool operator>(const Float4& other) const;
    DLLEXPORT inline bool operator==(const Float4& other) const;
    DLLEXPORT inline bool operator!=(const Float4& other) const;

    // ------------------ Functions ------------------ //
    DLLEXPORT inline float GetX() const;
    DLLEXPORT inline float GetY() const;
    DLLEXPORT inline float GetZ() const;
    DLLEXPORT inline float GetW() const;
    DLLEXPORT inline void SetX(const float& val);
    DLLEXPORT inline void SetY(const float& val);
    DLLEXPORT inline void SetZ(const float& val);
    DLLEXPORT inline void SetW(const float& val);

    // add all elements together //
    DLLEXPORT inline float HAdd() const;

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const;

    // getting min and max of objects //
    DLLEXPORT inline Float4 MinElements(const Float4& other) const;
    DLLEXPORT inline Float4 MaxElements(const Float4& other) const;

    // value clamping //
    DLLEXPORT inline Float4 Clamp(const Float4& min, const Float4& max);

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float4& val) const;

    // length of the vector //
    DLLEXPORT inline float Length() const;

    // normalizes the vector //
    DLLEXPORT inline Float4 Normalize() const;

    // safe version of normalization //
    DLLEXPORT inline Float4 NormalizeSafe(const Float4& safer = Float4(0, 0, 0, 1)) const;

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const;

    // does linear interpolation between vectors and coefficient f,
    // not limited to range [0,1], courtesy of ozz-animation //
    DLLEXPORT inline Float4 Lerp(const Float4& other, float f) const;

    // does SPHERICAL interpolation between quaternions //
    DLLEXPORT inline Float4 Slerp(const Float4& other, float f) const;

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float4& other, float tolerance) const;

    //! Converts a quaternion to Axis (and skips outputting the angle)
    //! \note Must be normalized
    DLLEXPORT inline Float3 ToAxis() const;

    //! Converts a quaternion to angle (and skips outputting the Axis)
    //! \note Must be normalized
    DLLEXPORT inline float ToAngle() const;

    //! Inverts a quaternion
    DLLEXPORT inline Float4 Inverse() const;

    //! Rotates a vector by this quaternion
    DLLEXPORT inline Float3 RotateVector(const Float3& vector) const;

    // ------------------------------------ //
    // All zeros //
    DLLEXPORT inline static Float4 zero();

    // all ones //
    DLLEXPORT inline static Float4 one();

    // unitary vectors for ozz support //
    // x
    DLLEXPORT inline static Float4 x_axis();
    // y
    DLLEXPORT inline static Float4 y_axis();
    // z
    DLLEXPORT inline static Float4 z_axis();
    // w
    DLLEXPORT inline static Float4 w_axis();

#ifdef LEVIATHAN_FULL
    DLLEXPORT Float4(const Ogre::Quaternion& quat);

    DLLEXPORT Float4(const Ogre::ColourValue& colour);

    DLLEXPORT inline operator Ogre::Quaternion() const;

    DLLEXPORT inline operator Ogre::ColourValue() const;

    DLLEXPORT inline operator Ogre::Vector4() const;

    DLLEXPORT Float4(const btQuaternion& colour);

    DLLEXPORT inline operator btQuaternion() const;
#endif // LEVIATHAN_USING_OGRE

    // ----------------- Quaternions ------------------- //
    DLLEXPORT static inline Float4 CreateQuaternionFromAngles(const Float3& angles);

    //! \note This quaternion has to be normalized
    //! \see
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
    DLLEXPORT inline Float3 QuaternionToEuler() const;

    DLLEXPORT inline Float4 QuaternionMultiply(const Float4& other) const;

    DLLEXPORT inline Float4 QuaternionReverse() const;

    DLLEXPORT static inline Float4 IdentityQuaternion();

    // Math from here: https://stackoverflow.com/questions/12435671/quaternion-lookat-function
    DLLEXPORT static inline Float4 QuaternionLookAt(
        const Float3& sourcepoint, const Float3& target);

    //! \note axis must be normalized
    //!
    //! This resource is a life saver:
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/index.htm
    DLLEXPORT static inline Float4 CreateQuaternionFromAxisAngle(
        const Float3& axis, float angle);

    DLLEXPORT static inline Float4 CreateAxisAngleFromEuler(const Float3& angles);

    VALUE_TYPE(Float4);

    // ----------------- casts ------------------- //

    // ------------------------------------ //

    float X = 0;
    float Y = 0;
    float Z = 0;
    float W = 0;

    // specific colours //
    static const Float4 ColourBlack;
    static const Float4 ColourWhite;
    static const Float4 ColourTransparent;

    // Use these from other libraries/executables to avoid linker errors //
    DLLEXPORT static const Float4& GetColourBlack();
    DLLEXPORT static const Float4& GetColourWhite();
    DLLEXPORT static const Float4& GetColourTransparent();
};

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Float4& value);

}
