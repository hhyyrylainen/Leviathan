// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

#include "Logger.h"

#include <cmath>
#include <stdexcept>
#include <tuple>

// Uncomment for debugging and major slow-downs
// #define CHECK_FOR_NANS

#ifdef CHECK_FOR_NANS
#define DO_NAN_CHECK    \
    {                   \
        CheckForNans(); \
    }
#else
#define DO_NAN_CHECK
#endif // CHECK_FOR_NANS

#ifdef LEVIATHAN_USING_BULLET
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#endif

#ifdef LEVIATHAN_USING_OGRE
#include "bsfUtility/Image/BsColor.h"
#include "bsfUtility/Math/BsQuaternion.h"
#include "bsfUtility/Math/BsVector3.h"
#include "bsfUtility/Math/BsVector4.h"
#endif // LEVIATHAN_USING_OGRE

namespace Leviathan {


struct PotentiallySetIndex {

    inline PotentiallySetIndex(size_t index) : ValueSet(true), Index(index) {}
    inline PotentiallySetIndex() = default;

    inline operator bool() const
    {

        return ValueSet;
    }

    inline operator size_t() const
    {
#ifdef _DEBUG
        LEVIATHAN_ASSERT(
            ValueSet, "PotentiallySetIndex size_t() called when ValueSet is false");
#endif // _DEBUG

        return Index;
    }

    bool operator==(const PotentiallySetIndex& other) const
    {

        if(!ValueSet)
            return ValueSet == other.ValueSet;

        return Index == other.Index;
    }

    PotentiallySetIndex& operator=(const PotentiallySetIndex& other)
    {

        ValueSet = other.ValueSet;
        Index = other.Index;
        return *this;
    }

    inline PotentiallySetIndex& operator=(const size_t& value)
    {

        ValueSet = true;
        Index = value;
        return *this;
    }

    inline bool IsSet() const
    {

        return ValueSet;
    }

    bool ValueSet = false;
    size_t Index = 0;
};

struct StartEndIndex {

    using Index = PotentiallySetIndex;

    inline StartEndIndex(size_t start, size_t end) : Start(start), End(end) {}

    inline StartEndIndex(size_t start) : Start(start) {}

    inline StartEndIndex() = default;

    //! Reset the Start and End to unset
    inline void Reset()
    {
        Start = Index();
        End = Index();
    }

    //! Calculates the length of the indexes between start and end
    //! \returns The length or if either is unset 0 Or if Start > End
    inline size_t Length() const
    {
        if(!Start || !End || static_cast<size_t>(Start) > static_cast<size_t>(End))
            return 0;

        return 1 + (static_cast<size_t>(End) - static_cast<size_t>(Start));
    }


    Index Start;
    Index End;
};

struct Int2 {
public:
    DLLEXPORT constexpr Int2() noexcept = default;

    DLLEXPORT constexpr Int2(int x, int y) noexcept : X(x), Y(y) {}
    DLLEXPORT constexpr explicit Int2(int data) noexcept : X(data), Y(data) {}

    // access operator //
    DLLEXPORT inline int& operator[](int nindex)
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
    // add elements //
    DLLEXPORT constexpr Int2 operator+(const Int2& other) const noexcept
    {
        return Int2(X + other.X, Y + other.Y);
    }

