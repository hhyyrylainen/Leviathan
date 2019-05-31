// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
// This file shouldn't be included in any headers as this pulls in a ton of stuff
#include "BindHelpers.h"
#include "Logger.h"

#include "Entities/Components.h"
#include "Entities/GameWorld.h"

namespace Leviathan {

template<class WorldType>
bool BindGameWorldBaseMethods(asIScriptEngine* engine, const char* classname)
{

    if(engine->RegisterObjectMethod(classname, "ObjectID CreateEntity()",
           asMETHOD(WorldType, CreateEntity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Use this rather than DestroyEntity
    if(engine->RegisterObjectMethod(classname, "void QueueDestroyEntity(ObjectID id)",
           asMETHOD(WorldType, QueueDestroyEntity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname, "bool DestroyEntity(ObjectID id)",
           asMETHOD(WorldType, DestroyEntity), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "void SetEntitysParent(ObjectID child, ObjectID parent)",
           asMETHOD(WorldType, SetEntitysParent), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname, "PhysicalWorld@ GetPhysicalWorld()",
           asMETHOD(WorldType, GetPhysicalWorld), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "int GetPhysicalMaterial(const string &in name)",
           asMETHOD(WorldType, GetPhysicalMaterial), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // if(engine->RegisterObjectMethod(classname,
    //        "RayCastHitEntity@ CastRayGetFirstHit(Float3 start, Float3 end)",
    //        asMETHOD(WorldType, CastRayGetFirstHitProxy), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    if(engine->RegisterObjectMethod(classname,
           "bs::Ray CastRayFromCamera(float x, float y) const",
           asMETHOD(WorldType, CastRayFromCamera), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Support for script types
    if(engine->RegisterObjectMethod(classname,
           "array<ObjectID>@ GetRemovedIDsForComponents(array<uint16>@ componenttypes)",
           asMETHOD(WorldType, GetRemovedIDsForComponents), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "array<ObjectID>@ GetRemovedIDsForScriptComponents(array<string>@ typenames)",
           asMETHOD(WorldType, GetRemovedIDsForScriptComponents), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "bool RegisterScriptComponentType(const string &in name, ComponentFactoryFunc@ "
           "factory)",
           asMETHOD(WorldType, RegisterScriptComponentType), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "bool RegisterScriptSystem(const string &in name, ScriptSystem@ systemobject)",
           asMETHOD(WorldType, RegisterScriptSystem), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "bool UnregisterScriptSystem(const string &in name)",
           asMETHOD(WorldType, UnregisterScriptSystem), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "ScriptComponentHolder@ GetScriptComponentHolder(const string &in name)",
           asMETHOD(WorldType, GetScriptComponentHolder), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "ScriptSystem@ GetScriptSystem(const string &in name)",
           asMETHOD(WorldType, GetScriptSystem), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    if(engine->RegisterObjectMethod(classname, "bs::Scene GetScene()",
           asMETHOD(WorldType, GetScene), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Somewhat questionable Ogre shortcuts that should probably be component types
    if(engine->RegisterObjectMethod(classname, "void SetSunlight()",
           asMETHOD(WorldType, SetSunlight), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname, "void RemoveSunlight()",
           asMETHOD(WorldType, RemoveSunlight), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // if(engine->RegisterObjectMethod(classname,
    //        "void SetLightProperties(const Ogre::ColourValue &in diffuse, const "
    //        "Ogre::ColourValue &in specular, const Ogre::Vector3 &in direction, float power,
    //        " "const Ogre::ColourValue &in upperhemisphere, const Ogre::ColourValue &in "
    //        "lowerhemisphere, const Ogre::Vector3 &in hemispheredir, float envmapscale
    //        = 1.0f)", asMETHOD(WorldType, SetLightProperties), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // ------------------------------------ //
    // These are inefficient versions of the get methods, prefer the ones in derived classes
    if(engine->RegisterObjectMethod(classname,
           "Physics@ BaseWorldGetComponentPhysics(ObjectID id)",
           asMETHODPR(WorldType, template GetComponent<Physics>, (ObjectID), Physics&),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
           "Position@ BaseWorldGetComponentPosition(ObjectID id)",
           asMETHODPR(WorldType, template GetComponent<Position>, (ObjectID), Position&),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

template<class WorldType>
bool BindStandardWorldMethods(asIScriptEngine* engine, const char* classname)
{

    if(!BindGameWorldBaseMethods<WorldType>(engine, classname))
        return false;

#include "Generated/StandardWorldBindings.h"

    ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF_STRING(
        GameWorld, std::string("GameWorld"), WorldType, std::string(classname));

    return true;
}
} // namespace Leviathan
