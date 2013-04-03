#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
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
bool EngineComponent::Release(bool all){
	return true;
}
bool EngineComponent::IsInited(){
	return Inited;
}