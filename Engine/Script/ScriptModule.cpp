// ------------------------------------ //
#include "ScriptModule.h"

#include "Common/StringOperations.h"
#include "Events/CallableObject.h"
#include "Events/Event.h"
#include "FileSystem.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "Iterators/StringIterator.h"
#include "ScriptExecutor.h"
#include "add_on/serializer/serializer.h"

#include <boost/filesystem.hpp>
using namespace Leviathan;
// ------------------------------------ //
ScriptModule::ScriptModule(
    asIScriptEngine* engine, const std::string& name, int id, const std::string& source) :
    Name(name),
    Source(source), ID(id), ScriptBuilder(new CScriptBuilder())
{
    {
        Lock lock(ModuleBuildingMutex);
        ModuleName = std::string(source + "_;" + Convert::ToString<int>(LatestAssigned));
        LatestAssigned++;
    }

    // module will always be started //
    ScriptBuilder->StartNewModule(engine, ModuleName.c_str());

    // setup include resolver //
    ScriptBuilder->SetIncludeCallback(ScriptModuleIncludeCallback, this);

    // Apply the default mask //
    ScriptBuilder->GetModule()->SetAccessMask(AccessMask);

    ListenerDataBuilt = false;
}

ScriptModule::~ScriptModule() {}

DLLEXPORT void Leviathan::ScriptModule::Release()
{
    GUARD_LOCK();

    // Leave from the bridge //
    if(ArgsBridge)
        ArgsBridge->LeaveModule();


#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

    _StopFileMonitoring(guard);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES

    // We'll need to destroy the module from the engine //
    ASModule = NULL;
    ScriptExecutor::Get()->GetASEngine()->DiscardModule(ModuleName.c_str());

    // And then delete the builder //
    SAFE_DELETE(ScriptBuilder);
    ListenerDataBuilt = false;
}

int Leviathan::ScriptModule::LatestAssigned = 0;

const std::map<std::string, int> Leviathan::ScriptModule::ListenerNameType = {
    {LISTENERNAME_ONSHOW, LISTENERVALUE_ONSHOW}, {LISTENERNAME_ONHIDE, LISTENERVALUE_ONHIDE},
    {LISTENERNAME_ONLISTENUPDATE, LISTENERVALUE_ONSHOW},
    {LISTENERNAME_ONCLICK, LISTENERVALUE_ONCLICK},
    {LISTENERNAME_ONVALUECHANGE, LISTENERVALUE_ONVALUECHANGE},
    {LISTENERNAME_ONINIT, LISTENERVALUE_ONINIT},
    {LISTENERNAME_ONRELEASE, LISTENERVALUE_ONRELEASE},
    {LISTENERNAME_ONSUBMIT, LISTENERVALUE_ONSUBMIT},
    {LISTENERNAME_ONTICK, LISTENERVALUE_ONTICK},
    {LISTENERNAME_ONCLOSECLICKED, LISTENERVALUE_ONCLOSECLICKED},
    {LISTENERNAME_LISTSELECTIONACCEPTED, LISTENERVALUE_LISTSELECTIONACCEPTED}};

