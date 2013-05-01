#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TYPES
#include "Types.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
// --- AllocatedBinaryBlock --- //


AllocatedBinaryBlock::AllocatedBinaryBlock(){

}
AllocatedBinaryBlock::AllocatedBinaryBlock(unsigned char* ptr, int elements, bool release){
	Buffer = ptr;
	BufferSize = elements;
	Release = release;
}
AllocatedBinaryBlock::~AllocatedBinaryBlock(){
	if(Release){
		delete Buffer;
	}
}
unsigned char* AllocatedBinaryBlock::Get(int& Elements){
	Elements = BufferSize;
	return Buffer;
}
	//private:
	//	char* Buffer;
	//	int BufferSize;

	//	bool Release;
	//};


// ---- Int2 ---- //
Int2::Int2(){
	Val[0] = 0;
	Val[1] = 0;
}
Int2::Int2(int x, int y){
	Val[0] = x;
	Val[1] = y;
}
Int2::Int2(int data){
	Val[0] = data;
	Val[1] = data;
}

DLLEXPORT Leviathan::Int2::Int2(const Int2 &other){
	Val[0] = other.Val[0];
	Val[1] = other.Val[1];
}

//Int3::Int3(const Int3& other){
//
//
//}
//
//
//Int3& Int3::operator=(const Int3& other){
//
//
//}

Int2 Int2::operator +(Int2 val){
	Int2 ret;
	for(int i = 0; i < 2; i++){
		ret.Val[i] = (this->Val[i] + val[i]);
	}
	return ret;
}
int Int2::operator[](const int nIndex) const{
	return Val[nIndex];
}
// ---- Int3 ---- //
Int3::Int3(){
	Val[0] = 0;
	Val[1] = 0;
	Val[2] = 0;
}
Int3::Int3(int x, int y, int z){
	Val[0] = x;
	Val[1] = y;
	Val[2] = z;
}
Int3::Int3(int data){
	Val[0] = data;
	Val[1] = data;
	Val[2] = data;
}
//Int3::Int3(const Int3& other){
//
//
//}
//
//
//Int3& Int3::operator=(const Int3& other){
//
//
//}

Int3 Int3::operator +(Int3 val){
	Int3 ret;
	for(int i = 0; i < 3; i++){
		ret.Val[i] = (this->Val[i] + val[i]);
	}
	return ret;
}
int Int3::operator[](const int nIndex) const{
	return Val[nIndex];
}

DLLEXPORT Int3 Leviathan::Int3::operator-(const Int3& other) const{
	return Int3(this->Val[0]-other.Val[0], this->Val[1]-other.Val[1], this->Val[2]-other.Val[2]);
}

DLLEXPORT int Leviathan::Int3::AddAllTogether() const{
	return Val[0]+Val[1]+Val[2];
}
// ---- Float2 ---- //
Float2::Float2(){
	Val[0] = 0.0f;
	Val[1] = 0.0f;
}
Float2::Float2(float x, float y){
	Val[0] = x;
	Val[1] = y;
}
Float2::Float2(float data){
	Val[0] = data;
	Val[1] = data;
}

DLLEXPORT float Leviathan::Float2::GetX() const{
	return Val[0];
}

DLLEXPORT float Leviathan::Float2::GetY() const{
	return Val[1];
}

Float2 Float2::operator +(Float2 val){
	Float2 ret;
	for(int i = 0; i < 2; i++){
		ret.Val[i] = (this->Val[i] + val[i]);
	}
	return ret;
}
float Leviathan::Float2::operator[](const int nIndex) const{
	return Val[nIndex];
}

DLLEXPORT void Leviathan::Float2::SetX(const float &val){
	Val[0] = val;
}

DLLEXPORT void Leviathan::Float2::SetY(const float &val){
	Val[1] = val;
}

// ---- Float3 ---- //
Leviathan::Float3::Float3(){
	Val[0] = 0.0f;
	Val[1] = 0.0f;
	Val[2] = 0.0f;
}
Leviathan::Float3::Float3(float x, float y, float z){
	Val[0] = x;
	Val[1] = y;
	Val[2] = z;
}
Leviathan::Float3::Float3(float data){
	Val[0] = data;
	Val[1] = data;
	Val[2] = data;
}
Leviathan::Float3::Float3(const Float3 &other){
	// deep copy //
	// actually no new used to store, deep copy not required //
	// this could be let the compiler do
	Val[0] = other.Val[0];
	Val[1] = other.Val[1];
	Val[2] = other.Val[2];
}

Float3 Leviathan::Float3::operator +(const Float3 &other) const{
	return Float3(this->Val[0]+other.Val[0], this->Val[1]+other.Val[1], this->Val[2]+other.Val[2]);
}
float Leviathan::Float3::operator[](const int nIndex) const{
	return Val[nIndex];
}

DLLEXPORT Float3 Leviathan::Float3::operator-(const Float3& other) const{
	return Float3(this->Val[0]-other.Val[0], this->Val[1]-other.Val[1], this->Val[2]-other.Val[2]);
}

