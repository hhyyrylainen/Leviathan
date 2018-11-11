#include "Float4.h"

using namespace Leviathan;

const Float4 Float4::ColourBlack = Float4(0, 0, 0, 1);
const Float4 Float4::ColourWhite = Float4(1, 1, 1, 1);
const Float4 Float4::ColourTransparent = Float4(0, 0, 0, 0);

DLLEXPORT Float4::Float4(float f1, float f2, float f3, float f4)
{
    X = f1;
    Y = f2;
    Z = f3;
    W = f4;
    DO_NAN_CHECK;
}
DLLEXPORT Float4::Float4(Float2 floats, float f3, float f4)
{
    X = floats.X;
    Y = floats.Y;
    Z = f3;
    W = f4;
    DO_NAN_CHECK;
}
DLLEXPORT Float4::Float4(Float3 floats, float f4)
{
    X = floats.X;
    Y = floats.Y;
    Z = floats.Z;
    W = f4;
    DO_NAN_CHECK;
}
DLLEXPORT Float4::Float4(float val)
{
    X = Y = Z = W = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline bool Float4::HasInvalidValues() const
{
    if(!std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z) || !std::isfinite(W)) {
        return true;
    }

    return false;
}

DLLEXPORT inline void Float4::CheckForNans()
{
    if(HasInvalidValues()) {
        DEBUG_BREAK;
        throw std::runtime_error("Float4 has NaNs (or infinites in it) in it!");
    }
}

