
#ifndef LEVIATHAN_TYPES
#define LEVIATHAN_TYPES
// ----------------- //
// ------------------------------------ //

// ------------------------------------ //
// ---- includes ---- //
#include "OGRE/OgreQuaternion.h"
#include "OGRE/OgreColourValue.h"
#include "OGRE/OgreVector3.h"
#include "OGRE/OgreVector4.h"


namespace Leviathan{


#define NORMALIZATION_TOLERANCE	1e-6f

	// just a key index class //
	class CharWithIndex{
	public:
		CharWithIndex();
		CharWithIndex(wchar_t character, int index);
		~CharWithIndex();

		wchar_t Char;
		int Index;
	};

	struct IntWstring{
	public:
		IntWstring();
		IntWstring(const wstring& wstr, int value);
		~IntWstring();

		wstring* GetString();
		int GetValue();

		void SetString(const wstring& wstr);
		void SetValue(int value);

		shared_ptr<wstring> Wstr;
		int Value;
	};

	class AllocatedBinaryBlock{
	public:
		AllocatedBinaryBlock();
		AllocatedBinaryBlock(unsigned char* ptr, int elements, bool release);
		~AllocatedBinaryBlock();
		//~AllocatedBinaryBlock(bool force);

		unsigned char* Get(int& Elements);

	private:
		unsigned char* Buffer;
		int BufferSize; // this is actually size of the array NOT size of array in BYTES

		bool Release;
	};

	struct Int1{
	public:
		DLLEXPORT Int1();
		DLLEXPORT Int1(int data);

		// ------------------------------------ //
		DLLEXPORT Int1 operator +(const Int1& val);
		//DLLEXPORT int operator[]() const;

		DLLEXPORT operator int() const;

		DLLEXPORT int GetIntValue() const;
		DLLEXPORT void SetIntValue(int val);
		// ------------------------------------ //

		int iVal;
	};

	struct Int2{
	public:
		DLLEXPORT Int2();
		DLLEXPORT Int2(int x, int y);
		DLLEXPORT explicit Int2(int data);
		DLLEXPORT Int2(const Int2 &other);

		// ------------------------------------ //
		DLLEXPORT Int2 operator +(const Int2 &val);
		DLLEXPORT int operator[](const int nIndex) const;
		// ------------------------------------ //

		DLLEXPORT void SetData(const int &data){ X = data; Y = data; };
		DLLEXPORT void SetData(const int &data1, const int &data2){ X = data1; Y = data2; };

		int X, Y;
	};

	struct Int3{
	public:
		DLLEXPORT Int3();
		DLLEXPORT Int3(int x, int y, int z);
		DLLEXPORT explicit Int3(int data);

		// ------------------------------------ //
		DLLEXPORT Int3 operator +(const Int3 &val);
		DLLEXPORT int operator[](const int nIndex) const;
		DLLEXPORT Int3 operator -(const Int3& other) const;
		DLLEXPORT int AddAllTogether() const;
		// ------------------------------------ //

		int X, Y, Z;
	};

	struct Int4{
	public:
		DLLEXPORT Int4();
		DLLEXPORT Int4(int x, int y, int z, int w);
		DLLEXPORT explicit Int4(int data);

		// ------------------------------------ //
		DLLEXPORT Int4& operator +(const Int4 &val);
		DLLEXPORT int operator[](const int nIndex) const;
		DLLEXPORT Int4& operator -(const Int4& val);
		DLLEXPORT int AddAllTogether() const;
		// ------------------------------------ //

		int X, Y, Z, W;
	};

	struct UINT4{
	public:
		DLLEXPORT UINT4(UINT u1, UINT u2, UINT u3, UINT u4);
		DLLEXPORT UINT4();
		DLLEXPORT operator UINT*();
		//DLLEXPORT UINT& operator[](const int nIndex);
		//DLLEXPORT UINT4 operator*(UINT);

		UINT X, Y, Z, W;
	};

	//// not required //
	struct Float1{
	public:
		DLLEXPORT inline Float1(){
		}
		DLLEXPORT inline Float1(const float &data){
			X = data;
		}

		DLLEXPORT inline operator float& (){
			return X;
		}

		DLLEXPORT operator float() const{
			return X;
		}
		// ------------------------------------ //
		DLLEXPORT inline Float1 operator +(const Float1& val){
			return X+val.X;
		}
		DLLEXPORT inline Float1 operator +(const float& val){
			return X+val;
		}

