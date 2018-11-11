#include "Float3.h"

using namespace Leviathan;

const Float3 Float3::UnitVForward = Float3(0.f, 0.f, -1.f);
const Float3 Float3::UnitVUp = Float3(0.f, 1.f, 0.f);
const Float3 Float3::Zeroed = Float3::zero();

DLLEXPORT Float3::Float3(float x, float y, float z)
{
    X = x;
    Y = y;
    Z = z;
    DO_NAN_CHECK;
}
DLLEXPORT Float3::Float3(Float2 floats, float z)
{
    X = floats.X;
    Y = floats.Y;
    Z = z;
    DO_NAN_CHECK;
}
DLLEXPORT Float3::Float3(float data)
{
    X = Y = Z = data;
    DO_NAN_CHECK;
}
DLLEXPORT Float3::Float3(const Int3& values)
{
    X = static_cast<float>(values.X);
    Y = static_cast<float>(values.Y);
    Z = static_cast<float>(values.Z);

    DO_NAN_CHECK;
}
DLLEXPORT Float3::Float3(const Float3& other)
{
    // copy values //
    X = other.X;
    Y = other.Y;
    Z = other.Z;

    DO_NAN_CHECK;
}

DLLEXPORT inline bool Float3::HasInvalidValues() const
{
    if(!std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z)) {
        return true;
    }

    return false;
}

DLLEXPORT inline void Float3::CheckForNans()
{
    if(HasInvalidValues()) {
        DEBUG_BREAK;
        throw std::runtime_error("Float3 has NaNs (or infinites in it) in it!");
    }
}

