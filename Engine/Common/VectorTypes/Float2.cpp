#include "Float2.h"

using namespace Leviathan;

DLLEXPORT constexpr Float2::Float2(float x, float y) : X(x), Y(y)
{
    DO_NAN_CHECK;
}

DLLEXPORT constexpr Float2::Float2(float data) : X(data), Y(data)
{
    DO_NAN_CHECK;
}

DLLEXPORT inline bool Float2::HasInvalidValues() const noexcept
{
    return !std::isfinite(X) || !std::isfinite(Y);
}

DLLEXPORT inline void Float2::CheckForNans() const
{
    if(HasInvalidValues()) {
        DEBUG_BREAK;
        throw std::runtime_error("Float2 has NaNs (or infinites in it) in it!");
    }
}

DLLEXPORT constexpr float& Float2::operator[](int nindex)
{
    switch(nindex) {
	    case 0: return X;
	    case 1: return Y;
	    default: break;
    }

    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

// ------------------- Operators ----------------- //

DLLEXPORT constexpr Float2 Float2::operator+(const Float2& other) const noexcept
{
    return Float2(X + other.X, Y + other.Y);
}

DLLEXPORT inline Float2& Float2::operator+=(const Float2& other) noexcept
{
    X += other.X;
    Y += other.Y;
    return *this;
}

DLLEXPORT constexpr Float2 Float2::operator-(const Float2& other) const noexcept
{
    return Float2(X - other.X, Y - other.Y);
}

DLLEXPORT constexpr Float2 Float2::operator-() const noexcept
{
    return Float2(-X, -Y);
}

DLLEXPORT inline Float2& Float2::operator-=(const Float2& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    return *this;
}

DLLEXPORT constexpr Float2 Float2::operator+() const noexcept
{
    return Float2(*this);
}

DLLEXPORT constexpr Float2 Float2::operator*(const Float2& other) const noexcept
{
    return Float2(X * other.X, Y * other.Y);
}

DLLEXPORT inline Float2& Float2::operator*=(const Float2& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float2 Float2::operator*(float val) const noexcept
{
    return Float2(X * val, Y * val);
}

DLLEXPORT inline Float2& Float2::operator*=(float val) noexcept
{
    X *= val;
    Y *= val;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float2 Float2::operator/(const Float2& other) const
{
    return Float2(X / other.X, Y / other.Y);
}

DLLEXPORT inline Float2& Float2::operator/=(const Float2& other)
{
    X /= other.X;
    Y /= other.Y;
    DO_NAN_CHECK;
    return *this;
}

DLLEXPORT constexpr Float2 Float2::operator/(float val) const
{
    return Float2(X / val, Y / val);
}

DLLEXPORT inline Float2& Float2::operator/=(float val)
{
    X /= val;
    Y /= val;
    DO_NAN_CHECK;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Float2::operator<(const Float2& other) const noexcept
{
    return std::tie(X, Y) < std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Float2::operator<=(const Float2& other) const noexcept
{
    return std::tie(X, Y) <= std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Float2::operator>(const Float2& other) const noexcept
{
    return std::tie(X, Y) > std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Float2::operator>=(const Float2& other) const noexcept
{
    return std::tie(X, Y) >= std::tie(other.X, other.Y);
}

DLLEXPORT constexpr bool Float2::operator==(const Float2& other) const noexcept
{
    return X == other.X && Y == other.Y;
}

DLLEXPORT constexpr bool Float2::operator!=(const Float2& other) const noexcept
{
    return X != other.X || Y != other.Y;
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr float Float2::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr float Float2::GetY() const noexcept
{
    return Y;
}

DLLEXPORT inline void Float2::SetX(float val)
{
    X = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Float2::SetY(float val)
{
    Y = val;
    DO_NAN_CHECK;
}

DLLEXPORT constexpr float Float2::HAdd() const noexcept
{
    return X + Y;
}

DLLEXPORT inline float Float2::HAddAbs() const noexcept
{
    return std::fabs(X) + std::fabs(Y);
}

DLLEXPORT constexpr Float2 Float2::MinElements(const Float2& other) const noexcept
{
    return Float2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
}

DLLEXPORT constexpr Float2 Float2::MaxElements(const Float2& other) const noexcept
{
    return Float2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
}

DLLEXPORT constexpr Float2 Float2::Clamp(const Float2& min, const Float2& max) const noexcept
{
    const Float2 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr float Float2::Dot(const Float2& val) const noexcept
{
    return X * val.X + Y * val.Y;
}

DLLEXPORT inline float Float2::Length() const noexcept
{
    return sqrt(X * X + Y * Y);
}

DLLEXPORT constexpr float Float2::LengthSquared() const noexcept
{
    return X * X + Y * Y;
}

DLLEXPORT inline Float2 Float2::Normalize() const
{
    const float length = Length();
    if(length == 0)
        return Float2(0, 0);
    return Float2(X / length, Y / length);
}

DLLEXPORT inline Float2 Float2::NormalizeSafe(const Float2& safer) const noexcept
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

DLLEXPORT inline bool Float2::IsNormalized() const noexcept
{
    // is absolute -1.f under normalization tolerance //
    return fabs(X * X + Y * Y - 1.0f) < NORMALIZATION_TOLERANCE;
}

DLLEXPORT constexpr Float2 Float2::Lerp(const Float2& other, float f) const noexcept
{
    return Float2((other.X - X) * f + X, (other.Y - Y) * f + Y);
}

DLLEXPORT constexpr bool Float2::Compare(const Float2& other, float tolerance) const noexcept
{
    const Float2 difference = (*this) - other;
    return difference.Dot(difference) < tolerance * tolerance;
}

// ------------------------------------ //

DLLEXPORT constexpr Float2 Float2::zero() noexcept
{
    return Float2(0.f);
}

DLLEXPORT constexpr Float2 Float2::one() noexcept
{
    return Float2(1.f);
}

DLLEXPORT constexpr Float2 Float2::x_axis() noexcept
{
    return Float2(1.f, 0.f);
}

DLLEXPORT constexpr Float2 Float2::y_axis() noexcept
{
    return Float2(0.f, 1.f);
}

// ------------------------------------ //

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Leviathan::Float2& value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}