		DLLEXPORT inline float GetFloatValue() const{
			return X;
		}
		DLLEXPORT inline void SetFloatValue(const float &val){
			X = val;
		}
		// ------------------------------------ //

		float X;
	};

	// ----------------- Float types ------------------- //
	// refactored to match declarations in ozz vec_float //

	// functions inlined just like in ozz, for speed, I guess //
	// mostly rewritten to match ozz implementation, just in case and not to break anything //

	struct Float2{
	public:
		DLLEXPORT inline Float2(){};
		DLLEXPORT inline Float2(float x, float y){
			X = x;
			Y = y;
		}
		DLLEXPORT inline explicit Float2(float both){
			X = Y = both;
		}

		// access operator //
		DLLEXPORT inline float& operator[](const int &nindex){
			switch (nindex){
			case 0: return X;
			case 1: return Y;
				// this should NEVER be hit //
			default: __assume(0);
			}
		}

		// ------------------- Operators ----------------- //
		// add elements //
		DLLEXPORT inline Float2 operator +(const Float2 &val) const{
			return Float2(X+val.X, Y+val.Y);
		}

		DLLEXPORT inline Float2* operator +=(const Float2 &val){
			X +=val.X;
			Y +=val.Y;
			return this;
		}
		// subtracts all elements //
		DLLEXPORT inline Float2 operator-(const Float2 &val) const{
			return Float2(X-val.X, Y-val.Y);
		}
		// negates all elements //
		DLLEXPORT inline Float2 operator-() const{
			return Float2(-X, -Y);
		}
		// multiplies elements together //
		DLLEXPORT inline Float2 operator*(const Float2 &val) const{
			return Float2(X*val.X, Y*val.Y);
		}
		// multiply  by scalar f //
		DLLEXPORT inline Float2 operator*(float f) const{
			return Float2(X*f, Y*f);
		}

		DLLEXPORT inline Float2* operator*=(float f){
			X *= f;
			Y *= f;
			return this;
		}
		// divides all elements //
		DLLEXPORT inline Float2 operator/(const Float2 &val) const{
			return Float2(X/val.X, Y/val.Y);
		}
		// divides by float //
		DLLEXPORT inline Float2 operator/(float f) const{
			return Float2(X/f, Y/f);
		}
		// ---- comparison operators ---- //
		// element by element comparison with operators //
		DLLEXPORT inline bool operator <(const Float2 &other) const{
			return X < other.X && Y < other.Y;
		};
		DLLEXPORT inline bool operator <=(const Float2 &other) const{
			return X <= other.X && Y <= other.Y;
		};
		DLLEXPORT inline bool operator >(const Float2 &other) const{
			return X > other.X && Y > other.Y;
		};
		DLLEXPORT inline bool operator >=(const Float2 &other) const{
			return X >= other.X && Y >= other.Y;
		};
		DLLEXPORT inline bool operator ==(const Float2 &other) const{
			return X == other.X && Y == other.Y;
		};
		DLLEXPORT inline bool operator !=(const Float2 &other) const{
			return X != other.X && Y != other.Y;
		};
		// ------------------ Functions ------------------ //
		DLLEXPORT inline float GetX() const{return X;}
		DLLEXPORT inline float GetY() const{return Y;}
		DLLEXPORT inline void SetX(const float &val){ X = val; };
		DLLEXPORT inline void SetY(const float &val){ Y= val; };

		// add all elements together //
		DLLEXPORT inline float HAdd() const{
			return X+Y;
		}
		// Add all elements together after abs() is called on each element //
		DLLEXPORT inline float HAddAbs() const{
			return abs(X)+abs(Y);
		}
		// getting min and max of objects //
		DLLEXPORT inline Float2 MinElements(const Float2 &other) const{
			return Float2(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y);
		}
		DLLEXPORT inline Float2 MaxElements(const Float2 &other) const{
			return Float2(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y);
		}
		// value clamping //
		DLLEXPORT inline Float2 Clamp(const Float2 &min, const Float2 &max){
			const Float2 minval = this->MinElements(max);
			return min.MaxElements(minval);
		}

