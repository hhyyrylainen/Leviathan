#include "Int3.h"

using namespace Leviathan;

DLLEXPORT constexpr Int3::Int3(int x, int y, int z) : X(x), Y(y), Z(z) {}

DLLEXPORT constexpr Int3::Int3(Int2 ints, int z) : X(ints.X), Y(ints.Y), Z(z) {}

DLLEXPORT constexpr Int3::Int3(int data) : X(data), Y(data), Z(data) {}

DLLEXPORT constexpr int& Int3::operator[](int nindex)
{
    switch(nindex) {
	    case 0: return X;
	    case 1: return Y;
	    case 2: return Z;
	    default: break;
    }
    LEVIATHAN_ASSERT(0, "invalid [] access");
    return X;
}

// ------------------- Operators ----------------- //

DLLEXPORT constexpr Int3 Int3::operator+(const Int3& other) const noexcept
{
    return Int3(X + other.X, Y + other.Y, Z + other.Z);
}

DLLEXPORT inline Int3& Int3::operator+=(const Int3& other) noexcept
{
    X += other.X;
    Y += other.Y;
    Z += other.Z;
    return *this;
}

DLLEXPORT inline Int3& Int3::operator-=(const Int3& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator-(const Int3& other) const noexcept
{
    return Int3(X - other.X, Y - other.Y, Z - other.Z);
}

DLLEXPORT constexpr Int3 Int3::operator-() const noexcept
{
    return Int3(-X, -Y, -Z);
}

DLLEXPORT constexpr Int3 Int3::operator+() const noexcept
{
    return Int3(*this);
}

DLLEXPORT constexpr Int3 Int3::operator*(const Int3& other) const noexcept
{
    return Int3(X * other.X, Y * other.Y, Z * other.Z);
}

DLLEXPORT inline Int3& Int3::operator*=(const Int3& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    Z *= other.Z;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator/(int val) const
{
    return Int3(X / val, Y / val, Z / val);
}

DLLEXPORT inline Int3& Int3::operator/=(int val)
{
    X /= val;
    Y /= val;
    Z /= val;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator*(int val) const noexcept
{
    return Int3(X * val, Y * val, Z * val);
}

DLLEXPORT inline Int3& Int3::operator*=(int val) noexcept
{
    X *= val;
    Y *= val;
    Z *= val;
    return *this;
}

DLLEXPORT constexpr Int3 Int3::operator/(const Int3& other) const
{
    return Int3(X / other.X, Y / other.Y, Z / other.Z);
}

DLLEXPORT inline Int3& Int3::operator/=(const Int3& other)
{
    X /= other.X;
    Y /= other.Y;
    Z /= other.Z;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Int3::operator<(const Int3& other) const noexcept
{
    return X < other.X && Y < other.Y && Z < other.Z;
}

DLLEXPORT constexpr bool Int3::operator<=(const Int3& other) const noexcept
{
    return X <= other.X && Y <= other.Y && Z <= other.Z;
}

DLLEXPORT constexpr bool Int3::operator>(const Int3& other) const noexcept
{
    return X > other.X && Y > other.Y && Z > other.Z;
}

DLLEXPORT constexpr bool Int3::operator>=(const Int3& other) const noexcept
{
    return X >= other.X && Y >= other.Y && Z > other.Z;
}

DLLEXPORT constexpr bool Int3::operator==(const Int3& other) const noexcept
{
    return X == other.X && Y == other.Y && Z == other.Z;
}

DLLEXPORT constexpr bool Int3::operator!=(const Int3& other) const noexcept
{
    return !(*this == other);
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr int Int3::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr int Int3::GetY() const noexcept
{
    return Y;
}

DLLEXPORT constexpr int Int3::GetZ() const noexcept
{
    return Z;
}

DLLEXPORT inline void Int3::SetX(int val)
{
    X = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Int3::SetY(int val)
{
    Y = val;
    DO_NAN_CHECK;
}

DLLEXPORT inline void Int3::SetZ(int val)
{
    Z = val;
    DO_NAN_CHECK;
}

DLLEXPORT constexpr int Int3::HAdd() const noexcept
{
    return X + Y + Z;
}

DLLEXPORT inline unsigned int Int3::HAddAbs() const noexcept
{
    return std::abs(X) + std::abs(Y) + std::abs(Z);
}

DLLEXPORT constexpr Int3 Int3::MinElements(const Int3& other) const noexcept
{
    return Int3(
        X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
}

DLLEXPORT constexpr Int3 Int3::MaxElements(const Int3& other) const noexcept
{
    return Int3(
        X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
}

DLLEXPORT constexpr Int3 Int3::Clamp(const Int3& min, const Int3& max) const noexcept
{
    const Int3 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr int Int3::Dot(const Int3& val) const noexcept
{
    return X * val.X + Y * val.Y + Z * val.Z;
}

DLLEXPORT constexpr Int3 Int3::Cross(const Int3& val) const
{
    return Int3(Y * val.Z - val.Y * Z, Z * val.X - val.Z * X, X * val.Y - val.X * Y);
}

DLLEXPORT inline float Int3::Length() const noexcept
{
    return sqrt(X * X + Y * Y + Z * Z);
}

DLLEXPORT constexpr unsigned int Int3::LengthSquared() const noexcept
{
    return X * X + Y * Y + Z * Z;
}

// ------------------------------------ //

DLLEXPORT constexpr Int3 Int3::zero() noexcept
{
    return Int3(0);
}

DLLEXPORT constexpr Int3 Int3::one() noexcept
{
    return Int3(1);
}

DLLEXPORT constexpr Int3 Int3::x_axis() noexcept
{
    return Int3(1, 0, 0);
}

DLLEXPORT constexpr Int3 Int3::y_axis() noexcept
{
    return Int3(0, 1, 0);
}

DLLEXPORT constexpr Int3 Int3::z_axis() noexcept
{
    return Int3(0, 0, 1);
}

// ------------------------------------ //

DLLEXPORT std::ostream& Leviathan::operator<<(std::ostream& stream, const Int3& value)
{
    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << "]";
    return stream;
}
