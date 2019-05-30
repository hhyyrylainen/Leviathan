// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

#include "Logger.h"

#include <cmath>
#include <stdexcept>

// Uncomment for debugging and major slow downs
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
    DLLEXPORT Int2()
    {
        X = 0;
        Y = 0;
    }
    DLLEXPORT Int2(int x, int y)
    {
        X = x;
        Y = y;
    }
    DLLEXPORT explicit Int2(int data)
    {
        X = data;
        Y = data;
    }
    DLLEXPORT Int2(const Int2& other)
    {
        X = other.X;
        Y = other.Y;
    }

    // ------------------------------------ //
    DLLEXPORT Int2 operator+(const Int2& val)
    {
        return Int2(X + val.X, Y + val.Y);
    }

    DLLEXPORT int operator[](const int nIndex) const
    {
        switch(nIndex) {
        case 0: return X;
        case 1: return Y;
        }

        LEVIATHAN_ASSERT(0, "invalid [] access");
        return 0;
    }

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Int2 operator+(const Int2& val) const
    {
        return Int2(X + val.X, Y + val.Y);
    }

    DLLEXPORT inline Int2* operator+=(const Int2& val)
    {
        X += val.X;
        Y += val.Y;
        return this;
    }
    // subtracts all elements //
    DLLEXPORT inline Int2 operator-(const Int2& val) const
    {
        return Int2(X - val.X, Y - val.Y);
    }
    // negates all elements //
    DLLEXPORT inline Int2 operator-() const
    {
        return Int2(-X, -Y);
    }
    // multiplies elements together //
    DLLEXPORT inline Int2 operator*(const Int2& val) const
    {
        return Int2(X * val.X, Y * val.Y);
    }
    // multiply  by scalar f //
    DLLEXPORT inline Int2 operator*(int f) const
    {
        return Int2(X * f, Y * f);
    }

    DLLEXPORT inline Int2* operator*=(int f)
    {
        X *= f;
        Y *= f;
        return this;
    }
    // divides all elements //
    DLLEXPORT inline Int2 operator/(const Int2& val) const
    {
        return Int2(X / val.X, Y / val.Y);
    }
    // divides by int //
    DLLEXPORT inline Int2 operator/(int f) const
    {
        return Int2(X / f, Y / f);
    }
    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Int2& other) const
    {
        return X < other.X && Y < other.Y;
    };
    DLLEXPORT inline bool operator<=(const Int2& other) const
    {
        return X <= other.X && Y <= other.Y;
    };
    DLLEXPORT inline bool operator>(const Int2& other) const
    {
        return X > other.X && Y > other.Y;
    };
    DLLEXPORT inline bool operator>=(const Int2& other) const
    {
        return X >= other.X && Y >= other.Y;
    };
    DLLEXPORT inline bool operator==(const Int2& other) const
    {
        return X == other.X && Y == other.Y;
    };
    DLLEXPORT inline bool operator!=(const Int2& other) const
    {
        return X != other.X && Y != other.Y;
    };

    // ------------------------------------ //

    DLLEXPORT void SetData(const int& data)
    {
        X = data;
        Y = data;
    };
    DLLEXPORT void SetData(const int& data1, const int& data2)
    {
        X = data1;
        Y = data2;
    };

    VALUE_TYPE(Int2);

    int X, Y;
};

struct Int3 {
public:
    DLLEXPORT Int3()
    {
        X = 0;
        Y = 0;
        Z = 0;
    }
    DLLEXPORT Int3(int x, int y, int z)
    {
        X = x;
        Y = y;
        Z = z;
    }
    DLLEXPORT explicit Int3(int data)
    {
        // save a bit of space //
        X = Y = Z = data;
    }

    // ------------------------------------ //
    DLLEXPORT Int3 operator+(const Int3& val) const
    {
        return Int3(X + val.X, Y + val.Y, Z + val.Z);
    }

