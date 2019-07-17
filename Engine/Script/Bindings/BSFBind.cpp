// ------------------------------------ //
#include "BSFBind.h"

#include "Define.h"
#include "Logger.h"

// For Float type conversions
#include "Common/Types.h"

#include "Engine.h"
#include "Exceptions.h"
#include "Rendering/Graphics.h"

#include "BsApplication.h"
#include "bsfCore/Material/BsMaterial.h"
#include "bsfCore/Material/BsShader.h"
#include "bsfCore/Scene/BsSceneObject.h"
#include "bsfEngine/Resources/BsBuiltinResources.h"
#include "bsfUtility/Math/BsMatrix4.h"
#include "bsfUtility/Math/BsRay.h"

#include <type_traits>

using namespace Leviathan;
// ------------------------------------ //

#define CHECK_SELF \
    if(!self)      \
        throw InvalidState("Invalid bsf object handle");

// Proxies etc.
// ------------------------------------ //
void ColorProxy(void* memory, float r, float g, float b, float a)
{
    new(memory) bs::Color(r, g, b, a);
}

void ColorVector4Proxy(void* memory, const bs::Vector4& values)
{
    new(memory) bs::Color(values.x, values.y, values.z, values.w);
}

void MatrixProxy(void* memory, const bs::Vector3& position, const bs::Vector3& scale,
    const bs::Quaternion& orientation)
{
    new(memory) bs::Matrix4;
    static_cast<bs::Matrix4*>(memory)->setTRS(position, orientation, scale);
}

void MatrixProxyUninitialized(void* memory)
{
    new(memory) bs::Matrix4;
}

void DegreeProxy(void* memory, float degree)
{
    new(memory) bs::Degree(degree);
}

void DegreeProxyRadian(void* memory, const bs::Radian& radian)
{
    new(memory) bs::Degree(radian);
}

bs::Radian DegreeToRadianCast(bs::Degree* self)
{
    return *self;
}

void RadianProxy(void* memory, float radian)
{
    new(memory) bs::Radian(radian);
}

void RadianProxyDegree(void* memory, const bs::Degree& degree)
{
    new(memory) bs::Radian(degree);
}

bs::Degree RadianToDegreeCast(bs::Radian* self)
{
    return *self;
}

void QuaternionProxyValues(void* memory, float W, float X, float Y, float Z)
{
    new(memory) bs::Quaternion(W, X, Y, Z);
}

void QuaternionProxyAroundAxis(
    void* memory, const bs::Radian& radian, const bs::Vector3& vector)
{
    new(memory) bs::Quaternion(vector, radian);
}

void QuaternionProxyAroundAxisOtherWay(
    void* memory, const bs::Vector3& vector, const bs::Radian& radian)
{
    new(memory) bs::Quaternion(vector, radian);
}

void Vector3Proxy(void* memory, float x, float y, float z)
{
    new(memory) bs::Vector3(x, y, z);
}

void Vector4Proxy(void* memory, float x, float y, float z, float w)
{
    new(memory) bs::Vector4(x, y, z, w);
}

void PlaneFromNormalProxy(void* memory, const bs::Vector3& normal, float f)
{
    new(memory) bs::Plane(normal, f);
}

void RayProxy(void* memory, const bs::Vector3& origin, const bs::Vector3& direction)
{
    new(memory) bs::Ray(origin, direction);
}

bool RayIntersectsPlaneProxy(const bs::Ray* self, const bs::Plane& plane, float& distance)
{
    bool intersects;

    std::tie(intersects, distance) = self->intersects(plane);
    return intersects;
}

void SceneObjectSetParent(
    bs::HSceneObject& self, const bs::HSceneObject& parent, bool keepWorldTransform)
{
    CHECK_SELF;
    self->setParent(parent, keepWorldTransform);
}

void SceneObjectRemoveFromParent(bs::HSceneObject& self)
{
    CHECK_SELF;
    self->setParent(bs::HSceneObject(), false);
}

