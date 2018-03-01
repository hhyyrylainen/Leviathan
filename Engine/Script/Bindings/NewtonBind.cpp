// ------------------------------------ //
#include "NewtonBind.h"

#include "Newton/PhysicalWorld.h"
#include <Newton.h>

#include "Define.h"
#include "Logger.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {
bool BindNewtonTypes(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("NewtonCollision", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NewtonCollision",
           "void CompoundCollisionBeginAddRemove()",
           asFUNCTION(NewtonCompoundCollisionBeginAddRemove), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NewtonCollision", "void CompoundCollisionEndAddRemove()",
           asFUNCTION(NewtonCompoundCollisionEndAddRemove), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NewtonCollision",
           "void CompoundCollisionAddSubCollision(NewtonCollision@ convexCollision)",
           asFUNCTION(NewtonCompoundCollisionAddSubCollision), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("NewtonCollision",
           "void CompoundCollisionRemoveSubCollision(NewtonCollision@ convexCollision)",
           asFUNCTION(NewtonCompoundCollisionRemoveSubCollision), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("NewtonBody", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
} // namespace Leviathan
// ------------------------------------ //
bool Leviathan::BindNewton(asIScriptEngine* engine)
{

    if(!BindNewtonTypes(engine))
        return false;

    // These classes are Leviathan classes so these should not be in the newton namespace
    if(engine->RegisterObjectType("PhysicalWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // TODO: should these be somehow reference counted?
    if(engine->RegisterObjectMethod("PhysicalWorld",
           "void DestroyCollision(NewtonCollision@ collision)",
           asMETHOD(PhysicalWorld, DestroyCollision), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicalWorld",
           "NewtonCollision@ CreateCompoundCollision()",
           asMETHOD(PhysicalWorld, CreateCompoundCollision), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicalWorld",
           "NewtonCollision@ CreateSphere(float radius, const Ogre::Matrix4 &in offset = "
           "Ogre::Matrix4::IDENTITY)",
           asMETHOD(PhysicalWorld, CreateSphere), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    if(engine->RegisterObjectMethod("PhysicalWorld",
           "NewtonBody@ CreateBodyFromCollision(NewtonCollision@ collision)",
           asMETHOD(PhysicalWorld, CreateBodyFromCollision), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicalWorld", "void DestroyBody(NewtonBody@ body)",
           asMETHOD(PhysicalWorld, DestroyBody), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}