    DLLEXPORT Int3 operator*(int val) const
    {
        return Int3(X * val, Y * val, Z * val);
    }

    DLLEXPORT int operator[](const int nIndex) const
    {
        switch(nIndex) {
        case 0: return X;
        case 1: return Y;
        case 2: return Z;
        }

        LEVIATHAN_ASSERT(0, "invalid Int3[] access");
        return 0;
    }
    DLLEXPORT Int3 operator-(const Int3& other) const
    {
        return Int3(X - other.X, Y - other.Y, Z - other.Z);
    }

    DLLEXPORT inline Int3* operator*=(int f)
    {
        X *= f;
        Y *= f;
        Z *= f;
        return this;
    }

    DLLEXPORT int AddAllTogether() const
    {
        return X + Y + Z;
    }

    VALUE_TYPE(Int3);

    // ------------------------------------ //

    int X, Y, Z;
};

struct Int4 {
public:
    DLLEXPORT Int4()
    {
        X = Y = Z = W = 0;
    }
    DLLEXPORT Int4(int x, int y, int z, int w) : X(x), Y(y), Z(z), W(w) {}
    DLLEXPORT explicit Int4(int data)
    {
        X = Y = Z = W = data;
    }

    // ------------------------------------ //
    DLLEXPORT Int4& operator+(const Int4& val)
    {
        X += val.X;
        Y += val.Y;
        Z += val.Z;
        W += val.W;
        return *this;
    }
    DLLEXPORT int operator[](const int nIndex) const
    {
        switch(nIndex) {
        case 0: return X;
        case 1: return Y;
        case 2: return Z;
        case 3: return W;
        }

        LEVIATHAN_ASSERT(0, "invalid Int4[] access");
        return 0;
    }
    DLLEXPORT Int4& operator-(const Int4& val)
    {
        X -= val.X;
        Y -= val.Y;
        Z -= val.Z;
        W -= val.W;
        return *this;
    }
    DLLEXPORT int AddAllTogether() const
    {
        return X + Y + Z + W;
    }
    // ------------------------------------ //

    int X, Y, Z, W;
};

// ----------------- Float types ------------------- //
// refactored to match declarations in ozz vec_float //

// functions inlined just like in ozz, for speed, I guess //
// mostly rewritten to match ozz implementation, just in case and not to break anything //

struct Float2 {
public:
    DLLEXPORT inline Float2()
    {
        X = Y = 0;
    };
    DLLEXPORT inline Float2(float x, float y)
    {
        X = x;
        Y = y;
        DO_NAN_CHECK;
    }
    DLLEXPORT inline explicit Float2(float both)
    {
        X = Y = both;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const
    {
        if(!std::isfinite(X) || !std::isfinite(Y)) {
            return true;
        }

        return false;
    }

    DLLEXPORT inline void CheckForNans()
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float2 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](const int& nindex)
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
    DLLEXPORT inline Float2 operator+(const Float2& val) const
    {
        return Float2(X + val.X, Y + val.Y);
    }

    DLLEXPORT inline Float2* operator+=(const Float2& val)
    {
        X += val.X;
        Y += val.Y;
        return this;
    }
    // subtracts all elements //
    DLLEXPORT inline Float2 operator-(const Float2& val) const
    {
        return Float2(X - val.X, Y - val.Y);
    }
    // negates all elements //
    DLLEXPORT inline Float2 operator-() const
    {
        return Float2(-X, -Y);
    }
    // multiplies elements together //
    DLLEXPORT inline Float2 operator*(const Float2& val) const
    {
        return Float2(X * val.X, Y * val.Y);
    }
    // multiply  by scalar f //
    DLLEXPORT inline Float2 operator*(float f) const
    {
        return Float2(X * f, Y * f);
    }

