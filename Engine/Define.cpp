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


EngineComponent::EngineComponent(){
    
}

DLLEXPORT Leviathan::Object::Object(){

}

DLLEXPORT Leviathan::Object::~Object(){

}


// Junk function which contains things that might break //
void DoImportantThings(){

    QUICK_ERROR_MESSAGE;
}

