#pragma once

#include "Define.h"
#include "Float2.h"
#include "Float3.h"
#include "VectorTypesCommon.h"

namespace Leviathan {

struct Float4 {
public:
    DLLEXPORT constexpr Float4() noexcept = default;

    DLLEXPORT constexpr Float4(float f1, float f2, float f3, float f4);
    DLLEXPORT constexpr Float4(Float2 floats, float f3, float f4);
    DLLEXPORT constexpr Float4(Float3 floats, float f4);
    DLLEXPORT constexpr explicit Float4(float data);

    DLLEXPORT inline bool HasInvalidValues() const noexcept;
    DLLEXPORT inline void CheckForNans() const;

    // access operator //
    DLLEXPORT constexpr float& operator[](int nindex);

    //! return first value of {X, Y, Z, W} as a pointer
    DLLEXPORT constexpr operator float*() noexcept;

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT constexpr Float4 operator+(const Float4& other) const noexcept;
    DLLEXPORT inline Float4& operator+=(const Float4& other) noexcept;

    // subtracts all elements //
    DLLEXPORT constexpr Float4 operator-(const Float4& other) const noexcept;
    DLLEXPORT inline Float4& operator-=(const Float4& other) noexcept;

    // negates all elements //
    DLLEXPORT constexpr Float4 operator-() const noexcept;

    // returns the vector //
    DLLEXPORT constexpr Float4 operator+() const noexcept;

    // multiplies elements together //
    DLLEXPORT constexpr Float4 operator*(const Float4& other) const noexcept;
    DLLEXPORT inline Float4& operator*=(const Float4& other) noexcept;

    // Divides all elements by float //
    DLLEXPORT constexpr Float4 operator/(float val) const;
    DLLEXPORT inline Float4& operator/=(float val);

    // multiply  by scalar f //
    DLLEXPORT constexpr Float4 operator*(float val) const noexcept;
    DLLEXPORT inline Float4& operator*=(float val) noexcept;

    // divides all elements //
    DLLEXPORT constexpr Float4 operator/(const Float4& other) const;
    DLLEXPORT inline Float4& operator/=(const Float4& other);

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Float4& other) const noexcept;
    DLLEXPORT constexpr bool operator<=(const Float4& other) const noexcept;
    DLLEXPORT constexpr bool operator>(const Float4& other) const noexcept;
    DLLEXPORT constexpr bool operator>=(const Float4& other) const noexcept;
    DLLEXPORT constexpr bool operator==(const Float4& other) const noexcept;
    DLLEXPORT constexpr bool operator!=(const Float4& other) const noexcept;

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr float GetX() const noexcept;
    DLLEXPORT constexpr float GetY() const noexcept;
    DLLEXPORT constexpr float GetZ() const noexcept;
    DLLEXPORT constexpr float GetW() const noexcept;

    // setters //
    DLLEXPORT inline void SetX(float val);
    DLLEXPORT inline void SetY(float val);
    DLLEXPORT inline void SetZ(float val);
    DLLEXPORT inline void SetW(float val);

    // add all elements together //
    DLLEXPORT constexpr float HAdd() const noexcept;

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline float HAddAbs() const noexcept;

    // getting min and max of objects //
    DLLEXPORT constexpr Float4 MinElements(const Float4& other) const noexcept;
    DLLEXPORT constexpr Float4 MaxElements(const Float4& other) const noexcept;

    // value clamping //
    DLLEXPORT constexpr Float4 Clamp(const Float4& min, const Float4& max) const noexcept;

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT constexpr float Dot(const Float4& val) const noexcept;

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept;
    DLLEXPORT constexpr float LengthSquared() const noexcept;

    // normalizes the vector //
    DLLEXPORT inline Float4 Normalize() const;

    // safe version of normalization //
    DLLEXPORT inline Float4 NormalizeSafe(const Float4& safer = Float4(1, 0, 0, 0)) const noexcept;

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept;

    // does linear interpolation between vectors and coefficient f, not limited to range
    // [0,1], courtesy of ozz-animation //
    DLLEXPORT constexpr Float4 Lerp(const Float4& other, float f) const noexcept;

    // does SPHERICAL interpolation between quaternions //
    DLLEXPORT inline Float4 Slerp(const Float4& other, float f) const;

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT constexpr bool Compare(const Float4& other, float tolerance) const noexcept;

    //! Converts a quaternion to Axis (and skips outputting the angle)
    //! \note Must be normalized
    DLLEXPORT inline Float3 ToAxis() const;

    //! Converts a quaternion to angle (and skips outputting the Axis)
    //! \note Must be normalized
    DLLEXPORT inline float ToAngle() const noexcept;

    //! Inverts a quaternion
    DLLEXPORT inline Float4 Inverse() const noexcept;

    //! Rotates a vector by this quaternion
    DLLEXPORT constexpr Float3 RotateVector(const Float3& vector) const;

    // ------------------------------------ //
    // functions to be compatible with ozz functions //
    // all zero values object //
    DLLEXPORT constexpr static Float4 zero() noexcept;

    // all ones //
    DLLEXPORT constexpr static Float4 one() noexcept;

    // unitary vectors //
    // x axis
    DLLEXPORT constexpr static Float4 x_axis() noexcept;

    // y axis
    DLLEXPORT constexpr static Float4 y_axis() noexcept;

    // z axis
    DLLEXPORT constexpr static Float4 z_axis() noexcept;

    // w axis
    DLLEXPORT constexpr static Float4 w_axis() noexcept;

    // ----------------- casts ------------------- //
    // Should this macro be replaced by a constexpr if in the cpp file?
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
    DLLEXPORT inline Float3 QuaternionToEuler() const noexcept;

    DLLEXPORT constexpr Float4 QuaternionMultiply(const Float4& other) const noexcept;

    DLLEXPORT constexpr Float4 QuaternionReverse() const noexcept;

    DLLEXPORT static constexpr Float4 IdentityQuaternion() noexcept;

    // Math from here: https://stackoverflow.com/questions/12435671/quaternion-lookat-function
    DLLEXPORT static inline Float4 QuaternionLookAt(
        const Float3& sourcepoint, const Float3& target);

    //! \note axis must be normalized
    //!
    //! This resource is a life saver:
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/index.htm
    DLLEXPORT static inline Float4 CreateQuaternionFromAxisAngle(
        const Float3& axis, float angle) noexcept;

	// NOT IMPLEMENTED,
    DLLEXPORT static inline Float4 CreateAxisAngleFromEuler(const Float3& angles);

    VALUE_TYPE(Float4);

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