    DLLEXPORT inline Float2& operator*=(float f)
    {
        X *= f;
        Y *= f;
        DO_NAN_CHECK;
        return *this;
    }
    // divides all elements //
    DLLEXPORT inline Float2 operator/(const Float2& val) const
    {
        return Float2(X / val.X, Y / val.Y);
    }
    // divides by float //
    DLLEXPORT inline Float2 operator/(float f) const
    {
        return Float2(X / f, Y / f);
    }
    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float2& other) const
    {
        return X < other.X && Y < other.Y;
    };
    DLLEXPORT inline bool operator<=(const Float2& other) const
    {
        return X <= other.X && Y <= other.Y;
    };
    DLLEXPORT inline bool operator>(const Float2& other) const
    {
        return X > other.X && Y > other.Y;
    };
    DLLEXPORT inline bool operator>=(const Float2& other) const
    {
        return X >= other.X && Y >= other.Y;
    };
    DLLEXPORT inline bool operator==(const Float2& other) const
    {
        return X == other.X && Y == other.Y;
    };
    DLLEXPORT inline bool operator!=(const Float2& other) const
    {
        return X != other.X && Y != other.Y;
    };
    // ------------------ Functions ------------------ //
    DLLEXPORT inline float GetX() const
    {
        return X;
    }
    DLLEXPORT inline float GetY() const
    {
        return Y;
    }
    DLLEXPORT inline void SetX(const float& val)
    {
        X = val;
        DO_NAN_CHECK;
    };
    DLLEXPORT inline void SetY(const float& val)
    {
        Y = val;
        DO_NAN_CHECK;
    };

    // add all elements together //
    DLLEXPORT inline float HAdd() const
    {
        return X + Y;
    }
    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const
    {
        return std::fabs(X) + std::fabs(Y);
    }
    // getting min and max of objects //
    DLLEXPORT inline Float2 MinElements(const Float2& other) const
    {
        return Float2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
    }
    DLLEXPORT inline Float2 MaxElements(const Float2& other) const
    {
        return Float2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
    }
    // value clamping //
    DLLEXPORT inline Float2 Clamp(const Float2& min, const Float2& max)
    {
        const Float2 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float2& val) const
    {
        return X * val.X + Y * val.Y;
    }
    // length of the vector //
    DLLEXPORT inline float Length() const
    {
        return sqrt(X * X + Y * Y);
    }
    // normalizes the vector //
    DLLEXPORT inline Float2 Normalize() const
    {
        const float length = Length();
        if(length == 0)
            return Float2(0, 0);
        return Float2(X / length, Y / length);
    }
    // safe version of normalization //
    DLLEXPORT inline Float2 NormalizeSafe(const Float2& safer) const
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
    DLLEXPORT inline bool IsNormalized() const
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y - 1.0f) < NORMALIZATION_TOLERANCE;
    }
    // does linear interpolation between vectors and coefficient f, not limited to range [0,1],
    // courtesy of ozz-animation //
    DLLEXPORT inline Float2 Lerp(const Float2& other, float f) const
    {
        return Float2((other.X - X) * f + X, (other.Y - Y) * f + Y);
    }
    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float2& other, float tolerance) const
    {
        const Float2 difference = (*this) - other;
        return difference.Dot(difference) < tolerance * tolerance;
    }
    // ------------------------------------ //
    // static returns //
    // creates a Float2 with all zeros //
    DLLEXPORT inline static Float2 zero()
    {
        return Float2(0.f, 0.f);
    }
    // creates a Float2 with all ones //
    DLLEXPORT inline static Float2 one()
    {
        return Float2(1.f, 1.f);
    }

    // unitary vector x, to work with ozz declarations //
    DLLEXPORT inline static Float2 x_asix()
    {
        return Float2(1.f, 0.f);
    }
    // unitary vector y //
    DLLEXPORT inline static Float2 y_axis()
    {
        return Float2(0.f, 1.f);
    }
    // ----------------- casts ------------------- //
    // waiting for Microsoft's compilers to add support for "explicit" here //
    // DLLEXPORT inline operator D3DXVECTOR2(){
    //	return D3DXVECTOR2(X, Y);
    //}

    VALUE_TYPE(Float2);

    // data //
    float X, Y;
};
struct Float3 {
public:
    // Needed for returning from scripts
    DLLEXPORT Float3()
    {
        X = Y = Z = 0;
    };
    DLLEXPORT Float3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
        DO_NAN_CHECK;
    }
    DLLEXPORT Float3(Float2 floats, float z)
    {
        X = floats.X;
        Y = floats.Y;
        Z = z;
        DO_NAN_CHECK;
    }
    DLLEXPORT explicit Float3(float data)
    {
        X = Y = Z = data;
        DO_NAN_CHECK;
    }
    DLLEXPORT Float3(const Int3& values)
    {
        X = static_cast<float>(values.X);
        Y = static_cast<float>(values.Y);
        Z = static_cast<float>(values.Z);

        DO_NAN_CHECK;
    }
    DLLEXPORT Float3(const Float3& other)
    {
        // copy values //
        X = other.X;
        Y = other.Y;
        Z = other.Z;

        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const
    {
        if(!std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z)) {
            return true;
        }

        return false;
    }

    DLLEXPORT inline void CheckForNans()
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float3 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](const int& nindex)
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
    DLLEXPORT inline Float3 operator+(const Float3& val) const
    {
        return Float3(X + val.X, Y + val.Y, Z + val.Z);
    }
    DLLEXPORT inline Float3& operator+=(const Float3& val)
    {
        X += val.X;
        Y += val.Y;
        Z += val.Z;
        return *this;
    }
    DLLEXPORT inline Float3& operator-=(const Float3& val)
    {
        X -= val.X;
        Y -= val.Y;
        Z -= val.Z;
        return *this;
    }

    // subtracts all elements //
    DLLEXPORT inline Float3 operator-(const Float3& val) const
    {
        return Float3(X - val.X, Y - val.Y, Z - val.Z);
    }
    // negates all elements //
    DLLEXPORT inline Float3 operator-() const
    {
        return Float3(-X, -Y, -Z);
    }
    // multiplies elements together //
    DLLEXPORT inline Float3 operator*(const Float3& val) const
    {
        return Float3(X * val.X, Y * val.Y, Z * val.Z);
    }
    // Divides all elements by float //
    DLLEXPORT inline Float3 operator/(const float& val) const
    {
        return Float3(X / val, Y / val, Z / val);
    }
    DLLEXPORT inline Float3& operator/=(const float& val)
    {
        X /= val;
        Y /= val;
        Z /= val;
        DO_NAN_CHECK;
        return *this;
    }
    // multiply  by scalar f //
    DLLEXPORT inline Float3 operator*(float f) const
    {
        return Float3(X * f, Y * f, Z * f);
    }
    DLLEXPORT inline Float3& operator*=(float f)
    {
        X *= f;
        Y *= f;
        Z *= f;
        DO_NAN_CHECK;
        return *this;
    }
    // divides all elements //
    DLLEXPORT inline Float3 operator/(const Float3& val) const
    {
        return Float3(X / val.X, Y / val.Y, Z / val.Z);
    }
    // divides by float //
    DLLEXPORT inline Float3 operator/(float f) const
    {
        return Float3(X / f, Y / f, Z / f);
    }
    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float3& other) const
    {
        return X < other.X && Y < other.Y && Z < other.Z;
    };
    DLLEXPORT inline bool operator<=(const Float3& other) const
    {
        return X <= other.X && Y <= other.Y && Z <= other.Z;
    };
    DLLEXPORT inline bool operator>(const Float3& other) const
    {
        return X > other.X && Y > other.Y && Z > other.Z;
    };
    DLLEXPORT inline bool operator>=(const Float3& other) const
    {
        return X >= other.X && Y >= other.Y && Z > other.Z;
    };
    DLLEXPORT inline bool operator==(const Float3& other) const
    {
        return X == other.X && Y == other.Y && Z == other.Z;
    };
    DLLEXPORT inline bool operator!=(const Float3& other) const
    {
        return !(*this == other);
    };
    // ------------------ Functions ------------------ //
    DLLEXPORT inline float GetX() const
    {
        return X;
    };
    DLLEXPORT inline float GetY() const
    {
        return Y;
    };
    DLLEXPORT inline float GetZ() const
    {
        return Z;
    };
    DLLEXPORT inline void SetX(const float& val)
    {
        X = val;
        DO_NAN_CHECK;
    };
    DLLEXPORT inline void SetY(const float& val)
    {
        Y = val;
        DO_NAN_CHECK;
    };
    DLLEXPORT inline void SetZ(const float& val)
    {
        Z = val;
        DO_NAN_CHECK;
    };

    // add all elements together //
    DLLEXPORT inline float HAdd() const
    {
        return X + Y + Z;
    }
    // Add all elements together absoluted (abs()) //
    DLLEXPORT inline float HAddAbs() const
    {
        return std::abs(X) + std::abs(Y) + std::abs(Z);
    }
    // getting min and max of objects //
    DLLEXPORT inline Float3 MinElements(const Float3& other) const
    {
        return Float3(
            X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
    }
    DLLEXPORT inline Float3 MaxElements(const Float3& other) const
    {
        return Float3(
            X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
    }
    // value clamping //
    DLLEXPORT inline Float3 Clamp(const Float3& min, const Float3& max)
    {
        const Float3 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    DLLEXPORT inline Float3 DegreesToRadians()
    {

        return Float3(X * DEGREES_TO_RADIANS, Y * DEGREES_TO_RADIANS, Z * DEGREES_TO_RADIANS);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float3& val) const
    {
        return X * val.X + Y * val.Y + Z * val.Z;
    }
    DLLEXPORT inline Float3 Cross(const Float3& val) const
    {
        return Float3(Y * val.Z - val.Y * Z, Z * val.X - val.Z * X, X * val.Y - val.X * Y);
    }
    // length of the vector //
    DLLEXPORT inline float Length() const
    {
        return sqrt(X * X + Y * Y + Z * Z);
    }
    DLLEXPORT inline float LengthSquared() const
    {
        return X * X + Y * Y + Z * Z;
    }
    // normalizes the vector //
    DLLEXPORT inline Float3 Normalize() const
    {
        const float length = Length();
        if(length == 0)
            return Float3(0, 0, 0);
        return Float3(X / length, Y / length, Z / length);
    }
    // safe version of normalization //
    DLLEXPORT inline Float3 NormalizeSafe(const Float3& safer = Float3(1, 0, 0)) const
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
    DLLEXPORT inline bool IsNormalized() const
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y + Z * Z - 1.0f) < NORMALIZATION_TOLERANCE;
    }
    // does linear interpolation between vectors and coefficient f, not limited to range
    // [0,1], courtesy of ozz-animation //
    DLLEXPORT inline Float3 Lerp(const Float3& other, float f) const
    {
        return Float3((other.X - X) * f + X, (other.Y - Y) * f + Y, (other.Z - Z) * f + Z);
    }
    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float3& other, float tolerance) const
    {
        const Float3 difference = (*this) - other;
        return difference.Dot(difference) < tolerance * tolerance;
    }

    DLLEXPORT static inline Float3 CreateVectorFromAngles(const float& yaw, const float& pitch)
    {
        return Float3(-sin(yaw * DEGREES_TO_RADIANS), sin(pitch * DEGREES_TO_RADIANS),
            -cos(yaw * DEGREES_TO_RADIANS))
            .NormalizeSafe(Zeroed);
    }
    // ------------------------------------ //
    // functions to be compatible with ozz functions //
    // all zero values object //
    DLLEXPORT inline static Float3 zero()
    {
        return Float3(0.f, 0.f, 0.f);
    }
    // all ones //
    DLLEXPORT inline static Float3 one()
    {
        return Float3(1.f, 1.f, 1.f);
    }
    // unitary vectors //
    // x axis
    DLLEXPORT inline static Float3 x_axis()
    {
        return Float3(1.f, 0.f, 0.f);
    }

    // y axis
    DLLEXPORT inline static Float3 y_axis()
    {
        return Float3(0.f, 1.f, 0.f);
    }

    // z axis
    DLLEXPORT inline static Float3 z_axis()
    {
        return Float3(0.f, 0.f, 1.f);
    }
    // ----------------- casts ------------------- //
