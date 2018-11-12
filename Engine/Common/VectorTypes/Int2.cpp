#include "Int2.h"

using namespace Leviathan;

DLLEXPORT constexpr Int2::Int2(int x, int y) noexcept : X(x), Y(y) {}

DLLEXPORT constexpr Int2::Int2(int data) noexcept : X(data), Y(data) {}

// ------------------------------------ //

DLLEXPORT constexpr int& Int2::operator[](int nindex)
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

DLLEXPORT constexpr Int2 Int2::operator+(const Int2& other) const noexcept
{
    return Int2(X + other.X, Y + other.Y);
}

DLLEXPORT inline Int2& Int2::operator+=(const Int2& other) noexcept
{
    X += other.X;
    Y += other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator-(const Int2& other) const noexcept
{
    return Int2(X - other.X, Y - other.Y);
}

DLLEXPORT constexpr Int2 Int2::operator-() const noexcept
{
    return Int2(-X, -Y);
}

DLLEXPORT inline Int2& Int2::operator-=(const Int2& other) noexcept
{
    X -= other.X;
    Y -= other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator+() const noexcept
{
    return Int2(*this);
}

DLLEXPORT constexpr Int2 Int2::operator*(const Int2& other) const noexcept
{
    return Int2(X * other.X, Y * other.Y);
}

DLLEXPORT inline Int2& Int2::operator*=(const Int2& other) noexcept
{
    X *= other.X;
    Y *= other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator*(int val) const noexcept
{
    return Int2(X * val, Y * val);
}

DLLEXPORT inline Int2& Int2::operator*=(int val) noexcept
{
    X *= val;
    Y *= val;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator/(const Int2& other) const
{
    return Int2(X / other.X, Y / other.Y);
}

DLLEXPORT inline Int2& Int2::operator/=(const Int2& other)
{
    X /= other.X;
    Y /= other.Y;
    return *this;
}

DLLEXPORT constexpr Int2 Int2::operator/(int val) const
{
    return Int2(X / val, Y / val);
}

DLLEXPORT inline Int2& Int2::operator/=(int val)
{
    X /= val;
    Y /= val;
    return *this;
}

// ---- comparison operators ---- //

DLLEXPORT constexpr bool Int2::operator<(const Int2& other) const noexcept
{
    return X < other.X && Y < other.Y;
}

DLLEXPORT constexpr bool Int2::operator<=(const Int2& other) const noexcept
{
    return X <= other.X && Y <= other.Y;
}

DLLEXPORT constexpr bool Int2::operator>(const Int2& other) const noexcept
{
    return X > other.X && Y > other.Y;
}

DLLEXPORT constexpr bool Int2::operator>=(const Int2& other) const noexcept
{
    return X >= other.X && Y >= other.Y;
}

DLLEXPORT constexpr bool Int2::operator==(const Int2& other) const noexcept
{
    return X == other.X && Y == other.Y;
}

DLLEXPORT constexpr bool Int2::operator!=(const Int2& other) const noexcept
{
    return X != other.X && Y != other.Y;
}

// ------------------ Functions ------------------ //

DLLEXPORT constexpr int Int2::GetX() const noexcept
{
    return X;
}

DLLEXPORT constexpr int Int2::GetY() const noexcept
{
    return Y;
}

DLLEXPORT inline void Int2::SetX(int val)
{
    X = val;
}

DLLEXPORT inline void Int2::SetY(int val)
{
    Y = val;
}

DLLEXPORT constexpr int Int2::HAdd() const noexcept
{
    return X + Y;
}

DLLEXPORT inline unsigned int Int2::HAddAbs() const noexcept
{
    return std::abs(X) + std::abs(Y);
}

DLLEXPORT constexpr Int2 Int2::MinElements(const Int2& other) const noexcept
{
    return Int2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
}

DLLEXPORT constexpr Int2 Int2::MaxElements(const Int2& other) const noexcept
{
    return Int2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
}

DLLEXPORT constexpr Int2 Int2::Clamp(const Int2& min, const Int2& max) const noexcept
{
    const Int2 minval = this->MinElements(max);
    return min.MaxElements(minval);
}

// ----------------- Vector math ------------------- //

DLLEXPORT constexpr int Int2::Dot(const Int2& val) const noexcept
{
    return X * val.X + Y * val.Y;
}

DLLEXPORT inline float Int2::Length() const noexcept
{
    return sqrt(X * X + Y * Y);
}

DLLEXPORT constexpr unsigned int Int2::LengthSquared() const noexcept
{
    return X * X + Y * Y;
}

// ------------------------------------ //

DLLEXPORT constexpr Int2 Int2::zero() noexcept
{
    return Int2(0);
}

DLLEXPORT constexpr Int2 Int2::one() noexcept
{
    return Int2(1);
}

DLLEXPORT constexpr Int2 Int2::x_axis() noexcept
{
    return Int2(1, 0);
}

DLLEXPORT constexpr Int2 Int2::y_axis() noexcept
{
    return Int2(0, 1);
}

// ------------------------------------ //

DLLEXPORT std::ostream& Leviathan::operator<<(
    std::ostream& stream, const Leviathan::Int2& value)
{
    stream << "[" << value.X << ", " << value.Y << "]";
    return stream;
}

// ------------------------------------ //
// Why these two?
DLLEXPORT void Int2::SetData(const int& data)
{
    X = data;
    Y = data;
}

DLLEXPORT void Int2::SetData(const int& data1, const int& data2)
{
    X = data1;
    Y = data2;
}