    DLLEXPORT inline Int2& operator+=(const Int2& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT constexpr Int2 operator-(const Int2& other) const noexcept
    {
        return Int2(X - other.X, Y - other.Y);
    }

    DLLEXPORT inline Int2& operator-=(const Int2& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        return *this;
    }

    // negates all elements //
    DLLEXPORT constexpr Int2 operator-() const noexcept
    {
        return Int2(-X, -Y);
    }

    // returns the vector //
    DLLEXPORT constexpr Int2 operator+() const noexcept
    {
        return Int2(*this);
    }

    // multiplies elements together //
    DLLEXPORT constexpr Int2 operator*(const Int2& other) const noexcept
    {
        return Int2(X * other.X, Y * other.Y);
    }

    DLLEXPORT inline Int2& operator*=(const Int2& other) noexcept
    {
        X *= other.X;
        Y *= other.Y;
        return *this;
    }

    // Divides all elements by int //
    DLLEXPORT constexpr Int2 operator/(int val) const
    {
        return Int2(X / val, Y / val);
    }

    DLLEXPORT inline Int2& operator/=(int val)
    {
        X /= val;
        Y /= val;
        return *this;
    }

    // multiply  by scalar f //
    DLLEXPORT constexpr Int2 operator*(int val) const noexcept
    {
        return Int2(X * val, Y * val);
    }

    DLLEXPORT inline Int2& operator*=(int val) noexcept
    {
        X *= val;
        Y *= val;
        return *this;
    }

    // divides all elements //
    DLLEXPORT constexpr Int2 operator/(const Int2& other) const
    {
        return Int2(X / other.X, Y / other.Y);
    }

    DLLEXPORT inline Int2& operator/=(const Int2& other)
    {
        X /= other.X;
        Y /= other.Y;
        return *this;
    }

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Int2& other) const noexcept
    {
        return std::tie(X, Y) < std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator<=(const Int2& other) const noexcept
    {
        return std::tie(X, Y) <= std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator>(const Int2& other) const noexcept
    {
        return std::tie(X, Y) > std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator>=(const Int2& other) const noexcept
    {
        return std::tie(X, Y) >= std::tie(other.X, other.Y);
    }

    DLLEXPORT constexpr bool operator==(const Int2& other) const noexcept
    {
        return X == other.X && Y == other.Y;
    }

    DLLEXPORT constexpr bool operator!=(const Int2& other) const noexcept
    {
        return X != other.X || Y != other.Y;
    }

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr int GetX() const noexcept
    {
        return X;
    }

    DLLEXPORT constexpr int GetY() const noexcept
    {
        return Y;
    }

    // setters //
    DLLEXPORT inline void SetX(int val)
    {
        X = val;
    }

    DLLEXPORT inline void SetY(int val)
    {
        Y = val;
    }

    // add all elements together //
    DLLEXPORT constexpr int HAdd() const noexcept
    {
        return X + Y;
    }

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline unsigned int HAddAbs() const noexcept
    {
        return std::abs(X) + std::abs(Y);
    }

    // getting min and max of objects //
    DLLEXPORT constexpr Int2 MinElements(const Int2& other) const noexcept
    {
        return Int2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
    }

    DLLEXPORT constexpr Int2 MaxElements(const Int2& other) const noexcept
    {
        return Int2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
    }

    // value clamping //
    DLLEXPORT constexpr Int2 Clamp(const Int2& min, const Int2& max) const noexcept
    {
        const Int2 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // length of the vector //
    DLLEXPORT inline float Length() const noexcept
    {
        return std::sqrt(static_cast<float>(LengthSquared()));
    }

    DLLEXPORT constexpr unsigned int LengthSquared() const noexcept
    {
        return X * X + Y * Y;
    }

    VALUE_TYPE(Int2);

    // data //
    int X = 0;
    int Y = 0;
};

struct Int3 {
public:
    DLLEXPORT constexpr Int3() noexcept = default;

    DLLEXPORT constexpr Int3(int x, int y, int z) : X(x), Y(y), Z(z) {}
    DLLEXPORT constexpr Int3(Int2 ints, int z) : X(ints.X), Y(ints.Y), Z(z) {}
    DLLEXPORT constexpr explicit Int3(int data) : X(data), Y(data), Z(data) {}

    // access operator //
    DLLEXPORT inline int& operator[](int nindex)
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
    // add elements //
    DLLEXPORT constexpr Int3 operator+(const Int3& other) const noexcept
    {
        return Int3(X + other.X, Y + other.Y, Z + other.Z);
    }

    DLLEXPORT inline Int3& operator+=(const Int3& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT constexpr Int3 operator-(const Int3& other) const noexcept
    {
        return Int3(X - other.X, Y - other.Y, Z - other.Z);
    }

    DLLEXPORT inline Int3& operator-=(const Int3& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;
        return *this;
    }

    // negates all elements //
    DLLEXPORT constexpr Int3 operator-() const noexcept
    {
        return Int3(-X, -Y, -Z);
    }

    // returns the vector //
    DLLEXPORT constexpr Int3 operator+() const noexcept
    {
        return Int3(*this);
    }

    // multiplies elements together //
    DLLEXPORT constexpr Int3 operator*(const Int3& other) const noexcept
    {
        return Int3(X * other.X, Y * other.Y, Z * other.Z);
    }

    DLLEXPORT inline Int3& operator*=(const Int3& other) noexcept
    {
        X *= other.X;
        Y *= other.Y;
        Z *= other.Z;
        return *this;
    }

    // Divides all elements by int //
    DLLEXPORT constexpr Int3 operator/(int val) const
    {
        return Int3(X / val, Y / val, Z / val);
    }

    DLLEXPORT inline Int3& operator/=(int val)
    {
        X /= val;
        Y /= val;
        Z /= val;
        return *this;
    }

    // multiply  by scalar f //
    DLLEXPORT constexpr Int3 operator*(int val) const noexcept
    {
        return Int3(X * val, Y * val, Z * val);
    }

    DLLEXPORT inline Int3& operator*=(int val) noexcept
    {
        X *= val;
        Y *= val;
        Z *= val;
        return *this;
    }

    // divides all elements //
    DLLEXPORT constexpr Int3 operator/(const Int3& other) const
    {
        return Int3(X / other.X, Y / other.Y, Z / other.Z);
    }

    DLLEXPORT inline Int3& operator/=(const Int3& other)
    {
        X /= other.X;
        Y /= other.Y;
        Z /= other.Z;
        return *this;
    }

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT constexpr bool operator<(const Int3& other) const noexcept
    {
        return std::tie(X, Y, Z) < std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT constexpr bool operator<=(const Int3& other) const noexcept
    {
        return std::tie(X, Y, Z) <= std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT constexpr bool operator>(const Int3& other) const noexcept
    {
        return std::tie(X, Y, Z) > std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT constexpr bool operator>=(const Int3& other) const noexcept
    {
        return std::tie(X, Y, Z) >= std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT constexpr bool operator==(const Int3& other) const noexcept
    {
        return X == other.X && Y == other.Y && Z == other.Z;
    }

    DLLEXPORT constexpr bool operator!=(const Int3& other) const noexcept
    {
        return X != other.X || Y != other.Y || Z != other.Z;
    }

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT constexpr int GetX() const noexcept
    {
        return X;
    }

    DLLEXPORT constexpr int GetY() const noexcept
    {
        return Y;
    }

    DLLEXPORT constexpr int GetZ() const noexcept
    {
        return Z;
    }

    // setters //
    DLLEXPORT inline void SetX(int val)
    {
        X = val;
    }

    DLLEXPORT inline void SetY(int val)
    {
        Y = val;
    }

    DLLEXPORT inline void SetZ(int val)
    {
        Z = val;
    }

    // add all elements together //
    DLLEXPORT constexpr int HAdd() const noexcept
    {
        return X + Y + Z;
    }

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline unsigned int HAddAbs() const noexcept
    {
        return std::abs(X) + std::abs(Y) + std::abs(Z);
    }

    // getting min and max of objects //
    DLLEXPORT constexpr Int3 MinElements(const Int3& other) const noexcept
    {
        return Int3(
            X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
    }

    DLLEXPORT constexpr Int3 MaxElements(const Int3& other) const noexcept
    {
        return Int3(
            X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
    }

    // value clamping //
    DLLEXPORT constexpr Int3 Clamp(const Int3& min, const Int3& max) const noexcept
    {
        const Int3 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // length of the vector //
    DLLEXPORT inline float Length() const noexcept
    {
        return std::sqrt(static_cast<float>(LengthSquared()));
    }

    DLLEXPORT constexpr unsigned int LengthSquared() const noexcept
    {
        return X * X + Y * Y + Z * Z;
    }

    // ------------------------------------ //

    VALUE_TYPE(Int3);

    int X = 0;
    int Y = 0;
    int Z = 0;
};

struct Float2 {
public:
    DLLEXPORT inline Float2() noexcept = default;

    DLLEXPORT inline Float2(float x, float y) : X(x), Y(y)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline Float2(const Int2& values) noexcept :
        X(static_cast<float>(values.X)), Y(static_cast<float>(values.Y))
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline explicit Float2(float data) : X(data), Y(data)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const noexcept
    {
        return !std::isfinite(X) || !std::isfinite(Y);
    }

    DLLEXPORT inline void CheckForNans() const
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float2 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](int nindex)
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
    // add elements //
    DLLEXPORT inline Float2 operator+(const Float2& other) const noexcept
    {
        return Float2(X + other.X, Y + other.Y);
    }

    DLLEXPORT inline Float2& operator+=(const Float2& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT inline Float2 operator-(const Float2& other) const noexcept
    {
        return Float2(X - other.X, Y - other.Y);
    }

    DLLEXPORT inline Float2& operator-=(const Float2& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        return *this;
    }

    // negates all elements //
    DLLEXPORT inline Float2 operator-() const noexcept
    {
        return Float2(-X, -Y);
    }

    // returns the vector //
    DLLEXPORT inline Float2 operator+() const noexcept
    {
        return Float2(*this);
    }

    // multiplies elements together //
    DLLEXPORT inline Float2 operator*(const Float2& other) const noexcept
    {
        return Float2(X * other.X, Y * other.Y);
    }

    DLLEXPORT inline Float2& operator*=(const Float2& other) noexcept
    {
        X *= other.X;
        Y *= other.Y;
        DO_NAN_CHECK;
        return *this;
    }

    // Divides all elements by float //
    DLLEXPORT inline Float2 operator/(float val) const
    {
        return Float2(X / val, Y / val);
    }

    DLLEXPORT inline Float2& operator/=(float val)
    {
        X /= val;
        Y /= val;
        DO_NAN_CHECK;
        return *this;
    }

    // multiply  by scalar f //
    DLLEXPORT inline Float2 operator*(float val) const noexcept
    {
        return Float2(X * val, Y * val);
    }

    DLLEXPORT inline Float2& operator*=(float val) noexcept
    {
        X *= val;
        Y *= val;
        DO_NAN_CHECK;
        return *this;
    }

    // divides all elements //
    DLLEXPORT inline Float2 operator/(const Float2& other) const
    {
        return Float2(X / other.X, Y / other.Y);
    }

    DLLEXPORT inline Float2& operator/=(const Float2& other)
    {
        X /= other.X;
        Y /= other.Y;
        DO_NAN_CHECK;
        return *this;
    }

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float2& other) const noexcept
    {
        return std::tie(X, Y) < std::tie(other.X, other.Y);
    }

    DLLEXPORT inline bool operator<=(const Float2& other) const noexcept
    {
        return std::tie(X, Y) <= std::tie(other.X, other.Y);
    }

    DLLEXPORT inline bool operator>(const Float2& other) const noexcept
    {
        return std::tie(X, Y) > std::tie(other.X, other.Y);
    }

    DLLEXPORT inline bool operator>=(const Float2& other) const noexcept
    {
        return std::tie(X, Y) >= std::tie(other.X, other.Y);
    }

    DLLEXPORT inline bool operator==(const Float2& other) const noexcept
    {
        return X == other.X && Y == other.Y;
    }

    DLLEXPORT inline bool operator!=(const Float2& other) const noexcept
    {
        return X != other.X || Y != other.Y;
    }

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT inline float GetX() const noexcept
    {
        return X;
    }

    DLLEXPORT inline float GetY() const noexcept
    {
        return Y;
    }

    // setters //
    DLLEXPORT inline void SetX(float val)
    {
        X = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetY(float val)
    {
        Y = val;
        DO_NAN_CHECK;
    }

    // add all elements together //
    DLLEXPORT inline float HAdd() const noexcept
    {
        return X + Y;
    }

    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const noexcept
    {
        return std::fabs(X) + std::fabs(Y);
    }

    // getting min and max of objects //
    DLLEXPORT inline Float2 MinElements(const Float2& other) const noexcept
    {
        return Float2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
    }

    DLLEXPORT inline Float2 MaxElements(const Float2& other) const noexcept
    {
        return Float2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
    }

    // value clamping //
    DLLEXPORT inline Float2 Clamp(const Float2& min, const Float2& max) const noexcept
    {
        const Float2 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float2& val) const noexcept
    {
        return X * val.X + Y * val.Y;
    }

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept
    {
        return std::sqrt(LengthSquared());
    }

    DLLEXPORT inline float LengthSquared() const noexcept
    {
        return X * X + Y * Y;
    }

    // normalizes the vector //
    DLLEXPORT inline Float2 Normalize() const
    {
        const float length = Length();
        if(length == 0)
            return Float2(0, 0);
        return (*this) / length;
    }

    // safe version of normalization //
    DLLEXPORT inline Float2 NormalizeSafe(const Float2& safer) const noexcept
    {
        // security //
        LEVIATHAN_ASSERT(safer.IsNormalized(), "safer not normalized");
        if(LengthSquared() == 0)
            return safer;
        const float length = Length();
        return (*this) / length;
    }

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y - 1.0f) < NORMALIZATION_TOLERANCE;
    }

    // does linear interpolation between vectors and coefficient f, not limited to range [0,1],
    // courtesy of ozz-animation //
    DLLEXPORT inline Float2 Lerp(const Float2& other, float f) const noexcept
    {
        return Float2((other.X - X) * f + X, (other.Y - Y) * f + Y);
    }

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float2& other, float tolerance) const noexcept
    {
        const Float2 difference = (*this) - other;
        return difference.Dot(difference) < tolerance * tolerance;
    }

    // ----------------- casts ------------------- //
    // waiting for Microsoft's compilers to add support for "explicit" here //
    // DLLEXPORT inline operator D3DXVECTOR2(){
    //	return D3DXVECTOR2(X, Y);
    //}

    VALUE_TYPE(Float2);

    // data //
    float X = 0;
    float Y = 0;
};

struct Float3 {
public:
    DLLEXPORT inline Float3() noexcept = default;

    DLLEXPORT inline Float3(float x, float y, float z) : X(x), Y(y), Z(z)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline Float3(Float2 floats, float z) : X(floats.X), Y(floats.Y), Z(z)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline explicit Float3(float data) : X(data), Y(data), Z(data)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline Float3(const Int3& values) noexcept :
        X(static_cast<float>(values.X)), Y(static_cast<float>(values.Y)),
        Z(static_cast<float>(values.Z))
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const noexcept
    {
        return !std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z);
    }

    DLLEXPORT inline void CheckForNans() const
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float3 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](int nindex)
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
    // add elements //
    DLLEXPORT inline Float3 operator+(const Float3& other) const noexcept
    {
        return Float3(X + other.X, Y + other.Y, Z + other.Z);
    }

    DLLEXPORT inline Float3& operator+=(const Float3& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT inline Float3 operator-(const Float3& other) const noexcept
    {
        return Float3(X - other.X, Y - other.Y, Z - other.Z);
    }

    DLLEXPORT inline Float3& operator-=(const Float3& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;
        return *this;
    }

    // negates all elements //
    DLLEXPORT inline Float3 operator-() const noexcept
    {
        return Float3(-X, -Y, -Z);
    }

    // returns the vector //
    DLLEXPORT inline Float3 operator+() const noexcept
    {
        return Float3(*this);
    }

    // multiplies elements together //
    DLLEXPORT inline Float3 operator*(const Float3& other) const noexcept
    {
        return Float3(X * other.X, Y * other.Y, Z * other.Z);
    }

    DLLEXPORT inline Float3& operator*=(const Float3& other) noexcept
    {
        X *= other.X;
        Y *= other.Y;
        Z *= other.Z;
        DO_NAN_CHECK;
        return *this;
    }

    // Divides all elements by float //
    DLLEXPORT inline Float3 operator/(float val) const
    {
        return Float3(X / val, Y / val, Z / val);
    }

    DLLEXPORT inline Float3& operator/=(float val)
    {
        X /= val;
        Y /= val;
        Z /= val;
        DO_NAN_CHECK;
        return *this;
    }

    // multiply  by scalar f //
    DLLEXPORT inline Float3 operator*(float val) const noexcept
    {
        return Float3(X * val, Y * val, Z * val);
    }

    DLLEXPORT inline Float3& operator*=(float val) noexcept
    {
        X *= val;
        Y *= val;
        Z *= val;
        DO_NAN_CHECK;
        return *this;
    }

    // divides all elements //
    DLLEXPORT inline Float3 operator/(const Float3& other) const
    {
        return Float3(X / other.X, Y / other.Y, Z / other.Z);
    }

    DLLEXPORT inline Float3& operator/=(const Float3& other)
    {
        X /= other.X;
        Y /= other.Y;
        Z /= other.Z;
        DO_NAN_CHECK;
        return *this;
    }

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float3& other) const noexcept
    {
        return std::tie(X, Y, Z) < std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT inline bool operator<=(const Float3& other) const noexcept
    {
        return std::tie(X, Y, Z) <= std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT inline bool operator>(const Float3& other) const noexcept
    {
        return std::tie(X, Y, Z) > std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT inline bool operator>=(const Float3& other) const noexcept
    {
        return std::tie(X, Y, Z) >= std::tie(other.X, other.Y, other.Z);
    }

    DLLEXPORT inline bool operator==(const Float3& other) const noexcept
    {
        return X == other.X && Y == other.Y && Z == other.Z;
    }

    DLLEXPORT inline bool operator!=(const Float3& other) const noexcept
    {
        return X != other.X || Y != other.Y || Z != other.Z;
    }

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT inline float GetX() const noexcept
    {
        return X;
    }

    DLLEXPORT inline float GetY() const noexcept
    {
        return Y;
    }

    DLLEXPORT inline float GetZ() const noexcept
    {
        return Z;
    }

    // setters //
    DLLEXPORT inline void SetX(float val)
    {
        X = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetY(float val)
    {
        Y = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetZ(float val)
    {
        Z = val;
        DO_NAN_CHECK;
    }

    // add all elements together //
    DLLEXPORT inline float HAdd() const noexcept
    {
        return X + Y + Z;
    }

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline float HAddAbs() const noexcept
    {
        return std::abs(X) + std::abs(Y) + std::abs(Z);
    }

    // getting min and max of objects //
    DLLEXPORT inline Float3 MinElements(const Float3& other) const noexcept
    {
        return Float3(
            X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
    }

    DLLEXPORT inline Float3 MaxElements(const Float3& other) const noexcept
    {
        return Float3(
            X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
    }

    // value clamping //
    DLLEXPORT inline Float3 Clamp(const Float3& min, const Float3& max) const noexcept
    {
        const Float3 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    DLLEXPORT inline Float3 DegreesToRadians() const noexcept
    {
        return Float3(X * DEGREES_TO_RADIANS, Y * DEGREES_TO_RADIANS, Z * DEGREES_TO_RADIANS);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float3& val) const noexcept
    {
        return X * val.X + Y * val.Y + Z * val.Z;
    }


    DLLEXPORT inline Float3 Cross(const Float3& val) const
    {
        return Float3(Y * val.Z - val.Y * Z, Z * val.X - val.Z * X, X * val.Y - val.X * Y);
    }

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept
    {
        return std::sqrt(LengthSquared());
    }

    DLLEXPORT inline float LengthSquared() const noexcept
    {
        return X * X + Y * Y + Z * Z;
    }

    // normalizes the vector //
    DLLEXPORT inline Float3 Normalize() const
    {
        const float length = Length();
        if(length == 0)
            return Float3(0, 0, 0);
        return (*this) / length;
    }

    // safe version of normalization //
    DLLEXPORT inline Float3 NormalizeSafe(const Float3& safer = Float3(1, 0, 0)) const noexcept
    {
        // security //
        LEVIATHAN_ASSERT(safer.IsNormalized(), "safer not normalized");
        if(LengthSquared() == 0)
            return safer;
        const float length = Length();
        return (*this) / length;
    }

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y + Z * Z - 1.0f) < NORMALIZATION_TOLERANCE;
    }

    // does linear interpolation between vectors and coefficient f, not limited to range
    // [0,1], courtesy of ozz-animation //
    DLLEXPORT inline Float3 Lerp(const Float3& other, float f) const noexcept
    {
        return Float3((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z);
    }

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float3& other, float tolerance) const noexcept
    {
        const Float3 difference = (*this) - other;
        return difference.Dot(difference) < tolerance * tolerance;
    }

    DLLEXPORT static inline Float3 CreateVectorFromAngles(
        const float& yaw, const float& pitch) noexcept
    {
        return Float3(-sin(yaw * DEGREES_TO_RADIANS), sin(pitch * DEGREES_TO_RADIANS),
            -cos(yaw * DEGREES_TO_RADIANS))
            .NormalizeSafe(Zeroed);
    }

    // ----------------- casts ------------------- //
#ifdef LEVIATHAN_USING_OGRE
    DLLEXPORT inline Float3(const bs::Vector3& vec) :
        // copy values //
        X(vec.x), Y(vec.y), Z(vec.z)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline operator bs::Vector3() const
    {
        return bs::Vector3(X, Y, Z);
    }

    DLLEXPORT inline Float3(const btVector3& vec) :
        // copy values //
        X(vec.x()), Y(vec.y()), Z(vec.z())
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline operator btVector3() const
    {
        return btVector3(X, Y, Z);
    }

#endif // LEVIATHAN_USING_OGRE
    // ------------------------------------ //

    VALUE_TYPE(Float3);

    float X = 0;
    float Y = 0;
    float Z = 0;

    DLLEXPORT static const Float3 UnitVForward;
    DLLEXPORT static const Float3 UnitVUp;
    DLLEXPORT static const Float3 Zeroed;
};

struct Float4 {
public:
    DLLEXPORT inline Float4() noexcept = default;

    DLLEXPORT inline Float4(float f1, float f2, float f3, float f4) :
        X(f1), Y(f2), Z(f3), W(f4)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline Float4(Float2 floats, float f3, float f4) :
        X(floats.X), Y(floats.Y), Z(f3), W(f4)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline Float4(Float3 floats, float f4) :
        X(floats.X), Y(floats.Y), Z(floats.Z), W(f4)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline explicit Float4(float data) : X(data), Y(data), Z(data), W(data)
    {
        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const noexcept
    {
        return !std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z) ||
               !std::isfinite(W);
    }

    DLLEXPORT inline void CheckForNans() const
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float4 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](int nindex)
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

    // Is this a good idea?
    //! return first value of {X, Y, Z, W} as a pointer
    DLLEXPORT inline operator float*() noexcept
    {
        // this should be always confirmed to work //
        return &X;
    }

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Float4 operator+(const Float4& other) const noexcept
    {
        return Float4(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
    }

    DLLEXPORT inline Float4& operator+=(const Float4& other) noexcept
    {
        X += other.X;
        Y += other.Y;
        Z += other.Z;
        W += other.W;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT inline Float4 operator-(const Float4& other) const noexcept
    {
        return Float4(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
    }

    DLLEXPORT inline Float4& operator-=(const Float4& other) noexcept
    {
        X -= other.X;
        Y -= other.Y;
        Z -= other.Z;
        W -= other.W;
        return *this;
    }

    // negates all elements //
    DLLEXPORT inline Float4 operator-() const noexcept
    {
        return Float4(-X, -Y, -Z, -W);
    }

    // returns the vector //
    DLLEXPORT inline Float4 operator+() const noexcept
    {
        return Float4(*this);
    }

    // multiplies elements together //
    DLLEXPORT inline Float4 operator*(const Float4& other) const noexcept
    {
        return Float4(X * other.X, Y * other.Y, Z * other.Z, W * other.W);
    }

    DLLEXPORT inline Float4& operator*=(const Float4& other) noexcept
    {
        X *= other.X;
        Y *= other.Y;
        Z *= other.Z;
        W *= other.W;
        DO_NAN_CHECK;
        return *this;
    }

    // Divides all elements by float //
    DLLEXPORT inline Float4 operator/(float val) const
    {
        return Float4(X / val, Y / val, Z / val, W / val);
    }

    DLLEXPORT inline Float4& operator/=(float val)
    {
        X /= val;
        Y /= val;
        Z /= val;
        W /= val;
        DO_NAN_CHECK;
        return *this;
    }

    // multiply  by scalar f //
    DLLEXPORT inline Float4 operator*(float val) const noexcept
    {
        return Float4(X * val, Y * val, Z * val, W * val);
    }

    DLLEXPORT inline Float4& operator*=(float val) noexcept
    {
        X *= val;
        Y *= val;
        Z *= val;
        W *= val;
        DO_NAN_CHECK;
        return *this;
    }

    // divides all elements //
    DLLEXPORT inline Float4 operator/(const Float4& other) const
    {
        return Float4(X / other.X, Y / other.Y, Z / other.Z, W / other.W);
    }

    DLLEXPORT inline Float4& operator/=(const Float4& other)
    {
        X /= other.X;
        Y /= other.Y;
        Z /= other.Z;
        W /= other.W;
        DO_NAN_CHECK;
        return *this;
    }

    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float4& other) const noexcept
    {
        return std::tie(X, Y, Z, W) < std::tie(other.X, other.Y, other.Z, other.W);
    }

    DLLEXPORT inline bool operator<=(const Float4& other) const noexcept
    {
        return std::tie(X, Y, Z, W) <= std::tie(other.X, other.Y, other.Z, other.W);
    }

    DLLEXPORT inline bool operator>(const Float4& other) const noexcept
    {
        return std::tie(X, Y, Z, W) > std::tie(other.X, other.Y, other.Z, other.W);
    }

    DLLEXPORT inline bool operator>=(const Float4& other) const noexcept
    {
        return std::tie(X, Y, Z, W) >= std::tie(other.X, other.Y, other.Z, other.W);
    }

    DLLEXPORT inline bool operator==(const Float4& other) const noexcept
    {
        return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
    }

    DLLEXPORT inline bool operator!=(const Float4& other) const noexcept
    {
        return X != other.X || Y != other.Y || Z != other.Z || W != other.W;
    }

    // ------------------ Functions ------------------ //
    // getters //
    DLLEXPORT inline float GetX() const noexcept
    {
        return X;
    }

    DLLEXPORT inline float GetY() const noexcept
    {
        return Y;
    }

    DLLEXPORT inline float GetZ() const noexcept
    {
        return Z;
    }

    DLLEXPORT inline float GetW() const noexcept
    {
        return W;
    }

    // setters //
    DLLEXPORT inline void SetX(float val)
    {
        X = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetY(float val)
    {
        Y = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetZ(float val)
    {
        Z = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline void SetW(float val)
    {
        W = val;
        DO_NAN_CHECK;
    }

    // add all elements together //
    DLLEXPORT inline float HAdd() const noexcept
    {
        return X + Y + Z + W;
    }

    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline float HAddAbs() const noexcept
    {
        return std::abs(X) + std::abs(Y) + std::abs(Z) + std::abs(W);
    }

    // getting min and max of objects //
    DLLEXPORT inline Float4 MinElements(const Float4& other) const noexcept
    {
        return Float4(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y,
            Z < other.Z ? Z : other.Z, W < other.W ? W : other.W);
    }

    DLLEXPORT inline Float4 MaxElements(const Float4& other) const noexcept
    {
        return Float4(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y,
            Z > other.Z ? Z : other.Z, W > other.W ? W : other.W);
    }

    // value clamping //
    DLLEXPORT inline Float4 Clamp(const Float4& min, const Float4& max) const noexcept
    {
        const Float4 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float4& val) const noexcept
    {
        return X * val.X + Y * val.Y + Z * val.Z + W * val.W;
    }

    // length of the vector //
    DLLEXPORT inline float Length() const noexcept
    {
        return std::sqrt(LengthSquared());
    }

    DLLEXPORT inline float LengthSquared() const noexcept
    {
        return X * X + Y * Y + Z * Z + W * W;
    }

    // normalizes the vector //
    DLLEXPORT inline Float4 Normalize() const
    {
        const float length = Length();

        if(length == 0) {
            // Returns an identity quaternion
            return Float4(0, 0, 0, 1);
        }

        return (*this) / length;
    }

    // safe version of normalization //
    DLLEXPORT inline Float4 NormalizeSafe(
        const Float4& safer = Float4(1, 0, 0, 0)) const noexcept
    {
        // security //
        LEVIATHAN_ASSERT(safer.IsNormalized(), "safer not normalized");
        if(LengthSquared() == 0)
            return safer;
        const float length = Length();
        return (*this) / length;
    }

    // checks is the vector normalized //
    DLLEXPORT inline bool IsNormalized() const noexcept
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y + Z * Z + W * W - 1.0f) < NORMALIZATION_TOLERANCE;
    }

    // does linear interpolation between vectors and coefficient f, not limited to range
    // [0,1], courtesy of ozz-animation //
    DLLEXPORT inline Float4 Lerp(const Float4& other, float f) const noexcept
    {
        return Float4((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z,
            (other.W - W) * f + W);
    }

    // does SPHERICAL interpolation between quaternions //
    DLLEXPORT inline Float4 Slerp(const Float4& other, float f) const
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
            return ((*this) * sinf(angle * (1 - f)) + quaternion3 * sinf(angle * f)) /
                   sinf(angle);

        } else {
            // small angle, linear interpolation will be fine //
            return this->Lerp(quaternion3, f);
        }
    }

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float4& other, float tolerance) const noexcept
    {
        const Float4 difference = (*this) - other;
        return difference.Dot(difference) < tolerance * tolerance;
    }

    //! Converts a quaternion to Axis (and skips outputting the angle)
    //! \note Must be normalized
    DLLEXPORT inline Float3 ToAxis() const
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
    DLLEXPORT inline float ToAngle() const noexcept
    {
        return 2 * std::acos(W);
    }

    //! Inverts a quaternion
    DLLEXPORT inline Float4 Inverse() const noexcept
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

    //! Rotates a vector by this quaternion
    DLLEXPORT inline Float3 RotateVector(const Float3& vector) const
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
        const Float3 uv1 = qvec.Cross(vector);
        const Float3 uuv = qvec.Cross(uv1) * 2.0f;
        const Float3 uv2 = uv1 * 2.0f * W;

        return vector + uv2 + uuv;
    }

    // ----------------- casts ------------------- //
    // Should this macro be replaced by a inline if in the cpp file?
#ifdef LEVIATHAN_USING_OGRE
    DLLEXPORT inline Float4(const bs::Quaternion& quat)
    {
        // copy values //
        X = quat.x;
        Y = quat.y;
        Z = quat.z;
        W = quat.w;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline Float4(const bs::Color& colour)
    {
        // copy values //
        X = colour.r;
        Y = colour.g;
        Z = colour.b;
        W = colour.a;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline operator bs::Quaternion() const
    {
        // Ogre has these in different order
        return bs::Quaternion(W, X, Y, Z);
    }

    DLLEXPORT inline operator bs::Color() const
    {
        return bs::Color(X, Y, Z, W);
    }

    DLLEXPORT inline operator bs::Vector4() const
    {
        return bs::Vector4(X, Y, Z, W);
    }

    DLLEXPORT inline Float4(const btQuaternion& colour)
    {
        // copy values //
        X = colour.x();
        Y = colour.y();
        Z = colour.z();
        W = colour.w();
        DO_NAN_CHECK;
    }

    DLLEXPORT inline operator btQuaternion() const
    {
        return btQuaternion(X, Y, Z, W);
    }

#endif // LEVIATHAN_USING_OGRE

    // ----------------- Quaternions ------------------- //
    DLLEXPORT static inline Float4 CreateQuaternionFromAngles(const Float3& angles)
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
            // X
            cosz * cosy * sinx - sinz * siny * cosx,
            // Y
            cosz * siny * cosx + sinz * cosy * sinx,
            // Z
            sinz * cosy * cosx - cosz * siny * sinx,
            // W
            cosz * cosy * cosx * sinz * siny * sinx);
    }

    //! \note This quaternion has to be normalized
    //! \see
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
    DLLEXPORT inline Float3 QuaternionToEuler() const noexcept
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

        return Float3(atan2(2 * Y * W - 2 * X * Z, 1 - 2 * sqy - 2 * sqz), asin(2 * test),
            atan2(2 * X * W - 2 * Y * Z, 1 - 2 * sqx - 2 * sqz));
    }

    DLLEXPORT inline Float4 QuaternionMultiply(const Float4& other) const noexcept
    {
        return Float4(X * other.X + X * other.W + Y * other.Z - Z * other.Y,
            W * other.Y - X * other.Z + Y * other.W + Z * other.X,
            W * other.Z + X * other.Y - Y * other.X + Z * other.W,
            W * other.W - X * other.X - Y * other.Y - Z * other.Z);
    }

    DLLEXPORT inline Float4 QuaternionReverse() const noexcept
    {
        // reverse vector //
        return Float4(-X, -Y, -Z, W);
    }

    DLLEXPORT static inline Float4 IdentityQuaternion() noexcept
    {
        return Float4(0.f, 0.f, 0.f, 1.f);
    }

    // Math from here: https://stackoverflow.com/questions/12435671/quaternion-lookat-function
    DLLEXPORT static inline Float4 QuaternionLookAt(
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

    //! \note axis must be normalized
    //!
    //! This resource is a life saver:
    //! http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/index.htm
    DLLEXPORT static inline Float4 CreateQuaternionFromAxisAngle(
        const Float3& axis, float angle) noexcept
    {
        const float s = static_cast<float>(std::sin(angle / 2.0));
        const float w = static_cast<float>(std::cos(angle / 2.0));
        return Float4(axis * s, w);
    }

    // TODO: Implement this (Float4::CreateAxisAngleFromEuler)
    DLLEXPORT static inline Float4 CreateAxisAngleFromEuler(const Float3& angles)
    {
        throw std::exception();
        // return Float4();
    }

    VALUE_TYPE(Float4);

    float X = 0;
    float Y = 0;
    float Z = 0;
    float W = 0;

    // specific colours //
    DLLEXPORT static const Float4 ColourBlack;
    DLLEXPORT static const Float4 ColourWhite;
    DLLEXPORT static const Float4 ColourTransparent;

    // Use these from other libraries/executables to avoid linker errors //
    DLLEXPORT static const Float4& GetColourBlack();
    DLLEXPORT static const Float4& GetColourWhite();
    DLLEXPORT static const Float4& GetColourTransparent();
};

// Stream operators //
DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Leviathan::Float4& value);

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Leviathan::Float3& value);

DLLEXPORT std::ostream& operator<<(std::ostream& stream, const Leviathan::Float2& value);

DLLEXPORT std::ostream& operator<<(
    std::ostream& stream, const Leviathan::StartEndIndex& value);

DLLEXPORT std::ostream& operator<<(
    std::ostream& stream, const Leviathan::PotentiallySetIndex& value);

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Float2;
using Leviathan::Float3;
using Leviathan::Float4;
using Leviathan::Int2;
using Leviathan::Int3;
#endif
