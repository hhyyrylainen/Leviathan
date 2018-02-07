// ------------------------------------ //
#include "OgreBind.h"

#include "Define.h"
#include "Logger.h"

#include "OgreColourValue.h"
#include "OgreRoot.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
void ColourValueProxy(void* memory, float r, float g, float b, float a)
{
    new(memory) Ogre::ColourValue(r, g, b, a);
}

void MatrixProxy(void* memory)
{
    new(memory) Ogre::Matrix4;
}

// This is needed because directly registering
// Ogre::Root::getSingletonPtr() with angelscript does weird stuff
Ogre::Root* ScriptGetOgre()
{

    Ogre::Root* root = Ogre::Root::getSingletonPtr();

    LEVIATHAN_ASSERT(root != nullptr, "Script called GetOgre while Ogre isn't initialized");
    return root;
}

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {

bool BindColour(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("ColourValue", sizeof(Ogre::ColourValue),
           asOBJ_VALUE | asGetTypeTraits<Ogre::ColourValue>() | asOBJ_POD) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ColourValue", asBEHAVE_CONSTRUCT,
           "void f(float r, float g, float b, float a = 1.0)", asFUNCTION(ColourValueProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "ColourValue", "float r", asOFFSET(Ogre::ColourValue, r)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "ColourValue", "float g", asOFFSET(Ogre::ColourValue, g)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "ColourValue", "float b", asOFFSET(Ogre::ColourValue, b)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "ColourValue", "float a", asOFFSET(Ogre::ColourValue, a)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindMatrix4(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("Matrix4", sizeof(Ogre::Matrix4),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Matrix4>() | asOBJ_POD) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(MatrixProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("Ogre::Matrix4") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Ogre::Matrix4 IDENTITY",
           const_cast<Ogre::Matrix4*>(&Ogre::Matrix4::IDENTITY)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("Ogre") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}

// ------------------------------------ //

bool BindScene(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("SceneNode", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

} // namespace Leviathan
// ------------------------------------ //
bool Leviathan::BindOgre(asIScriptEngine* engine)
{

    // This doesn't need to be restored if we fail //
    if(engine->SetDefaultNamespace("Ogre") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindColour(engine))
        return false;

    if(!BindMatrix4(engine))
        return false;

    if(!BindScene(engine))
        return false;

    if(engine->RegisterObjectType("Root", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------ Global functions ------------------ //
    if(engine->RegisterGlobalFunction(
           "Root@ GetOgre()", asFUNCTION(ScriptGetOgre), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