		// ----------------- Vector math ------------------- //
		// dot product of the vectors //
		DLLEXPORT inline float Dot(const Float2 &val) const{
			return X*val.X+Y*val.Y;
		}
		// length of the vector //
		DLLEXPORT inline float Length() const{
			return sqrt(X*X+Y*Y);
		}
		// normalizes the vector //
		DLLEXPORT inline Float2 Normalize() const{
			const float length = Length();
			if(length == 0)
				return Float2(0, 0);
			return Float2(X/length, Y/length);
		}
		// safe version of normalization //
		DLLEXPORT inline Float2 NormalizeSafe(const Float2 &safer) const{
			// security //
			assert(safer.IsNormalized() && "safer not normalized");
			const float len = X*X+Y*Y;
			if(len == 0){
				return safer;
			}
			const float length = sqrt(len);
			return Float2(X/length, Y/length);
		}
		// checks is the vector normalized //
		DLLEXPORT inline bool IsNormalized() const{
			// is absolute -1.f under normalization tolerance //
			return fabs(X*X+Y*Y -1.0f) < NORMALIZATION_TOLERANCE;
		}
		// does linear interpolation between vectors and coefficient f, not limited to range [0,1], courtesy of ozz-animation //
		DLLEXPORT inline Float2 Lerp(const Float2 &other, float f) const{
			return Float2((other.X-X) * f + X, (other.Y-Y)*f+Y);
		}
		// compares distance between vectors to tolerance, returns true if less //
		DLLEXPORT inline bool Compare(const Float2 &other, float tolerance) const{
			const Float2 difference = (*this)-other;
			return difference.Dot(difference) < tolerance*tolerance;
		}
		// ------------------------------------ //
		// static returns //
		// creates a Float2 with all zeros //
		DLLEXPORT inline static Float2 zero(){
			return Float2(0.f, 0.f);
		}
		// creates a Float2 with all ones //
		DLLEXPORT inline static Float2 one(){
			return Float2(1.f, 1.f);
		}

		// unitary vector x, to work with ozz declarations //
		DLLEXPORT inline static Float2 x_asix(){
			return Float2(1.f, 0.f);
		}
		// unitary vector y //
		DLLEXPORT inline static Float2 y_axis(){
			return Float2(0.f, 1.f);
		}
		// ----------------- casts ------------------- //
		// waiting for Microsoft's compilers to add support for "explicit" here //
		//DLLEXPORT inline operator D3DXVECTOR2(){
		//	return D3DXVECTOR2(X, Y);
		//}


		// data //
		float X, Y;
	};
	struct Float3{
	public:
		DLLEXPORT Float3(){};
		DLLEXPORT Float3(float x, float y, float z){
			X = x;
			Y = y;
			Z = z;
		}
		DLLEXPORT Float3(Float2 floats, float z){
			X = floats.X;
			Y = floats.Y;
			Z = z;
		}
		DLLEXPORT explicit Float3(float data){
			X = Y = Z = data;
		}
		DLLEXPORT Float3(const Float3 &other){
			// copy values //
			X = other.X;
			Y = other.Y;
			Z = other.Z;
		}
		DLLEXPORT Float3(const Ogre::Vector3 &vec){
			// copy values //
			X = vec.x;
			Y = vec.y;
			Z = vec.z;
		}

		// access operator //
		DLLEXPORT inline float& operator[](const int &nindex){
			switch (nindex){
			case 0: return X;
			case 1: return Y;
			case 2: return Z;
			}
            throw exception();
		}

