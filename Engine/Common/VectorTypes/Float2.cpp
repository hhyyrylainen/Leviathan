#include "Float2.h"

using namespace Leviathan;

DLLEXPORT inline Float2::Float2(float x, float y)
{
    X = x;
    Y = y;
    DO_NAN_CHECK;
}
DLLEXPORT inline Float2::Float2(float both)
{
    X = Y = both;
    DO_NAN_CHECK;
}

DLLEXPORT inline bool Float2::HasInvalidValues() const
{
    if(!std::isfinite(X) || !std::isfinite(Y)) {
        return true;
    }

    return false;
}

DLLEXPORT inline void Float2::CheckForNans()
{
    if(HasInvalidValues()) {
        DEBUG_BREAK;
        throw std::runtime_error("Float2 has NaNs (or infinites in it) in it!");
    }
}

// access operator //
DLLEXPORT inline float& Float2::operator[](const int& nindex)
{
    switch(nindex) {
    case 0: return X;
    case 1: return Y;
    }

    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

// ------------------- Operators ----------------- //
// add elements //
DLLEXPORT inline Float2 Float2::operator+(const Float2& val) const
{
    return Float2(X + val.X, Y + val.Y);
}

DLLEXPORT inline Float2* Float2::operator+=(const Float2& val)
{
    X += val.X;
    Y += val.Y;
    return this;
}
// subtracts all elements //
DLLEXPORT inline Float2 Float2::operator-(const Float2& val) const
{
    return Float2(X - val.X, Y - val.Y);
}
// negates all elements //
DLLEXPORT inline Float2 Float2::operator-() const
{
    return Float2(-X, -Y);
}
// multiplies elements together //
DLLEXPORT inline Float2 Float2::operator*(const Float2& val) const
{
    return Float2(X * val.X, Y * val.Y);
}
// multiply  by scalar f //
DLLEXPORT inline Float2 Float2::operator*(float f) const
{
    return Float2(X * f, Y * f);
}

DLLEXPORT inline Float2& Float2::operator*=(float f)
{
    X *= f;
    Y *= f;
    DO_NAN_CHECK;
    return *this;
}
// divides all elements //
DLLEXPORT inline Float2 Float2::operator/(const Float2& val) const
{
    return Float2(X / val.X, Y / val.Y);
}
// divides by float //
DLLEXPORT inline Float2 Float2::operator/(float f) const
{
    return Float2(X / f, Y / f);
}
// ---- comparison operators ---- //
// element by element comparison with operators //
DLLEXPORT inline bool Float2::operator<(const Float2& other) const
{
    return X < other.X && Y < other.Y;
};
DLLEXPORT inline bool Float2::operator<=(const Float2& other) const
{
    return X <= other.X && Y <= other.Y;
};
DLLEXPORT inline bool Float2::operator>(const Float2& other) const
{
    return X > other.X && Y > other.Y;
};
DLLEXPORT inline bool Float2::operator>=(const Float2& other) const
{
    return X >= other.X && Y >= other.Y;
};
DLLEXPORT inline bool Float2::operator==(const Float2& other) const
{
    return X == other.X && Y == other.Y;
};
DLLEXPORT inline bool Float2::operator!=(const Float2& other) const
{
    return X != other.X && Y != other.Y;
};
// ------------------ Functions ------------------ //
DLLEXPORT inline float Float2::GetX() const
{
    return X;
}
DLLEXPORT inline float Float2::GetY() const
{
    return Y;
}
DLLEXPORT inline void Float2::SetX(const float& val)
{
    X = val;
    DO_NAN_CHECK;
};
DLLEXPORT inline void Float2::SetY(const float& val)
{
    Y = val;
    DO_NAN_CHECK;
};

// add all elements together //
DLLEXPORT inline float Float2::HAdd() const
{
    return X + Y;
}
// Add all elements together after abs() is called on each element //
DLLEXPORT inline float Float2::HAddAbs() const
{
    return std::fabs(X) + std::fabs(Y);
}
// getting min and max of objects //
DLLEXPORT inline Float2 Float2::MinElements(const Float2& other) const
{
    return Float2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
}
DLLEXPORT inline Float2 Float2::MaxElements(const Float2& other) const
{
    return Float2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
}
// value clamping //
DLLEXPORT inline Float2 Float2::Clamp(const Float2& min, const Float2& max)
{
    const Float2 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //
// dot product of the vectors //
DLLEXPORT inline float Float2::Dot(const Float2& val) const
{
    return X * val.X + Y * val.Y;
}
// length of the vector //
DLLEXPORT inline float Float2::Length() const
{
    return sqrt(X * X + Y * Y);
}
// normalizes the vector //
DLLEXPORT inline Float2 Float2::Normalize() const
{
    const float length = Length();
    if(length == 0)
        return Float2(0, 0);
    return Float2(X / length, Y / length);
}
// safe version of normalization //
DLLEXPORT inline Float2 Float2::NormalizeSafe(const Float2& safer) const
{
    // security //
    LEVIATHAN_ASSERT(safer.IsNormalized(), "safer not normalized");
    const float len = X * X + Y * Y;
    if(len == 0) {
        return safer;
    }
    const float length = sqrt(len);
    return Float2(X / length, Y / length);
}
// checks is the vector normalized //
DLLEXPORT inline bool Float2::IsNormalized() const
{
    // is absolute -1.f under normalization tolerance //
    return fabs(X * X + Y * Y - 1.0f) < NORMALIZATION_TOLERANCE;
}
// does linear interpolation between vectors and coefficient f, not limited to range [0,1],
// courtesy of ozz-animation //
DLLEXPORT inline Float2 Float2::Lerp(const Float2& other, float f) const
{
    return Float2((other.X - X) * f + X, (other.Y - Y) * f + Y);
}
// compares distance between vectors to tolerance, returns true if less //
DLLEXPORT inline bool Float2::Compare(const Float2& other, float tolerance) const
{
    const Float2 difference = (*this) - other;
    return difference.Dot(difference) < tolerance * tolerance;
}
// ------------------------------------ //
// static returns //
// creates a Float2 with all zeros //
DLLEXPORT inline Float2 Float2::zero()
{
    return Float2(0.f, 0.f);
}
// creates a Float2 with all ones //
DLLEXPORT inline Float2 Float2::one()
{
    return Float2(1.f, 1.f);
}

// unitary vector x, to work with ozz declarations //
DLLEXPORT inline Float2 Float2::x_asix()
{
    return Float2(1.f, 0.f);
}
// unitary vector y //
DLLEXPORT inline Float2 Float2::y_axis()
{
    return Float2(0.f, 1.f);
}

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Leviathan::Float2& value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}
