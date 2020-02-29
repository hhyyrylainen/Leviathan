// ------------------------------------ //
#include "BindDefinitions.h"

#include "Common/Matrix.h"
#include "Common/Plane.h"
#include "Common/Quaternion.h"
#include "Common/Ray.h"
#include "Common/Types.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

ObjectID NULL_OBJECT_WRAPPER = NULL_OBJECT;

Quaternion IdentityQuaternion = Quaternion::IDENTITY;

// Float2
void Float2ConstructorProxy(void* memory)
{
    new(memory) Float2();
}

void Float2ConstructorProxyAll(void* memory, float x, float y)
{
    new(memory) Float2(x, y);
}

void Float2ConstructorProxySingle(void* memory, float all)
{
    new(memory) Float2(all);
}

void Float2ConstructorProxyCopy(void* memory, const Float2& other)
{
    new(memory) Float2(other);
}

void Float2DestructorProxy(void* memory)
{
    reinterpret_cast<Float2*>(memory)->~Float2();
}

// Float3
void Float3ConstructorProxy(void* memory)
{
    new(memory) Float3();
}

void Float3ConstructorProxyAll(void* memory, float x, float y, float z)
{
    new(memory) Float3(x, y, z);
}

void Float3ConstructorProxySingle(void* memory, float all)
{
    new(memory) Float3(all);
}

void Float3ConstructorProxyCopy(void* memory, const Float3& other)
{
    new(memory) Float3(other);
}

void Float3ConstructorProxyFromInt3(void* memory, const Int3& values)
{
    new(memory) Float3(values);
}

void Float3DestructorProxy(void* memory)
{
    reinterpret_cast<Float3*>(memory)->~Float3();
}

// Float4
void Float4ConstructorProxy(void* memory)
{
    new(memory) Float4();
}

void Float4ConstructorProxyAll(void* memory, float x, float y, float z, float w)
{
    new(memory) Float4(x, y, z, w);
}

void Float4ConstructorProxySingle(void* memory, float all)
{
    new(memory) Float4(all);
}

void Float4ConstructorProxyCopy(void* memory, const Float4& other)
{
    new(memory) Float4(other);
}

void Float4DestructorProxy(void* memory)
{
    reinterpret_cast<Float4*>(memory)->~Float4();
}

// Quaternion
void QuaternionConstructorProxy(void* memory)
{
    new(memory) Quaternion();
}

void QuaternionConstructorProxyAll(void* memory, float x, float y, float z, float w)
{
    new(memory) Quaternion(x, y, z, w);
}

void QuaternionConstructorProxyCopy(void* memory, const Quaternion& other)
{
    new(memory) Quaternion(other);
}

void QuaternionConstructorAxisProxy(void* memory, const Float3& axis, Radian angle)
{
    new(memory) Quaternion(axis, angle);
}

void QuaternionConstructorFloat4Proxy(void* memory, const Float4& obj)
{
    new(memory) Quaternion(obj);
}

void QuaternionDestructorProxy(void* memory)
{
    reinterpret_cast<Quaternion*>(memory)->~Quaternion();
}
// ------------------------------------ //
// Int2
void Int2ConstructorProxy(void* memory)
{
    new(memory) Int2();
}

void Int2ConstructorProxyAll(void* memory, int x, int y)
{
    new(memory) Int2(x, y);
}

void Int2ConstructorProxySingle(void* memory, int all)
{
    new(memory) Int2(all);
}

void Int2ConstructorProxyCopy(void* memory, const Int2& other)
{
    new(memory) Int2(other);
}

void Int2ListConstructor(void* memory, int* list)
{
    new(memory) Int2(list[0], list[1]);
}

void Int2DestructorProxy(void* memory)
{
    reinterpret_cast<Int2*>(memory)->~Int2();
}

// Int3
void Int3ConstructorProxy(void* memory)
{
    new(memory) Int3();
}

void Int3ConstructorProxyAll(void* memory, int x, int y, int z)
{
    new(memory) Int3(x, y, z);
}

