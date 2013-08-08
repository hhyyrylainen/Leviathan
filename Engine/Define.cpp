#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

// visual leak detector //
#include <vld.h>





//Object* Object::LApp = NULL;
bool Object::IsThis(Object* compare){
	return this == compare;
}
//Object* Object::GetApp(){
//	return LApp;
//
//}
//void Object::SetApp(Object* app){
//	LApp = app;
//}

EngineComponent::EngineComponent(){

}
bool EngineComponent::Init(){
	return true;
}
void EngineComponent::Release(){
}

DLLEXPORT Leviathan::Object::Object(){

}

DLLEXPORT Leviathan::Object::~Object(){

}

//DLLEXPORT bool Leviathan::Object::IsThis(Object* compare){
//	return this == compare;
//}
