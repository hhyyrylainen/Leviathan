// ------------------------------------ //
#include "Types.h"

#include "Define.h"
using namespace Leviathan;
using namespace std;
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
	X = 0;
	Y = 0;
}
Int2::Int2(int x, int y){
	X = x;
	Y = y;
}
Int2::Int2(int data){
	X = data;
	Y = data;
}

DLLEXPORT Leviathan::Int2::Int2(const Int2 &other){
	X = other.X;
	Y = other.Y;
}

Int2 Int2::operator +(const Int2 &val){
	return Int2(X+val.X, Y+val.Y);
}
int Int2::operator[](const int nIndex) const{
	switch(nIndex){
	case 0: return X;
	case 1: return Y;
	}

    throw std::exception();
}
// ---- Int3 ---- //
Int3::Int3(){
	X = 0;
	Y = 0;
	Z = 0;
}
Int3::Int3(int x, int y, int z){
	X = x;
	Y = y;
	Z = z;
}
Int3::Int3(int data){
	// save a bit of space //
	X = Y = Z = data;
}

Int3 Int3::operator +(const Int3 &val){
	return Int3(X+val.X, Y+val.Y, Z+val.Z);
}
int Int3::operator[](const int nIndex) const{
	switch(nIndex){
	case 0: return X;
	case 1: return Y;
	case 2: return Z;
	}

    throw exception();
}

DLLEXPORT Int3 Leviathan::Int3::operator-(const Int3& other) const{
	return Int3(X-other.X, Y-other.Y, Z-other.Z);
}

DLLEXPORT int Leviathan::Int3::AddAllTogether() const{
	return X+Y+Z;
}
// ------------------ Int4 ------------------ //
DLLEXPORT Leviathan::Int4::Int4(){
	X = Y = Z = W = 0;
}

DLLEXPORT Leviathan::Int4::Int4(int x, int y, int z, int w) : X(x), Y(y), Z(z), W(w){

}

DLLEXPORT Leviathan::Int4::Int4(int data){
	X = Y = Z = W = data;
}

DLLEXPORT Int4& Leviathan::Int4::operator+(const Int4 &val){
	X += val.X;
	Y += val.Y;
	Z += val.Z;
	W += val.W;
	return *this;
}

DLLEXPORT int Leviathan::Int4::operator[](const int nIndex) const{
	switch(nIndex){
	case 0: return X;
	case 1: return Y;
	case 2: return Z;
	case 3: return W;
	}

    throw exception();
}

DLLEXPORT Int4& Leviathan::Int4::operator-(const Int4& val){
	X -= val.X;
	Y -= val.Y;
	Z -= val.Z;
	W -= val.W;
	return *this;
}

DLLEXPORT int Leviathan::Int4::AddAllTogether() const{
	return X+Y+Z+W;
}
// ------------------ Int1 ------------------ //
Leviathan::Int1::Int1(){
}

Leviathan::Int1::Int1(int data){
	iVal = data;
}

Leviathan::Int1 Leviathan::Int1::operator+(const Int1& val){
	return Int1(val.GetIntValue());
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


DLLEXPORT Float3 Float3::DegreesToRadians(){

    return Float3(X*DEGREES_TO_RADIANS, Y*DEGREES_TO_RADIANS, Z*DEGREES_TO_RADIANS);
}

DLLEXPORT Float3 Float3::CreateVectorFromAngles(const float &yaw, const float &pitch)
{
    return Float3(-sin(yaw*DEGREES_TO_RADIANS), sin(pitch*DEGREES_TO_RADIANS),
        -cos(yaw*DEGREES_TO_RADIANS)).NormalizeSafe(Zeroed);
}



// specific colours //

const Float4 Leviathan::Float4::ColourBlack = Float4(0, 0, 0, 1);

const Float4 Leviathan::Float4::ColourWhite = Float4(1, 1, 1, 1);

const Float4 Leviathan::Float4::ColourTransparent = Float4(0, 0, 0, 0);

const Float3 Leviathan::Float3::UnitVForward = Float3(0.f, 0.f, -1.f);

const Float3 Leviathan::Float3::Zeroed = Float3::zero();



DLLEXPORT const Float4& Leviathan::Float4::GetColourBlack(){
	return ColourBlack;
}

DLLEXPORT const Float4& Leviathan::Float4::GetColourWhite(){
	return ColourWhite;
}

DLLEXPORT const Float4& Leviathan::Float4::GetColourTransparent(){
	return ColourTransparent;
}
// ------------------ Stream operators ------------------ //
DLLEXPORT std::ostream& Leviathan::operator <<(std::ostream &stream,
    const Leviathan::Float4 &value)
{

    stream << "[" << value.X << ", " << value.Y << ", " << value.Z << ", " << value.W << "]";
    return stream;
}