DLLEXPORT void Leviathan::Float3::operator/=(float val){
	// divide all by value //
	Val[0] /= val;
	Val[1] /= val;
	Val[2] /= val;
}
DLLEXPORT bool Leviathan::Float3::operator>(const float& val) const{
	// lets try this //
	return (this->AddAllTogether() > val);
}

DLLEXPORT bool Leviathan::Float3::operator<(const float& val) const{
	// lets try this //
	return (this->AddAllTogether() < val);
}

DLLEXPORT bool Leviathan::Float3::operator==(const Float3& other) const{
	// check are floats the same in certain accuracy //
	if(X()-other.X() < EPSILON && X()-other.X() > -EPSILON){
		if(Y()-other.Y() < EPSILON && Y()-other.Y() > -EPSILON){
			if(Z()-other.Z() < EPSILON && Z()-other.Z() > -EPSILON){
				return true;
			}
		}
	}
	return false;
}


DLLEXPORT float Leviathan::Float3::AddAllTogether() const{
	return Val[0]+Val[1]+Val[2];
}

DLLEXPORT float Leviathan::Float3::Lenght() const{
	return sqrt((Val[0]*Val[0]) + (Val[1]*Val[1]) + (Val[2]*Val[2]));
}

DLLEXPORT void Leviathan::Float3::Normalize(){
	// get lenght and divide all components by it //
	(*this) /= Lenght();;
}

DLLEXPORT Float3 Leviathan::Float3::Cross(const Float3& other) const{
	return Float3(this->Y()*other.Z()-this->Z()*other.Y(), this->Z()*other.X()-this->X()*other.Z(), this->X()*other.Y()-this->Y()*other.X());
}


DLLEXPORT Float3 Leviathan::Float3::Dot(const Float3& other) const{
	return Float3(X()+other.X(), Y()+other.Y(), Z()+other.Z());
}


DLLEXPORT float Leviathan::Float3::X() const{
	return (*this)[0];
}

DLLEXPORT float Leviathan::Float3::Y() const{
	return (*this)[1];
}

DLLEXPORT float Leviathan::Float3::Z() const{
	return (*this)[2];
}
DLLEXPORT void Leviathan::Float3::X(const float &x){
	Val[0] = x;
}

DLLEXPORT void Leviathan::Float3::Y(const float &y){
	Val[1] = y;
}

DLLEXPORT void Leviathan::Float3::Z(const float &z){
	Val[2] = z;
}







// ---- Float4 ---- //
Float4::Float4(float Float1, float Float2, float Float3, float Float4){
Ops[0] = Float1;
Ops[1] = Float2;
Ops[2] = Float3;
Ops[3]  = Float4;
}
Float4::Float4(){
	Ops[0] = 0;
	Ops[1] = 0;
	Ops[2] = 0;
	Ops[3] = 0;
}
Float4::operator float*(){

	return Ops;
}
float* Float4::operator--(){
	Ops[0] -= 0.01f;
	Ops[1] -= 0.01f;
	Ops[2] -= 0.01f;
	Ops[3] -= 0.01f;		
	return Ops;
}
float* Float4::operator++(){
	Ops[0] += 0.01f;
	Ops[1] += 0.01f;
	Ops[2] += 0.01f;
	Ops[3] += 0.01f;		
	return Ops;
}
float Float4::operator--(int){
	Ops[0] -= 0.01f;
	Ops[1] -= 0.01f;
	Ops[2] -= 0.01f;
	Ops[3] -= 0.01f;		
	return Ops[0]+Ops[1]+Ops[2]+Ops[3];
}
float Float4::operator++(int){
	Ops[0] += 0.01f;
	Ops[1] += 0.01f;
	Ops[2] += 0.01f;
	Ops[3] += 0.01f;		
	return Ops[0]+Ops[1]+Ops[2]+Ops[3];
}
bool Float4::operator <(float val){
	int count = 0;
	for(unsigned int i = 0; i < 5; i++){
		if(Ops[i] < val){
			count++;
		}
	}
	if(count == 4)
		return true;
	return false;
}
bool Float4::operator >(float val){
	int count = 0;
	for(unsigned int i = 0; i < 5; i++){
		if(Ops[i] > val){
			count++;
		}
	}
	if(count == 4)
		return true;
	return false;
}
float& Float4::operator[](const int nIndex){


	return Ops[nIndex];


}

