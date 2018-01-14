// ------------------------------------ //
#include "EntityBind.h"

#include "Generated/StandardWorld.h"

#include "Entities/GameWorld.h"

#include "StandardWorldBindHelper.h"

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
    ANGELSCRIPT_REGISTER_REF_TYPE("RayCastHitEntity", RayCastHitEntity);
    
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

    
    if(engine->RegisterObjectType("RenderNode", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("RenderNode", "bool Marked",
            asOFFSET(RenderNode, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("Sendable", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Sendable", "bool Marked",
            asOFFSET(Sendable, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("Received", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Received", "bool Marked",
            asOFFSET(Received, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("Model", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Model", "bool Marked",
            asOFFSET(Model, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("BoxGeometry", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("BoxGeometry", "bool Marked",
            asOFFSET(BoxGeometry, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("ManualObject", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("ManualObject", "bool Marked",
            asOFFSET(ManualObject, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("Camera", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Camera", "bool Marked",
            asOFFSET(Camera, Marked)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Camera", "uint8 FOVY",
            asOFFSET(Camera, FOVY)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    } 

    if(engine->RegisterObjectProperty("Camera", "bool SoundPerceiver",
            asOFFSET(Camera, SoundPerceiver)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectType("Plane", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Plane", "bool Marked",
            asOFFSET(Plane, Marked)) < 0)
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

    if(!BindComponentTypes(engine))
        return false;

    if(!BindGameWorldBaseMethods<GameWorld>(engine, "GameWorld"))
        return false;

    // Component get functions //
    // GameWorld

    // ------------------------------------ //
    if(engine->RegisterObjectType("StandardWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindStandardWorldMethods<StandardWorld>(engine, "StandardWorld"))
        return false;
    
    return true;
}

void Leviathan::RegisterEntity(asIScriptEngine* engine, std::map<int, std::string> &typeids){

    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("GameWorld"), "GameWorld"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("StandardWorld"), "StandardWorld"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("RayCastHitEntity"),
            "RayCastHitEntity"));
    
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Physics"), "Physics"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Position"), "Position"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Camera"), "Camera"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Plane"), "Plane"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("ManualObject"), "ManualObject"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("BoxGeometry"), "BoxGeometry"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Model"), "Model"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("RenderNode"), "RenderNode"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Received"), "Received"));
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Sendable"), "Sendable"));
}