// access operator //
DLLEXPORT inline float& Float3::operator[](const int& nindex)
{
    switch(nindex) {
    case 0: return X;
    case 1: return Y;
    case 2: return Z;
    }
    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

// ------------------- Operators ----------------- //
// add elements //
DLLEXPORT inline Float3 Float3::operator+(const Float3& val) const
{
    return Float3(X + val.X, Y + val.Y, Z + val.Z);
}
DLLEXPORT inline Float3& Float3::operator+=(const Float3& val)
{
    X += val.X;
    Y += val.Y;
    Z += val.Z;
    return *this;
}
DLLEXPORT inline Float3& Float3::operator-=(const Float3& val)
{
    X -= val.X;
    Y -= val.Y;
    Z -= val.Z;
    return *this;
}

// subtracts all elements //
DLLEXPORT inline Float3 Float3::operator-(const Float3& val) const
{
    return Float3(X - val.X, Y - val.Y, Z - val.Z);
}
// negates all elements //
DLLEXPORT inline Float3 Float3::operator-() const
{
    return Float3(-X, -Y, -Z);
}
// multiplies elements together //
DLLEXPORT inline Float3 Float3::operator*(const Float3& val) const
{
    return Float3(X * val.X, Y * val.Y, Z * val.Z);
}
// Divides all elements by float //
DLLEXPORT inline Float3 Float3::operator/(const float& val) const
{
    return Float3(X / val, Y / val, Z / val);
}
DLLEXPORT inline Float3& Float3::operator/=(const float& val)
{
    X /= val;
    Y /= val;
    Z /= val;
    DO_NAN_CHECK;
    return *this;
}
// multiply  by scalar f //
DLLEXPORT inline Float3 Float3::operator*(float f) const
{
    return Float3(X * f, Y * f, Z * f);
}
DLLEXPORT inline Float3& Float3::operator*=(float f)
{
    X *= f;
    Y *= f;
    Z *= f;
    DO_NAN_CHECK;
    return *this;
}
// divides all elements //
DLLEXPORT inline Float3 Float3::operator/(const Float3& val) const
{
    return Float3(X / val.X, Y / val.Y, Z / val.Z);
}
// ---- comparison operators ---- //
// element by element comparison with operators //
DLLEXPORT inline bool Float3::operator<(const Float3& other) const
{
    return X < other.X && Y < other.Y && Z < other.Z;
};
DLLEXPORT inline bool Float3::operator<=(const Float3& other) const
{
    return X <= other.X && Y <= other.Y && Z <= other.Z;
};
DLLEXPORT inline bool Float3::operator>(const Float3& other) const
{
    return X > other.X && Y > other.Y && Z > other.Z;
};
DLLEXPORT inline bool Float3::operator>=(const Float3& other) const
{
    return X >= other.X && Y >= other.Y && Z > other.Z;
};
DLLEXPORT inline bool Float3::operator==(const Float3& other) const
{
    return X == other.X && Y == other.Y && Z == other.Z;
};
DLLEXPORT inline bool Float3::operator!=(const Float3& other) const
{
    return !(*this == other);
};
// ------------------ Functions ------------------ //
DLLEXPORT inline float Float3::GetX() const
{
    return X;
};
DLLEXPORT inline float Float3::GetY() const
{
    return Y;
};
DLLEXPORT inline float Float3::GetZ() const
{
    return Z;
};
DLLEXPORT inline void Float3::SetX(const float& val)
{
    X = val;
    DO_NAN_CHECK;
};
DLLEXPORT inline void Float3::SetY(const float& val)
{
    Y = val;
    DO_NAN_CHECK;
};
DLLEXPORT inline void Float3::SetZ(const float& val)
{
    Z = val;
    DO_NAN_CHECK;
};

// add all elements together //
DLLEXPORT inline float Float3::HAdd() const
{
    return X + Y + Z;
}
// Add all elements together absoluted (abs()) //
DLLEXPORT inline float Float3::HAddAbs() const
{
    return std::abs(X) + std::abs(Y) + std::abs(Z);
}
// getting min and max of objects //
DLLEXPORT inline Float3 Float3::MinElements(const Float3& other) const
{
    return Float3(
        X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
}
DLLEXPORT inline Float3 Float3::MaxElements(const Float3& other) const
{
    return Float3(
        X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
}
// value clamping //
DLLEXPORT inline Float3 Float3::Clamp(const Float3& min, const Float3& max)
{
    const Float3 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

DLLEXPORT inline Float3 Float3::DegreesToRadians()
{

    return Float3(X * DEGREES_TO_RADIANS, Y * DEGREES_TO_RADIANS, Z * DEGREES_TO_RADIANS);
}

// ----------------- Vector math ------------------- //
// dot product of the vectors //
DLLEXPORT inline float Float3::Dot(const Float3& val) const
{
    return X * val.X + Y * val.Y + Z * val.Z;
}
DLLEXPORT inline Float3 Float3::Cross(const Float3& val) const
{
    return Float3(Y * val.Z - val.Y * Z, Z * val.X - val.Z * X, X * val.Y - val.X * Y);
}
// length of the vector //
DLLEXPORT inline float Float3::Length() const
{
    return sqrt(X * X + Y * Y + Z * Z);
}
DLLEXPORT inline float Float3::LengthSquared() const
{
    return X * X + Y * Y + Z * Z;
}
// normalizes the vector //
DLLEXPORT inline Float3 Float3::Normalize() const
{
    const float length = Length();
    if(length == 0)
        return Float3(0, 0, 0);
    return Float3(X / length, Y / length, Z / length);
}
// safe version of normalization //
DLLEXPORT inline Float3 Float3::NormalizeSafe(const Float3& safer) const
{
    // security //
    // assert(safer.IsNormalized() && "safer not normalized");
    const float len = X * X + Y * Y + Z * Z;
    if(len == 0) {
        return safer;
    }
    const float length = sqrt(len);
    return Float3(X / length, Y / length, Z / length);
}
// checks is the vector normalized //
DLLEXPORT inline bool Float3::IsNormalized() const
{
    // is absolute -1.f under normalization tolerance //
    return fabs(X * X + Y * Y + Z * Z - 1.0f) < NORMALIZATION_TOLERANCE;
}
// does linear interpolation between vectors and coefficient f, not limited to range
// [0,1], courtesy of ozz-animation //
DLLEXPORT inline Float3 Float3::Lerp(const Float3& other, float f) const
{
    return Float3((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z);
}
// compares distance between vectors to tolerance, returns true if less //
DLLEXPORT inline bool Float3::Compare(const Float3& other, float tolerance) const
{
    const Float3 difference = (*this) - other;
    return difference.Dot(difference) < tolerance * tolerance;
}

DLLEXPORT inline Float3 Float3::CreateVectorFromAngles(
    const float& yaw, const float& pitch)
{
    return Float3(-sin(yaw * DEGREES_TO_RADIANS), sin(pitch * DEGREES_TO_RADIANS),
        -cos(yaw * DEGREES_TO_RADIANS))
        .NormalizeSafe(Zeroed);
}
// ------------------------------------ //
// functions to be compatible with ozz functions //
// all zero values object //
DLLEXPORT inline Float3 Float3::zero()
{
    return Float3(0.f, 0.f, 0.f);
}
// all ones //
DLLEXPORT inline Float3 Float3::one()
{
    return Float3(1.f, 1.f, 1.f);
}
// unitary vectors //
// x axis
DLLEXPORT inline Float3 Float3::x_axis()
{
    return Float3(1.f, 0.f, 0.f);
}

// y axis
DLLEXPORT inline Float3 Float3::y_axis()
{
    return Float3(0.f, 1.f, 0.f);
}

// z axis
DLLEXPORT inline Float3 Float3::z_axis()
{
    return Float3(0.f, 0.f, 1.f);
}
// ----------------- casts ------------------- //
#ifdef LEVIATHAN_FULL
DLLEXPORT Float3::Float3(const Ogre::Vector3& vec)
{
    // copy values //
    X = vec.x;
    Y = vec.y;
    Z = vec.z;
    DO_NAN_CHECK;
}

DLLEXPORT inline Float3::operator Ogre::Vector3() const
{
    return Ogre::Vector3(X, Y, Z);
}

DLLEXPORT Float3::Float3(const btVector3& vec)
{
    // copy values //
    X = vec.x();
    Y = vec.y();
    Z = vec.z();
    DO_NAN_CHECK;
}

DLLEXPORT inline Float3::operator btVector3() const
{
    return btVector3(X, Y, Z);
}
#endif // LEVIATHAN_USING_OGRE

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Float3& value)
{
    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << "]";
    return stream;
}
