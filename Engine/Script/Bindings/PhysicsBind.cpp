// ------------------------------------ //
#include "PhysicsBind.h"

#include "Physics/PhysicalWorld.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsShape.h"

#include "Define.h"
#include "Logger.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {
bool BindShape(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("PhysicsShape", PhysicsShape);

    return true;
}

bool BindBody(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("PhysicsBody", PhysicsBody);

    if(engine->RegisterObjectMethod("PhysicsBody", "Float3 GetVelocity() const",
           asMETHODPR(PhysicsBody, GetVelocity, () const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "void SetVelocity(const Float3 &in velocity) const",
           asMETHOD(PhysicsBody, SetVelocity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody", "Float3 GetAngularVelocity() const",
           asMETHODPR(PhysicsBody, GetAngularVelocity, () const, Float3),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "void SetAngularVelocity(const Float3 &in omega) const",
           asMETHOD(PhysicsBody, SetAngularVelocity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody", "void ClearVelocity() const",
           asMETHOD(PhysicsBody, ClearVelocity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody", "Float3 GetTorque() const",
           asMETHODPR(PhysicsBody, GetTorque, () const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody", "void ApplyTorque(const Float3 &in torque)",
           asMETHOD(PhysicsBody, ApplyTorque), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody", "void SetMass(float mass)",
           asMETHOD(PhysicsBody, SetMass), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody", "float get_Mass() const",
           asMETHOD(PhysicsBody, GetMass), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "bool SetPosition(const Float3 &in pos, const Float4 &in orientation)",
           asMETHOD(PhysicsBody, SetPosition), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "bool SetOnlyOrientation(const Float4 &in orientation)",
           asMETHOD(PhysicsBody, SetOnlyOrientation), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "void GiveImpulse(const Float3 &in deltaspeed, const Float3 &in point = Float3(0))",
           asMETHOD(PhysicsBody, GiveImpulse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "void SetDamping(float linear, float angular)", asMETHOD(PhysicsBody, SetDamping),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("PhysicsBody",
           "bool CreatePlaneConstraint(const Float3 &in planenormal = "
           "Float3(0, 1, 0))",
           asMETHOD(PhysicsBody, CreatePlaneConstraint), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
} // namespace Leviathan
// ------------------------------------ //
bool Leviathan::BindPhysics(asIScriptEngine* engine)
{
    if(!BindShape(engine))
        return false;

    if(!BindBody(engine))
        return false;

    // These classes are Leviathan classes so these should not be in the newton namespace
    if(engine->RegisterObjectType("PhysicalWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // if(engine->RegisterObjectMethod("PhysicalWorld",
    //        "NewtonCollision@ CreateCompoundCollision()",
    //        asMETHOD(PhysicalWorld, CreateCompoundCollision), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    if(engine->RegisterObjectMethod("PhysicalWorld",
           "PhysicsShape@ CreateSphere(float radius)",
           asMETHOD(PhysicalWorld, CreateSphereWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //



    return true;
}
