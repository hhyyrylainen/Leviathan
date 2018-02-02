// ------------------------------------ //
#include "NewtonBind.h"

#include "Newton/PhysicalWorld.h"
#include <Newton.h>

#include "Logger.h"
#include "Define.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan{
bool BindNewtonTypes(asIScriptEngine* engine){

    
    // This doesn't need to be restored if we fail //
    // if(engine->SetDefaultNamespace("Newton") < 0)
    // {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }
    
    if(engine->RegisterObjectType("NewtonCollision", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // if(engine->SetDefaultNamespace("") < 0)
    // {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }
    
    return true;
}
}
// ------------------------------------ //
bool Leviathan::BindNewton(asIScriptEngine* engine){

    if(!BindNewtonTypes(engine))
        return false;

    // These classes are Leviathan classes so these should not be in the newton namespace
    if(engine->RegisterObjectType("PhysicalWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    // TODO: should these be somehow reference counted?
    
    if(engine->RegisterObjectMethod("PhysicalWorld",
            "NewtonCollision@ CreateCompoundCollision()",
            asMETHOD(PhysicalWorld, CreateCompoundCollision), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}

    if(engine->RegisterObjectMethod("PhysicalWorld",
            "NewtonCollision@ DestroyCollision()",
            asMETHOD(PhysicalWorld, DestroyCollision), asCALL_THISCALL) < 0)
	{
		ANGELSCRIPT_REGISTERFAIL;
	}
    

    return true;
}

void Leviathan::RegisterNewton(asIScriptEngine* engine,
    std::map<int, std::string> &typeids)
{
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("NewtonCollision"),
            "NewtonCollision"));
}



