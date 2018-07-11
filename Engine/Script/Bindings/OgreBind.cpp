// ------------------------------------ //
#include "OgreBind.h"

#include "Define.h"
#include "Logger.h"

// For Float type conversions
#include "Common/Types.h"

#include "Animation/OgreSkeletonAnimation.h"
#include "Animation/OgreSkeletonInstance.h"
#include "OgreColourValue.h"
#include "OgreItem.h"
#include "OgreMesh2.h"
#include "OgreRoot.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
void ColourValueProxy(void* memory, Ogre::Real r, Ogre::Real g, Ogre::Real b, Ogre::Real a)
{
    new(memory) Ogre::ColourValue(r, g, b, a);
}

void ColourValueVector4Proxy(void* memory, const Ogre::Vector4& values)
{
    new(memory) Ogre::ColourValue(values.x, values.y, values.z, values.w);
}

void MatrixProxy(void* memory, const Ogre::Vector3& position, const Ogre::Vector3& scale,
    const Ogre::Quaternion& orientation)
{
    new(memory) Ogre::Matrix4;
    static_cast<Ogre::Matrix4*>(memory)->makeTransform(position, scale, orientation);
}

void MatrixProxyUninitialized(void* memory)
{
    new(memory) Ogre::Matrix4;
}

void DegreeProxy(void* memory, Ogre::Real degree)
{
    new(memory) Ogre::Degree(degree);
}

void DegreeProxyRadian(void* memory, const Ogre::Radian& radian)
{
    new(memory) Ogre::Degree(radian);
}

Ogre::Radian DegreeToRadianCast(Ogre::Degree* self)
{
    return *self;
}

void RadianProxy(void* memory, Ogre::Real radian)
{
    new(memory) Ogre::Radian(radian);
}

void RadianProxyDegree(void* memory, const Ogre::Degree& degree)
{
    new(memory) Ogre::Radian(degree);
}

Ogre::Degree RadianToDegreeCast(Ogre::Radian* self)
{
    return *self;
}

void QuaternionProxyAroundAxis(
    void* memory, const Ogre::Radian& radian, const Ogre::Vector3& vector)
{
    new(memory) Ogre::Quaternion(radian, vector);
}

void Vector3Proxy(void* memory, Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
    new(memory) Ogre::Vector3(x, y, z);
}

void Vector4Proxy(void* memory, Ogre::Real x, Ogre::Real y, Ogre::Real z, Ogre::Real w)
{
    new(memory) Ogre::Vector4(x, y, z, w);
}

void PlaneFromNormalProxy(void* memory, const Ogre::Vector3& normal, Ogre::Real f)
{
    new(memory) Ogre::Plane(normal, f);
}

void RayProxy(void* memory, const Ogre::Vector3& origin, const Ogre::Vector3& direction)
{
    new(memory) Ogre::Ray(origin, direction);
}

bool RayIntersectsPlaneProxy(
    const Ogre::Ray* self, const Ogre::Plane& plane, Ogre::Real& distance)
{
    bool intersects;

    std::tie(intersects, distance) = self->intersects(plane);
    return intersects;
}

void SceneNodeAddChildProxy(Ogre::SceneNode* self, Ogre::SceneNode* child)
{
    if(child)
        self->addChild(child);
}

void SceneNodeRemoveFromParent(Ogre::SceneNode* self)
{
    Ogre::SceneNode* parent = self->getParentSceneNode();
    if(parent)
        parent->removeChild(self);
}

void ItemSetMaterialProxy(Ogre::Item* self, const std::string& material)
{
    if(self)
        self->setMaterialName(material);
}

void ItemSetDataBlockProxy(Ogre::Item* self, const std::string& datablock)
{
    if(self)
        self->setDatablock(datablock);
}

void ItemSetCustomParameterProxy(Ogre::Item* self, int index, const Ogre::Vector4& value)
{
    if(self->getSubItem(0))
        self->getSubItem(0)->setCustomParameter(index, value);
}

Ogre::Mesh* ItemGetMeshProxy(Ogre::Item* self)
{
    return self->getMesh().get();
}

Ogre::SkeletonAnimation* SkeletonInstanceGetAnimationProxy(
    Ogre::SkeletonInstance* self, const std::string& name)
{
    try {
        return self->getAnimation(name);
    } catch(const Ogre::Exception&) {

        return nullptr;
    }
}

