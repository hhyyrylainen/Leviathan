// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
// This file shouldn't be included in any headers as this pulls in a ton of stuff
#include "BindHelpers.h"
#include "Logger.h"

#include "Entities/GameWorld.h"
#include "Entities/Components.h"

namespace Leviathan{

template<class WorldType>
    bool BindGameWorldBaseMethods(asIScriptEngine* engine, const char* classname){

    if(engine->RegisterObjectMethod(classname,
            "ObjectID CreateEntity()",
            asMETHOD(WorldType, CreateEntity), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
            "bool DestroyEntity(ObjectID id)",
            asMETHOD(WorldType, DestroyEntity), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
            "PhysicalWorld@ GetPhysicalWorld()",
            asMETHOD(WorldType, GetPhysicalWorld), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
            "RayCastHitEntity@ CastRayGetFirstHit(Float3 start, Float3 end)",
            asMETHOD(WorldType, CastRayGetFirstHitProxy), asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // These are inefficient versions of the get methods, prefer the ones in derived classes
    if(engine->RegisterObjectMethod(classname,
            "Physics@ BaseWorldGetComponentPhysics(ObjectID id)",
            asMETHODPR(WorldType, template GetComponent<Physics>, (ObjectID), Physics&),
            asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(classname,
            "Position@ BaseWorldGetComponentPosition(ObjectID id)",
            asMETHODPR(WorldType, template GetComponent<Position>, (ObjectID), Position&),
            asCALL_THISCALL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    return true;
}

template<class WorldType>
    bool BindStandardWorldMethods(asIScriptEngine* engine, const char* classname){

    if(!BindGameWorldBaseMethods<WorldType>(engine, classname))
        return false;

    #include "Generated/StandardWorldBindings.h"

    ANGLESCRIPT_BASE_CLASS_CASTS_NO_REF_STRING(GameWorld, std::string("GameWorld"),
        WorldType, std::string(classname));
    
    return true;
}
}