void SceneObjectSetPosition(bs::HSceneObject& self, const bs::Vector3& position)
{
    CHECK_SELF;
    self->setPosition(position);
}

bs::Vector3 SceneObjectGetPosition(bs::HSceneObject& self)
{
    CHECK_SELF;
    return self->getLocalTransform().getPosition();
}

void SceneObjectSetRotation(bs::HSceneObject& self, const bs::Quaternion& orientation)
{
    CHECK_SELF;
    self->setRotation(orientation);
}

void MaterialSetTextureProxy(
    bs::HMaterial& self, const std::string& name, const bs::HTexture& texture)
{
    if(!self || self->isDestroyed()) {

        asGetActiveContext()->SetException("method called on invalid HMaterial instance");
        return;
    }

    self->setTexture(name.c_str(), texture);
}

void MaterialSetVec44Proxy(
    bs::HMaterial& self, const std::string& name, const bs::Vector4& value)
{
    if(!self || self->isDestroyed()) {

        asGetActiveContext()->SetException("method called on invalid HMaterial instance");
        return;
    }

    self->setVec4(name.c_str(), value);
}


template<class T>
void DefaultHandleConstructor(void* memory)
{
    new(memory) T();
}

template<class T>
void HandleCopyConstructor(void* memory, const T& other)
{
    new(memory) T(other);
}

template<class T>
void HandleDestructor(void* memory)
{
    ((T*)memory)->~T();
}

template<class T>
bool HandleHasValue(T& self)
{
    return self.operator bool();
}

template<class HandleT, class HeldT, bool HandleParam>
bool ResourceHandleHasValue(HandleT& self)
{
    return self.operator int
               bs::TResourceHandle<HeldT, HandleParam>::template Bool_struct<HeldT>::*() == 1;
}

template<class T>
bool HandleIsNotDestroyed(T& self)
{
    return !self.isDestroyed();
}

template<class T>
void ResetHandle(T& self)
{
    self = nullptr;
}

// ------------------------------------ //
// Utility helpers
void ShaderFromNameFactory(void* memory, const std::string& name)
{
    auto shader = Engine::Get()->GetGraphics()->LoadShaderByName(name);

    if(!shader) {
        asGetActiveContext()->SetException(
            "no shader could be loaded with the specified name");
        return;
    }

    new(memory) bs::HShader(std::move(shader));
}

void ShaderFromType(void* memory, bs::BuiltinShader type)
{
    auto resource = bs::gBuiltinResources().getBuiltinShader(type);

    if(!resource) {
        asGetActiveContext()->SetException("invalid inbuilt shader");
        return;
    }

    new(memory) bs::HShader(std::move(resource));
}

void MaterialFromShaderFactory(void* memory, const bs::HShader& shader)
{
    auto material = bs::Material::create(shader);

    if(!material) {
        asGetActiveContext()->SetException("failed to create a material from the shader");
        return;
    }

    new(memory) bs::HMaterial(std::move(material));
}

void TextureFromNameFactory(void* memory, const std::string& name)
{
    auto texture = Engine::Get()->GetGraphics()->LoadTextureByName(name);

    if(!texture) {
        asGetActiveContext()->SetException(
            "no texture could be loaded with the specified name");
        return;
    }

    new(memory) bs::HTexture(std::move(texture));
}



// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {


template<class HandleT, class HeldT>
bool RegisterBSFShandleType(const char* type, asIScriptEngine* engine)
{
    if(engine->RegisterObjectType(
           type, sizeof(HandleT), asOBJ_VALUE | asGetTypeTraits<HandleT>()) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour(type, asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(DefaultHandleConstructor<HandleT>), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour(type, asBEHAVE_CONSTRUCT,
           ("void f(" + std::string(type) + " &in other)").c_str(),
           asFUNCTION(HandleCopyConstructor<HandleT>), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour(type, asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(HandleDestructor<HandleT>), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(type,
           (std::string(type) + "& opAssign(" + type + " &in other)").c_str(),
           asMETHODPR(HandleT, operator=,(const HandleT&), HandleT&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(type, "void reset()", asFUNCTION(ResetHandle<HandleT>),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if constexpr(std::is_base_of_v<bs::GameObjectHandleBase, HandleT>) {
        if(engine->RegisterObjectMethod(type, "bool valid() const",
               asFUNCTION(HandleIsNotDestroyed<HandleT>), asCALL_CDECL_OBJFIRST) < 0) {
            ANGELSCRIPT_REGISTERFAIL;
        }
    } else {

        if constexpr(std::is_base_of_v<bs::TResourceHandleBase<true>, HandleT>) {
            if(engine->RegisterObjectMethod(type, "bool valid() const",
                   asFUNCTION((ResourceHandleHasValue<HandleT, HeldT, true>)),
                   asCALL_CDECL_OBJFIRST) < 0) {
                ANGELSCRIPT_REGISTERFAIL;
            } else if constexpr(std::is_base_of_v<bs::TResourceHandleBase<false>, HandleT>) {
                if(engine->RegisterObjectMethod(type, "bool valid() const",
                       asFUNCTION((ResourceHandleHasValue<HandleT, HeldT, false>)),
                       asCALL_CDECL_OBJFIRST) < 0) {
                    ANGELSCRIPT_REGISTERFAIL;
                }
            } else {
                if(engine->RegisterObjectMethod(type, "bool valid() const",
                       asFUNCTION(HandleHasValue<HandleT>), asCALL_CDECL_OBJFIRST) < 0) {
                    ANGELSCRIPT_REGISTERFAIL;
                }
            }
        }
    }


    return true;
}

// For float binding
bool BindBSFTypeDefs(asIScriptEngine* engine)
{
    if(engine->RegisterTypedef("Scene", "int32") < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // if constexpr(std::is_same_v<float, float>) {
    //     if(engine->RegisterTypedef("float", "float") < 0) {

    //         ANGELSCRIPT_REGISTERFAIL;
    //     }
    // } else if constexpr(std::is_same_v<float, double>) {
    //     if(engine->RegisterTypedef("float", "double") < 0) {

    //         ANGELSCRIPT_REGISTERFAIL;
    //     }
    // } else {
    //     // Would really love this to be a static assert but apparently that doesn't work
    //     LOG_FATAL("Unknown float used while trying to bind as stuff");
    // }

    return true;
}

bool BindVector3(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Vector3", sizeof(bs::Vector3),
           asOBJ_VALUE | asGetTypeTraits<bs::Vector3>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT,
           "void f(float x, float y, float z)", asFUNCTION(Vector3Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "float x", asOFFSET(bs::Vector3, x)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "float y", asOFFSET(bs::Vector3, y)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector3", "float z", asOFFSET(bs::Vector3, z)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs::Vector3") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty(
           "const bs::Vector3 UNIT_X", const_cast<bs::Vector3*>(&bs::Vector3::UNIT_X)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty(
           "const bs::Vector3 UNIT_Y", const_cast<bs::Vector3*>(&bs::Vector3::UNIT_Y)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty(
           "const bs::Vector3 UNIT_Z", const_cast<bs::Vector3*>(&bs::Vector3::UNIT_Z)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindVector4(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Vector4", sizeof(bs::Vector4),
           asOBJ_VALUE | asGetTypeTraits<bs::Vector4>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Vector4", asBEHAVE_CONSTRUCT,
           "void f(float x, float y, float z, float w)", asFUNCTION(Vector4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "float x", asOFFSET(bs::Vector4, x)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "float y", asOFFSET(bs::Vector4, y)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "float z", asOFFSET(bs::Vector4, z)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Vector4", "float w", asOFFSET(bs::Vector4, w)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindColour(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Color", sizeof(bs::Color),
           asOBJ_VALUE | asGetTypeTraits<bs::Color>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT,
           "void f(float r, float g, float b, float a = 1.0)", asFUNCTION(ColorProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT,
           "void f(const Vector4 &in values)", asFUNCTION(ColorVector4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Color", "float r", asOFFSET(bs::Color, r)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Color", "float g", asOFFSET(bs::Color, g)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Color", "float b", asOFFSET(bs::Color, b)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Color", "float a", asOFFSET(bs::Color, a)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Color",
           "void getHSB(float &out hue, float &out saturation, float &out brightness) "
           "const",
           asMETHOD(bs::Color, getHSB), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs::Color") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "bs::Color fromHSB(float hue, float saturation, float brightness)",
           asFUNCTION(bs::Color::fromHSB), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindMatrix4(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Matrix4", sizeof(bs::Matrix4),
           asOBJ_VALUE | asGetTypeTraits<bs::Matrix4>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in trans, const Vector3 &in scale = bs::Vector3(1, 1, "
           "1), "
           "const Quaternion &in orientation = bs::Quaternion::IDENTITY)",
           asFUNCTION(MatrixProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Matrix4",
           "void setTRS(const Vector3 &in trans, const Vector3 &in scale = "
           "bs::Vector3(1, 1, 1), const Quaternion &in orientation = "
           "bs::Quaternion::IDENTITY)",
           asMETHOD(bs::Matrix4, setTRS), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Matrix4", "Vector3 getTranslation() const",
           asMETHODPR(bs::Matrix4, getTranslation, () const, bs::Vector3),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Matrix4",
           "void decomposition(Vector3 &out position, Quaternion &out rotation, Vector3 "
           "&out "
           "scale) const",
           asMETHOD(bs::Matrix4, decomposition), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // if(engine->RegisterObjectMethod("Matrix4", "Quaternion extractQuaternion() const",
    //        asMETHOD(bs::Matrix4, get), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }


    if(engine->SetDefaultNamespace("bs::Matrix4") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const bs::Matrix4 IDENTITY",
           const_cast<bs::Matrix4*>(&bs::Matrix4::IDENTITY)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}

bool BindAnglesAndQuaternion(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Radian", sizeof(bs::Radian),
           asOBJ_VALUE | asGetTypeTraits<bs::Radian>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Degree", sizeof(bs::Degree),
           asOBJ_VALUE | asGetTypeTraits<bs::Degree>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectType("Quaternion", sizeof(bs::Quaternion),
           asOBJ_VALUE | asGetTypeTraits<bs::Quaternion>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(float w, float x, float y, float z)", asFUNCTION(QuaternionProxyValues),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 xAxis() const",
           asMETHOD(bs::Quaternion, xAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 yAxis() const",
           asMETHOD(bs::Quaternion, yAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 zAxis() const",
           asMETHOD(bs::Quaternion, zAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Quaternion inverse() const",
           asMETHOD(bs::Quaternion, inverse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Vector3 opMul(const Vector3 &in vec) const",
           asMETHOD(bs::Quaternion, rotate), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "Vector3 rotate(const Vector3 &in vec) const", asMETHOD(bs::Quaternion, rotate),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "Quaternion opMul(const Quaternion &in vec) const",
           asMETHODPR(bs::Quaternion, operator*,(const bs::Quaternion&) const, bs::Quaternion),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "Quaternion& opAssign(const Quaternion &in quat)",
           asMETHODPR(bs::Quaternion, operator=,(const bs::Quaternion&), bs::Quaternion&),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float x", asOFFSET(bs::Quaternion, x)) <
        0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float y", asOFFSET(bs::Quaternion, y)) <
        0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float z", asOFFSET(bs::Quaternion, z)) <
        0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float w", asOFFSET(bs::Quaternion, w)) <
        0) {

        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->SetDefaultNamespace("bs::Quaternion") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const bs::Quaternion IDENTITY",
           const_cast<bs::Quaternion*>(&bs::Quaternion::IDENTITY)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs") < 0) {
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

    if(engine->RegisterObjectMethod("Radian", "float valueDegrees() const",
           asMETHOD(bs::Radian, valueDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "float valueRadians() const",
           asMETHOD(bs::Radian, valueRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "float valueAngleUnits() const",
           asMETHOD(bs::Radian, valueRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Radian opImplConv() const",
           asFUNCTION(DegreeToRadianCast), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "float valueDegrees() const",
           asMETHOD(bs::Degree, valueDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "float valueRadians() const",
           asMETHOD(bs::Degree, valueRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "float valueAngleUnits() const",
           asMETHOD(bs::Degree, valueDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Radian &in radian, const Vector3 &in vector)",
           asFUNCTION(QuaternionProxyAroundAxis), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in vector, const Radian &in radian)",
           asFUNCTION(QuaternionProxyAroundAxisOtherWay), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindPlane(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Plane", sizeof(bs::Plane),
           asOBJ_VALUE | asGetTypeTraits<bs::Plane>() | asOBJ_POD |
               asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in normal, float f)", asFUNCTION(PlaneFromNormalProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Plane", "Vector3 normal", asOFFSET(bs::Plane, normal)) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Plane", "float dy", asOFFSET(bs::Plane, d)) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindRay(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Ray", sizeof(bs::Ray),
           asOBJ_VALUE | asGetTypeTraits<bs::Ray>() | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Ray", asBEHAVE_CONSTRUCT,
           "void f(const Vector3 &in origin, const Vector3 &in direction)",
           asFUNCTION(RayProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Ray",
           "bool intersects(const Plane &in plane, float &out distance) const",
           asFUNCTION(RayIntersectsPlaneProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Ray", "Vector3 getPoint(float t) const",
           asMETHOD(bs::Ray, getPoint), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

// ------------------------------------ //
// bool BindRenderQueue(asIScriptEngine* engine)
// {
//     if(engine->RegisterObjectType("RenderQueue", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->SetDefaultNamespace("bs::RenderQueue") < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->RegisterEnum("Modes") < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->RegisterEnumValue("Modes", "V1_LEGACY", bs::RenderQueue::V1_LEGACY) < 0)
//     {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->RegisterEnumValue("Modes", "V1_FAST", bs::RenderQueue::V1_FAST) < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->RegisterEnumValue("Modes", "FAST", bs::RenderQueue::FAST) < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->SetDefaultNamespace("bs") < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     if(engine->RegisterObjectMethod("RenderQueue",
//            "void setRenderQueueMode(uint8 rqId, bs::RenderQueue::Modes newMode) const",
//            asMETHOD(bs::RenderQueue, setRenderQueueMode), asCALL_THISCALL) < 0) {
//         ANGELSCRIPT_REGISTERFAIL;
//     }

//     return true;
// }

// ------------------------------------ //

bool BindScene(asIScriptEngine* engine)
{
    if(!RegisterBSFShandleType<bs::HSceneObject, bs::SceneObject>("HSceneObject", engine))
        return false;

    if(engine->RegisterObjectMethod("HSceneObject",
           "void setParent(const HSceneObject &in parent, bool keepWorldTransform)",
           asFUNCTION(SceneObjectSetParent), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HSceneObject", "void removeFromParent()",
           asFUNCTION(SceneObjectRemoveFromParent), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HSceneObject", "void setPosition(const Vector3 &in pos)",
           asFUNCTION(SceneObjectSetPosition), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HSceneObject", "Vector3 getPosition() const",
           asFUNCTION(SceneObjectGetPosition), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HSceneObject",
           "void setOrientation(const Quaternion &in quaterion)",
           asFUNCTION(SceneObjectSetRotation), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HSceneObject",
           "void setRotation(const Quaternion &in quaterion)",
           asFUNCTION(SceneObjectSetRotation), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }



    // ------------------------------------ //
    // HRenderable
    if(!RegisterBSFShandleType<bs::HRenderable, bs::Renderable>("HRenderable", engine))
        return false;


    // ------------------------------------ //
    // Scene


    // if(engine->RegisterObjectMethod("SceneManager", "RenderQueue& getRenderQueue()",
    //        asMETHOD(bs::SceneManager, getRenderQueue), asCALL_THISCALL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    return true;
}

bool BindTextures(asIScriptEngine* engine)
{
    if(!RegisterBSFShandleType<bs::HTexture, bs::Texture>("HTexture", engine))
        return false;

    if(engine->RegisterObjectBehaviour("HTexture", asBEHAVE_CONSTRUCT,
           "void f(const string &in name)", asFUNCTION(TextureFromNameFactory),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindMaterials(asIScriptEngine* engine)
{
    if(engine->RegisterEnum("BuiltinShader") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("bs::BuiltinShader") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(
        "BuiltinShader", "Standard", bs::BuiltinShader::Standard);

    ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(
        "BuiltinShader", "Transparent", bs::BuiltinShader::Transparent);

    ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(
        "BuiltinShader", "ParticlesUnlit", bs::BuiltinShader::ParticlesUnlit);

    ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(
        "BuiltinShader", "ParticlesLit", bs::BuiltinShader::ParticlesLit);

    ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(
        "BuiltinShader", "ParticlesLitOpaque", bs::BuiltinShader::ParticlesLitOpaque);

    ANGELSCRIPT_REGISTER_ENUM_VALUE_WITH_NAME(
        "BuiltinShader", "Decal", bs::BuiltinShader::Decal);

    if(engine->SetDefaultNamespace("bs") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!RegisterBSFShandleType<bs::HShader, bs::Shader>("HShader", engine))
        return false;

    if(engine->RegisterObjectBehaviour("HShader", asBEHAVE_CONSTRUCT,
           "void f(const string &in name)", asFUNCTION(ShaderFromNameFactory),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("HShader", asBEHAVE_CONSTRUCT,
           "void f(BuiltinShader type)", asFUNCTION(ShaderFromType),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!RegisterBSFShandleType<bs::HMaterial, bs::Material>("HMaterial", engine))
        return false;

    if(engine->RegisterObjectBehaviour("HMaterial", asBEHAVE_CONSTRUCT,
           "void f(const HShader &in shader)", asFUNCTION(MaterialFromShaderFactory),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HMaterial",
           "void setTexture(const string &in name, const HTexture &in texture)",
           asFUNCTION(MaterialSetTextureProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("HMaterial",
           "void setVec4(const string &in name, const Vector4 &in value)",
           asFUNCTION(MaterialSetVec44Proxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }



    return true;
}

bool BindMeshes(asIScriptEngine* engine)
{
    // ------------------------------------ //
    // Mesh
    if(!RegisterBSFShandleType<bs::HMesh, bs::Mesh>("HMesh", engine))
        return false;

    return true;
}

} // namespace Leviathan
// ------------------------------------ //
bool Leviathan::BindBSF(asIScriptEngine* engine)
{
    // This doesn't need to be restored if we fail //
    if(engine->SetDefaultNamespace("bs") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(!BindBSFTypeDefs(engine))
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

    if(!BindTextures(engine))
        return false;

    if(!BindMaterials(engine))
        return false;

    if(!BindMeshes(engine))
        return false;

    // if(!BindRenderQueue(engine))
    //     return false;

    if(!BindScene(engine))
        return false;

    // if(engine->RegisterObjectType("Root", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    // ------------------ Global functions ------------------ //
    // if(engine->RegisterGlobalFunction(
    //        "Root@ Getbs()", asFUNCTION(ScriptGetbs), asCALL_CDECL) < 0) {
    //     ANGELSCRIPT_REGISTERFAIL;
    // }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