void Int3ConstructorProxySingle(void* memory, int all)
{
    new(memory) Int3(all);
}

void Int3ConstructorProxyCopy(void* memory, const Int3& other)
{
    new(memory) Int3(other);
}

void Int3DestructorProxy(void* memory)
{
    reinterpret_cast<Int3*>(memory)->~Int3();
}
// ------------------------------------ //
// Radian
void RadianConstructorProxy(void* memory)
{
    new(memory) Radian();
}

void RadianConstructorValueProxy(void* memory, float value)
{
    new(memory) Radian(value);
}

void RadianConstructorCopyProxy(void* memory, const Radian& other)
{
    new(memory) Radian(other);
}

void RadianConstructorDegreeProxy(void* memory, const Degree& degrees)
{
    new(memory) Radian(degrees);
}

void RadianDestructorProxy(void* memory)
{
    reinterpret_cast<Radian*>(memory)->~Radian();
}
// ------------------------------------ //
// Degree
void DegreeConstructorProxy(void* memory)
{
    new(memory) Degree();
}

void DegreeConstructorValueProxy(void* memory, float value)
{
    new(memory) Degree(value);
}

void DegreeConstructorCopyProxy(void* memory, const Degree& other)
{
    new(memory) Degree(other);
}

void DegreeConstructorRadianProxy(void* memory, const Radian& radians)
{
    new(memory) Degree(radians);
}

void DegreeDestructorProxy(void* memory)
{
    reinterpret_cast<Degree*>(memory)->~Degree();
}

// ------------------------------------ //
// Matrix4
void Matrix4ConstructorProxy(void* memory)
{
    new(memory) Matrix4();
}

void Matrix4DestructorProxy(void* memory)
{
    reinterpret_cast<Matrix4*>(memory)->~Matrix4();
}

// ------------------------------------ //
// Ray
void RayConstructorProxy(void* memory)
{
    new(memory) Ray();
}

void RayDestructorProxy(void* memory)
{
    reinterpret_cast<Ray*>(memory)->~Ray();
}

// ------------------------------------ //
// Plane
void PlaneConstructorVectorProxy(void* memory, const Float3& normal, float distance)
{
    new(memory) Plane(normal, distance);
}

void PlaneDestructorProxy(void* memory)
{
    reinterpret_cast<Plane*>(memory)->~Plane();
}

static auto UnitVUpProxy = Float3::UnitVUp;

static auto UnitXAxisProxy = Float3::UnitXAxis;

static auto UnitYAxisProxy = Float3::UnitYAxis;

static auto UnitZAxisProxy = Float3::UnitZAxis;

