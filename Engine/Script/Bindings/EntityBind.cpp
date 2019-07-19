// ------------------------------------ //
#include "EntityBind.h"

#include "Generated/StandardWorld.h"

#include "Entities/GameWorld.h"
#include "Entities/ScriptComponentHolder.h"
#include "Entities/ScriptSystemWrapper.h"

#include "StandardWorldBindHelper.h"


using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
void ScriptSystemUsesProxyName(void* memory, const std::string& name)
{
    new(memory) ScriptSystemUses(name);
}

void ScriptSystemUsesProxyType(void* memory, uint16_t componenttype)
{
    new(memory) ScriptSystemUses(componenttype);
}

void ScriptSystemUsesProxyInvalid(void* memory)
{
    new(memory) ScriptSystemUses();
}

void ScriptSystemUsesDestructorProxy(void* memory)
{
    static_cast<ScriptSystemUses*>(memory)->~ScriptSystemUses();
}

void SimpleAnimationConstructorProxy(void* memory, const std::string& name)
{
    new(memory) SimpleAnimation(name);
}

void SimpleAnimationDestructorProxy(void* memory)
{
    reinterpret_cast<SimpleAnimation*>(memory)->~SimpleAnimation();
}

void AnimatedAddHelper(Animated* self, const SimpleAnimation& animation)
{

    self->Animations.push_back(animation);
}