#ifdef LEVIATHAN_USING_OGRE
    DLLEXPORT Float3(const bs::Vector3& vec)
    {
        // copy values //
        X = vec.x;
        Y = vec.y;
        Z = vec.z;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline operator bs::Vector3() const
    {
        return bs::Vector3(X, Y, Z);
    }

    DLLEXPORT Float3(const btVector3& vec)
    {
        // copy values //
        X = vec.x();
        Y = vec.y();
        Z = vec.z();
        DO_NAN_CHECK;
    }

    DLLEXPORT inline operator btVector3() const
    {
        return btVector3(X, Y, Z);
    }
#endif // LEVIATHAN_USING_OGRE
    // ------------------------------------ //

    VALUE_TYPE(Float3);

    float X, Y, Z;

    static const Float3 UnitVForward;
    static const Float3 UnitVUp;
    static const Float3 Zeroed;
};
struct Float4 {
public:
    // Needed for returning from scripts
    DLLEXPORT Float4()
    {
        X = Y = Z = W = 0;
    };
    DLLEXPORT Float4(float f1, float f2, float f3, float f4)
    {
        X = f1;
        Y = f2;
        Z = f3;
        W = f4;
        DO_NAN_CHECK;
    }
    DLLEXPORT Float4(Float2 floats, float f3, float f4)
    {
        X = floats.X;
        Y = floats.Y;
        Z = f3;
        W = f4;
        DO_NAN_CHECK;
    }
    DLLEXPORT Float4(Float3 floats, float f4)
    {
        X = floats.X;
        Y = floats.Y;
        Z = floats.Z;
        W = f4;
        DO_NAN_CHECK;
    }
    DLLEXPORT explicit Float4(float val)
    {
        X = Y = Z = W = val;
        DO_NAN_CHECK;
    }

