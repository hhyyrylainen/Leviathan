// ------------------------------------ //
#include "ScriptExecutor.h"

#include "Application/Application.h"
#include "Iterators/StringIterator.h"

#include "ScriptNotifiers.h"

#include <add_on/scriptstdstring/scriptstdstring.h>
#include <add_on/scriptarray/scriptarray.h>
#include <add_on/scriptmath/scriptmathcomplex.h>
#include <add_on/scriptmath/scriptmath.h>
#include <add_on/scriptarray/scriptarray.h>
#include <add_on/scriptstdstring/scriptstdstring.h>
#include <add_on/scriptgrid/scriptgrid.h>
#include <add_on/scripthandle/scripthandle.h>
#include <add_on/datetime/datetime.h>
#include <add_on/weakref/weakref.h>
#include <add_on/scripthelper/scripthelper.h>
#include <add_on/scriptdictionary/scriptdictionary.h>

#include "ScriptModule.h"

// Bindings
#include "Bindings/CommonEngineBind.h"
#include "Bindings/GuiScriptBind.h"
#include "Bindings/TypesBind.h"
#include "Bindings/EntityBind.h"

using namespace Leviathan;
// ------------------------------------ //


namespace Leviathan{

void ScriptMessageCallback(const asSMessageInfo *msg, void *param){

    if(msg->type == asMSGTYPE_WARNING){
        
        Logger::Get()->Write(std::string("[SCRIPT] [WARNING] ") + msg->section + " (" +
            std::to_string(msg->row) + ", " + std::to_string(msg->col) + ") : " +
            msg->message);
        
    } else if(msg->type == asMSGTYPE_INFORMATION){

        Logger::Get()->Write(std::string("[SCRIPT] [INFO] ") + msg->section + " (" +
            std::to_string(msg->row) + ", " + std::to_string(msg->col) + ") : " +
            msg->message);
        
    } else {

        Logger::Get()->Write(std::string("[SCRIPT] [ERROR] ") + msg->section + " (" +
            std::to_string(msg->row) + ", " + std::to_string(msg->col) + ") : " +
            msg->message);
    }
}

}



ScriptExecutor::ScriptExecutor() : engine(NULL), AllocatedScriptModules(){
    
    instance = this;

    // Initialize AngelScript //
    engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if(engine == NULL){

        Logger::Get()->Error("ScriptExecutor: Init: asCreateScriptEngine failed");
        Logger::Get()->Info("ScriptExecutor: tried to init angelscript version " +
            Convert::ToString(ANGELSCRIPT_VERSION));
        Logger::Get()->Write("Did you use a wrong angelscript version? copy header files to "
            "leviathan/Angelscript/include from your angelscript.zip");
        throw Exception("Failed to init angelscript");
    }

    // set callback to error report function //
    engine->SetMessageCallback(asFUNCTION(ScriptMessageCallback), 0, asCALL_CDECL);

    // math functions //
    RegisterScriptMath(engine);
    RegisterScriptMathComplex(engine);

    // register script string type //
    RegisterStdString(engine);
    RegisterScriptArray(engine, true);
    // register other script extensions //
    RegisterStdStringUtils(engine);

    
    RegisterScriptDateTime(engine);
    
    // register dictionary object //
    RegisterScriptDictionary(engine);
    
    // Register the grid addon //
    RegisterScriptGrid(engine);

    // Register reference handles //
    RegisterScriptHandle(engine);

    RegisterScriptWeakRef(engine);

    // use various binding functions //
    // register global functions and classes //
    if(!BindTypes(engine))
        throw Exception("BindTypes failed");    

    if(!BindEngineCommon(engine))
        throw Exception("BindEngineCommon failed");
        
    if(!BindGUI(engine))
        throw Exception("BindGUI failed");
    
    if(!BindEntity(engine))
        throw Exception("BindEntity failed");

    // Bind notifiers //
    if(!RegisterNotifiersWithAngelScript(engine)){
        // failed //
        LOG_ERROR("ScriptExecutor: Init: AngelScript: register Notifier types failed");
        throw Exception("Script bind failed");
    }

    // bind application specific //
    auto leviathanengine = Engine::GetEngine();

    if(leviathanengine){

        if(!leviathanengine->GetOwningApplication()->InitLoadCustomScriptTypes(engine)){

            LOG_ERROR("ScriptExecutor: Init: AngelScript: application register failed");
            throw Exception("Script bind failed");
        }        
    }

    ScanAngelScriptTypes();
}
ScriptExecutor::~ScriptExecutor(){

    {
        Lock lock(ModulesLock);
        auto end = AllocatedScriptModules.end();
        for(auto iter = AllocatedScriptModules.begin(); iter != end; ++iter){

            (*iter)->Release();
        }

        // release/delete all modules //
        AllocatedScriptModules.clear();
    }

    // release AngelScript //
    SAFE_RELEASE(engine);

    // release these to stop VLD complains //
    EngineTypeIDS.clear();
    EngineTypeIDSInverted.clear();

    instance = NULL;
    
    if(engine){

        engine->Release();
        engine = NULL;
    }
}

