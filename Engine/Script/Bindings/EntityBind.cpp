// ------------------------------------ //
#include "EntityBind.h"

#include "Entities/GameWorld.h"


using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //


// ------------------------------------ //
// Start of the actual bind
namespace Leviathan{

// Called from BindRayCast
bool BindNewtonBody(asIScriptEngine* engine){

    // Bind the NewtonBody as non counted handle and don't register
    // any methods since it will only be used to compare the
    // pointer //
    if(engine->RegisterObjectType("NewtonBody", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    return true;
}


bool BindRayCast(asIScriptEngine* engine){

    if(!BindNewtonBody(engine))
        return false;

    // Result class for ray cast //
    if(engine->RegisterObjectType("RayCastHitEntity", 0, asOBJ_REF) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("RayCastHitEntity", asBEHAVE_ADDREF, "void f()",
            asMETHOD(RayCastHitEntity, AddRefProxy), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("RayCastHitEntity", asBEHAVE_RELEASE, "void f()",
            asMETHOD(RayCastHitEntity, ReleaseProxy), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectMethod("RayCastHitEntity", "Float3 GetPosition()",
            asMETHOD(RayCastHitEntity, GetPosition), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Compare function //
    if(engine->RegisterObjectMethod("RayCastHitEntity",
            "bool DoesBodyMatchThisHit(NewtonBody@ body)",
            asMETHOD(RayCastHitEntity, DoesBodyMatchThisHit), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    

    return true;
}

bool BindComponentTypes(asIScriptEngine* engine){

    if(engine->RegisterObjectType("Physics", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Currently does nothing
    if(engine->RegisterObjectProperty("Physics", "bool Marked",
            asOFFSET(Physics, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    if(engine->RegisterObjectMethod("Physics", "Float3 GetVelocity() const",
            asMETHODPR(Physics, GetVelocity, () const, Float3), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Physics", "NewtonBody@ GetBody() const",
            asMETHODPR(Physics, GetBody, () const, NewtonBody*), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Position
    if(engine->RegisterObjectType("Position", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Position", "bool Marked",
            asOFFSET(Position, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Position", "Float3 _Position",
            asOFFSET(Position, Members._Position)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Position", "Float4 _Orientation",
            asOFFSET(Position, Members._Orientation)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    return true;
}
}

bool Leviathan::BindEntity(asIScriptEngine* engine){

    // TODO: add reference counting for GameWorld
    if(engine->RegisterObjectType("GameWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindRayCast(engine))
        return false;

    if(engine->RegisterObjectMethod("GameWorld",
            "RayCastHitEntity@ CastRayGetFirstHit(Float3 start, Float3 end)",
            asMETHOD(GameWorld, CastRayGetFirstHitProxy), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypes(engine))
        return false;

    // Component get functions //
    // GameWorld
    if(engine->RegisterObjectMethod("GameWorld", "Physics@ GetComponentPhysics(ObjectID id)",
            asMETHODPR(GameWorld, GetComponent<Physics>, (ObjectID), Physics&),
            asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("GameWorld", "Position@ GetComponentPosition(ObjectID id)",
            asMETHODPR(GameWorld, GetComponent<Position>, (ObjectID), Position&),
            asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    
    
    
    return true;
}

void Leviathan::RegisterEntity(asIScriptEngine* engine, std::map<int, std::string> &typeids){

    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("GameWorld"), "GameWorld"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("RayCastHitEntity"),
            "RayCastHitEntity"));
    
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Physics"), "Physics"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Position"), "Position"));
}





