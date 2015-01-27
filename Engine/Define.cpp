#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

#ifdef LEVIATHAN_USES_VLD
// visual leak detector //
#include <vld.h>
#endif // LEVIATHAN_USES_VLD






//Object* Object::LApp = NULL;
bool Object::IsThis(Object* compare){
    return this == compare;
}
//Object* Object::GetApp(){
//  return LApp;
//
//}
//void Object::SetApp(Object* app){
//  LApp = app;
//}

EngineComponent::EngineComponent() : Inited(false){
    
}
bool EngineComponent::Init(){
    return false;
}
void EngineComponent::Release(){
}

DLLEXPORT Leviathan::Object::Object(){

}

DLLEXPORT Leviathan::Object::~Object(){

}


// Junk function which contains things that might break //
void DoImportantThings(){

    QUICK_ERROR_MESSAGE;
}