Float4 Float4::operator*(float multiplier){
	return Float4((*this[0]*multiplier), (*this[1]*multiplier), (*this[2]*multiplier), (*this[3]*multiplier));
}
// ---- ObjectRenderingFace ---- //
//ObjectRenderingFace::ObjectRenderingFace(){
//	IDs[0] = 0;
//	IDs[1] = 0;
//	IDs[2] = 0;
//	IDs[3]  = 0;
//	IDs[4] = 0;
//	IDs[5] = 0;
//	IDs[6] = 0;
//	IDs[7]  = 0;
//	IDs[8]  = 0;
//}
//
//ObjectRenderingFace::ObjectRenderingFace(int vert1, int tcord1, int fnorm1, int vert2, int tcord2, int fnorm2, int vert3, int tcord3, int fnorm3){
//	IDs[0] = vert1;
//	IDs[1] = tcord1;
//	IDs[2] = fnorm1;
//	IDs[3]  = vert2;
//	IDs[4] = tcord2;
//	IDs[5] = fnorm2;
//	IDs[6] = vert3;
//	IDs[7]  = tcord3;
//	IDs[8]  = fnorm3;
//}
//ObjectRenderingFace::ObjectRenderingFace(int vert1, int fnorm1, int vert2, int fnorm2, int vert3, int fnorm3){
//	IDs[0] = vert1;
//	IDs[1] = 0;
//	IDs[2] = fnorm1;
//	IDs[3]  = vert2;
//	IDs[4] = 0;
//	IDs[5] = fnorm2;
//	IDs[6] = vert3;
//	IDs[7]  = 0;
//	IDs[8]  = fnorm3;
//}
//ObjectRenderingFace::ObjectRenderingFace(int vert1, int tcord1 , int vert2, int tcord2, int vert3, int tcord3){
//	IDs[0] = vert1;
//	IDs[1] = tcord1;
//	IDs[2] = 0;
//	IDs[3]  = vert2;
//	IDs[4] = tcord2;
//	IDs[5] = 0;
//	IDs[6] = vert3;
//	IDs[7]  = tcord3;
//	IDs[8]  = 0;
//}
//ObjectRenderingFace::ObjectRenderingFace(int vert1 , int vert2,int vert3){
//	IDs[0] = vert1;
//	IDs[1] = 0;
//	IDs[2] = 0;
//	IDs[3]  = vert2;
//	IDs[4] = 0;
//	IDs[5] = 0;
//	IDs[6] = vert3;
//	IDs[7]  = 0;
//	IDs[8]  = 0;
//}

// ---- IntWstring ---- //
IntWstring::IntWstring(){
	Wstr = NULL;
	Value = -1;
}
IntWstring::IntWstring(const wstring& wstr, int value){
	Wstr = new wstring(wstr);
	Value = value;
}
IntWstring::~IntWstring(){
	if(Wstr){
		delete Wstr;
		Wstr = NULL;
	}
}

wstring* IntWstring::GetString(){
	return Wstr;
}

int IntWstring::GetValue(){
	return Value;
}

void IntWstring::SetString(wstring& wstr){
	if(Wstr){
		delete Wstr;
		Wstr = NULL;
	}
	Wstr = new wstring(wstr);
}

void IntWstring::SetValue(int value){
	Value = value;
}

Leviathan::Int1::Int1(){
	iVal = VAL_NOUPDATE;
}

Leviathan::Int1::Int1(int data){
	iVal = data;
}

Leviathan::Int1 Leviathan::Int1::operator+(const Int1& val){
	return Int1(val.GetIntValue());
}

int Leviathan::Int1::operator[](const int nIndex) const{
	return iVal;
}

Leviathan::Int1::operator int() const{
	return iVal;
}

int Leviathan::Int1::GetIntValue() const{
	return iVal;
}

void Leviathan::Int1::SetIntValue(int val){
	iVal = val;
}

// ----- CharWithIndex ------ //

Leviathan::CharWithIndex::CharWithIndex(){
	Char = L' ';
	Index = -1;
}

Leviathan::CharWithIndex::CharWithIndex(wchar_t character, int index) : Char(character), Index(index){
	
}

Leviathan::CharWithIndex::~CharWithIndex(){

}

// ------------ Float1 ---------------- //

DLLEXPORT Leviathan::Float1::Float1(){
	fVal = VAL_NOUPDATE;
}

DLLEXPORT Leviathan::Float1::Float1(float data){
	fVal = data;
}

DLLEXPORT Float1 Leviathan::Float1::operator+(const Float1& val){
	return Float1(fVal+val);
}

DLLEXPORT float Leviathan::Float1::operator[](const int nIndex) const{
	return fVal;
}

DLLEXPORT Leviathan::Float1::operator float() const{
	return fVal;
}

DLLEXPORT float Leviathan::Float1::GetFloatValue() const{
	return fVal;
}

DLLEXPORT void Leviathan::Float1::SetFloatValue(float val){
	fVal = val;
}

DLLEXPORT Leviathan::UINT4::UINT4(UINT UINT1, UINT UINT2, UINT UINT3, UINT UINT4){
	Ops[0] = UINT1;
	Ops[1] = UINT2;
	Ops[2] = UINT3;
	Ops[3] = UINT4;
}

DLLEXPORT Leviathan::UINT4::UINT4(){
	Ops[0] = 0;
	Ops[1] = 0;
	Ops[2] = 0;
	Ops[3] = 0;
}

DLLEXPORT Leviathan::UINT4::operator UINT*(){
	return &Ops[0];
}

DLLEXPORT UINT& Leviathan::UINT4::operator[](const int nIndex){
	return Ops[nIndex];
}