DLLEXPORT ScriptExecutor* Leviathan::ScriptExecutor::Get(){
    return instance;
}

ScriptExecutor* Leviathan::ScriptExecutor::instance = NULL;
// ------------------------------------ //
DLLEXPORT std::shared_ptr<VariableBlock> Leviathan::ScriptExecutor::RunSetUp(
    ScriptScript* scriptobject, ScriptRunningSetup* parameters)
{
    // Get the ScriptModule for the script //
    ScriptModule* scrptmodule = scriptobject->GetModule();
    
    if(!scrptmodule){
        
        // report error and exit //
        Logger::Get()->Error("ScriptExecutor: RunSetUp: trying to run an empty module");
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Use the actual running function //
    return RunSetUp(scrptmodule, parameters);
}

DLLEXPORT std::shared_ptr<VariableBlock> Leviathan::ScriptExecutor::RunSetUp(
    ScriptModule* scrptmodule, ScriptRunningSetup* parameters)
{
    
    if(!scrptmodule){
        Logger::Get()->Error("ScriptExecutor: RunSetUp: trying to run without a module");
        return NULL;
    }

    // Load the actual script //
    asIScriptModule* Module = scrptmodule->GetModule();
    
    if(!Module){
        
        // report error and exit //
        Logger::Get()->Error("ScriptExecutor: RunSetUp: cannot run an invalid script module: "+
            scrptmodule->GetInfoString());
        
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Get a function pointer to the start function //
    asIScriptFunction *func = NULL;

    // Get the entry function from the module //
    if(!parameters->FullDeclaration){

        func = Module->GetFunctionByName(parameters->Entryfunction.c_str());
    } else {

        func = Module->GetFunctionByDecl(parameters->Entryfunction.c_str());
    }

    if(!_CheckScriptFunctionPtr(func, parameters, scrptmodule)){
        
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Create a running context for the function //
    asIScriptContext* ScriptContext = _GetContextForExecution();

    if(!ScriptContext)
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));

    if(!_PrepareContextForPassingParameters(func, ScriptContext, parameters, scrptmodule)){

        _DoneWithContext(ScriptContext);
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }


    // Get the function parameter info //
    FunctionParameterInfo* paraminfo = scrptmodule->GetParamInfoForFunction(func);

    // Pass the parameters //
    if(!_SetScriptParameters(ScriptContext, parameters, scrptmodule, paraminfo)){

        // Failed passing the parameters //
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Run the script //

    // Could use a timeout here //
    int retcode = ScriptContext->Execute();

    // Get the return value //
    auto returnvalue = _GetScriptReturnedVariable(retcode, ScriptContext, parameters, func,
        scrptmodule, paraminfo);

    // Release the context //
    _DoneWithContext(ScriptContext);

    // Return the returned value //
    return returnvalue;
}

DLLEXPORT std::shared_ptr<VariableBlock> Leviathan::ScriptExecutor::RunSetUp(
    asIScriptFunction* function, ScriptRunningSetup* parameters)
{
    // Find a script module by the name //
    // TODO: find by AngelScript module pointer
    auto locked = GetModuleByAngelScriptName(function->GetModuleName()).lock();

    ScriptModule* scrptmodule = locked ? locked.get(): NULL;
    
    if(!scrptmodule){
        
        // report error and exit //
        if(parameters->ErrorOnNonExistingFunction)
            LOG_ERROR(std::string("ScriptExecutor: RunSetUp: the module is no longer "
                    "available: ") + function->GetModuleName());
        
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    if(!_CheckScriptFunctionPtr(function, parameters, scrptmodule)){

        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Create a running context for the function //
    asIScriptContext* ScriptContext = _GetContextForExecution();

    if(!ScriptContext)
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));

    if(!_PrepareContextForPassingParameters(function, ScriptContext, parameters, scrptmodule)){

        _DoneWithContext(ScriptContext);
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }


    // Get the function parameter info //
    FunctionParameterInfo* paraminfo = scrptmodule->GetParamInfoForFunction(function);

    // Pass the parameters //
    if(!_SetScriptParameters(ScriptContext, parameters, scrptmodule, paraminfo)){

        // Failed passing the parameters //
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Run the script //

    // Could use a timeout here //
    int retcode = ScriptContext->Execute();

    // Get the return value //
    auto returnvalue = _GetScriptReturnedVariable(retcode, ScriptContext, parameters, function,
        scrptmodule, paraminfo);

    // Release the context //
    _DoneWithContext(ScriptContext);

    // Return the returned value //
    return returnvalue;
}
// ------------------------------------ //
bool Leviathan::ScriptExecutor::_SetScriptParameters(asIScriptContext* ScriptContext,
    ScriptRunningSetup* parameters, ScriptModule* scrptmodule,
    FunctionParameterInfo* paraminfo)
{
    // Get the number of parameters expected //
    auto parameterc = static_cast<asUINT>(paraminfo->ParameterTypeIDS.size());

    // Start passing the parameters provided by the application //
    for(asUINT i = 0; i < parameterc; ++i){

        // no more parameters //
        if(i >= parameters->Parameters.size())
            break;

        // Try to pass the parameter //
        switch(paraminfo->MatchingDataBlockTypes[i]){
            case DATABLOCK_TYPE_INT:
            {
                int tmpparam = 0;

                if(!parameters->Parameters[i]->ConvertAndAssingToVariable(tmpparam)){

                    goto scriptexecutorpassparamsinvalidconversionparam;
                }

                ScriptContext->SetArgDWord(i, tmpparam);
            }
            break;
            case DATABLOCK_TYPE_FLOAT:
            {
                float tmpparam = 0;

                if(!parameters->Parameters[i]->ConvertAndAssingToVariable(tmpparam)){

                    goto scriptexecutorpassparamsinvalidconversionparam;
                }

                ScriptContext->SetArgFloat(i, tmpparam);
            }
            break;
            case DATABLOCK_TYPE_BOOL: case DATABLOCK_TYPE_CHAR:
            {
                char tmpparam = 0;

                if(!parameters->Parameters[i]->ConvertAndAssingToVariable(tmpparam)){

                    goto scriptexecutorpassparamsinvalidconversionparam;
                }

                ScriptContext->SetArgByte(i, tmpparam);
            }
            break;
            case DATABLOCK_TYPE_STRING:
            {

                std::string* varpointer = static_cast<std::string*>(
                    *parameters->Parameters[i]);

                ScriptContext->SetArgObject(i, varpointer);
                
            }
            break;
            case DATABLOCK_TYPE_WSTRING:
            {
                // save as a string that can be retrieved //
                // not done
                goto scriptexecutorpassparamsinvalidconversionparam;
            }
            break;
            case DATABLOCK_TYPE_VOIDPTR:
            {
                // we need to make sure that script object name matches engine object name //
                if(paraminfo->ParameterDeclarations[i] !=
                    parameters->Parameters[i]->GetName())
                {
                    // non matching pointer types //
                    LOG_ERROR("Mismatching ptr types, comparing " + 
                        paraminfo->ParameterDeclarations[i] + " to passed type of " +
                        parameters->Parameters[i]->GetName());
                    
                } else {
                    
                    // types match, we can pass in the raw pointer //
                    void* ptrtostuff = static_cast<void*>(*parameters->Parameters[i]);
                    ScriptContext->SetArgAddress(i, ptrtostuff);
                }
            }
            break;
            default:
                goto scriptexecutorpassparamsinvalidconversionparam;
        }

        continue;

scriptexecutorpassparamsinvalidconversionparam:


        Logger::Get()->Error("ScriptExecutor: RunScript: pass parameters failed func: "+
            parameters->Entryfunction+" param number: "+
            Convert::ToString(i)+" in: "+scrptmodule->GetInfoString());
        
        return false;
    }

    // It didn't fail //
    return true;
}

std::shared_ptr<VariableBlock> Leviathan::ScriptExecutor::_GetScriptReturnedVariable(
    int retcode, asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
    asIScriptFunction* func, ScriptModule* scrptmodule, FunctionParameterInfo* paraminfo)
{
    // Check the return type //
    if(retcode != asEXECUTION_FINISHED){
        // something went wrong //

        // The execution didn't finish as we had planned. Determine why.
        if(retcode == asEXECUTION_ABORTED){
            // code took too long //
        } else if(retcode == asEXECUTION_EXCEPTION){
            // script caused an exception //
            const asIScriptFunction* exceptionfunc = ScriptContext->GetExceptionFunction();

            int linenumber = ScriptContext->GetExceptionLineNumber();

            Logger::Get()->Error(std::string("[SCRIPT] [EXCEPTION] ") +
                ScriptContext->GetExceptionString() + ", function: " + func->GetDeclaration() +
                "\n\t in " + exceptionfunc->GetScriptSectionName() + "(" +
                Convert::ToString(linenumber) + ") " + scrptmodule->GetInfoString());

            PrintAdditionalExcept(ScriptContext);
        }
        
        return std::shared_ptr<VariableBlock>(new VariableBlock(-1));
    }

    // Successfully executed, try to fetch return value //
    if(paraminfo->ReturnTypeID != 0){
        // return type isn't void //
        switch(paraminfo->ReturnMatchingDataBlock){
            case DATABLOCK_TYPE_INT:
            {
                return std::shared_ptr<VariableBlock>(new VariableBlock(
                        new IntBlock(ScriptContext->GetReturnDWord())));
            }
            break;
            case DATABLOCK_TYPE_FLOAT:
            {
                return std::shared_ptr<VariableBlock>(new VariableBlock(
                        new FloatBlock(ScriptContext->GetReturnFloat())));
            }
            break;
            case DATABLOCK_TYPE_BOOL:
            {
                return std::shared_ptr<VariableBlock>(new VariableBlock(
                        new BoolBlock(ScriptContext->GetReturnByte() != 0)));
            }
            break;
            case DATABLOCK_TYPE_CHAR:
            {
                return std::shared_ptr<VariableBlock>(new VariableBlock(
                        new CharBlock(ScriptContext->GetReturnByte())));
            }
            break;
            case DATABLOCK_TYPE_STRING:
            {

                // TODO: check do we need to delete this
                
                std::string* varpointer = reinterpret_cast<std::string*>(
                    ScriptContext->GetReturnObject());

                if(varpointer){

                    return std::make_shared<VariableBlock>(new StringBlock(*varpointer));
                    
                } else {

                    return std::make_shared<VariableBlock>(new VoidPtrBlock((void*)nullptr));
                }
            }
            break;
        }

        Logger::Get()->Info("[SCRIPT] return type not supported "+
            paraminfo->ReturnTypeDeclaration);
        
        return std::shared_ptr<VariableBlock>(new VariableBlock(
                std::string("Return type not supported")));
    }

    // No return value //
    return NULL;
}

bool Leviathan::ScriptExecutor::_CheckScriptFunctionPtr(asIScriptFunction* func,
    ScriptRunningSetup* parameters, ScriptModule* scrptmodule)
{
    // Check is it NULL //
    if(func == NULL){
        // Set exists state //
        parameters->ScriptExisted = false;

        // Check should we print an error //
        if(parameters->PrintErrors && parameters->ErrorOnNonExistingFunction){
            
            LOG_ERROR("ScriptExecutor: RunScript: Could not find starting function: " +
                parameters->Entryfunction + " in: " + scrptmodule->GetInfoString());
            
            scrptmodule->PrintFunctionsInModule();
        }

        // Not valid //
        return false;
    }

    // Set exists state //
    parameters->ScriptExisted = true;

    return true;
}

bool Leviathan::ScriptExecutor::_PrepareContextForPassingParameters(asIScriptFunction* func,
    asIScriptContext* ScriptContext, ScriptRunningSetup* parameters, ScriptModule* scrptmodule)
{
    if(ScriptContext->Prepare(func) < 0){
        
        Logger::Get()->Error("ScriptExecutor: RunScript: prepare context failed, func: " +
                parameters->Entryfunction + " in: " + scrptmodule->GetInfoString());
        
        return false;
    }

    return true;
}

// ------------------------------------ //
asIScriptContext* Leviathan::ScriptExecutor::_GetContextForExecution(){

    // TODO: pool context and detect already acive context and push state and use that
    asIScriptContext* ScriptContext = engine->CreateContext();

    if(!ScriptContext){

        Logger::Get()->Error("ScriptExecutor: RunScript: Failed to create a context ");
        return NULL;
    }

    return ScriptContext;
}

void Leviathan::ScriptExecutor::_DoneWithContext(asIScriptContext* context){
    context->Release();
}
// ------------------------------------ //
DLLEXPORT std::weak_ptr<ScriptModule> Leviathan::ScriptExecutor::GetModule(const int &ID){
    // loop modules and return a ptr to matching id //
    Lock lock(ModulesLock);
    
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
        if(AllocatedScriptModules[i]->GetID() == ID)
            return AllocatedScriptModules[i];
    }
    
    return std::shared_ptr<ScriptModule>(NULL);
}

DLLEXPORT std::weak_ptr<ScriptModule> Leviathan::ScriptExecutor::GetModuleByAngelScriptName(
    const char* nameofmodule)
{
    // Find a matching name //
    std::string module(nameofmodule);

    Lock lock(ModulesLock);
    
    // TODO: check could this be checked by comparing pointers
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
        if(AllocatedScriptModules[i]->GetModuleName() == module)
            return AllocatedScriptModules[i];
    }

    return std::shared_ptr<ScriptModule>(NULL);
}
// ------------------------------------ //
DLLEXPORT std::weak_ptr<ScriptModule> Leviathan::ScriptExecutor::CreateNewModule(
    const std::string &name, const std::string &source, const int &modulesid
    /*= IDFactory::GetID()*/)
{
    // create new module to a smart pointer //
    auto tmpptr = std::make_shared<ScriptModule>(engine, name, modulesid, source);

    // add to vector and return //
    Lock lock(ModulesLock);
    AllocatedScriptModules.push_back(tmpptr);
    return tmpptr;
}

DLLEXPORT void Leviathan::ScriptExecutor::DeleteModule(ScriptModule* ptrtomatch){

    Lock lock(ModulesLock);
    
    // find module based on pointer and remove //
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
        if(AllocatedScriptModules[i].get() == ptrtomatch){

            AllocatedScriptModules[i]->Release();
            // remove //
            AllocatedScriptModules.erase(AllocatedScriptModules.begin()+i);
            return;
        }
    }
}