		// ------------------- Operators ----------------- //
		// add elements //
		DLLEXPORT inline Float3 operator +(const Float3 &val) const{
			return Float3(X+val.X, Y+val.Y, Z+val.Z);
		}
		DLLEXPORT inline Float3* operator +=(const Float3 &val){
			X += val.X;
			Y += val.Y;
			Z += val.Z;
			return this;
		}
		// subtracts all elements //
		DLLEXPORT inline Float3 operator-(const Float3 &val) const{
			return Float3(X-val.X, Y-val.Y, Z-val.Z);
		}
		// negates all elements //
		DLLEXPORT inline Float3 operator-() const{
			return Float3(-X, -Y, -Z);
		}
		// multiplies elements together //
		DLLEXPORT inline Float3 operator*(const Float3 &val) const{
			return Float3(X*val.X, Y*val.Y, Z*val.Z);
		}
		// Divides all elements by float //
		DLLEXPORT inline Float3 operator/(const float &val) const{
			return Float3(X/val, Y/val, Z/val);
		}
		DLLEXPORT inline Float3* operator/=(const float &val){
			X /= val;
			Y /= val;
			Z /= val;
			return this;
		}
		// multiply  by scalar f //
		DLLEXPORT inline Float3 operator*(float f) const{
			return Float3(X*f, Y*f, Z*f);
		}
		DLLEXPORT inline Float3* operator*=(float f){
			X *= f;
			Y *= f;
			Z *= f;
			return this;
		}
		// divides all elements //
		DLLEXPORT inline Float3 operator/(const Float3 &val) const{
			return Float3(X/val.X, Y/val.Y, Z/val.Z);
		}
		// divides by float //
		DLLEXPORT inline Float3 operator/(float f) const{
			return Float3(X/f, Y/f, Z/f);
		}
		// ---- comparison operators ---- //
		// element by element comparison with operators //
		DLLEXPORT inline bool operator <(const Float3 &other) const{
			return X < other.X && Y < other.Y && Z < other.Z;
		};
		DLLEXPORT inline bool operator <=(const Float3 &other) const{
			return X <= other.X && Y <= other.Y && Z <= other.Z;
		};
		DLLEXPORT inline bool operator >(const Float3 &other) const{
			return X > other.X && Y > other.Y && Z > other.Z;
		};
		DLLEXPORT inline bool operator >=(const Float3 &other) const{
			return X >= other.X && Y >= other.Y && Z > other.Z;
		};
		DLLEXPORT inline bool operator ==(const Float3 &other) const{
			return X == other.X && Y == other.Y && Z == other.Z;
		};
		DLLEXPORT inline bool operator !=(const Float3 &other) const{
			return X != other.X && Y != other.Y && Z != other.Z;
		};
		// ------------------ Functions ------------------ //
		DLLEXPORT inline float GetX() const{return X;};
		DLLEXPORT inline float GetY() const{return Y;};
		DLLEXPORT inline float GetZ() const{return Z;};
		DLLEXPORT inline void SetX(const float &val){ X = val; };
		DLLEXPORT inline void SetY(const float &val){ Y= val; };
		DLLEXPORT inline void SetZ(const float &val){ Z= val; };

		// add all elements together //
		DLLEXPORT inline float HAdd() const{
			return X+Y+Z;
		}
		// Add all elements together absoluted (abs()) //
		DLLEXPORT inline float HAddAbs() const{
			return abs(X)+abs(Y)+abs(Z);
		}
		// getting min and max of objects //
		DLLEXPORT inline Float3 MinElements(const Float3 &other) const{
			return Float3(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z);
		}
		DLLEXPORT inline Float3 MaxElements(const Float3 &other) const{
			return Float3(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z);
		}
		// value clamping //
		DLLEXPORT inline Float3 Clamp(const Float3 &min, const Float3 &max){
			const Float3 minval = this->MinElements(max);
			return min.MaxElements(minval);
		}


		DLLEXPORT inline Float3 DegreesToRadians(){

			return Float3(X*DEGREES_TO_RADIANS, Y*DEGREES_TO_RADIANS, Z*DEGREES_TO_RADIANS);
		}

		// ----------------- Vector math ------------------- //
		// dot product of the vectors //
		DLLEXPORT inline float Dot(const Float3 &val) const{
			return X*val.X+Y*val.Y+Z*val.Z;
		}
		DLLEXPORT inline Float3 Cross(const Float3& val) {
			return Float3(Y*val.Z - val.Y*Z, Z*val.X - val.Z * X, X*val.Y - val.X*Y);
		}
		// length of the vector //
		DLLEXPORT inline float Length() const{
			return sqrt(X*X+Y*Y+Z*Z);
		}
		// normalizes the vector //
		DLLEXPORT inline Float3 Normalize() const{
			const float length = Length();
			if(length == 0)
				return Float3(0, 0, 0);
			return Float3(X/length, Y/length, Z/length);
		}
		// safe version of normalization //
		DLLEXPORT inline Float3 NormalizeSafe(const Float3 &safer = Float3(1, 0, 0)) const{
			// security //
			//assert(safer.IsNormalized() && "safer not normalized");
			const float len = X*X+Y*Y+Z*Z;
			if(len == 0){
				return safer;
			}
			const float length = sqrt(len);
			return Float3(X/length, Y/length, Z/length);
		}
		// checks is the vector normalized //
		DLLEXPORT inline bool IsNormalized() const{
			// is absolute -1.f under normalization tolerance //
			return fabs(X*X+Y*Y+Z*Z -1.0f) < NORMALIZATION_TOLERANCE;
		}
		// does linear interpolation between vectors and coefficient f, not limited to range [0,1], courtesy of ozz-animation //
		DLLEXPORT inline Float3 Lerp(const Float3 &other, float f) const{
			return Float3((other.X-X)*f + X, (other.Y-Y)*f + Y, (other.Z-Z)*f + Z);
		}
		// compares distance between vectors to tolerance, returns true if less //
		DLLEXPORT inline bool Compare(const Float3 &other, float tolerance) const{
			const Float3 difference = (*this)-other;
			return difference.Dot(difference) < tolerance*tolerance;
		}