// ------------------------------------ //
DLLEXPORT asIScriptModule* Leviathan::ScriptModule::GetModule(Lock& guard)
{

    // we need to check build state //
    if(ScriptState == SCRIPTBUILDSTATE_READYTOBUILD) {

        _BuildTheModule(guard);

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

        _StartMonitoringFiles(guard);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES

        if(ScriptState != SCRIPTBUILDSTATE_BUILT) {

            return NULL;
        }

    } else if(ScriptState == SCRIPTBUILDSTATE_FAILED ||
              ScriptState == SCRIPTBUILDSTATE_DISCARDED) {

        return NULL;
    }

    // Return the saved pointer or fetch the pointer //
    if(!ASModule) {

        // Get module from the engine //
        ASModule = ScriptExecutor::Get()->GetASEngine()->GetModule(
            ModuleName.c_str(), asGM_ONLY_IF_EXISTS);

        if(!ASModule) {

            // The module is invalid //
            LOG_ERROR("ScriptModule: GetModule: module is no longer anywhere "
                      "to be found in the AS engine");

            ScriptState = SCRIPTBUILDSTATE_DISCARDED;
            return NULL;
        }
    }

    return ASModule;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<ScriptScript> Leviathan::ScriptModule::GetScriptInstance()
{

    return std::make_shared<ScriptScript>(ID, ScriptExecutor::Get()->GetModule(ID));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::DoesListenersContainSpecificListener(
    const std::string& listenername, const std::string* generictype /*= NULL*/)
{
    GUARD_LOCK();
    // find from the map //
    auto itr = _GetIteratorOfListener(guard, listenername, generictype);

    if(itr != FoundListenerFunctions.end()) {

        // it exists //
        return true;
    }
    // no matching listener //
    return false;
}

DLLEXPORT void Leviathan::ScriptModule::GetListOfListeners(
    std::vector<std::shared_ptr<ValidListenerData>>& receiver)
{
    GUARD_LOCK();
    // build info if not built //
    if(!ListenerDataBuilt)
        _BuildListenerList(guard);
    // reserve space to be more efficient //
    receiver.reserve(FoundListenerFunctions.size());


    for(auto iter = FoundListenerFunctions.begin(); iter != FoundListenerFunctions.end();
        ++iter) {
        receiver.push_back(iter->second);
    }
}

DLLEXPORT std::string Leviathan::ScriptModule::GetListeningFunctionName(
    const std::string& listenername, const std::string* generictype /*= NULL*/)
{
    GUARD_LOCK();

    // Call search function and check if it found anything //
    auto itr = _GetIteratorOfListener(guard, listenername, generictype);


    if(itr != FoundListenerFunctions.end()) {
        // Get name from pointer //
        return std::string(itr->second->FuncPtr->GetName());
    }

    // Nothing found //
    return "";
}
// ------------------------------------ //
DLLEXPORT std::string Leviathan::ScriptModule::GetInfoString()
{

    return "ScriptModule(" + Convert::ToString(ID) + ") " + Name + ", from: " + Source;
}

DLLEXPORT void Leviathan::ScriptModule::DeleteThisModule()
{

    // Tell script interface to unload this //
    ScriptExecutor::Get()->DeleteModule(this);
}
// ------------------------------------ //
void Leviathan::ScriptModule::_BuildListenerList(Lock& guard)
{

    // We need to find functions with metadata specifying which listener it is //
    asIScriptModule* mod = GetModule(guard);

    if(!mod) {
        // Module failed to build //
        return;
    }

    asUINT funccount = mod->GetFunctionCount();
    // Loop all and check the ones with promising names //
    for(asUINT i = 0; i < funccount; i++) {

        asIScriptFunction* tmpfunc = mod->GetFunctionByIndex(i);


        // Get metadata for this and process it //
        _ProcessMetadataForFunc(tmpfunc, mod);
    }

    // Data is now built //
    ListenerDataBuilt = true;
}

void Leviathan::ScriptModule::_ProcessMetadataForFunc(
    asIScriptFunction* func, asIScriptModule* mod)
{
    // Start of by getting metadata string //
    const auto& metaEntries = ScriptBuilder->GetMetadataForFunc(func);

    if(metaEntries.empty())
        return;

    std::string meta = metaEntries.front();
    StringOperations::RemovePreceedingTrailingSpaces(meta);

    if(meta.size() < 1) {
        // Too short for anything //
        return;
    }

    // Do all kinds of checks on the metadata //
    if(meta[0] == '@') {
        // Some specific special function, check which //

        // We need some iterating here //
        StringIterator itr(meta);

        // need to skip first character don't want @ to be in the name //
        itr.MoveToNext();

        // get until assignment //
        auto metaname =
            itr.GetUntilEqualityAssignment<std::string>(EQUALITYCHARACTER_TYPE_EQUALITY);

        // check name //
        if(*metaname == "Listener") {
            // it's a listener function //

            // get string in quotes to find out what it is //
            auto listenername = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

            std::string localname = *listenername;

            // if it is generic listener we need to get it's type //
            if(*listenername == "Generic") {

                auto generictype = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

                if(generictype->size() == 0) {

                    LOG_WARNING("ScriptModule: ProcessMetadata: Generic listener has "
                                "no type defined (expected declaration like \""
                                "[@Listener=\"Generic\", @Type=\"ScoreUpdated\"]\"");
                    return;
                }

                // mash together a name //
                std::string mangledname = "Generic:" + *generictype + ";";
                auto restofmeta = itr.GetUntilEnd<std::string>();

                FoundListenerFunctions[mangledname] =
                    std::shared_ptr<ValidListenerData>(new ValidListenerData(func,
                        listenername.release(), restofmeta.release(), generictype.release()));

                return;
            }


            // make iterator to skip spaces //
            itr.SkipWhiteSpace();

            // match listener's name with OnFunction type //
            auto positerator = ListenerNameType.find(*listenername);

            if(positerator != ListenerNameType.end()) {
                // found a match, store info //
                auto restofmeta = itr.GetUntilEnd<std::string>();
                FoundListenerFunctions[localname] = std::shared_ptr<ValidListenerData>(
                    new ValidListenerData(func, listenername.release(), restofmeta.release()));

                return;
            }
            // we shouldn't have gotten here, error //
            Logger::Get()->Error(
                "ScriptModule: ProcessMetadata: invalid Listener name, " + *listenername);
        }


    } else {

        LOG_WARNING("ScriptModule: unknown metadata: " + meta);
    }
}


std::map<std::string, std::shared_ptr<ValidListenerData>>::iterator
    Leviathan::ScriptModule::_GetIteratorOfListener(Lock& guard,
        const std::string& listenername, const std::string* generictype /*= NULL*/)
{
    // build info if not built //
    if(!ListenerDataBuilt)
        _BuildListenerList(guard);

    // find from the map //
    auto itr = FoundListenerFunctions.end();

    // different implementations for generic finding, because the name isn't usable as is //
    if(!generictype) {
        // default find is fine for known types //
        itr = FoundListenerFunctions.find(listenername);

    } else {
        // we need to find the right one by comparing strings inside the objects //
        for(auto iter = FoundListenerFunctions.begin(); iter != FoundListenerFunctions.end();
            ++iter) {
            // strings are sorted alphabetically so we can skip until "Generic" //
            if(iter->first.at(0) != 'G')
                continue;
            // check for matching generic name //
            if(*iter->second->GenericTypeName == *generictype) {
                // found right one, copy iterator and return //
                itr = iter;
                break;
            }
        }
    }
    // return whatever we might have found at this point //
    return itr;
}

DLLEXPORT void Leviathan::ScriptModule::PrintFunctionsInModule()
{
    GUARD_LOCK();

    // list consoles' global variables //
    Logger::Get()->Info(Name + " instance functions: ");

    // List the user functions in the module
    asIScriptModule* mod = GetModule(guard);

    const asUINT FuncCount = mod->GetFunctionCount();

    for(asUINT n = 0; n < FuncCount; n++) {
        // get function //
        asIScriptFunction* func = mod->GetFunctionByIndex(n);
        // print the function //
        Logger::Get()->Write(
            std::string("> ") + func->GetName() + "(" + func->GetDeclaration() + ")");
    }

    Logger::Get()->Write("[END]");
}

DLLEXPORT int Leviathan::ScriptModule::ScriptModuleIncludeCallback(
    const char* include, const char* from, CScriptBuilder* builder, void* userParam)
{
    std::string file(include);
    std::string infile(from);

    ScriptModule* module = reinterpret_cast<ScriptModule*>(userParam);
    // The module has to be locked during this call

    // Resolve it to an usable path //
    const auto resolved = ResolvePathToScriptFile(file, infile);

    if(resolved.empty() || !boost::filesystem::exists(resolved)) {

        Logger::Get()->Error("ScriptModule: IncludeCallback: couldn't resolve include "
                             "(even with full search), file: " +
                             file + " included in: " + infile + ", while compiling " +
                             module->GetInfoString());

        // Including failed
        return -1;
    }

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

    module->_AddFileToMonitorIfNotAlready(resolved);
#endif // SCRIPTMODULE_LISTENFORFILECHANGES

    return builder->AddSectionFromFile(resolved.c_str());
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ScriptModule::GetScriptSegmentCount() const
{
    return ScriptSourceSegments.size();
}

DLLEXPORT std::shared_ptr<ScriptSourceFileData> Leviathan::ScriptModule::GetScriptSegment(
    size_t index) const
{
    if(index >= ScriptSourceSegments.size())
        return NULL;

    return ScriptSourceSegments[index];
}

DLLEXPORT bool Leviathan::ScriptModule::AddScriptSegment(
    std::shared_ptr<ScriptSourceFileData> data)
{
    GUARD_LOCK();
    // Check is it already there //
    for(size_t i = 0; i < ScriptSourceSegments.size(); i++) {

        if(ScriptSourceSegments[i]->SourceFile == data->SourceFile &&
            (abs(ScriptSourceSegments[i]->StartLine - data->StartLine) <= 2)) {

            return false;
        }
    }

    ScriptSourceSegments.push_back(data);

    // Needs to be built next //
    ScriptState = SCRIPTBUILDSTATE_READYTOBUILD;
    return true;
}

DLLEXPORT bool Leviathan::ScriptModule::AddScriptSegmentFromFile(const std::string& file)
{
    GUARD_LOCK();

    std::string expanded;

    try {
        expanded = boost::filesystem::canonical(file).generic_string();
    } catch(const boost::filesystem::filesystem_error&) {
        // Doesn't exist //
        return false;
    }

    // Check is it already there //
    for(size_t i = 0; i < ScriptSourceSegments.size(); i++) {

        if(ScriptSourceSegments[i]->SourceFile == file) {

            return false;
        }
    }

    // Load the source code from the file //
    std::string scriptdata;
    FileSystem::ReadFileEntirely(expanded, scriptdata);

    ScriptSourceSegments.push_back(
        std::make_shared<ScriptSourceFileData>(expanded, 1, scriptdata));

    // Needs to be built next //
    ScriptState = SCRIPTBUILDSTATE_READYTOBUILD;
    return true;
}

DLLEXPORT const std::string& Leviathan::ScriptModule::GetModuleName() const
{
    return ModuleName;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptModule::SetAsInvalid()
{
    GUARD_LOCK();

    ScriptState = SCRIPTBUILDSTATE_DISCARDED;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::ReLoadModuleCode()
{
    GUARD_LOCK();

    // The module must be still valid //
    if(ScriptState == SCRIPTBUILDSTATE_DISCARDED ||
        (!GetModule() && ScriptState != SCRIPTBUILDSTATE_FAILED))
        return false;


    Logger::Get()->Info("Reloading " + GetInfoString());

    if(ArgsBridge) {

        // Do a OnRelease event call //
        const std::string& listenername =
            CallableObject::GetListenerNameFromType(EVENT_TYPE_RELEASE);

        // check does the script contain right listeners //
        if(DoesListenersContainSpecificListener(listenername)) {

            // Get the parameters //
            auto sargs = ArgsBridge->GetProvider()->GetParametersForRelease();
            // sargs.Parameters need to be somehow translated to the new style
            DEBUG_BREAK;
            sargs->SetEntrypoint(GetListeningFunctionName(listenername));

            // Run the script //
            auto result = ScriptExecutor::Get()->RunScript<void>(shared_from_this(), *sargs);

            if(result.Result != SCRIPT_RUN_RESULT::Success) {

                LOG_ERROR("ScriptModule: failed to run auto release, skipping reload");
                return false;
            } else {

                Logger::Get()->Info("ScriptModule: ran auto release");
            }
        }
    }


    // Store the old values //
    // CSerializer backup;
    // backup.Store(ASModule);

    // Discard the old module //
    ASModule = NULL;
    if(GetModule())
        ScriptExecutor::Get()->GetASEngine()->DiscardModule(ModuleName.c_str());

    // The builder must be created again //
    SAFE_DELETE(ScriptBuilder);

    // Setup the new builder //
    ScriptBuilder = new CScriptBuilder();

    asIScriptEngine* engine = ScriptExecutor::Get()->GetASEngine();

    // Add 'R' to the module name to mark reload //
    ModuleName += "R";

    ScriptBuilder->StartNewModule(engine, ModuleName.c_str());
    ScriptBuilder->SetIncludeCallback(ScriptModuleIncludeCallback, this);
    ListenerDataBuilt = false;

    // Build the module //
    _BuildTheModule(guard);

    if(ScriptState != SCRIPTBUILDSTATE_BUILT || !GetModule()) {

        // Failed to build //
        return false;
    }

    // Restore the data //
    // backup.Restore(GetModule());

    // Logger::Get()->Info("Successfully restored "+GetInfoStd::String());

    if(ArgsBridge) {

        // Do a OnRelease event call //
        const std::string& listenername =
            CallableObject::GetListenerNameFromType(EVENT_TYPE_INIT);

        // check does the script contain right listeners //
        if(DoesListenersContainSpecificListener(listenername)) {

            // Get the parameters //
            auto sargs = ArgsBridge->GetProvider()->GetParametersForInit();
            // sargs.Parameters need to be somehow translated to the new style
            DEBUG_BREAK;
            sargs->SetEntrypoint(GetListeningFunctionName(listenername));


            // Run the script //
            auto result = ScriptExecutor::Get()->RunScript<void>(shared_from_this(), *sargs);

            if(result.Result != SCRIPT_RUN_RESULT::Success) {

                LOG_ERROR("ScriptModule: failed to run auto init after restore");
                // We don't want to signal failure as we have already overwritten our old stuff
            } else {

                Logger::Get()->Info("ScriptModule: ran auto init after restore");
            }
        }
    }

    // Succeeded //
    return true;
}

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

void Leviathan::ScriptModule::_StartMonitoringFiles(Lock& guard)
{

    // First add all the known source files //
    auto end = ScriptSourceSegments.end();
    for(auto iter = ScriptSourceSegments.begin(); iter != end; ++iter) {


        _AddFileToMonitorIfNotAlready((*iter)->SourceFile);
    }

    // The others should already have been included //
    // Create listeners and we are good to go //
    auto tmphandler = ResourceRefreshHandler::Get();

    if(!tmphandler) {

        return;
    }


    auto end2 = AlreadyMonitoredFiles.end();
    for(auto iter = AlreadyMonitoredFiles.begin(); iter != end2; ++iter) {

        // Skip if already added //
        if((*iter)->Added)
            continue;

        std::vector<const std::string*> targetfiles;

        // Add our file //
        targetfiles.push_back((*iter)->File.get());

        // Set this as added to avoid duplicates //
        (*iter)->Added = true;

        // Find all files that are in the same folder //
        std::string basepath = StringOperations::GetPath<std::string>(*(*iter)->File);

        for(auto iter2 = iter + 1; iter2 != end2; ++iter2) {

            if(basepath == StringOperations::GetPath<std::string>(*(*iter2)->File) &&
                !(*iter2)->Added) {

                // This is in the same folder and thus can be monitored by the same listener //
                targetfiles.push_back((*iter2)->File.get());

                // This, too, is now added //
                (*iter2)->Added = true;
            }
        }

        // Start monitoring for them //
        int listenerid;
        tmphandler->ListenForFileChanges(targetfiles,
            std::bind(&ScriptModule::_FileChanged, this, std::placeholders::_1,
                std::placeholders::_2),
            listenerid);

        FileListeners.push_back(listenerid);
    }
}

void Leviathan::ScriptModule::_StopFileMonitoring(Lock& guard)
{

    auto tmphandler = ResourceRefreshHandler::Get();

    if(tmphandler) {

        // Stop listening for all the files at once //
        auto end = FileListeners.end();
        for(auto iter = FileListeners.begin(); iter != end; ++iter) {

            tmphandler->StopListeningForFileChanges(*iter);
        }

        FileListeners.clear();
    }

    AlreadyMonitoredFiles.clear();


    // Everything should be cleared now //
}

void Leviathan::ScriptModule::_AddFileToMonitorIfNotAlready(const std::string& file)
{

    // Look for a matching string //
    auto end = AlreadyMonitoredFiles.end();
    for(auto iter = AlreadyMonitoredFiles.begin(); iter != end; ++iter) {

        if(*(*iter)->File == file)
            return;
    }


    // Add it as it isn't there yet //
    AlreadyMonitoredFiles.push_back(std::make_unique<AutomonitoredFile>(file));
}

void Leviathan::ScriptModule::_FileChanged(
    const std::string& file, ResourceFolderListener& caller)
{

    GUARD_LOCK();

    // This ignores multiple messages //
    if(!caller.IsAFileStillUpdated())
        return;

    // Mark everything as not updated //
    auto tmphandler = ResourceRefreshHandler::Get();

    if(tmphandler) {

        tmphandler->MarkListenersAsNotUpdated(FileListeners);
    }

    // Reload the module //
    if(!ReLoadModuleCode()) {

        Logger::Get()->Error(
            "ScriptModule: FileChanged: failed to reload the module, " + GetInfoString());
    }
}

#endif // SCRIPTMODULE_LISTENFORFILECHANGES
// ------------------------------------ //
void Leviathan::ScriptModule::_BuildTheModule(Lock& guard)
{
    // Apply the (possibly) updated access mask first //
    ScriptBuilder->GetModule()->SetAccessMask(AccessMask);

    // Add the source files before building //
    for(size_t i = 0; i < ScriptSourceSegments.size(); i++) {
        if(ScriptBuilder->AddSectionFromMemory(ScriptSourceSegments[i]->SourceFile.c_str(),
               ScriptSourceSegments[i]->SourceCode->c_str(), 0,
               ScriptSourceSegments[i]->StartLine - 1) < 0) {
            Logger::Get()->Error("ScriptModule: GetModule: failed to build unbuilt module "
                                 "(adding source file '" +
                                 ScriptSourceSegments[i]->SourceFile + "' failed), " +
                                 GetInfoString());

            ScriptState = SCRIPTBUILDSTATE_FAILED;
            return;
        }
    }

    // Build it //
    int result;
    {
        // Only one script can be built at a time so a lock is required //
        Lock lock(ModuleBuildingMutex);
        result = ScriptBuilder->BuildModule();
    }

    if(result < 0) {
        // failed to build //
        Logger::Get()->Error(
            "ScriptModule: GetModule: failed to build unbuilt module, " + GetInfoString());

        ScriptState = SCRIPTBUILDSTATE_FAILED;
        return;
    }

    ASModule = ScriptBuilder->GetModule();
    ScriptState = SCRIPTBUILDSTATE_BUILT;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::OnAddedToBridge(
    std::shared_ptr<ScriptArgumentsProviderBridge> bridge)
{
    LEVIATHAN_ASSERT(this == bridge->GetModule(), "OnAddedToBridge it's not ours!");

    if(ArgsBridge) {

        return false;
    }

    ArgsBridge = bridge;
    return true;
}
// ------------------------------------ //
DLLEXPORT std::string ScriptModule::ResolvePathToScriptFile(const std::string& inputfilename,
    const std::string& relativepath, bool checkworkdirrelative /*= true*/)
{
    // The canonical calls here are probably not needed as the as
    // script builder makes all paths absolute

    // Check first relative, absolute, and then search //
    const auto asRelative = boost::filesystem::path(relativepath) / inputfilename;
    if(boost::filesystem::is_regular_file(asRelative)) {
        return boost::filesystem::canonical(asRelative).generic_string();

    } else if(checkworkdirrelative && boost::filesystem::is_regular_file(inputfilename)) {

        return boost::filesystem::canonical(inputfilename).generic_string();

    } else {

        // This returns an empty string for us //
        std::string searched = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
            StringOperations::RemoveExtension<std::string>(inputfilename),
            StringOperations::GetExtension<std::string>(inputfilename), false);

        if(searched.empty())
            return searched;

        return boost::filesystem::canonical(searched).generic_string();
    }
}

// ------------------------------------ //
Mutex Leviathan::ScriptModule::ModuleBuildingMutex;

// ------------------ ValidListenerData ------------------ //
Leviathan::ValidListenerData::ValidListenerData(
    asIScriptFunction* funcptr, std::string* name, std::string* metadataend) :
    FuncPtr(funcptr),
    ListenerName(name), RestOfMeta(metadataend)
{
    // increase references //
    FuncPtr->AddRef();
}

Leviathan::ValidListenerData::ValidListenerData(asIScriptFunction* funcptr, std::string* name,
    std::string* metadataend, std::string* generictypename) :
    FuncPtr(funcptr),
    ListenerName(name), RestOfMeta(metadataend), GenericTypeName(generictypename)
{
    // increase references //
    FuncPtr->AddRef();
}

Leviathan::ValidListenerData::~ValidListenerData()
{
    // decrease reference  //
    FuncPtr->Release();
}
// ------------------ ScriptSourceFileData ------------------ //
DLLEXPORT ScriptSourceFileData::ScriptSourceFileData(
    const std::string& file, int line, const std::string& code) :
    SourceFile(file),
    StartLine(line), SourceCode(std::make_shared<std::string>(code))
{}
