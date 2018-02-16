// ------------------------------------ //
#include "OgreBind.h"

#include "Define.h"
#include "Logger.h"

#include "OgreColourValue.h"
#include "OgreRoot.h"

// For Float type conversions
#include "Common/Types.h"

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

void DegreeProxy(void* memory, float degree)
{
    new(memory) Ogre::Degree(degree);
}

void DegreeProxyRadian(void* memory, const Ogre::Radian& radian)
{
    new(memory) Ogre::Degree(radian);
}

void RadianProxy(void* memory, float radian)
{
    new(memory) Ogre::Radian(radian);
}

void RadianProxyDegree(void* memory, const Ogre::Degree& degree)
{
    new(memory) Ogre::Radian(degree);
}

void QuaternionProxyAroundAxis(
    void* memory, const Ogre::Radian& radian, const Ogre::Vector3& vector)
{
    new(memory) Ogre::Quaternion(radian, vector);
}

void Vector3Proxy(void* memory, float x, float y, float z)
{

    new(memory) Ogre::Vector3(x, y, z);
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


bool BindVector3(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Vector3", sizeof(Ogre::Vector3),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Vector3>() | asOBJ_POD) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT,
           "void f(float x, float y, float z)", asFUNCTION(Vector3Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "float x", asOFFSET(Ogre::Vector3, x)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "float y", asOFFSET(Ogre::Vector3, y)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "float z", asOFFSET(Ogre::Vector3, z)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    return true;
}

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

bool BindAnglesAndQuaternion(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("Radian", sizeof(Ogre::Radian),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Radian>() | asOBJ_POD) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Degree", sizeof(Ogre::Degree),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Degree>() | asOBJ_POD) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Quaternion", sizeof(Ogre::Quaternion),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Quaternion>() | asOBJ_POD) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_CONSTRUCT, "void f(float radians)",
           asFUNCTION(RadianProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_CONSTRUCT,
           "void f(const Degree &in degree)", asFUNCTION(RadianProxyDegree),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_CONSTRUCT, "void f(float degrees)",
           asFUNCTION(DegreeProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_CONSTRUCT,
           "void f(const Radian &in radian)", asFUNCTION(DegreeProxyRadian),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Radian &in radian, const Vector3 &in vector)",
           asFUNCTION(QuaternionProxyAroundAxis), asCALL_CDECL_OBJFIRST) < 0) {
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

    if(!BindVector3(engine))
        return false;

    if(!BindColour(engine))
        return false;

    if(!BindAnglesAndQuaternion(engine))
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