    DLLEXPORT inline bool HasInvalidValues() const
    {
        if(!std::isfinite(X) || !std::isfinite(Y) || !std::isfinite(Z) || !std::isfinite(W)) {
            return true;
        }

        return false;
    }

    DLLEXPORT inline void CheckForNans()
    {
        if(HasInvalidValues()) {
            DEBUG_BREAK;
            throw std::runtime_error("Float4 has NaNs (or infinites in it) in it!");
        }
    }

    // access operator //
    DLLEXPORT inline float& operator[](const int& nindex)
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
    DLLEXPORT inline operator float*()
    {
        // this should be always confirmed to work //
        return &X;
    }

    // ------------------- Operators ----------------- //
    // add elements //
    DLLEXPORT inline Float4 operator+(const Float4& val) const
    {
        return Float4(X + val.X, Y + val.Y, Z + val.Z, W + val.W);
    }
    // subtracts all elements //
    DLLEXPORT inline Float4 operator-(const Float4& val) const
    {
        return Float4(X - val.X, Y - val.Y, Z - val.Z, W - val.W);
    }
    // negates all elements //
    DLLEXPORT inline Float4 operator-() const
    {
        return Float4(-X, -Y, -Z, -W);
    }
    // multiplies elements together //
    DLLEXPORT inline Float4 operator*(const Float4& val) const
    {
        return Float4(X * val.X, Y * val.Y, Z * val.Z, W * val.W);
    }
    // multiply  by scalar f //
    DLLEXPORT inline Float4 operator*(float f) const
    {
        return Float4(X * f, Y * f, Z * f, W * f);
    }
    // divides all elements //
    DLLEXPORT inline Float4 operator/(const Float4& val) const
    {
        return Float4(X / val.X, Y / val.Y, Z / val.Z, W / val.W);
    }
    // divides by float //
    DLLEXPORT inline Float4 operator/(float f) const
    {
        return Float4(X / f, Y / f, Z / f, W / f);
    }
    // ---- comparison operators ---- //
    // element by element comparison with operators //
    DLLEXPORT inline bool operator<(const Float4& other) const
    {
        return !(*this == other);
    };
    DLLEXPORT inline bool operator>(const Float4& other) const
    {
        return !(*this == other);
    };
    DLLEXPORT inline bool operator==(const Float4& other) const
    {
        return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
    };
    DLLEXPORT inline bool operator!=(const Float4& other) const
    {
        return X != other.X && Y != other.Y && Z != other.Z && W != other.W;
    };
    // ------------------ Functions ------------------ //
    DLLEXPORT inline float GetX() const
    {
        return X;
    };
    DLLEXPORT inline float GetY() const
    {
        return Y;
    };
    DLLEXPORT inline float GetZ() const
    {
        return Z;
    };
    DLLEXPORT inline float GetW() const
    {
        return W;
    };
    DLLEXPORT inline void SetX(const float& val)
    {
        X = val;
        DO_NAN_CHECK;
    };
    DLLEXPORT inline void SetY(const float& val)
    {
        Y = val;
        DO_NAN_CHECK;
    };
    DLLEXPORT inline void SetZ(const float& val)
    {
        Z = val;
        DO_NAN_CHECK;
    };
    DLLEXPORT inline void SetW(const float& val)
    {
        W = val;
        DO_NAN_CHECK;
    };

