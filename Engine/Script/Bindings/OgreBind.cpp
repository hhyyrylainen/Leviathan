// ------------------------------------ //
#include "OgreBind.h"

#include "Logger.h"
#include "Define.h"

#include "OgreRoot.h"
#include "OgreColourValue.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
void ColourValueProxy(void* memory, float r, float g, float b, float a){
    new(memory) Ogre::ColourValue(r, g, b, a);
}

// This is needed because directly registering
// Ogre::Root::getSingletonPtr() with angelscript does weird stuff
Ogre::Root* ScriptGetOgre(){

    Ogre::Root* root = Ogre::Root::getSingletonPtr();

    LEVIATHAN_ASSERT(root != nullptr, "Script called GetOgre while Ogre isn't initialized");
    return root;
}

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan{

bool BindColour(asIScriptEngine* engine){

    if(engine->RegisterObjectType("ColourValue", sizeof(Ogre::ColourValue),
            asOBJ_VALUE | asGetTypeTraits<Ogre::ColourValue>() | asOBJ_POD) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ColourValue", asBEHAVE_CONSTRUCT,
            "void f(float r, float g, float b, float a = 1.0)", 
            asFUNCTION(ColourValueProxy), asCALL_CDECL_OBJFIRST) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("ColourValue", "float r",
            asOFFSET(Ogre::ColourValue, r)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("ColourValue", "float g",
            asOFFSET(Ogre::ColourValue, g)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("ColourValue", "float b",
            asOFFSET(Ogre::ColourValue, b)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("ColourValue", "float a",
            asOFFSET(Ogre::ColourValue, a)) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    return true;
}
}
// ------------------------------------ //
bool Leviathan::BindOgre(asIScriptEngine* engine){

    // This doesn't need to be restored if we fail //
    if(engine->SetDefaultNamespace("Ogre") < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    if(!BindColour(engine))
        return false;

    if(engine->RegisterObjectType("Root", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }
    
    // ------------------ Global functions ------------------ //
    if(engine->RegisterGlobalFunction("Root@ GetOgre()",
            asFUNCTION(ScriptGetOgre), asCALL_CDECL) < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0)
    {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

void Leviathan::RegisterOgre(asIScriptEngine* engine,
    std::map<int, std::string> &typeids)
{
    typeids.insert(std::make_pair(engine->GetTypeIdByDecl("Ogre::Colour"), "Ogre::Colour"));
}



