#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_SHADERDATATYPES
#include "ShaderDataTypes.h"
#endif
using namespace Leviathan;
// -----------------// ---- Bone matrice classes ---- //------------------- //

DLLEXPORT Leviathan::BoneTransformsPointerBase::~BoneTransformsPointerBase(){
	// do nothing //

}

////// OLD version
//Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsBufferTypeTiny* data) : TinyData(data), SmallData(NULL),
//	MediumData(NULL), LargeData(NULL), HugeData(NULL), MaxData(NULL)
//{
//	// nothing to do in actual constructor //
//}
//// copies with different pointers //
//Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsBufferTypeSmall* data) : TinyData(NULL), SmallData(data),
//	MediumData(NULL), LargeData(NULL), HugeData(NULL), MaxData(NULL)
//{}
//Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsBufferTypeMedium* data) : TinyData(NULL), SmallData(NULL),
//	MediumData(data), LargeData(NULL), HugeData(NULL), MaxData(NULL)
//{}
//Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsBufferTypeLarge* data) : TinyData(NULL), SmallData(NULL),
//	MediumData(NULL), LargeData(data), HugeData(NULL), MaxData(NULL)
//{}
//Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsBufferTypeHuge* data) : TinyData(NULL), SmallData(NULL),
//	MediumData(NULL), LargeData(NULL), HugeData(data), MaxData(NULL)
//{}
//Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsBufferTypeMax* data) : TinyData(NULL), SmallData(NULL),
//	MediumData(NULL), LargeData(NULL), HugeData(NULL), MaxData(data)
//{}

// -------- implementations for the get bones methods -------- //
DLLEXPORT short Leviathan::BoneTransformsBufferTypeTiny::GetMaxBoneCount(){
	return MAX_BONES_TINY;
}

DLLEXPORT short Leviathan::BoneTransformsBufferTypeSmall::GetMaxBoneCount(){
	return MAX_BONES_SMALL;
}

DLLEXPORT short Leviathan::BoneTransformsBufferTypeMedium::GetMaxBoneCount(){
	return MAX_BONES_MEDIUM;
}

DLLEXPORT short Leviathan::BoneTransformsBufferTypeLarge::GetMaxBoneCount(){
	return MAX_BONES_LARGE;
}

DLLEXPORT short Leviathan::BoneTransformsBufferTypeHuge::GetMaxBoneCount(){
	return MAX_BONES_HUGE;
}

DLLEXPORT short Leviathan::BoneTransformsBufferTypeMax::GetMaxBoneCount(){
	return MAX_BONES_MAX;
}

// --------------- Functions for the pointer classes to the matrice classes --------------------- //
Leviathan::BoneTransformsPointerTiny::BoneTransformsPointerTiny(BoneTransformsBufferTypeTiny* data){
	Data = data;
}

DLLEXPORT  short Leviathan::BoneTransformsPointerTiny::GetMaxBoneCount(){
	return MAX_BONES_TINY;
}
// ----- ----- //
Leviathan::BoneTransformsPointerSmall::BoneTransformsPointerSmall(BoneTransformsBufferTypeSmall* data){
	Data = data;
}

DLLEXPORT  short Leviathan::BoneTransformsPointerSmall::GetMaxBoneCount(){
	return MAX_BONES_SMALL;
}
// ----- ----- //
Leviathan::BoneTransformsPointerMedium::BoneTransformsPointerMedium(BoneTransformsBufferTypeMedium* data){
	Data = data;
}

DLLEXPORT  short Leviathan::BoneTransformsPointerMedium::GetMaxBoneCount(){
	return MAX_BONES_MEDIUM;
}
// ----- ----- //
Leviathan::BoneTransformsPointerLarge::BoneTransformsPointerLarge(BoneTransformsBufferTypeLarge* data){
	Data = data;
}

DLLEXPORT  short Leviathan::BoneTransformsPointerLarge::GetMaxBoneCount(){
	return MAX_BONES_LARGE;
}
// ----- ----- //
Leviathan::BoneTransformsPointerHuge::BoneTransformsPointerHuge(BoneTransformsBufferTypeHuge* data){
	Data = data;
}

DLLEXPORT  short Leviathan::BoneTransformsPointerHuge::GetMaxBoneCount(){
	return MAX_BONES_HUGE;
}
// ----- ----- //
Leviathan::BoneTransformsPointerMax::BoneTransformsPointerMax(BoneTransformsBufferTypeMax* data){
	Data = data;
}

DLLEXPORT  short Leviathan::BoneTransformsPointerMax::GetMaxBoneCount(){
	return MAX_BONES_MAX;
}
// ------------------------------------ //

Leviathan::BoneTransfromBufferWrapper::BoneTransfromBufferWrapper(BoneTransformsPointerBase* data){
	// set data //
	Data = data;
	// get count from the object //
	BoneCount = Data->GetMaxBoneCount();
}

// ------------------------------------ //

// ------------------------------------ //