    // add all elements together //
    DLLEXPORT inline float HAdd() const
    {
        return X + Y + Z + W;
    }
    // Add all elements together after abs() is called on each element //
    DLLEXPORT inline float HAddAbs() const
    {
        return std::abs(X) + std::abs(Y) + std::abs(Z) + std::abs(W);
    }
    // getting min and max of objects //
    DLLEXPORT inline Float4 MinElements(const Float4& other) const
    {
        return Float4(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y,
            Z < other.Z ? Z : other.Z, W < other.W ? W : other.W);
    }
    DLLEXPORT inline Float4 MaxElements(const Float4& other) const
    {
        return Float4(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y,
            Z > other.Z ? Z : other.Z, W > other.W ? W : other.W);
    }
    // value clamping //
    DLLEXPORT inline Float4 Clamp(const Float4& min, const Float4& max)
    {
        const Float4 minval = this->MinElements(max);
        return min.MaxElements(minval);
    }

    // ----------------- Vector math ------------------- //
    // dot product of the vectors //
    DLLEXPORT inline float Dot(const Float4& val) const
    {
        return X * val.X + Y * val.Y + Z * val.Z + W * val.W;
    }

    // length of the vector //
    DLLEXPORT inline float Length() const
    {
        return sqrt(X * X + Y * Y + Z * Z + W * W);
    }

