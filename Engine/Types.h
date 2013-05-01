#ifndef LEVIATHAN_TYPES
#define LEVIATHAN_TYPES
// ----------------- //
#ifndef LEVIATHAN_INCLUDE
#include "Include.h"
#endif
namespace Leviathan{

	// a hacky combine class //
	
	template<class Base, class Base2>
	class CombinedClass : public Base, public Base2{
	public:
		DLLEXPORT void ThisIsCombinedClassType() const{
			return;
		}
		DLLEXPORT const wstring& GetBaseClassNames() const{

			return wstring(Convert::StringToWstring(string(typeid(Base).name()+" , "+typeid(Base2).name())));
		}
	};

	// just a key index class //
	class CharWithIndex{
	public:
		CharWithIndex();
		CharWithIndex(wchar_t character, int index);
		~CharWithIndex();

		wchar_t Char;
		int Index;
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

		// ------------ //
		DLLEXPORT Int1 operator +(const Int1& val);
		DLLEXPORT int operator[](const int nIndex) const;

		DLLEXPORT operator int() const;

		DLLEXPORT int GetIntValue() const;
		DLLEXPORT void SetIntValue(int val);
		// ------------ //

		int iVal;



	};

	struct Float1{
	public:
		DLLEXPORT Float1();
		DLLEXPORT Float1(float data);

		// ------------ //
		DLLEXPORT Float1 operator +(const Float1& val);
		DLLEXPORT float operator[](const int nIndex) const;

		DLLEXPORT operator float() const;

		DLLEXPORT float GetFloatValue() const;
		DLLEXPORT void SetFloatValue(float val);
		// ------------ //

		float fVal;
	};

	struct Int2{
	public:
		DLLEXPORT Int2();
		DLLEXPORT Int2(int x, int y);
		DLLEXPORT Int2(int data);
		DLLEXPORT Int2(const Int2 &other);

		// ------------ //
		DLLEXPORT Int2 operator +(Int2 val);
		DLLEXPORT int operator[](const int nIndex) const;
		// ------------ //

		DLLEXPORT inline void SetData(const int &data){ Val[0] = data; Val[1] = data; };
		DLLEXPORT inline void SetData(const int &data1, const int &data2){ Val[0] = data1; Val[1] = data2; };

		int Val[2];



	};

	struct Int3{
	public:
		DLLEXPORT Int3();
		//DLLEXPORT Int3(const Int3& other);
		DLLEXPORT Int3(int x, int y, int z);
		DLLEXPORT Int3(int data);

		// ------------ //
		//DLLEXPORT Int3& operator=(const Int3& other);
		DLLEXPORT Int3 operator +(Int3 val);
		DLLEXPORT int operator[](const int nIndex) const;
		DLLEXPORT Int3 operator -(const Int3& other) const;
		DLLEXPORT int AddAllTogether() const;
		// ------------ //

		int Val[3];



	};

	struct Float2{
	public:
		DLLEXPORT Float2();
		DLLEXPORT Float2(float x, float y);
		DLLEXPORT Float2(float data);

		// ------------ //
		DLLEXPORT Float2 operator +(Float2 val);
		DLLEXPORT float operator[](const int nIndex) const;
		// ------------ //
		DLLEXPORT float GetX() const;
		DLLEXPORT float GetY() const;
		DLLEXPORT void SetX(const float &val);
		DLLEXPORT void SetY(const float &val);
		float Val[2];



	};
	struct Float3{
	public:
		DLLEXPORT Float3();
		DLLEXPORT Float3(float x, float y, float z);
		DLLEXPORT Float3(float data);
		DLLEXPORT Float3(const Float3 &other);

		// ------------ //
		DLLEXPORT Float3 operator +(const Float3 &other) const;
		DLLEXPORT void operator /=(float val);
		DLLEXPORT float operator[](const int nIndex) const;
		DLLEXPORT Float3 operator -(const Float3& other) const;
		DLLEXPORT float AddAllTogether() const;
		DLLEXPORT bool operator >(const float& val) const;
		DLLEXPORT bool operator <(const float& val) const;
		DLLEXPORT bool operator ==(const Float3& other) const;
		// ------------ //


		// 3d vector functions //
		DLLEXPORT float Lenght() const;
		DLLEXPORT void Normalize();
		DLLEXPORT Float3 Cross(const Float3& other) const;
		DLLEXPORT Float3 Dot(const Float3& other) const;

		// ------------ //
		DLLEXPORT float X() const;
		DLLEXPORT float Y() const;
		DLLEXPORT float Z() const;
		DLLEXPORT void X(const float &x);
		DLLEXPORT void Y(const float &y);
		DLLEXPORT void Z(const float &z);
		float Val[3];



	};
	struct Float4
	{
	public:
		DLLEXPORT Float4(float Float1, float Float2, float Float3, float Float4);
		DLLEXPORT Float4();
		DLLEXPORT operator float*();
		DLLEXPORT float* operator--();
		DLLEXPORT float* operator++();
		DLLEXPORT float operator--(int);
		DLLEXPORT float operator++(int);
		DLLEXPORT bool operator <(float val);
		DLLEXPORT bool operator >(float val);
		DLLEXPORT float& operator[](const int nIndex);
		DLLEXPORT Float4 operator*(float);
	private:
		float Ops[4];
	};

	struct UINT4{
	public:
		DLLEXPORT UINT4(UINT UINT1, UINT UINT2, UINT UINT3, UINT UINT4);
		DLLEXPORT UINT4();
		DLLEXPORT operator UINT*();
		DLLEXPORT UINT& operator[](const int nIndex);
		//DLLEXPORT UINT4 operator*(UINT);

	private:
		UINT Ops[4];
	};

	struct IntWstring{
	public:
		IntWstring();
		IntWstring(const wstring& wstr, int value);
		~IntWstring();
		
		wstring* GetString();
		int GetValue();

		void SetString(wstring& wstr);
		void SetValue(int value);

		wstring* Wstr;
		int Value;
	};
	//struct VERTEX
	//{
	//	FLOAT X, Y, Z;      // position
	//	Float2 Texcoord;    // texture coordinate
	//};
	//struct ObjectRenderingFace{
	//public:
	//	DLLEXPORT ObjectRenderingFace();
	//	DLLEXPORT ObjectRenderingFace(int vert1, int tcord1, int fnorm1, int vert2, int tcord2, int fnorm2, int vert3, int tcord3, int fnorm3);
	//	//DLLEXPORT ObjectRenderingFace(int vert1, int fnorm1, int vert2, int fnorm2, int vert3, int fnorm3);
	//	//DLLEXPORT ObjectRenderingFace(int vert1, int tcord1 , int vert2, int tcord2, int vert3, int tcord3);
	//	DLLEXPORT ObjectRenderingFace(int vert1 , int vert2,int vert3);

	//	int IDs[9];



	//};

	//struct VERTEX
	//{
	//	D3DXVECTOR3 position;
	//	D3DXVECTOR3 normal;
	//	D3DXVECTOR2 texcoord;
	//};

	//struct Material
	//{
	//	WCHAR   strName[MAX_PATH];

	//	D3DXVECTOR3 vAmbient;
	//	D3DXVECTOR3 vDiffuse;
	//	D3DXVECTOR3 vSpecular;

	//	int nShininess;
	//	float fAlpha;

	//	bool bSpecular;

	//	WCHAR   strTexture[MAX_PATH];
	//	ID3D10ShaderResourceView* pTextureRV10;
	//	ID3D10EffectTechnique*  pTechnique;
	//};
}
#endif