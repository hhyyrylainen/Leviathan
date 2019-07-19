// ------------------------------------ //
#include "TypesBind.h"

#include "Common/Types.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //

ObjectID NULL_OBJECT_WRAPPER = NULL_OBJECT;

Float4 IdentityQuaternion = Float4::IdentityQuaternion();

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
// BSF conversions
void Vector3Float3Proxy(void* memory, const Float3& vector)
{
    new(memory) bs::Vector3(vector);
}

void Float3Vector3Proxy(void* memory, const bs::Vector3& vector)
{

    new(memory) Float3(vector);
}

void QuaternionFloat4Proxy(void* memory, const Float4& vector)
{
    new(memory) bs::Quaternion(vector);
}

void Float4QuaternionProxy(void* memory, const bs::Quaternion& quat)
{
    new(memory) Float4(quat);
}

Float4 ConvertQuaternionToFloat4(bs::Quaternion* self)
{
    return Float4(*self);
}

void Vector4Float4Proxy(void* memory, const Float4& values)
{
    new(memory) bs::Vector4(values);
}

void ColorFloat4Proxy(void* memory, const Float4& values)
{
    new(memory) bs::Color(values);
}

void Float4ColorProxy(void* memory, const bs::Color& values)
{
    new(memory) Float4(values);
}

static auto UnitVUpProxy = Float3::UnitVUp;

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

    if(engine->RegisterObjectMethod(
           "Float4", "Float3 ToAxis() const", asMETHOD(Float4, ToAxis), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "float ToAngle() const",
           asMETHOD(Float4, ToAngle), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "Float4 Inverse() const",
           asMETHOD(Float4, Inverse), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4",
           "Float3 RotateVector(const Float3 &in vector) const",
           asMETHOD(Float4, RotateVector), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4",
           "Float4 Slerp(const Float4 &in other, float fraction) const",
           asMETHOD(Float4, Slerp), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float4", "bool HasInvalidValues() const",
           asMETHOD(Float4, HasInvalidValues), asCALL_THISCALL) < 0) {
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
    // Named ones
    if(engine->SetDefaultNamespace("Float4") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalProperty("const Float4 IdentityQuaternion", IdentityQuaternion) <
        0) {

        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterGlobalFunction(
           "Float4 QuaternionLookAt(const Float3 &in sourcepoint, const Float3 &in target)",
           asFUNCTION(Float4::QuaternionLookAt), asCALL_CDECL) < 0) {

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
bool BindBSFConversions(asIScriptEngine* engine)
{
    if(engine->RegisterObjectBehaviour("bs::Vector3", asBEHAVE_CONSTRUCT,
           "void f(const Float3 &in vector)", asFUNCTION(Vector3Float3Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("bs::Vector4", asBEHAVE_CONSTRUCT,
           "void f(const Float4 &in values)", asFUNCTION(Vector4Float4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("bs::Color", asBEHAVE_CONSTRUCT,
           "void f(const Float4 &in values)", asFUNCTION(ColorFloat4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float3", asBEHAVE_CONSTRUCT,
           "void f(const bs::Vector3 &in vector)", asFUNCTION(Float3Vector3Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Float3", "bs::Vector3 opImplConv() const",
           asMETHOD(Float3, operator bs::Vector3), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("bs::Quaternion", asBEHAVE_CONSTRUCT,
           "void f(const Float4 &in vector)", asFUNCTION(QuaternionFloat4Proxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_CONSTRUCT,
           "void f(const bs::Quaternion &in quat)", asFUNCTION(Float4QuaternionProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Float4", asBEHAVE_CONSTRUCT,
           "void f(const bs::Color &in colour)", asFUNCTION(Float4ColorProxy),
           asCALL_CDECL_OBJFIRST) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("bs::Quaternion", "Float4 opImplConv() const",
           asFUNCTION(ConvertQuaternionToFloat4), asCALL_CDECL_OBJFIRST) < 0) {
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

    // Register common float types //
    if(!BindFloat2(engine))
        return false;

    if(!BindFloat3(engine))
        return false;

    if(!BindFloat4(engine))
        return false;


    if(!BindBSFConversions(engine))
        return false;

    if(!BindTypeDefs(engine))
        return false;

    return true;
}