    // normalizes the vector //
    DLLEXPORT inline Float4 Normalize() const
    {
        const float length = Length();

        if(length == 0) {
            // Returns an identity quaternion
            return Float4(0, 0, 0, 1);
        }

        return Float4(X / length, Y / length, Z / length, W / length);
    }
    // safe version of normalization //
    DLLEXPORT inline Float4 NormalizeSafe(const Float4& safer = Float4(0, 0, 0, 1)) const
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
    DLLEXPORT inline bool IsNormalized() const
    {
        // is absolute -1.f under normalization tolerance //
        return fabs(X * X + Y * Y + Z * Z + W * W - 1.0f) < NORMALIZATION_TOLERANCE;
    }
    // does linear interpolation between vectors and coefficient f,
    // not limited to range [0,1], courtesy of ozz-animation //
    DLLEXPORT inline Float4 Lerp(const Float4& other, float f) const
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

            float angle = acosf(dot);
            return ((*this) * sinf(angle * (1 - f)) + quaternion3 * sinf(angle * f)) /
                   sinf(angle);

        } else {
            // small angle, linear interpolation will be fine //
            return this->Lerp(quaternion3, f);
        }
    }

    // compares distance between vectors to tolerance, returns true if less //
    DLLEXPORT inline bool Compare(const Float4& other, float tolerance) const
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
    DLLEXPORT inline float ToAngle() const
    {

        return 2 * std::acos(W);
    }

    //! Inverts a quaternion
    DLLEXPORT inline Float4 Inverse() const
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

        // Math taken from bs::Quaternion
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
    DLLEXPORT inline static Float4 zero()
    {
        return Float4(0.f, 0.f, 0.f, 0.f);
    }

    // all ones //
    DLLEXPORT inline static Float4 one()
    {
        return Float4(1.f, 1.f, 1.f, 1.f);
    }
    // unitary vectors for ozz support //
    // x
    DLLEXPORT inline static Float4 x_axis()
    {
        return Float4(1.f, 0.f, 0.f, 0.f);
    }
    // y
    DLLEXPORT inline static Float4 y_axis()
    {
        return Float4(0.f, 1.f, 0.f, 0.f);
    }
    // z
    DLLEXPORT inline static Float4 z_axis()
    {
        return Float4(0.f, 0.f, 1.f, 0.f);
    }
    // w
    DLLEXPORT inline static Float4 w_axis()
    {
        return Float4(0.f, 0.f, 0.f, 1.f);
    }