// access operator //
DLLEXPORT inline float& Float4::operator[](const int& nindex)
{
    switch(nindex) {
    case 0: return X;
    case 1: return Y;
    case 2: return Z;
    case 3: return W;
    }

    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

//! return first value of {X, Y, Z, W} as a pointer
DLLEXPORT inline Float4::operator float*()
{
    // this should be always confirmed to work //
    return &X;
}

// ------------------- Operators ----------------- //
// add elements //
DLLEXPORT inline Float4 Float4::operator+(const Float4& val) const
{
    return Float4(X + val.X, Y + val.Y, Z + val.Z, W + val.W);
}
// subtracts all elements //
DLLEXPORT inline Float4 Float4::operator-(const Float4& val) const
{
    return Float4(X - val.X, Y - val.Y, Z - val.Z, W - val.W);
}
// negates all elements //
DLLEXPORT inline Float4 Float4::operator-() const
{
    return Float4(-X, -Y, -Z, -W);
}
// multiplies elements together //
DLLEXPORT inline Float4 Float4::operator*(const Float4& val) const
{
    return Float4(X * val.X, Y * val.Y, Z * val.Z, W * val.W);
}
// multiply  by scalar f //
DLLEXPORT inline Float4 Float4::operator*(float f) const
{
    return Float4(X * f, Y * f, Z * f, W * f);
}
// divides all elements //
DLLEXPORT inline Float4 Float4::operator/(const Float4& val) const
{
    return Float4(X / val.X, Y / val.Y, Z / val.Z, W / val.W);
}
// divides by float //
DLLEXPORT inline Float4 Float4::operator/(float f) const
{
    return Float4(X / f, Y / f, Z / f, W / f);
}
// ---- comparison operators ---- //
// element by element comparison with operators //
DLLEXPORT inline bool Float4::operator<(const Float4& other) const
{
    return !(*this == other);
};
DLLEXPORT inline bool Float4::operator>(const Float4& other) const
{
    return !(*this == other);
};
DLLEXPORT inline bool Float4::operator==(const Float4& other) const
{
    return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
};
DLLEXPORT inline bool Float4::operator!=(const Float4& other) const
{
    return X != other.X && Y != other.Y && Z != other.Z && W != other.W;
};
// ------------------ Functions ------------------ //
DLLEXPORT inline float Float4::GetX() const
{
    return X;
};
DLLEXPORT inline float Float4::GetY() const
{
    return Y;
};
DLLEXPORT inline float Float4::GetZ() const
{
    return Z;
};
DLLEXPORT inline float Float4::GetW() const
{
    return W;
};
DLLEXPORT inline void Float4::SetX(const float& val)
{
    X = val;
    DO_NAN_CHECK;
};
DLLEXPORT inline void Float4::SetY(const float& val)
{
    Y = val;
    DO_NAN_CHECK;
};
DLLEXPORT inline void Float4::SetZ(const float& val)
{
    Z = val;
    DO_NAN_CHECK;
};
DLLEXPORT inline void Float4::SetW(const float& val)
{
    W = val;
    DO_NAN_CHECK;
};

// add all elements together //
DLLEXPORT inline float Float4::HAdd() const
{
    return X + Y + Z + W;
}
// Add all elements together after abs() is called on each element //
DLLEXPORT inline float Float4::HAddAbs() const
{
    return std::abs(X) + std::abs(Y) + std::abs(Z) + std::abs(W);
}
// getting min and max of objects //
DLLEXPORT inline Float4 Float4::MinElements(const Float4& other) const
{
    return Float4(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y,
        Z < other.Z ? Z : other.Z, W < other.W ? W : other.W);
}
DLLEXPORT inline Float4 Float4::MaxElements(const Float4& other) const
{
    return Float4(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y,
        Z > other.Z ? Z : other.Z, W > other.W ? W : other.W);
}
// value clamping //
DLLEXPORT inline Float4 Float4::Clamp(const Float4& min, const Float4& max)
{
    const Float4 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //
// dot product of the vectors //
DLLEXPORT inline float Float4::Dot(const Float4& val) const
{
    return X * val.X + Y * val.Y + Z * val.Z + W * val.W;
}

// length of the vector //
DLLEXPORT inline float Float4::Length() const
{
    return sqrt(X * X + Y * Y + Z * Z + W * W);
}

// normalizes the vector //
DLLEXPORT inline Float4 Float4::Normalize() const
{
    const float length = Length();

    if(length == 0) {
        // Returns an identity quaternion
        return Float4(0, 0, 0, 1);
    }

    return Float4(X / length, Y / length, Z / length, W / length);
}
// safe version of normalization //
DLLEXPORT inline Float4 Float4::NormalizeSafe(const Float4& safer) const
{
    // security //
    const float len = X * X + Y * Y + Z * Z + W * W;
    if(len == 0) {
        return safer;
    }

    const float length = sqrt(len);
    return Float4(X / length, Y / length, Z / length, W / length);
}
// checks is the vector normalized //
DLLEXPORT inline bool Float4::IsNormalized() const
{
    // is absolute -1.f under normalization tolerance //
    return fabs(X * X + Y * Y + Z * Z + W * W - 1.0f) < NORMALIZATION_TOLERANCE;
}
// does linear interpolation between vectors and coefficient f,
// not limited to range [0,1], courtesy of ozz-animation //
DLLEXPORT inline Float4 Float4::Lerp(const Float4& other, float f) const
{
    return Float4((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z,
        (other.W - W) * f + W);
}

// does SPHERICAL interpolation between quaternions //
DLLEXPORT inline Float4 Float4::Slerp(const Float4& other, float f) const
{
    // extra quaternion for calculations //
    Float4 quaternion3;

    // dot product of both //
    float dot = this->Dot(other);

    if(dot < 0) {

        dot = -dot;
        quaternion3 = -other;
    } else {
        quaternion3 = other;
    }

    if(dot < 0.95f) {

        float angle = acosf(dot);
        return ((*this) * sinf(angle * (1 - f)) + quaternion3 * sinf(angle * f)) / sinf(angle);

    } else {
        // small angle, linear interpolation will be fine //
        return this->Lerp(quaternion3, f);
    }
}

// compares distance between vectors to tolerance, returns true if less //
DLLEXPORT inline bool Float4::Compare(const Float4& other, float tolerance) const
{
    const Float4 difference = (*this) - other;
    return difference.Dot(difference) < tolerance * tolerance;
}

//! Converts a quaternion to Axis (and skips outputting the angle)
//! \note Must be normalized
DLLEXPORT inline Float3 Float4::ToAxis() const
{

    const auto s = std::sqrt(1 - std::pow(W, 2));
    // Avoid division by zero (this small axis it can be basically converted directly)
    if(s > 0) {
        return Float3(X / s, Y / s, Z / s);
    } else {
        return Float3(X, Y, Z);
    }
}

//! Converts a quaternion to angle (and skips outputting the Axis)
//! \note Must be normalized
DLLEXPORT inline float Float4::ToAngle() const
{

    return 2 * std::acos(W);
}

//! Inverts a quaternion
DLLEXPORT inline Float4 Float4::Inverse() const
{

    const auto length = Length();
    if(length > 0.0f) {

        const auto inverted = 1.0f / length;
        return Float4(-X * inverted, -Y * inverted, -Z * inverted, W * inverted);
    } else {
        // Invalid inversing
        return Float4(0);
    }
}

//! Rotates a vector by this quaternion
DLLEXPORT inline Float3 Float4::RotateVector(const Float3& vector) const
{
    // // Alternative from
    // //
    // https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
    // const auto u = Float3(X, Y, Z);

    // // Complex math going on
    // return u * 2.0f * u.Dot(vector)
    //     + vector * (W*W - u.Dot(u))
    //     + u.Cross(vector) * 2.0f * W;

    // Math taken from Ogre::Quaternion
    Float3 uv;
    Float3 uuv;
    Float3 qvec(X, Y, Z);
    uv = qvec.Cross(vector);
    uuv = qvec.Cross(uv);
    uv *= 2.0f * W;
    uuv *= 2.0f;

    return vector + uv + uuv;
}

// ------------------------------------ //
// All zeros //
DLLEXPORT inline Float4 Float4::zero()
{
    return Float4(0.f, 0.f, 0.f, 0.f);
}

// all ones //
DLLEXPORT inline Float4 Float4::one()
{
    return Float4(1.f, 1.f, 1.f, 1.f);
}
// unitary vectors for ozz support //
// x
DLLEXPORT inline Float4 Float4::x_axis()
{
    return Float4(1.f, 0.f, 0.f, 0.f);
}
// y
DLLEXPORT inline Float4 Float4::y_axis()
{
    return Float4(0.f, 1.f, 0.f, 0.f);
}
// z
DLLEXPORT inline Float4 Float4::z_axis()
{
    return Float4(0.f, 0.f, 1.f, 0.f);
}
// w
DLLEXPORT inline Float4 Float4::w_axis()
{
    return Float4(0.f, 0.f, 0.f, 1.f);
}

#ifdef LEVIATHAN_FULL
DLLEXPORT Float4::Float4(const Ogre::Quaternion& quat)
{
    // copy values //
    X = quat.x;
    Y = quat.y;
    Z = quat.z;
    W = quat.w;
    DO_NAN_CHECK;
}

DLLEXPORT Float4::Float4(const Ogre::ColourValue& colour)
{
    // copy values //
    X = colour.r;
    Y = colour.g;
    Z = colour.b;
    W = colour.a;
    DO_NAN_CHECK;
}

DLLEXPORT inline Float4::operator Ogre::Quaternion() const
{
    // Ogre has these in different order
    return Ogre::Quaternion(W, X, Y, Z);
}

DLLEXPORT inline Float4::operator Ogre::ColourValue() const
{
    return Ogre::ColourValue(X, Y, Z, W);
}

DLLEXPORT inline Float4::operator Ogre::Vector4() const
{
    return Ogre::Vector4(X, Y, Z, W);
}

DLLEXPORT Float4::Float4(const btQuaternion& colour)
{
    // copy values //
    X = colour.x();
    Y = colour.y();
    Z = colour.z();
    W = colour.w();
    DO_NAN_CHECK;
}

DLLEXPORT inline Float4::operator btQuaternion() const
{
    return btQuaternion(X, Y, Z, W);
}
#endif // LEVIATHAN_USING_OGRE

// ----------------- Quaternions ------------------- //
DLLEXPORT inline Float4 Float4::CreateQuaternionFromAngles(const Float3& angles)
{
    // multiplied by 0.5 to get double the value //
    float cosx = cosf(0.5f * angles.X);
    float cosy = cosf(0.5f * angles.Y);
    float cosz = cosf(0.5f * angles.Z);

    float sinx = sinf(0.5f * angles.X);
    float siny = sinf(0.5f * angles.Y);
    float sinz = sinf(0.5f * angles.Z);


    Float4 quaternion((Float4)0);
    // compute quaternion //
    quaternion.X = cosz * cosy * sinx - sinz * siny * cosx;
    quaternion.Y = cosz * siny * cosx + sinz * cosy * sinx;
    quaternion.Z = sinz * cosy * cosx - cosz * siny * sinx;
    quaternion.W = cosz * cosy * cosx * sinz * siny * sinx;

    return quaternion;
}

//! \note This quaternion has to be normalized
//! \see
//! http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
DLLEXPORT inline Float3 Float4::QuaternionToEuler() const
{

    float test = X * Y + Z * W;

    if(test > 0.499) {
        // Singularity at north pole
        return Float3(2 * atan2(X, W), PI / 2, 0);
    }

    if(test < -0.499) {

        // Singularity at south pole
        return Float3(-2 * atan2(X, W), -PI / 2, 0);
    }

    float sqx = X * X;
    float sqy = Y * Y;
    float sqz = Z * Z;

    return Float3(atan2(2 * Y * W - 2 * X * Z, 1 - 2 * sqy - 2 * sqz), asin(2 * test),
        atan2(2 * X * W - 2 * Y * Z, 1 - 2 * sqx - 2 * sqz));
}

DLLEXPORT inline Float4 Float4::QuaternionMultiply(const Float4& other) const
{

    Float4 result;

    result.X = X * other.X + X * other.W + Y * other.Z - Z * other.Y;
    result.Y = W * other.Y - X * other.Z + Y * other.W + Z * other.X;
    result.Z = W * other.Z + X * other.Y - Y * other.X + Z * other.W;
    result.W = W * other.W - X * other.X - Y * other.Y - Z * other.Z;

    return result;
}

DLLEXPORT inline Float4 Float4::QuaternionReverse() const
{
    // reverse vector //
    return Float4(-X, -Y, -Z, W);
}

DLLEXPORT inline Float4 Float4::IdentityQuaternion()
{
    return Float4(0, 0, 0, 1);
}

// Math from here: https://stackoverflow.com/questions/12435671/quaternion-lookat-function
DLLEXPORT inline Float4 Float4::QuaternionLookAt(
    const Float3& sourcepoint, const Float3& target)
{
    const auto forward = (target - sourcepoint).NormalizeSafe();
    const float dot = Float3::UnitVForward.Dot(forward);

    if(std::abs(dot - (-1.0f)) < 0.000001f) {
        // Assumes up is Float3(0, 1, 0)
        return Float4(
            Float3::UnitVUp.X, Float3::UnitVUp.Y, Float3::UnitVUp.Z, 3.1415926535897932f);
    }
    if(std::abs(dot - 1.0f) < 0.000001f) {
        return Float4::IdentityQuaternion();
    }

    const float rotAngle = std::acos(dot);
    const Float3 rotAxis = Float3::UnitVForward.Cross(forward).Normalize();
    return CreateQuaternionFromAxisAngle(rotAxis, rotAngle);
}

//! \note axis must be normalized
//!
//! This resource is a life saver:
//! http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/index.htm
DLLEXPORT inline Float4 Float4::CreateQuaternionFromAxisAngle(const Float3& axis, float angle)
{
    const auto s = std::sin(angle / 2.0);
    const auto x = axis.X * s;
    const auto y = axis.Y * s;
    const auto z = axis.Z * s;
    const auto w = std::cos(angle / 2.0);
    return Float4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z),
        static_cast<float>(w));
}

DLLEXPORT inline Float4 Float4::CreateAxisAngleFromEuler(const Float3& angles)
{

    throw std::exception();
    // return Float4();
}

DLLEXPORT const Float4& Float4::GetColourBlack()
{
    return ColourBlack;
}

DLLEXPORT const Float4& Float4::GetColourWhite()
{
    return ColourWhite;
}

DLLEXPORT const Float4& Float4::GetColourTransparent()
{
    return ColourTransparent;
}

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Float4& value)
{
    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << ", " << value.W << "]";
    return stream;
}