		DLLEXPORT static inline Float3 CreateVectorFromAngles(const float &yaw, const float &pitch){

			return Float3(-sin(yaw*DEGREES_TO_RADIANS), sin(pitch*DEGREES_TO_RADIANS), -cos(yaw*DEGREES_TO_RADIANS)).
				//NormalizeSafe(Float3::UnitVForward);
				NormalizeSafe(Zeroed);
		}
		// ------------------------------------ //
		// functions to be compatible with ozz functions //
		// all zero values object //
		DLLEXPORT inline static Float3 zero(){
			return Float3(0.f, 0.f, 0.f);
		}
		// all ones //
		DLLEXPORT inline static Float3 one(){
			return Float3(1.f, 1.f, 1.f);
		}
		// unitary vectors //
		// x axis
		DLLEXPORT inline static Float3 x_axis(){
			return Float3(1.f, 0.f, 0.f);
		}

		// y axis
		DLLEXPORT inline static Float3 y_axis(){
			return Float3(0.f, 1.f, 0.f);
		}

		// z axis
		DLLEXPORT inline static Float3 z_axis(){
			return Float3(0.f, 0.f, 1.f);
		}
		// ----------------- casts ------------------- //
		// waiting for Microsoft's compilers to add support for "explicit" here //
		//DLLEXPORT inline operator D3DXVECTOR3(){
		//	return D3DXVECTOR3(X, Y, Z);
		//}

		DLLEXPORT inline operator Ogre::Vector3(){
			return Ogre::Vector3(X, Y, Z);
		}


		// ------------------------------------ //


		float X, Y, Z;

		static const Float3 UnitVForward;
		static const Float3 Zeroed;

	};
	struct Float4
	{
	public:
		DLLEXPORT Float4(){};
		DLLEXPORT Float4(float f1, float f2, float f3, float f4){
			X = f1;
			Y = f2;
			Z = f3;
			W = f4;
		}
		DLLEXPORT Float4(Float2 floats, float f3, float f4){
			X = floats.X;
			Y = floats.Y;
			Z = f3;
			W = f4;
		}
		DLLEXPORT Float4(Float3 floats, float f4){
			X = floats.X;
			Y = floats.Y;
			Z = floats.Z;
			W = f4;
		}
		DLLEXPORT explicit Float4(float val){
			X = Y = Z = W = val;
		}
		DLLEXPORT Float4(const Ogre::Quaternion &quat){
			// copy values //
			X = quat.x;
			Y = quat.y;
			Z = quat.z;
			W = quat.w;
		}

		// access operator //
		DLLEXPORT inline float& operator[](const int &nindex){
			switch (nindex){
			case 0: return X;
			case 1: return Y;
			case 2: return Z;
			case 3: return W;
				// this should NEVER be hit //
			default: __assume(0);
			}
		}

		//! return first value of {X, Y, Z, W} as a pointer
		DLLEXPORT inline operator float* (){
			// this should be always confirmed to work //
			return &X;
		}