DLLEXPORT bool Leviathan::ScriptExecutor::DeleteModuleIfNoExternalReferences(int ID){

    Lock lock(ModulesLock);
    
    // Find based on the id //
    for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
        if(AllocatedScriptModules[i]->GetID() == ID){
            // Check reference count //
            if(AllocatedScriptModules[i].use_count() != 1){
                // Other references exist //
                return false;
            }

            AllocatedScriptModules[i]->Release();

            // remove //
            AllocatedScriptModules.erase(AllocatedScriptModules.begin()+i);
            return true;
        }
    }
    // Nothing found //
    return false;
}
// ------------------------------------ //
void Leviathan::ScriptExecutor::PrintAdditionalExcept(asIScriptContext *ctx){
    
    // Print callstack as additional information //
    Logger::Get()->Write("// ------------------ CallStack ------------------ //\n");
    
    // Loop the stack starting from the frame below the current function
    // (actually might be nice to print the top frame too)
    for(asUINT n = 0; n < ctx->GetCallstackSize(); n++){
        
        // Get the function object //
        const asIScriptFunction* function = ctx->GetFunction(n);
        
        // If the function doesn't exist this frame is used internally by the script engine //
        if(function){
            
            // Check function type //
            if(function->GetFuncType() == asFUNC_SCRIPT){
                
                // Print info about the script function //
                Logger::Get()->Write(std::string("\t> ") + function->GetScriptSectionName() +
                    ":" + Convert::ToString(ctx->GetLineNumber(n)) +
                    function->GetDeclaration() + ":" +
                    Convert::ToString(ctx->GetLineNumber(n)) + "\n");
                
            } else {
                // Info about the application functions //
                // The context is being reused by the application for a nested call
                Logger::Get()->Write(std::string("\t> {...Application...}: ")
                    +function->GetDeclaration()+"\n");
            }
        } else {
            // The context is being reused by the script engine for a nested call
            Logger::Get()->Write("\t> {...Script internal...}\n");
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptExecutor::ScanAngelScriptTypes(){
    // Skip if already registered //
    if(!EngineTypeIDS.empty()){

        return;
    }
    
    // put basic types //
    EngineTypeIDS.insert(std::make_pair(engine->GetTypeIdByDecl("int"), "int"));
    EngineTypeIDS.insert(std::make_pair(engine->GetTypeIdByDecl("float"), "float"));
    EngineTypeIDS.insert(std::make_pair(engine->GetTypeIdByDecl("bool"), "bool"));
    EngineTypeIDS.insert(std::make_pair(engine->GetTypeIdByDecl("string"), "string"));
    EngineTypeIDS.insert(std::make_pair(engine->GetTypeIdByDecl("void"), "void"));

    
    // call some callbacks //
    RegisterEngineCommon(engine, EngineTypeIDS);
    RegisterGUI(engine, EngineTypeIDS);
    RegisterTypes(engine, EngineTypeIDS);
    RegisterEntity(engine, EngineTypeIDS);
    
    auto leviathanengine = Engine::GetEngine();
    if(leviathanengine){
        
        leviathanengine->GetOwningApplication()->RegisterCustomScriptTypes(engine, EngineTypeIDS);
    }

    // Invert the current list, since it should be final //
    for(auto iter = EngineTypeIDS.begin(); iter != EngineTypeIDS.end(); ++iter){

        EngineTypeIDSInverted.insert(make_pair(iter->second, iter->first));
    }
}

DLLEXPORT int ScriptExecutor::GetAngelScriptTypeID(const std::string &typesname){

    auto iter = EngineTypeIDSInverted.find(typesname);

    if(iter != EngineTypeIDSInverted.end()){
        return iter->second;
    }
            
    return -1;
}

// TODO: switch to boost bidirectional map
std::map<std::string, int> Leviathan::ScriptExecutor::EngineTypeIDSInverted;

std::map<int, std::string> Leviathan::ScriptExecutor::EngineTypeIDS;