SimpleAnimation* AnimatedGetHelper(Animated* self, uint64_t index)
{

    if(index >= self->Animations.size())
        return nullptr;

    return &self->Animations[index];
}

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {


bool BindRayCast(asIScriptEngine* engine)
{
    // // Result class for ray cast //
    // ANGELSCRIPT_REGISTER_REF_TYPE("RayCastHitEntity", RayCastHitEntity);

    // if(engine->RegisterObjectMethod("RayCastHitEntity", "Float3 GetPosition()",
    //        asMETHOD(RayCastHitEntity, GetPosition), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // // Compare function //
    // if(engine->RegisterObjectMethod("RayCastHitEntity",
    //        "bool DoesBodyMatchThisHit(NewtonBody@ body)",
    //        asMETHOD(RayCastHitEntity, DoesBodyMatchThisHit), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }


    return true;
}

static uint16_t PhysicsTYPEProxy = static_cast<uint16_t>(Physics::TYPE);
static uint16_t PositionTYPEProxy = static_cast<uint16_t>(Position::TYPE);
static uint16_t RenderNodeTYPEProxy = static_cast<uint16_t>(RenderNode::TYPE);
static uint16_t SendableTYPEProxy = static_cast<uint16_t>(Sendable::TYPE);
static uint16_t ReceivedTYPEProxy = static_cast<uint16_t>(Received::TYPE);
static uint16_t ModelTYPEProxy = static_cast<uint16_t>(Model::TYPE);
static uint16_t BoxGeometryTYPEProxy = static_cast<uint16_t>(BoxGeometry::TYPE);
static uint16_t CameraTYPEProxy = static_cast<uint16_t>(Camera::TYPE);
// static uint16_t ManualObjectTYPEProxy = static_cast<uint16_t>(ManualObject::TYPE);
// static uint16_t PlaneTYPEProxy = static_cast<uint16_t>(Plane::TYPE);
static uint16_t AnimatedTYPEProxy = static_cast<uint16_t>(Animated::TYPE);

//! Helper for BindComponentTypes
bool BindComponentTypeID(asIScriptEngine* engine, const char* name, uint16_t* value)
{
    if(engine->SetDefaultNamespace(name) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const uint16 TYPE", value) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindComponentTypes(asIScriptEngine* engine)
{
    // ------------------------------------ //
    // Position
    if(engine->RegisterObjectType("Position", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Position", "bool Marked", asOFFSET(Position, Marked)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "Position", "Float3 _Position", asOFFSET(Position, Members._Position)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "Position", "Float4 _Orientation", asOFFSET(Position, Members._Orientation)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Position", &PositionTYPEProxy))
        return false;

    // ------------------------------------ //
    // Physics
    if(engine->RegisterObjectType("Physics", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Currently does nothing
    if(engine->RegisterObjectProperty("Physics", "bool Marked", asOFFSET(Physics, Marked)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Physics", "PhysicsBody@ get_Body() const",
           asMETHOD(Physics, GetBodyWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Physics",
           "PhysicsBody@ CreatePhysicsBody(PhysicalWorld@ world, PhysicsShape@ shape, "
           "float mass, int physicsmaterialid = -1)",
           asMETHOD(Physics, CreatePhysicsBodyWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Physics",
           "bool ChangeShape(PhysicalWorld@ world, PhysicsShape@ shape)",
           asMETHOD(Physics, ChangeShapeWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Physics", "void JumpTo(Position@ positiontosync)",
           asMETHOD(Physics, JumpTo), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Physics", "void Release(PhysicalWorld@ world)",
           asMETHOD(Physics, Release), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Physics", &PhysicsTYPEProxy))
        return false;

    // ------------------------------------ //
    // RenderNode
    if(engine->RegisterObjectType("RenderNode", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "RenderNode", "bool Marked", asOFFSET(RenderNode, Marked)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "RenderNode", "Float3 Scale", asOFFSET(RenderNode, Scale)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "RenderNode", "bs::HSceneObject Node", asOFFSET(RenderNode, Node)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "RenderNode", "bool Hidden", asOFFSET(RenderNode, Hidden)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "RenderNode", &RenderNodeTYPEProxy))
        return false;

    // ------------------------------------ //
    // Sendable
    if(engine->RegisterObjectType("Sendable", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Sendable", "bool Marked", asOFFSET(Sendable, Marked)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Sendable", &SendableTYPEProxy))
        return false;

    // ------------------------------------ //
    // Received
    if(engine->RegisterObjectType("Received", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Received", "bool Marked", asOFFSET(Received, Marked)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Received", &ReceivedTYPEProxy))
        return false;

    // ------------------------------------ //
    // Model
    if(engine->RegisterObjectType("Model", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Model", "bool Marked", asOFFSET(Model, Marked)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Model", &ModelTYPEProxy))
        return false;

    if(engine->RegisterObjectProperty(
           "Model", "bs::HRenderable GraphicalObject", asOFFSET(Model, GraphicalObject)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "Model", "bs::HMaterial Material", asOFFSET(Model, Material)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Model", "string MeshName", asOFFSET(Model, MeshName)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // BoxGeometry
    if(engine->RegisterObjectType("BoxGeometry", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "BoxGeometry", "bool Marked", asOFFSET(BoxGeometry, Marked)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "BoxGeometry", &BoxGeometryTYPEProxy))
        return false;

    // // ------------------------------------ //
    // // ManualObject
    // if(engine->RegisterObjectType("ManualObject", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // if(engine->RegisterObjectProperty(
    //        "ManualObject", "bool Marked", asOFFSET(ManualObject, Marked)) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // if(!BindComponentTypeID(engine, "ManualObject", &ManualObjectTYPEProxy))
    //     return false;

    // ------------------------------------ //
    // Camera
    if(engine->RegisterObjectType("Camera", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Camera", "bool Marked", asOFFSET(Camera, Marked)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Camera", "uint8 FOV", asOFFSET(Camera, FOV)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "Camera", "bool SoundPerceiver", asOFFSET(Camera, SoundPerceiver)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Camera", &CameraTYPEProxy))
        return false;


    // // ------------------------------------ //
    // // Plane
    // if(engine->RegisterObjectType("Plane", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // if(engine->RegisterObjectProperty("Plane", "bool Marked", asOFFSET(Plane, Marked)) < 0)
    // {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // if(!BindComponentTypeID(engine, "Plane", &PlaneTYPEProxy))
    //     return false;

    // if(engine->RegisterObjectProperty(
    //        "Plane", "Ogre::Item@ GraphicalObject", asOFFSET(Plane, GraphicalObject)) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // ------------------------------------ //
    // Animated
    if(engine->RegisterObjectType("Animated", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Animated", "bool Marked", asOFFSET(Animated, Marked)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindComponentTypeID(engine, "Animated", &AnimatedTYPEProxy))
        return false;

    // if(engine->RegisterObjectProperty("Animated", "Ogre::Item@ GraphicalObject",
    //        asOFFSET(Animated, GraphicalObject)) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // ------------------------------------ //
    // SimpleAnimation
    if(engine->RegisterObjectType("SimpleAnimation", sizeof(SimpleAnimation),
           asOBJ_VALUE | asGetTypeTraits<SimpleAnimation>()) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("SimpleAnimation", asBEHAVE_CONSTRUCT,
           "void f(const string &in name)", asFUNCTION(SimpleAnimationConstructorProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("SimpleAnimation", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(SimpleAnimationDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "SimpleAnimation", "bool Loop", asOFFSET(SimpleAnimation, Loop)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("SimpleAnimation", "float SpeedFactor",
           asOFFSET(SimpleAnimation, SpeedFactor)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "SimpleAnimation", "bool Paused", asOFFSET(SimpleAnimation, Paused)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Animation helpers
    if(engine->RegisterObjectMethod("Animated",
           "void AddAnimation(const SimpleAnimation &in animation)",
           asFUNCTION(AnimatedAddHelper), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Animated", "SimpleAnimation& GetAnimation(uint64 index)",
           asFUNCTION(AnimatedGetHelper), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindScriptComponentTypeSupport(asIScriptEngine* engine)
{
    // ------------------------------------ //
    // ScriptSystemUses
    if(engine->RegisterObjectType("ScriptSystemUses", sizeof(ScriptSystemUses),
           asOBJ_VALUE | asGetTypeTraits<ScriptSystemUses>()) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("ScriptSystemUses", asBEHAVE_CONSTRUCT,
           "void f(const string &in name)", asFUNCTION(ScriptSystemUsesProxyName),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("ScriptSystemUses", asBEHAVE_CONSTRUCT,
           "void f(uint16 type)", asFUNCTION(ScriptSystemUsesProxyType),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("ScriptSystemUses", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(ScriptSystemUsesProxyInvalid), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("ScriptSystemUses", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(ScriptSystemUsesDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptSystemUses",
           "ScriptSystemUses& opAssign(const ScriptSystemUses &in other)",
           asMETHOD(ScriptSystemUses, operator=), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // ScriptComponent
    if(engine->RegisterInterface("ScriptComponent") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Factory definition
    if(engine->RegisterFuncdef("ScriptComponent@ ComponentFactoryFunc(GameWorld@ world)") <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterInterface("ScriptSystem") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Required operations
    if(engine->RegisterInterfaceMethod("ScriptSystem", "void Init(GameWorld@ world)") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterInterfaceMethod("ScriptSystem", "void Release()") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterInterfaceMethod("ScriptSystem", "void Run()") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterInterfaceMethod("ScriptSystem", "void CreateAndDestroyNodes()") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterInterfaceMethod("ScriptSystem", "void Clear()") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Component holder type
    ANGELSCRIPT_REGISTER_REF_TYPE("ScriptComponentHolder", ScriptComponentHolder);

    if(engine->RegisterObjectMethod("ScriptComponentHolder",
           "bool ReleaseComponent(ObjectID entity)",
           asMETHOD(ScriptComponentHolder, ReleaseComponent), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    // This is just a wrapper for the above function
    if(engine->RegisterObjectMethod("ScriptComponentHolder", "bool Destroy(ObjectID entity)",
           asMETHOD(ScriptComponentHolder, Destroy), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Not sure if scripts should be allowed to call this
    if(engine->RegisterObjectMethod("ScriptComponentHolder", "void ReleaseAllComponents()",
           asMETHOD(ScriptComponentHolder, ReleaseAllComponents), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptComponentHolder",
           "ScriptComponent@ Create(ObjectID entity)", asMETHOD(ScriptComponentHolder, Create),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptComponentHolder",
           "ScriptComponent@ Find(ObjectID entity)", asMETHOD(ScriptComponentHolder, Find),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ScriptComponentHolder", "array<ObjectID>@ GetIndex()",
           asMETHOD(ScriptComponentHolder, GetIndex), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Helpers for creating and destroying nodes
    // The second argument should be of type array<ThisSystemsCachedType>
    if(engine->RegisterGlobalFunction(
           "void ScriptSystemNodeHelper(GameWorld@ world, ?&in "
           "cachedcomponents, array<ScriptSystemUses> &in systemcomponents)",
           asFUNCTION(ScriptSystemNodeHelper), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
} // namespace Leviathan

bool Leviathan::BindEntity(asIScriptEngine* engine)
{

    // TODO: add reference counting for GameWorld
    if(engine->RegisterObjectType("GameWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindScriptComponentTypeSupport(engine))
        return false;

    if(!BindRayCast(engine))
        return false;

    if(!BindComponentTypes(engine))
        return false;

    if(!BindGameWorldBaseMethods<GameWorld>(engine, "GameWorld"))
        return false;

    // Component get functions //
    // GameWorld

    // ------------------------------------ //
    if(engine->RegisterObjectType("StandardWorld", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindStandardWorldMethods<StandardWorld>(engine, "StandardWorld"))
        return false;

    // ------------------------------------ //

    return true;
}