std::string MeshGetNameProxy(Ogre::Mesh* self)
{
    return self->getName();
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
// For Ogre::Real binding
bool BindOgreTypeDefs(asIScriptEngine* engine)
{
    if constexpr(std::is_same_v<Ogre::Real, float>) {
        if(engine->RegisterTypedef("Real", "float") < 0) {

            ANGELSCRIPT_REGISTERFAIL;
        }
    } else if constexpr(std::is_same_v<Ogre::Real, double>) {
        if(engine->RegisterTypedef("Real", "double") < 0) {

            ANGELSCRIPT_REGISTERFAIL;
        }
    } else {
        // Would really love this to be a static assert but apparently that doesn't work
        LOG_FATAL("Unknown Ogre::Real used while trying to bind as stuff");
    }

    return true;
}

bool BindVector3(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Vector3", sizeof(Ogre::Vector3),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Vector3>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT,
           "void f(Real x, Real y, Real z)", asFUNCTION(Vector3Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "Real x", asOFFSET(Ogre::Vector3, x)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "Real y", asOFFSET(Ogre::Vector3, y)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "Real z", asOFFSET(Ogre::Vector3, z)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    return true;
}

bool BindVector4(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Vector4", sizeof(Ogre::Vector4),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Vector4>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT,
           "void f(Real x, Real y, Real z, Real w)", asFUNCTION(Vector4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "Real x", asOFFSET(Ogre::Vector4, x)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "Real y", asOFFSET(Ogre::Vector4, y)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "Real z", asOFFSET(Ogre::Vector4, z)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "Real w", asOFFSET(Ogre::Vector4, w)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindColour(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("ColourValue", sizeof(Ogre::ColourValue),
           asOBJ_VALUE | asGetTypeTraits<Ogre::ColourValue>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ColourValue", asBEHAVE_CONSTRUCT,
           "void f(float r, float g, float b, float a = 1.0)", asFUNCTION(ColourValueProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("ColourValue", asBEHAVE_CONSTRUCT,
           "void f(const Vector4 &in values)", asFUNCTION(ColourValueVector4Proxy),
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

    if(engine->RegisterObjectMethod("ColourValue",
           "void getHSB(Real &out hue, Real &out saturation, Real &out brightness) const",
           asMETHOD(Ogre::ColourValue, getHSB), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("ColourValue",
           "void setHSB(Real hue, Real saturation, Real brightness)",
           asMETHOD(Ogre::ColourValue, setHSB), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindMatrix4(asIScriptEngine* engine)
{

    if(engine->RegisterObjectType("Matrix4", sizeof(Ogre::Matrix4),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Matrix4>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in trans, const Vector3 &in scale = Ogre::Vector3(1, 1, 1), "
           "const Quaternion &in orientation = Ogre::Quaternion::IDENTITY)",
           asFUNCTION(MatrixProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Matrix4",
           "void makeTransform(const Vector3 &in trans, const Vector3 &in scale = "
           "Ogre::Vector3(1, 1, 1), const Quaternion &in orientation = "
           "Ogre::Quaternion::IDENTITY)",
           asMETHOD(Ogre::Matrix4, makeTransform), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Matrix4", "Vector3 getTrans() const",
           asMETHODPR(Ogre::Matrix4, getTrans, () const, Ogre::Vector3),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectMethod("Matrix4", "Quaternion extractQuaternion() const",
           asMETHOD(Ogre::Matrix4, extractQuaternion), asCALL_THISCALL) < 0) {
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
           asOBJ_VALUE | asGetTypeTraits<Ogre::Radian>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Degree", sizeof(Ogre::Degree),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Degree>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Quaternion", sizeof(Ogre::Quaternion),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Quaternion>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 xAxis() const",
           asMETHOD(Ogre::Quaternion, xAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 yAxis() const",
           asMETHOD(Ogre::Quaternion, yAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 zAxis() const",
           asMETHOD(Ogre::Quaternion, zAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Quaternion Inverse() const",
           asMETHOD(Ogre::Quaternion, Inverse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 opMul(const Vector3 &in vec) const",
           asMETHODPR(Ogre::Quaternion, operator*,(const Ogre::Vector3&) const, Ogre::Vector3),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Alias to the above function
    if(engine->RegisterObjectMethod("Quaternion",
           "Vector3 RotateVector(const Vector3 &in vec) const",
           asMETHODPR(Ogre::Quaternion, operator*,(const Ogre::Vector3&) const, Ogre::Vector3),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("Ogre::Quaternion") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Ogre::Quaternion IDENTITY",
           const_cast<Ogre::Quaternion*>(&Ogre::Quaternion::IDENTITY)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("Ogre") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //

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

    if(engine->RegisterObjectMethod("Radian", "Degree opImplConv() const",
           asFUNCTION(RadianToDegreeCast), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "Real valueDegrees() const",
           asMETHOD(Ogre::Radian, valueDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "Real valueRadians() const",
           asMETHOD(Ogre::Radian, valueRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "Real valueAngleUnits() const",
           asMETHOD(Ogre::Radian, valueAngleUnits), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Radian opImplConv() const",
           asFUNCTION(DegreeToRadianCast), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Real valueDegrees() const",
           asMETHOD(Ogre::Degree, valueDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Real valueRadians() const",
           asMETHOD(Ogre::Degree, valueRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Real valueAngleUnits() const",
           asMETHOD(Ogre::Degree, valueAngleUnits), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Radian &in radian, const Vector3 &in vector)",
           asFUNCTION(QuaternionProxyAroundAxis), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindPlane(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Plane", sizeof(Ogre::Plane),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Plane>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in normal, Real f)", asFUNCTION(PlaneFromNormalProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty(
           "Plane", "Vector3 normal", asOFFSET(Ogre::Plane, normal)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Plane", "Real dy", asOFFSET(Ogre::Plane, d)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindRay(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Ray", sizeof(Ogre::Ray),
           asOBJ_VALUE | asGetTypeTraits<Ogre::Ray>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Ray", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in origin, const Vector3 &in direction)",
           asFUNCTION(RayProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Ray",
           "bool intersects(const Plane &in plane, Real &out distance) const",
           asFUNCTION(RayIntersectsPlaneProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Ray", "Vector3 getPoint(Real t) const",
           asMETHOD(Ogre::Ray, getPoint), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

// ------------------------------------ //
bool BindRenderQueue(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("RenderQueue", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("Ogre::RenderQueue") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterEnum("Modes") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterEnumValue("Modes", "V1_LEGACY", Ogre::RenderQueue::V1_LEGACY) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterEnumValue("Modes", "V1_FAST", Ogre::RenderQueue::V1_FAST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterEnumValue("Modes", "FAST", Ogre::RenderQueue::FAST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("Ogre") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("RenderQueue",
           "void setRenderQueueMode(uint8 rqId, Ogre::RenderQueue::Modes newMode) const",
           asMETHOD(Ogre::RenderQueue, setRenderQueueMode), asCALL_THISCALL) < 0) {
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

    // These methods are actually in the base Node class
    if(engine->RegisterObjectMethod("SceneNode", "void addChild(SceneNode@ child)",
           asFUNCTION(SceneNodeAddChildProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // New helper method
    if(engine->RegisterObjectMethod("SceneNode", "void removeFromParent()",
           asFUNCTION(SceneNodeRemoveFromParent), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode",
           "void setPosition(const Ogre::Vector3 &in pos)",
           asMETHODPR(Ogre::SceneNode, setPosition, (const Ogre::Vector3&), void),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode", "void setOrientation(Ogre::Quaternion quat)",
           asMETHODPR(Ogre::SceneNode, setOrientation, (Ogre::Quaternion), void),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    // ------------------------------------ //
    // Item
    if(engine->RegisterObjectType("Item", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Item", "void setMaterial(const string &in materialname)",
           asFUNCTION(ItemSetMaterialProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Item", "void setDatablock(const string &in datablock)",
           asFUNCTION(ItemSetDataBlockProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Item",
           "void setCustomParameter(int index, const Ogre::Vector4 &in value)",
           asFUNCTION(ItemSetCustomParameterProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Item", "void setRenderQueueGroup(uint8 queueID)",
           asMETHOD(Ogre::Item, setRenderQueueGroup), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Item", "SkeletonInstance@ getSkeletonInstance()",
           asMETHOD(Ogre::Item, getSkeletonInstance), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Item", "Mesh@ getMesh()", asFUNCTION(ItemGetMeshProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Scene
    if(engine->RegisterObjectType("SceneManager", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneManager", "RenderQueue& getRenderQueue()",
           asMETHOD(Ogre::SceneManager, getRenderQueue), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindSkeletons(asIScriptEngine* engine)
{
    // ------------------------------------ //
    // SkeletonAnimation
    if(engine->RegisterObjectType("SkeletonAnimation", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SkeletonAnimation", "void setEnabled(bool enabled)",
           asMETHOD(Ogre::SkeletonAnimation, setEnabled), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SkeletonAnimation", "bool getEnabled()",
           asMETHOD(Ogre::SkeletonAnimation, getEnabled), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SkeletonAnimation", "void setLoop(bool loop)",
           asMETHOD(Ogre::SkeletonAnimation, setLoop), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SkeletonAnimation", "void addTime(Real time)",
           asMETHOD(Ogre::SkeletonAnimation, addTime), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // SkeletonInstance
    if(engine->RegisterObjectType("SkeletonInstance", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SkeletonInstance",
           "SkeletonAnimation@ getAnimation(const string &in name)",
           asFUNCTION(SkeletonInstanceGetAnimationProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindMeshes(asIScriptEngine* engine)
{
    // ------------------------------------ //
    // Mesh
    if(engine->RegisterObjectType("Mesh", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Mesh", "string getName()", asFUNCTION(MeshGetNameProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
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

    if(!BindOgreTypeDefs(engine))
        return false;

    if(!BindVector3(engine))
        return false;

    if(!BindVector4(engine))
        return false;

    if(!BindColour(engine))
        return false;

    if(!BindAnglesAndQuaternion(engine))
        return false;

    if(!BindMatrix4(engine))
        return false;

    if(!BindPlane(engine))
        return false;

    if(!BindRay(engine))
        return false;

    if(!BindSkeletons(engine))
        return false;

    if(!BindMeshes(engine))
        return false;

    if(!BindRenderQueue(engine))
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