// ------------------------------------ //
// Start of the actual bind
namespace Leviathan {

bool BindFloat2(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Float2", sizeof(Float2),
           asOBJ_VALUE | asGetTypeTraits<Float2>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float2", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(Float2ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float2", asBEHAVE_CONSTRUCT, "void f(float value)",
           asFUNCTION(Float2ConstructorProxySingle), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float2", asBEHAVE_CONSTRUCT,
           "void f(float x, float y)", asFUNCTION(Float2ConstructorProxyAll),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float2", asBEHAVE_CONSTRUCT,
           "void f(const Float2 &in other)", asFUNCTION(Float2ConstructorProxyCopy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float2", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(Float2DestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    // Operators //
    if(engine->RegisterObjectMethod("Float2", "Float2& opAssign(const Float2 &in other)",
           asMETHODPR(Float2, operator=,(const Float2&), Float2&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float2", "Float2 opAdd(const Float2 &in other) const",
           asMETHODPR(Float2, operator+,(const Float2&) const, Float2), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float2", "Float2 opSub(const Float2 &in other) const",
           asMETHODPR(Float2, operator-,(const Float2&) const, Float2), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float2", "Float2 opMul(float multiply) const",
           asMETHODPR(Float2, operator*,(float) const, Float2), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float2", "Float2 Normalize() const",
           asMETHOD(Float2, Normalize), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(
           "Float2", "float HAddAbs()", asMETHOD(Float2, HAddAbs), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float2", "bool HasInvalidValues() const",
           asMETHOD(Float2, HasInvalidValues), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Direct access
    if(engine->RegisterObjectProperty("Float2", "float X", asOFFSET(Float2, X)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Float2", "float Y", asOFFSET(Float2, Y)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindFloat3(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Float3", sizeof(Float3),
           asOBJ_VALUE | asGetTypeTraits<Float3>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(Float3ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT, "void f(float value)",
           asFUNCTION(Float3ConstructorProxySingle), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT,
           "void f(float x, float y, float z)", asFUNCTION(Float3ConstructorProxyAll),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT,
           "void f(const Float3 &in other)", asFUNCTION(Float3ConstructorProxyCopy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT,
           "void f(const Int3 &in values)", asFUNCTION(Float3ConstructorProxyFromInt3),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(Float3DestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    // Operators //
    if(engine->RegisterObjectMethod("Float3", "Float3& opAssign(const Float3 &in other)",
           asMETHODPR(Float3, operator=,(const Float3&), Float3&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3 opAdd(const Float3 &in other) const",
           asMETHODPR(Float3, operator+,(const Float3&) const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3 opNeg() const",
           asMETHODPR(Float3, operator-,() const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "bool opEquals(const Float3 &in other) const",
           asMETHODPR(Float3, operator==,(const Float3&) const, bool), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3& opAddAssign(const Float3 &in other)",
           asMETHODPR(Float3, operator+=,(const Float3&), Float3&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3& opSubAssign(const Float3 &in other)",
           asMETHODPR(Float3, operator-=,(const Float3&), Float3&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3& opMulAssign(float value)",
           asMETHODPR(Float3, operator*=,(float), Float3&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3 opSub(const Float3 &in other) const",
           asMETHODPR(Float3, operator-,(const Float3&) const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3 opMul(float multiply) const",
           asMETHODPR(Float3, operator*,(float) const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3 opMul(const Float3 &in other) const",
           asMETHODPR(Float3, operator*,(const Float3&) const, Float3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3& opDivAssign(float value)",
           asMETHODPR(Float3, operator/=,(float), Float3&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectMethod("Float3", "Float3 Normalize() const",
           asMETHOD(Float3, Normalize), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "float HAddAbs() const",
           asMETHOD(Float3, HAddAbs), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod(
           "Float3", "float HAdd() const", asMETHOD(Float3, HAdd), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "float LengthSquared() const",
           asMETHOD(Float3, LengthSquared), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "float Dot(const Float3 &in val) const",
           asMETHOD(Float3, Dot), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "Float3 Cross(const Float3 &in val) const",
           asMETHOD(Float3, Cross), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "bool HasInvalidValues() const",
           asMETHOD(Float3, HasInvalidValues), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Direct access
    if(engine->RegisterObjectProperty("Float3", "float X", asOFFSET(Float3, X)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Float3", "float Y", asOFFSET(Float3, Y)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Float3", "float Z", asOFFSET(Float3, Z)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }


    // ------------------------------------ //
    // Named ones
    if(engine->SetDefaultNamespace("Float3") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Float3 UnitVUp", &UnitVUpProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Float3 UnitXAxis", &UnitXAxisProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Float3 UnitYAxis", &UnitYAxisProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Float3 UnitZAxis", &UnitZAxisProxy) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }



    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }



    return true;
}
// ------------------------------------ //
bool BindFloat4(asIScriptEngine* engine)
{
    // Float4
    if(engine->RegisterObjectType("Float4", sizeof(Float4),
           asOBJ_VALUE | asGetTypeTraits<Float4>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(Float4ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_CONSTRUCT, "void f(float value)",
           asFUNCTION(Float4ConstructorProxySingle), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_CONSTRUCT,
           "void f(float x, float y, float z, float w)", asFUNCTION(Float4ConstructorProxyAll),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_CONSTRUCT,
           "void f(const Float4 &in other)", asFUNCTION(Float4ConstructorProxyCopy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(Float4DestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Operators //
    if(engine->RegisterObjectMethod("Float4", "Float4& opAssign(const Float4 &in other)",
           asMETHODPR(Float4, operator=,(const Float4&), Float4&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "bool opEquals(const Float4 &in other) const",
           asMETHODPR(Float4, operator==,(const Float4&) const, bool), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "Float4 opAdd(const Float4 &in other) const",
           asMETHODPR(Float4, operator+,(const Float4&) const, Float4), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "Float4 opSub(const Float4 &in other) const",
           asMETHODPR(Float4, operator-,(const Float4&) const, Float4), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "Float4 opMul(float multiply) const",
           asMETHODPR(Float4, operator*,(float) const, Float4), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "Float4 opMul(const Float4 &in other) const",
           asMETHODPR(Float4, operator*,(const Float4&) const, Float4), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "Float4 Normalize() const",
           asMETHOD(Float4, Normalize), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "bool HasInvalidValues() const",
           asMETHOD(Float4, HasInvalidValues), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4",
           "void ConvertToHSB(float &out hue, float &out saturation, float &out brightness) "
           "const",
           asMETHOD(Float4, ConvertToHSB), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Direct access
    if(engine->RegisterObjectProperty("Float4", "float X", asOFFSET(Float4, X)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Float4", "float Y", asOFFSET(Float4, Y)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Float4", "float Z", asOFFSET(Float4, Z)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Float4", "float W", asOFFSET(Float4, W)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Namespace members
    if(engine->SetDefaultNamespace("Float4") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "Float4 FromHSB(float hue, float saturation, float brightness)",
           asFUNCTION(Float4::FromHSB), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindQuaternion(asIScriptEngine* engine)
{
    // Quaternion
    if(engine->RegisterObjectType("Quaternion", sizeof(Quaternion),
           asOBJ_VALUE | asGetTypeTraits<Quaternion>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(QuaternionConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(float x, float y, float z, float w)",
           asFUNCTION(QuaternionConstructorProxyAll), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Quaternion &in other)", asFUNCTION(QuaternionConstructorProxyCopy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Float4 &in copy)", asFUNCTION(QuaternionConstructorFloat4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Float3 &in axis, Radian angle)",
           asFUNCTION(QuaternionConstructorAxisProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Quaternion", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(QuaternionDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Operators //
    if(engine->RegisterObjectMethod("Quaternion",
           "Quaternion& opAssign(const Quaternion &in other)",
           asMETHODPR(Quaternion, operator=,(const Quaternion&), Quaternion&),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "bool opEquals(const Quaternion &in other) const",
           asMETHODPR(Quaternion, operator==,(const Quaternion&) const, bool),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "Quaternion opMul(const Quaternion &in other) const",
           asMETHODPR(Quaternion, operator*,(const Quaternion&) const, Quaternion),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "Float3 opMul(const Float3 &in vector) const",
           asMETHODPR(Quaternion, operator*,(const Float3&) const, Float3),
           asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Quaternion Normalize() const",
           asMETHOD(Quaternion, Normalize), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Float3 ToAxis() const",
           asMETHOD(Quaternion, ToAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "float ToAngle() const",
           asMETHOD(Quaternion, ToAngle), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Quaternion Inverse() const",
           asMETHOD(Quaternion, Inverse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion",
           "Quaternion Slerp(const Quaternion &in other, float fraction) const",
           asMETHOD(Quaternion, Slerp), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "bool HasInvalidValues() const",
           asMETHOD(Quaternion, HasInvalidValues), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Float3 XAxis() const",
           asMETHOD(Quaternion, XAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Float3 YAxis() const",
           asMETHOD(Quaternion, YAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Quaternion", "Float3 ZAxis() const",
           asMETHOD(Quaternion, ZAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Direct access
    if(engine->RegisterObjectProperty("Quaternion", "float X", asOFFSET(Quaternion, X)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float Y", asOFFSET(Quaternion, Y)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float Z", asOFFSET(Quaternion, Z)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Quaternion", "float W", asOFFSET(Quaternion, W)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Namespace members
    if(engine->SetDefaultNamespace("Quaternion") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Quaternion IDENTITY", IdentityQuaternion) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty(
           "const Quaternion IdentityQuaternion", IdentityQuaternion) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction("Quaternion LookAt(const Float3 &in "
                                      "sourcepoint, const Float3 &in target)",
           asFUNCTION(Quaternion::LookAt), asCALL_CDECL) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindInt2(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Int2", sizeof(Int2),
           asOBJ_VALUE | asGetTypeTraits<Int2>() | asOBJ_APP_CLASS_ALLINTS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int2", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(Int2ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int2", asBEHAVE_CONSTRUCT, "void f(int value)",
           asFUNCTION(Int2ConstructorProxySingle), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int2", asBEHAVE_CONSTRUCT, "void f(int x, int y)",
           asFUNCTION(Int2ConstructorProxyAll), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int2", asBEHAVE_CONSTRUCT,
           "void f(const Int2 &in other)", asFUNCTION(Int2ConstructorProxyCopy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int2", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(Int2DestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Int2", asBEHAVE_LIST_CONSTRUCT,
           "void f(const int &in) {int, int}", asFUNCTION(Int2ListConstructor),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Operators //
    if(engine->RegisterObjectMethod("Int2", "Int2& opAssign(const Int2 &in other)",
           asMETHODPR(Int2, operator=,(const Int2&), Int2&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Int2", "Int2 opAdd(const Int2 &in other) const",
           asMETHODPR(Int2, operator+,(const Int2&) const, Int2), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Int2", "Int2 opSub(const Int2 &in other) const",
           asMETHODPR(Int2, operator-,(const Int2&) const, Int2), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Int2", "Int2 opMul(int multiply) const",
           asMETHODPR(Int2, operator*,(int) const, Int2), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Direct access
    if(engine->RegisterObjectProperty("Int2", "int X", asOFFSET(Int2, X)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Int2", "int Y", asOFFSET(Int2, Y)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindInt3(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Int3", sizeof(Int3),
           asOBJ_VALUE | asGetTypeTraits<Int3>() | asOBJ_APP_CLASS_ALLINTS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int3", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(Int3ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int3", asBEHAVE_CONSTRUCT, "void f(int value)",
           asFUNCTION(Int3ConstructorProxySingle), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int3", asBEHAVE_CONSTRUCT,
           "void f(int x, int y, int z)", asFUNCTION(Int3ConstructorProxyAll),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int3", asBEHAVE_CONSTRUCT,
           "void f(const Int3 &in other)", asFUNCTION(Int3ConstructorProxyCopy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Int3", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(Int3DestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    // Operators //
    if(engine->RegisterObjectMethod("Int3", "Int3& opAssign(const Int3 &in other)",
           asMETHODPR(Int3, operator=,(const Int3&), Int3&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Int3", "Int3 opAdd(const Int3 &in other) const",
           asMETHODPR(Int3, operator+,(const Int3&) const, Int3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Int3", "Int3 opSub(const Int3 &in other) const",
           asMETHODPR(Int3, operator-,(const Int3&) const, Int3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Int3", "Int3 opMul(int multiply) const",
           asMETHODPR(Int3, operator*,(int) const, Int3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // Direct access
    if(engine->RegisterObjectProperty("Int3", "int X", asOFFSET(Int3, X)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Int3", "int Y", asOFFSET(Int3, Y)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectProperty("Int3", "int Z", asOFFSET(Int3, Z)) < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindRadian(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Radian", sizeof(Radian),
           asOBJ_VALUE | asGetTypeTraits<Radian>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(RadianConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_CONSTRUCT, "void f(float radians)",
           asFUNCTION(RadianConstructorValueProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_CONSTRUCT,
           "void f(const Radian &in other)", asFUNCTION(RadianConstructorCopyProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(RadianDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "Radian& opAssign(const Radian &in other)",
           asMETHODPR(Radian, operator=,(const Radian&), Radian&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "float ValueInRadians() const",
           asMETHOD(Radian, ValueInRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "float ValueInDegrees() const",
           asMETHOD(Radian, ValueInDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindDegree(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Degree", sizeof(Degree),
           asOBJ_VALUE | asGetTypeTraits<Degree>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(DegreeConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_CONSTRUCT, "void f(float degrees)",
           asFUNCTION(DegreeConstructorValueProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_CONSTRUCT,
           "void f(const Degree &in other)", asFUNCTION(DegreeConstructorCopyProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(DegreeDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Degree& opAssign(const Degree &in other)",
           asMETHODPR(Degree, operator=,(const Degree&), Degree&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "float ValueInRadians() const",
           asMETHOD(Degree, ValueInRadians), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "float ValueInDegrees() const",
           asMETHOD(Degree, ValueInDegrees), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    // ------------------------------------ //
    // Bind things needing both degree and radian
    if(engine->RegisterObjectBehaviour("Radian", asBEHAVE_CONSTRUCT,
           "void f(const Degree &in degrees)", asFUNCTION(RadianConstructorDegreeProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Degree", asBEHAVE_CONSTRUCT,
           "void f(const Degree &in degrees)", asFUNCTION(DegreeConstructorRadianProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "Radian& opAssign(const Degree &in degrees)",
           asMETHODPR(Radian, operator=,(const Degree&), Radian&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Degree", "Degree& opAssign(const Radian &in radians)",
           asMETHODPR(Degree, operator=,(const Radian&), Degree&), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    if(engine->RegisterObjectMethod("Degree", "Radian opImplCast() const",
           asMETHODPR(Degree, operator Radian,() const, Radian), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Radian", "Degree opImplCast() const",
           asMETHODPR(Radian, operator Degree,() const, Degree), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindMatrix4(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Matrix4", sizeof(Matrix4),
           asOBJ_VALUE | asGetTypeTraits<Matrix4>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(Matrix4ConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Matrix4", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(Matrix4DestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindRay(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Ray", sizeof(Ray),
           asOBJ_VALUE | asGetTypeTraits<Ray>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Ray", asBEHAVE_CONSTRUCT, "void f()",
           asFUNCTION(RayConstructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Ray", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(RayDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindPlane(asIScriptEngine* engine)
{
    if(engine->RegisterObjectType("Plane", sizeof(Plane),
           asOBJ_VALUE | asGetTypeTraits<Plane>() | asOBJ_APP_CLASS_ALLFLOATS) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Plane", asBEHAVE_CONSTRUCT,
           "void f(const Float3 &in normal, float distance)",
           asFUNCTION(PlaneConstructorVectorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }
    if(engine->RegisterObjectBehaviour("Plane", asBEHAVE_DESTRUCT, "void f()",
           asFUNCTION(PlaneDestructorProxy), asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
// ------------------------------------ //
bool BindTypeDefs(asIScriptEngine* engine)
{
    if(engine->RegisterTypedef("ObjectID", "int32") < 0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const ObjectID NULL_OBJECT", &NULL_OBJECT_WRAPPER) <
        0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}
} // namespace Leviathan

// ------------------------------------ //
// Main bind function
bool Leviathan::BindTypes(asIScriptEngine* engine)
{
    if(!BindInt2(engine))
        return false;

    if(!BindInt3(engine))
        return false;

    if(!BindRadian(engine))
        return false;

    if(!BindDegree(engine))
        return false;

    // Register common float types //
    if(!BindFloat2(engine))
        return false;

    if(!BindFloat3(engine))
        return false;

    if(!BindFloat4(engine))
        return false;

    if(!BindQuaternion(engine))
        return false;

    if(!BindMatrix4(engine))
        return false;

    if(!BindRay(engine))
        return false;

    if(!BindPlane(engine))
        return false;

    if(!BindTypeDefs(engine))
        return false;

    return true;
}
