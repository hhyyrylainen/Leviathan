#include "Float4.h"

using namespace Leviathan;

const Float4 Float4::ColourBlack = Float4(0, 0, 0, 1);
const Float4 Float4::ColourWhite = Float4(1, 1, 1, 1);
const Float4 Float4::ColourTransparent = Float4(0, 0, 0, 0);

DLLEXPORT constexpr Float4::Float4(float f1, float f2, float f3, float f4) :
    X(f1), Y(f2), Z(f3), W(f4)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float4::Float4(Float2 floats, float f3, float f4) :
    X(floats.X), Y(floats.Y), Z(f3), W(f4)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float4::Float4(Float3 floats, float f4) :
    X(floats.X), Y(floats.Y), Z(floats.Z), W(f4)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float4::Float4(float data) : X(data), Y(data), Z(data), W(data)
{
    DO_NAN_CHECK;
}

DLLEXPORT inline bool Float4::HasInvalidValues() const noexcept
{
    return !std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z) || !std::isfinite(W);
}

DLLEXPORT inline void Float4::CheckForNans() const
{
    if(HasInvalidValues()) {
        DEBUG_BREAK;
        throw std::runtime_error("Float4 has NaNs (or infinites in it) in it!");
    }
}

DLLEXPORT constexpr float& Float4::operator[](int nindex)
{
    switch(nindex) {
	    case 0: return X;
	    case 1: return Y;
	    case 2: return Z;
	    case 3: return W;
        default: break;
    }

    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

DLLEXPORT constexpr Float4::operator float*() noexcept
{
    // this should be always confirmed to work //
    return &X;
}

// ------------------- Operators ----------------- //

DLLEXPORT constexpr Float4 Float4::operator+(const Float4& other) const noexcept
{
    return Float4(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
}

DLLEXPORT inline Float4& Float4::operator+=(const Float4& other) noexcept
{
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    W += other.W;
    return *this;
}

DLLEXPORT constexpr Float4 Float4::operator-(const Float4& other) const noexcept
{
    return Float4(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
}

DLLEXPORT inline Float4& Float4::operator-=(const Float4& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    W -= other.W;
    return *this;
}

DLLEXPORT constexpr Float4 Float4::operator-() const noexcept
{
    return Float4(-X, -Y, -Z, -W);
}

DLLEXPORT constexpr Float4 Float4::operator+() const noexcept
{
    return Float4(*this);
}

DLLEXPORT constexpr Float4 Float4::operator*(const Float4& other) const noexcept
{
    return Float4(X * other.X, Y * other.Y, Z * other.Z, W * other.W);
}

DLLEXPORT inline Float4& Float4::operator*=(const Float4& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    Z *= other.Z;
    W *= other.W;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float4 Float4::operator*(float val) const noexcept
{
    return Float4(X * val, Y * val, Z * val, W * val);
}

DLLEXPORT inline Float4& Float4::operator*=(float val) noexcept
{
    X *= val;
    Y *= val;
    Z *= val;
    W *= val;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float4 Float4::operator/(const Float4& other) const
{
    return Float4(X / other.X, Y / other.Y, Z / other.Z, W / other.W);
}

DLLEXPORT inline Float4& Float4::operator/=(const Float4& other)
{
    X /= other.X;
    Y /= other.Y;
    Z /= other.Z;
    W /= other.W;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float4 Float4::operator/(float val) const
{
    return Float4(X / val, Y / val, Z / val, W / val);
}

DLLEXPORT inline Float4& Float4::operator/=(float val)
{
    X /= val;
    Y /= val;
    Z /= val;
    W /= val;
    DO_NAN_CHECK;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Float4::operator<(const Float4& other) const noexcept
{
    return X < other.X && Y < other.Y && Z < other.Z && W < other.W;
}

DLLEXPORT constexpr bool Float4::operator<=(const Float4& other) const noexcept
{
    return X <= other.X && Y <= other.Y && Z <= other.Z && W <= other.W;
}

DLLEXPORT constexpr bool Float4::operator>(const Float4& other) const noexcept
{
    return X > other.X && Y > other.Y && Z > other.Z && W > other.W;
}

DLLEXPORT constexpr bool Float4::operator>=(const Float4& other) const noexcept
{
    return X >= other.X && Y >= other.Y && Z >= other.Z && W >= other.W;
}

DLLEXPORT constexpr bool Float4::operator==(const Float4& other) const noexcept
{
    return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
}

DLLEXPORT constexpr bool Float4::operator!=(const Float4& other) const noexcept
{
    return X != other.X && Y != other.Y && Z != other.Z && W != other.W;
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr float Float4::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr float Float4::GetY() const noexcept
{
    return Y;
}

DLLEXPORT constexpr float Float4::GetZ() const noexcept
{
    return Z;
}

DLLEXPORT constexpr float Float4::GetW() const noexcept
{
    return W;
}

DLLEXPORT inline void Float4::SetX(float val)
{
    X = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Float4::SetY(float val)
{
    Y = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Float4::SetZ(float val)
{
    Z = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Float4::SetW(float val)
{
    W = val;
    DO_NAN_CHECK;
}

DLLEXPORT constexpr float Float4::HAdd() const noexcept
{
    return X + Y + Z + W;
}

DLLEXPORT inline float Float4::HAddAbs() const noexcept
{
    return std::abs(X) + std::abs(Y) + std::abs(Z) + std::abs(W);
}

DLLEXPORT constexpr Float4 Float4::MinElements(const Float4& other) const noexcept
{
    return Float4(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y,
        Z < other.Z ? Z : other.Z, W < other.W ? W : other.W);
}

DLLEXPORT constexpr Float4 Float4::MaxElements(const Float4& other) const noexcept
{
    return Float4(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y,
        Z > other.Z ? Z : other.Z, W > other.W ? W : other.W);
}

DLLEXPORT constexpr Float4 Float4::Clamp(const Float4& min, const Float4& max) const noexcept
{
    const Float4 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr float Float4::Dot(const Float4& val) const noexcept
{
    return X * val.X + Y * val.Y + Z * val.Z + W * val.W;
}

DLLEXPORT inline float Float4::Length() const noexcept
{
    return sqrt(X * X + Y * Y + Z * Z + W * W);
}

DLLEXPORT constexpr float Float4::LengthSquared() const noexcept
{
    return X * X + Y * Y + Z * Z + W * W;
}

DLLEXPORT inline Float4 Float4::Normalize() const
{
    const float length = Length();

    if(length == 0) {
        // Returns an identity quaternion
        return Float4(0, 0, 0, 1);
    }

    return Float4(X / length, Y / length, Z / length, W / length);
}

DLLEXPORT inline Float4 Float4::NormalizeSafe(const Float4& safer) const noexcept
{
    // security //
    const float len = X * X + Y * Y + Z * Z + W * W;
    if(len == 0) {
        return safer;
    }

    const float length = sqrt(len);
    return Float4(X / length, Y / length, Z / length, W / length);
}

DLLEXPORT inline bool Float4::IsNormalized() const noexcept
{
    // is absolute -1.f under normalization tolerance //
    return fabs(X * X + Y * Y + Z * Z + W * W - 1.0f) < NORMALIZATION_TOLERANCE;
}

DLLEXPORT constexpr Float4 Float4::Lerp(const Float4& other, float f) const noexcept
{
    return Float4((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z,
        (other.W - W) * f + W);
}

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
        const float angle = acosf(dot);
        return ((*this) * sinf(angle * (1 - f)) + quaternion3 * sinf(angle * f)) / sinf(angle);

    } else {
        // small angle, linear interpolation will be fine //
        return this->Lerp(quaternion3, f);
    }
}

DLLEXPORT constexpr bool Float4::Compare(const Float4& other, float tolerance) const noexcept
{
    const Float4 difference = (*this) - other;
    return difference.Dot(difference) < tolerance * tolerance;
}

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

DLLEXPORT inline float Float4::ToAngle() const noexcept
{
    return 2 * std::acos(W);
}

DLLEXPORT inline Float4 Float4::Inverse() const noexcept
{
    const auto length = Length();
    if(length > 0.0f) {
        const auto inverted = 1.0f / length;
        return Float4(-X * inverted, -Y * inverted, -Z * inverted, W * inverted);
    } else {
        // Invalid inversing
        return Float4(0.f);
    }
}

DLLEXPORT constexpr Float3 Float4::RotateVector(const Float3& vector) const
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
    Float3 qvec(X, Y, Z);
    Float3 uv = qvec.Cross(vector) * 2.0f * W;
    Float3 uuv = qvec.Cross(uv) * 2.0f;

    return vector + uv + uuv;
}

// ------------------------------------ //

DLLEXPORT constexpr Float4 Float4::zero() noexcept
{
    return Float4(0.f);
}

DLLEXPORT constexpr Float4 Float4::one() noexcept
{
    return Float4(1.f);
}

DLLEXPORT constexpr Float4 Float4::x_axis() noexcept
{
    return Float4(1.f, 0.f, 0.f, 0.f);
}

DLLEXPORT constexpr Float4 Float4::y_axis() noexcept
{
    return Float4(0.f, 1.f, 0.f, 0.f);
}

DLLEXPORT constexpr Float4 Float4::z_axis() noexcept
{
    return Float4(0.f, 0.f, 1.f, 0.f);
}

DLLEXPORT constexpr Float4 Float4::w_axis() noexcept
{
    return Float4(0.f, 0.f, 0.f, 1.f);
}

// ------------------------------------ //

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
    const float cosx = cosf(0.5f * angles.X);
    const float cosy = cosf(0.5f * angles.Y);
    const float cosz = cosf(0.5f * angles.Z);

    const float sinx = sinf(0.5f * angles.X);
    const float siny = sinf(0.5f * angles.Y);
    const float sinz = sinf(0.5f * angles.Z);

    return Float4(
    // compute quaternion //
	    cosz * cosy * sinx - sinz * siny * cosx,
	    cosz * siny * cosx + sinz * cosy * sinx,
	    sinz * cosy * cosx - cosz * siny * sinx,
        cosz * cosy * cosx * sinz * siny * sinx
	);
}

DLLEXPORT inline Float3 Float4::QuaternionToEuler() const noexcept
{
    const float test = X * Y + Z * W;

    if(test > 0.499) {
        // Singularity at north pole
        return Float3(2 * atan2(X, W), PI / 2, 0);
    }

    if(test < -0.499) {
        // Singularity at south pole
        return Float3(-2 * atan2(X, W), -PI / 2, 0);
    }

    const float sqx = X * X;
    const float sqy = Y * Y;
    const float sqz = Z * Z;

    return Float3(
		atan2(2 * Y * W - 2 * X * Z, 1 - 2 * sqy - 2 * sqz),
		asin(2 * test),
        atan2(2 * X * W - 2 * Y * Z, 1 - 2 * sqx - 2 * sqz)
	);
}

DLLEXPORT constexpr Float4 Float4::QuaternionMultiply(const Float4& other) const noexcept
{
    return Float4(
	    X * other.X + X * other.W + Y * other.Z - Z * other.Y,
	    W * other.Y - X * other.Z + Y * other.W + Z * other.X,
	    W * other.Z + X * other.Y - Y * other.X + Z * other.W,
	    W * other.W - X * other.X - Y * other.Y - Z * other.Z
	);
}

DLLEXPORT constexpr Float4 Float4::QuaternionReverse() const noexcept
{
    // reverse vector //
    return Float4(-X, -Y, -Z, W);
}

DLLEXPORT constexpr Float4 Float4::IdentityQuaternion() noexcept
{
    return Float4(0.f, 0.f, 0.f, 1.f);
}

DLLEXPORT inline Float4 Float4::QuaternionLookAt(
    const Float3& sourcepoint, const Float3& target)
{
    const auto forward = (target - sourcepoint).NormalizeSafe();
    const float dot = Float3::UnitVForward.Dot(forward);

    if(std::abs(dot - (-1.0f)) < 0.000001f) {
        // Assumes up is Float3(0, 1, 0)
        return Float4(Float3::UnitVUp, 3.1415926535897932f);
    }

    if(std::abs(dot - 1.0f) < 0.000001f) {
        return Float4::IdentityQuaternion();
    }

    const float rotAngle = std::acos(dot);
    const Float3 rotAxis = Float3::UnitVForward.Cross(forward).Normalize();
    return CreateQuaternionFromAxisAngle(rotAxis, rotAngle);
}

DLLEXPORT inline Float4 Float4::CreateQuaternionFromAxisAngle(const Float3& axis, float angle) noexcept
{
    const auto s = std::sin(angle / 2.0);
    const auto w = std::cos(angle / 2.0);
    return Float4(axis * s, w);
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