		// ------------------- Operators ----------------- //
		// add elements //
		DLLEXPORT inline Float4 operator +(const Float4 &val) const{
			return Float4(X+val.X, Y+val.Y, Z+val.Z, W+val.W);
		}
		// subtracts all elements //
		DLLEXPORT inline Float4 operator-(const Float4 &val) const{
			return Float4(X-val.X, Y-val.Y, Z-val.Z, W-val.W);
		}
		// negates all elements //
		DLLEXPORT inline Float4 operator-() const{
			return Float4(-X, -Y, -Z, -W);
		}
		// multiplies elements together //
		DLLEXPORT inline Float4 operator*(const Float4 &val) const{
			return Float4(X*val.X, Y*val.Y, Z*val.Z, W*val.W);
		}
		// multiply  by scalar f //
		DLLEXPORT inline Float4 operator*(float f) const{
			return Float4(X*f, Y*f, Z*f, W*f);
		}
		// divides all elements //
		DLLEXPORT inline Float4 operator/(const Float4 &val) const{
			return Float4(X/val.X, Y/val.Y, Z/val.Z, W/val.W);
		}
		// divides by float //
		DLLEXPORT inline Float4 operator/(float f) const{
			return Float4(X/f, Y/f, Z/f, W/f);
		}
		// ---- comparison operators ---- //
		// element by element comparison with operators //
		DLLEXPORT inline bool operator <(const Float4 &other) const{
			return !(*this == other);
		};
		DLLEXPORT inline bool operator >(const Float4 &other) const{
			return !(*this == other);
		};
		DLLEXPORT inline bool operator ==(const Float4 &other) const{
			return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
		};
		DLLEXPORT inline bool operator !=(const Float4 &other) const{
			return X != other.X && Y != other.Y && Z != other.Z && W != other.W;
		};
		// ------------------ Functions ------------------ //
		DLLEXPORT inline float GetX() const{return X;};
		DLLEXPORT inline float GetY() const{return Y;};
		DLLEXPORT inline float GetZ() const{return Z;};
		DLLEXPORT inline float GetW() const{return W;};
		DLLEXPORT inline void SetX(const float &val){ X = val; };
		DLLEXPORT inline void SetY(const float &val){ Y= val; };
		DLLEXPORT inline void SetZ(const float &val){ Z= val; };
		DLLEXPORT inline void SetW(const float &val){ W= val; };

		// add all elements together //
		DLLEXPORT inline float HAdd() const{
			return X+Y+Z+W;
		}
		// Add all elements together after abs() is called on each element //
		DLLEXPORT inline float HAddAbs() const{
			return abs(X)+abs(Y)+abs(Z)+abs(W);
		}
		// getting min and max of objects //
		DLLEXPORT inline Float4 MinElements(const Float4 &other) const{
			return Float4(X < other.X ? X : other.X, Y < other.Y ? Y : other.Y, Z < other.Z ? Z : other.Z, W < other.W ? W : other.W);
		}
		DLLEXPORT inline Float4 MaxElements(const Float4 &other) const{
			return Float4(X > other.X ? X : other.X, Y > other.Y ? Y : other.Y, Z > other.Z ? Z : other.Z, W > other.W ? W : other.W);
		}
		// value clamping //
		DLLEXPORT inline Float4 Clamp(const Float4 &min, const Float4 &max){
			const Float4 minval = this->MinElements(max);
			return min.MaxElements(minval);
		}

		// ----------------- Vector math ------------------- //
		// dot product of the vectors //
		DLLEXPORT inline float Dot(const Float4 &val) const{
			return X*val.X+Y*val.Y+Z*val.Z+W*val.W;
		}
		// length of the vector //
		DLLEXPORT inline float Length() const{
			return sqrt(X*X+Y*Y+Z*Z+W*W);
		}
		// normalizes the vector //
		DLLEXPORT inline Float4 Normalize() const{
			const float length = Length();
			if(length == 0)
				return Float4(0, 0, 0, 0);
			return Float4(X/length, Y/length, Z/length, W/length);
		}
		// safe version of normalization //
		DLLEXPORT inline Float4 NormalizeSafe(const Float4 &safer = Float4(0, 0, 0, 1)) const{
			// security //
			assert(safer.IsNormalized() && "safer not normalized");
			const float len = X*X+Y*Y+Z*Z+W*W;
			if(len == 0){
				return safer;
			}
			const float length = sqrt(len);
			return Float4(X/length, Y/length, Z/length, W/length);
		}
		// checks is the vector normalized //
		DLLEXPORT inline bool IsNormalized() const{
			// is absolute -1.f under normalization tolerance //
			return fabs(X*X+Y*Y+Z*Z+W*W -1.0f) < NORMALIZATION_TOLERANCE;
		}
		// does linear interpolation between vectors and coefficient f, not limited to range [0,1], courtesy of ozz-animation //
		DLLEXPORT inline Float4 Lerp(const Float4 &other, float f) const{
			return Float4((other.X-X)*f + X, (other.Y-Y)*f + Y, (other.Z-Z)*f + Z, (other.W-W)*f + W);
		}

