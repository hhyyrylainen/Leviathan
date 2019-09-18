#include "../PartialEngine.h"

#include "Handlers/IDFactory.h"
#include "Script/Bindings/BindHelpers.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


TEST_CASE("Basic script running", "[script]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int TestFunction(int Val1, int Val2){\n"
        "// do some time consuming stuff //\n"
        "Val1 *= Val1+Val2 % 15;\n"
        "Val2 /= Val2-Val1-Val2*25+2;\n"
        "return 42;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestFunction");

    auto returned = exec.RunScript<int>(mod, ssetup, 252134, 25552);

    // Check did it exist //
    CHECK(ssetup.ScriptExisted == true);
    REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(returned.Value == 42);

    mod->DeleteThisModule();
}

TEST_CASE("Basic new script running", "[script]")
{

    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    SECTION("Typeid checks")
    {

        CHECK(exec.ResolveStringToASID("string") ==
              AngelScriptTypeIDResolver<std::string>::Get(&exec));
    }

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int TestFunction(int Val1, float Val2, const string &in msg){\n"
        "return int(Val1 * Val2) + msg.length();\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("TestFunction").SetUseFullDeclaration(false);

    ScriptRunResult<int> result =
        exec.RunScript<int>(mod, ssetup, 2, 5, std::string("my string5"));

    REQUIRE(result.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(result.Value == 20);

    // TODO: add tests for type conversions once they are done

    mod->DeleteThisModule();
}

int MyRefCountedParam = 0;

class TestMyRefCounted : public ReferenceCounted {
public:
    REFERENCE_COUNTED_PTR_TYPE(TestMyRefCounted);

protected:
    // Copy this comment to any protected constructors that are meant to be
    // accessed through this:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    TestMyRefCounted()
    {
        ++MyRefCountedParam;
    }

    ~TestMyRefCounted()
    {

        --MyRefCountedParam;
    }
};

bool RegisterTestMyRefCounted(asIScriptEngine* engine)
{

    ANGELSCRIPT_REGISTER_REF_TYPE("TestMyRefCounted", TestMyRefCounted);
    return true;
}


TEST_CASE("Running scripts that don't take all ref counted parameters", "[script]")
{

    TestMyRefCounted::pointer ourObj = ReferenceCounted::MakeShared<TestMyRefCounted>();

    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    REQUIRE(RegisterTestMyRefCounted(exec.GetASEngine()));

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int TestFunction(int param1, TestMyRefCounted@ obj){\n"
        "if(obj !is null)\n"
        "return 2 * param1;\n"
        "else\n"
        "return 0;\n"
        "}\n"
        "int Test2(int param1){\n"
        "return param1;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    CHECK(ourObj->GetRefCount() == 1);

    SECTION("First basic run")
    {

        ScriptRunningSetup ssetup;
        ssetup.SetEntrypoint("TestFunction").SetUseFullDeclaration(false)
            //.SetPrintErrors(false)
            ;

        auto returned = exec.RunScript<int>(mod, ssetup, 2, ourObj.get());
        CHECK(ourObj->GetRefCount() == 1);

        // check did it run //
        CHECK(ssetup.ScriptExisted == true);
        REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

        CHECK(returned.Value == 4);
    }

    SECTION("Second run with ignored object handle")
    {

        ScriptRunningSetup ssetup;
        ssetup.SetEntrypoint("Test2").SetUseFullDeclaration(false)
            //.SetPrintErrors(false)
            ;

        auto returned = exec.RunScript<int>(mod, ssetup, 2, ourObj.get());
        CHECK(ourObj->GetRefCount() == 1);

        // check did it run //
        CHECK(ssetup.ScriptExisted == true);
        REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

        CHECK(returned.Value == 2);
    }

    CHECK(ourObj->GetRefCount() == 1);

    mod->DeleteThisModule();
}

TEST_CASE("Script global handles are released", "[script][engine]")
{

    TestMyRefCounted::pointer ourObj = ReferenceCounted::MakeShared<TestMyRefCounted>();

    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    REQUIRE(RegisterTestMyRefCounted(exec.GetASEngine()));

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "TestMyRefCounted@ GlobalVar;\n"
        "int TestFunction(TestMyRefCounted@ obj){\n"
        "@GlobalVar = obj;\n"
        "return 8;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    CHECK(ourObj->GetRefCount() == 1);

    SECTION("First basic run")
    {
        ScriptRunningSetup ssetup;
        ssetup.SetEntrypoint("TestFunction");

        auto returned = exec.RunScript<int>(mod, ssetup, ourObj.get());

        // check did it run //
        CHECK(ssetup.ScriptExisted == true);
        REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

        CHECK(returned.Value == 8);
    }

    CHECK(ourObj->GetRefCount() == 2);

    mod->DeleteThisModule();

    CHECK(ourObj->GetRefCount() == 1);
}

TEST_CASE("Passing and returning objects from RunScript", "[script]")
{

    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "uint64 CountLength(const string &in str){\n"
        "return str.length();\n"
        "}\n"
        "string GetStr(float val){\n"
        "return formatFloat(val);\n"
        "}\n"
        "GenericEvent@ GetEvent(){\n"
        "return GenericEvent(\"some event\");\n"
        "}\n"
        "GenericEvent@ PassEvent(GenericEvent@ event){\n"
        "return event;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("CountLength").SetUseFullDeclaration(false);

    auto returned =
        exec.RunScript<uint64_t>(mod, ssetup, std::string("string to count characters in"));

    REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(returned.Value == 29);

    std::string str2 = "other text that is different length";
    returned = exec.RunScript<uint64_t>(mod, ssetup, str2);

    REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(returned.Value == 35);


    ssetup.SetEntrypoint("GetStr");

    auto returned2 = exec.RunScript<std::string>(mod, ssetup, 2.0);

    REQUIRE(returned2.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(returned2.Value == "2");

    GenericEvent* gotEvent;

    {
        ssetup.SetEntrypoint("GetEvent");

        auto returned3 = exec.RunScript<GenericEvent*>(mod, ssetup);

        REQUIRE(returned3.Result == SCRIPT_RUN_RESULT::Success);

        CHECK(returned3.Value != nullptr);

        gotEvent = returned3.Value;
        CHECK(gotEvent->GetRefCount() == 1);
        gotEvent->AddRef();
        CHECK(gotEvent->GetRefCount() == 2);
    }

    CHECK(gotEvent->GetRefCount() == 1);

    // Check type str //
    CHECK(gotEvent->GetType() == "some event");


    gotEvent->Release();
    gotEvent = new GenericEvent("test event2");
    CHECK(gotEvent->GetRefCount() == 1);

    {
        ssetup.SetEntrypoint("PassEvent");

        auto returned3 = exec.RunScript<GenericEvent*>(mod, ssetup, gotEvent);

        REQUIRE(returned3.Result == SCRIPT_RUN_RESULT::Success);

        CHECK(returned3.Value != nullptr);

        CHECK(gotEvent->GetRefCount() == 2);

        REQUIRE(gotEvent == returned3.Value);
    }

    CHECK(gotEvent->GetRefCount() == 1);

    // Check type str //
    CHECK(gotEvent->GetType() == "test event2");
    gotEvent->Release();

    mod->DeleteThisModule();
}

TEST_CASE("Ignored returned object doesn't leak", "[script]")
{
    PartialEngine<false> engine;
    // Overwrite the logger in engine
    TestLogRequireError requireError;
    requireError.WarningsCountAsErrors = true;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "GenericEvent@ PassEvent(GenericEvent@ event){\n"
        "return event;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("PassEvent").SetUseFullDeclaration(false);

    GenericEvent* event = new GenericEvent("Test1322>");
    CHECK(event->GetRefCount() == 1);

    // Causes errors as this has to release
    auto returned = exec.RunScript<void>(mod, ssetup, event);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(event->GetRefCount() == 1);
    event->Release();


    mod->DeleteThisModule();
}


TEST_CASE("Creating events in scripts", "[script][event]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int TestFunction(){\n"
        "    GenericEvent@ event = GenericEvent(\"TestEvent\");\n"
        "\n"
        "    NamedVars@ tempvalues = event.GetNamedVars();\n"
        "    if(tempvalues is null)\n"
        "         return 0;\n"
        "    // Next four lines aren't good style //\n"
        "    NamedVars@ otherVals = @event.GetNamedVars();\n"
        "    if(@tempvalues !is @otherVals)\n"
        "         return 0;\n"
        "    if(@tempvalues is null)\n"
        "         return 0;\n"
        "    if(tempvalues !is otherVals)\n"
        "         return 0;\n"
        "\n"
        "    return 12;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("TestFunction");

    auto returned = exec.RunScript<int>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(returned.Value == 12);
}


TEST_CASE("Documentation samples compile", "[script]")
{

    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "NamedVars@ values = NamedVars();\n"
        "\n"
        "NamedVars@ GetVars(){\n"
        "\n"
        "    return values;\n"
        "}\n"
        "\n"
        "void Sample1(){\n"
        "\n"
        "    if(values is null)\n"
        "        return;\n"
        "\n"
        "    NamedVars@ other;\n"
        "\n"
        "    if(values is other)\n"
        "        return;\n"
        "\n"
        "    @other = @values;\n"
        "\n"
        "    NamedVars@ thirdVariable = GetVars();\n"
        "\n"
        "    @other = null;\n"
        "}\n"
        "\n"
        "NamedVars@ values2 = @NamedVars();\n"
        "\n"
        "void Sample2(){\n"
        "\n"
        "    if(@values2 is null)\n"
        "        return;\n"
        "\n"
        "    NamedVars@ other;\n"
        "\n"
        "    if(@values2 is @other)\n"
        "        return;\n"
        "\n"
        "    @other = @values;\n"
        "\n"
        "    NamedVars@ thirdVariable = GetVars();\n"
        "\n"
        "    @other = null;\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("Sample1");

    auto result = exec.RunScript<void>(mod, ssetup);

    CHECK(result.Result == SCRIPT_RUN_RESULT::Success);

    ssetup.SetEntrypoint("Sample2");

    result = exec.RunScript<void>(mod, ssetup);

    CHECK(result.Result == SCRIPT_RUN_RESULT::Success);
}

TEST_CASE("Script bound random (not tested elsewhere) functions work correctly", "[script]")
{
    PartialEngine<false> engine;
    engine.InitRandomForTest();

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();
    CHECK(mod->AddScriptSegmentFromFile("Data/Scripts/tests/StandardFunctionsTest.as"));

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestFunction1");

    auto returned = exec.RunScript<bool>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned.Value == true);

    ssetup.SetEntrypoint("TestFunction2");

    returned = exec.RunScript<bool>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned.Value == true);
}

TEST_CASE("BSF bound functions work correctly", "[script][bsf]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();
    CHECK(mod->AddScriptSegmentFromFile("Data/Scripts/tests/BSFFunctionsTest.as"));

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestAngleConversions");

    auto returned = exec.RunScript<bool>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned.Value == true);
}



asIScriptFunction* FactoryFunc = nullptr;
ScriptExecutor* ScriptExec = nullptr;

void TestPassFactoryIn(asIScriptFunction* func)
{
    FactoryFunc = func;
}

asIScriptObject* GetObject(int i)
{
    ScriptRunningSetup setup;

    auto result = ScriptExec->RunScript<asIScriptObject*>(FactoryFunc, nullptr, setup, i);

    REQUIRE(result.Result == SCRIPT_RUN_RESULT::Success);
    REQUIRE(result.Value != nullptr);

    result.Value->AddRef();
    return result.Value;
}

TEST_CASE("Passing factory to application and getting results and returning script object",
    "[script]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;
    ScriptExec = &exec;

    // Register our custom stuff //
    asIScriptEngine* as = exec.GetASEngine();

    REQUIRE(as->RegisterInterface("MyCoolInterface") >= 0);
    REQUIRE(as->RegisterInterfaceMethod("MyCoolInterface", "int GetValue()") >= 0);

    REQUIRE(as->RegisterFuncdef("MyCoolInterface@ CoolFactoryFunc(int i)") >= 0);

    REQUIRE(as->RegisterGlobalFunction("void TestPassFactoryIn(CoolFactoryFunc@ func)",
                asFUNCTION(TestPassFactoryIn), asCALL_CDECL) >= 0);

    REQUIRE(as->RegisterGlobalFunction("MyCoolInterface@ GetObject(int i)",
                asFUNCTION(GetObject), asCALL_CDECL) >= 0);


    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "class ImplClass : MyCoolInterface{\n"
        "ImplClass(int i){\n"
        "value = i;\n"
        "}\n"
        "int GetValue(){\n"
        "return value;\n"
        "}\n"
        "int value;\n"
        "}\n"
        "MyCoolInterface@ Factory(int i){\n"
        "return ImplClass(i);\n"
        "}\n"
        "MyCoolInterface@ TestFunction(){\n"
        "TestPassFactoryIn(@Factory);\n"
        "return GetObject(42);\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestFunction");

    auto returned = exec.RunScript<asIScriptObject*>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(returned.Value != nullptr);

    // Get the value from it //
    asIScriptFunction* objFunc = returned.Value->GetObjectType()->GetMethodByName("GetValue");

    REQUIRE(objFunc);

    // Not needed but done to test that this isn't used
    ssetup.SetEntrypoint("");

    auto objValueResult = exec.RunScriptMethod<int>(ssetup, objFunc, returned.Value);

    REQUIRE(objValueResult.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(objValueResult.Value == 42);

    if(FactoryFunc)
        FactoryFunc->Release();
    FactoryFunc = nullptr;
    ScriptExec = nullptr;
}

TEST_CASE("Pass by value objects to scripts work", "[script]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "Float3 PassFloat3(Float3 value){\n"
        "return value + Float3(1, 0, 2);\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("PassFloat3");

    // Causes errors as this has to release
    auto returned = exec.RunScript<Float3>(mod, ssetup, Float3(1, 2, 3));

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned.Value == Float3(2, 2, 5));


    mod->DeleteThisModule();
}


// asIScriptObject* CurrentASObject = nullptr;
// asUINT CurrentASObjectID = 0;
// TEST_CASE("Passing and returning variable argument type", "[script]")


#ifdef ANGELSCRIPT_HAS_TRANSLATE_CALLBACK

void ThrowStuff()
{

    throw std::runtime_error("My custom exception message");
}

TEST_CASE("Script exception errors report std::exception derived message", "[script]")
{
    PartialEngine<false> engine;
    TestLogMatchMessagesRegex log;
    log.MessagesToDetect.push_back({ReporterMatchMessagesRegex::MessageToLookFor(
        std::regex(R"(.*My custom exception message[^]*)"))});

    IDFactory ids;
    ScriptExecutor exec;

    REQUIRE(exec.GetASEngine()->RegisterGlobalFunction(
                "void ThrowStuff()", asFUNCTION(ThrowStuff), asCALL_CDECL) >= 0);

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "void TestFunction(){\n"
        "    ThrowStuff();\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestFunction");

    auto returned = exec.RunScript<int>(mod, ssetup);

    CHECK(returned.Result != SCRIPT_RUN_RESULT::Success);
    log.MessagesToDetect[0].CheckAndResetCountIsOne();
}

#endif // ANGELSCRIPT_HAS_TRANSLATE_CALLBACK

TEST_CASE("Script anonymous delegates don't leak GC objects", "[script]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();
    CHECK(mod->AddScriptSegmentFromFile("Data/Scripts/tests/AnonymousDelegateLeak.as"));

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("RunTest");

    auto returned = exec.RunScript<void>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
}

TEST_CASE("Script passing uint16_t works", "[script]")
{
    PartialEngine<false> engine;
    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int32 TestFunction(uint16 param){\n"
        "    return int32(param);\n"
        "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestFunction");

    const uint16_t value = 55;

    auto returned = exec.RunScript<int32_t>(mod, ssetup, value);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned.Value == value);
}