#ifdef LEVIATHAN_USING_OGRE
    DLLEXPORT Float4(const bs::Quaternion& quat)
    {
        // copy values //
        X = quat.x;
        Y = quat.y;
        Z = quat.z;
        W = quat.w;
        DO_NAN_CHECK;
    }

    DLLEXPORT Float4(const bs::Color& colour)
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

    DLLEXPORT Float4(const btQuaternion& colour)
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
    DLLEXPORT inline Float3 QuaternionToEuler() const
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

    DLLEXPORT inline Float4 QuaternionMultiply(const Float4& other) const
    {

        Float4 result;

        result.X = X * other.X + X * other.W + Y * other.Z - Z * other.Y;
        result.Y = W * other.Y - X * other.Z + Y * other.W + Z * other.X;
        result.Z = W * other.Z + X * other.Y - Y * other.X + Z * other.W;
        result.W = W * other.W - X * other.X - Y * other.Y - Z * other.Z;

        return result;
    }

    DLLEXPORT inline Float4 QuaternionReverse() const
    {
        // reverse vector //
        return Float4(-X, -Y, -Z, W);
    }

    DLLEXPORT static inline Float4 IdentityQuaternion()
    {
        return Float4(0, 0, 0, 1);
    }

    // Math from here: https://stackoverflow.com/questions/12435671/quaternion-lookat-function
    DLLEXPORT static inline Float4 QuaternionLookAt(
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
    DLLEXPORT static inline Float4 CreateQuaternionFromAxisAngle(
        const Float3& axis, float angle)
    {
        const auto s = std::sin(angle / 2.0);
        const auto x = axis.X * s;
        const auto y = axis.Y * s;
        const auto z = axis.Z * s;
        const auto w = std::cos(angle / 2.0);
        return Float4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z),
            static_cast<float>(w));
    }

    DLLEXPORT static inline Float4 CreateAxisAngleFromEuler(const Float3& angles)
    {

        throw std::exception();
        // return Float4();
    }

    VALUE_TYPE(Float4);

    // ----------------- casts ------------------- //

    // ------------------------------------ //

    float X, Y, Z, W;


    // specific colours //
    static const Float4 ColourBlack;
    static const Float4 ColourWhite;
    static const Float4 ColourTransparent;

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