		// does SPHERICAL interpolation between quaternions //
		DLLEXPORT inline Float4 Slerp(const Float4 &other, float f){
			// extra quaternion for calculations //
			Float4 quaternion3;

			// dot product of both //
			float dot = this->Dot(other);

			if(dot < 0){

				dot = -dot;
				quaternion3 = -other;
			} else {
				quaternion3 = other;
			}

			if(dot < 0.95f){

				float angle = acosf(dot);
				return ((*this)*sinf(angle*(1-f))+quaternion3*sinf(angle*f))/sinf(angle);

			} else {
				// small angle, linear interpolation will be fine //
				return this->Lerp(quaternion3, f);
			}

		}

		// compares distance between vectors to tolerance, returns true if less //
		DLLEXPORT inline bool Compare(const Float4 &other, float tolerance) const{
			const Float4 difference = (*this)-other;
			return difference.Dot(difference) < tolerance*tolerance;
		}
		// ------------------------------------ //
		// All zeros //
		DLLEXPORT inline static Float4 zero() {
			return Float4(0.f, 0.f, 0.f, 0.f);
		}

		// all ones //
		DLLEXPORT inline static Float4 one() {
			return Float4(1.f, 1.f, 1.f, 1.f);
		}
		// unitary vectors for ozz support //
		// x
		DLLEXPORT inline static Float4 x_axis() {
			return Float4(1.f, 0.f, 0.f, 0.f);
		}
		// y
		DLLEXPORT inline static Float4 y_axis() {
			return Float4(0.f, 1.f, 0.f, 0.f);
		}
		// z
		DLLEXPORT inline static Float4 z_axis() {
			return Float4(0.f, 0.f, 1.f, 0.f);
		}
		// w
		DLLEXPORT inline static Float4 w_axis() {
			return Float4(0.f, 0.f, 0.f, 1.f);
		}

        // ----------------- Quaternions ------------------- //
		DLLEXPORT static inline Float4 CreateQuaternionFromAngles(const Float3 &angles){
			// multiplied by 0.5 to get double the value //
			float cosx = cosf(0.5f*angles.X);
			float cosy = cosf(0.5f*angles.Y);
			float cosz = cosf(0.5f*angles.Z);

			float sinx = sinf(0.5f*angles.X);
			float siny = sinf(0.5f*angles.Y);
			float sinz = sinf(0.5f*angles.Z);


			Float4 quaternion((Float4)0);
			// compute quaternion //
			quaternion.X = cosz*cosy*sinx-sinz*siny*cosx;
			quaternion.Y = cosz*siny*cosx+sinz*cosy*sinx;
			quaternion.Z = sinz*cosy*cosx-cosz*siny*sinx;
			quaternion.W = cosz*cosy*cosx*sinz*siny*sinx;

			return quaternion;
		}

		DLLEXPORT inline Float4 QuaternionMultiply(const Float4 &other) const{

			Float4 result;

			result.X = X*other.X+X*other.W+Y*other.Z-Z*other.Y;
			result.Y = W*other.Y-X*other.Z+Y*other.W+Z*other.X;
			result.Z = W*other.Z+X*other.Y-Y*other.X+Z*other.W;
			result.W = W*other.W-X*other.X-Y*other.Y-Z*other.Z;

			return result;
		}

		DLLEXPORT inline Float4 QuaternionReverse() const{
			// reverse vector //
			return Float4(-X, -Y, -Z, W);
		}

		DLLEXPORT static inline Float4 IdentityQuaternion(){

			return Float4(0, 0, 0, 1);
		}




		DLLEXPORT static inline Float4 CreateAxisAngleFromEuler(const Float3 &angles){


			DEBUG_BREAK;
			return Float4();
		}


		// ----------------- casts ------------------- //
		//DLLEXPORT inline operator D3DXVECTOR4() const{
		//	return D3DXVECTOR4(X, Y, Z, W);
		//}
		//DLLEXPORT inline operator D3DXQUATERNION() const{
		//	return D3DXQUATERNION(X, Y, Z, W);
		//}

		DLLEXPORT inline operator Ogre::Quaternion() const{

			return Ogre::Quaternion(W, X, Y, Z);
		}

		DLLEXPORT inline operator Ogre::ColourValue() const{

			return Ogre::ColourValue(X, Y, Z, W);
		}
		DLLEXPORT inline operator Ogre::Vector4() const{

			return Ogre::Vector4(X, Y, Z, W);
		}

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


}
#endif
