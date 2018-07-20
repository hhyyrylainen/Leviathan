// ------------------------------------ //
#include "GameModule.h"

#include "Common/StringOperations.h"
#include "FileSystem.h"
#include "GameModuleLoader.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptExecutor.h"

#include <boost/filesystem.hpp>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT GameModule::GameModule(const std::string& filepath, const std::string& ownername,
    GameModuleLoader* loadedthrough) :
    EventableScriptObject(nullptr),
    OwnerName(ownername), Loader(loadedthrough)
{
    std::string file = filepath;

    if(!boost::filesystem::is_regular_file(file)) {

        // Couldn't find file //
        throw InvalidArgument("File doesn't exist '" + filepath + "'");
    }

    LoadedFromFile = file;

    // Load the file //
    auto ofile = ObjectFileProcessor::ProcessObjectFile(file, Logger::Get());

    if(!ofile) {

        throw InvalidArgument("File is invalid");
    }

    // Process the objects //
    if(ofile->GetTotalObjectCount() != 1) {

        throw InvalidArgument("File contains invalid number of objects, single GameModule "
                              "expected");
    }

    // Get various data from the header //
    ObjectFileProcessor::LoadValueFromNamedVars<std::string>(
        ofile->GetVariables(), "Version", Name, "-1", Logger::Get(), "GameModule:");

    auto gmobject = ofile->GetObjectFromIndex(0);

    Name = gmobject->GetName();

    // Handle the single object //
    ObjectFileList* properties = gmobject->GetListWithName("properties");
    ObjectFileTextBlock* sources = gmobject->GetTextBlockWithName("sourcefiles");
    ObjectFileTextBlock* exports = gmobject->GetTextBlockWithName("export");
    ObjectFileTextBlock* imports = gmobject->GetTextBlockWithName("import");

    if(!properties || !sources) {

        throw InvalidArgument("File contains invalid GameModule, properties or "
                              "sourcefiles not found");
    }

    // Copy data //
    if(sources->GetLineCount() < 1) {

        throw InvalidArgument("At least one source file expected in sourcefiles (even if this "
                              "module only consists of public definitions)");
    }

    const std::string moduleFilePath =
        boost::filesystem::path(StringOperations::GetPath(LoadedFromFile)).generic_string();

    // Resolve all files to their actual paths //
    for(size_t i = 0; i < sources->GetLineCount(); ++i) {

        // Skip empty lines to allow commenting out things //
        if(sources->GetLine(i).empty())
            continue;

        const std::string codeFile =
            ScriptModule::ResolvePathToScriptFile(sources->GetLine(i), moduleFilePath);

        if(codeFile.empty()) {
            throw InvalidArgument("GameModule(" + LoadedFromFile +
                                  ") can't find "
                                  "source file: " +
                                  sources->GetLine(i));
        } else {

            SourceFiles.push_back(codeFile);
        }
    }

    // Resolve export files
    if(exports) {

        for(size_t i = 0; i < exports->GetLineCount(); ++i) {

            // Skip empty lines to allow commenting out things //
            if(exports->GetLine(i).empty())
                continue;

            const std::string codeFile =
                ScriptModule::ResolvePathToScriptFile(exports->GetLine(i), moduleFilePath);

            if(codeFile.empty()) {
                throw InvalidArgument("GameModule(" + LoadedFromFile +
                                      ") can't find "
                                      "exported file: " +
                                      exports->GetLine(i));
            } else {

                ExportFiles.push_back(codeFile);
            }
        }
    }

    // Read properties //
    if(auto* data = properties->GetVariables().GetValueDirectRaw("ExtraAccess");
        data != nullptr) {

        std::string flags;
        if(data->GetVariableCount() != 1 ||
            !data->GetValue().ConvertAndAssingToVariable(flags)) {

            throw InvalidArgument(
                "GameModule(" + LoadedFromFile +
                ") has an invalid value for property 'ExtraAccess': not string");
        }

        try {
            ExtraAccess = ParseScriptAccess(flags);
        } catch(const InvalidArgument& e) {
            throw InvalidArgument(
                "GameModule(" + LoadedFromFile +
                ") has an invalid value for property 'ExtraAccess': " + e.what());
        }
    }

    // Import modules just need to be copied
    if(imports) {

        for(size_t i = 0; i < imports->GetLineCount(); ++i) {
            if(imports->GetLine(i).empty())
                continue;

            ImportModules.push_back(imports->GetLine(i));
        }
    }

    LEVIATHAN_ASSERT(!SourceFiles.empty(), "GameModule: empty source files");
}

DLLEXPORT GameModule::~GameModule()
{
    LoadedImportedModules.clear();

    ReleaseScript();

    UnRegisterAllEvents();

    Loader->GameModuleReportDestruction(*this);

    // // This would be a bit inconvenient with the new way modules are kept loaded
    // if(Scripting) {

    //     LOG_FATAL("GameModule: 'ReleaseScript' not called before destructor");
    // }
}
// ------------------------------------ //
DLLEXPORT bool GameModule::Init()
{
    IsCurrentlyInitializing = true;

    // Resolve imports first
    if(!ImportModules.empty()) {

        const auto desc = GetDescription(true);

        for(const auto& import : ImportModules) {

            // Load the dependency module. We give our full description to make debugging the
            // chain easier
            try {
                auto imported = Loader->Load(import, desc.c_str());

                LoadedImportedModules.push_back(imported);
            } catch(const NotFound& e) {

                LOG_ERROR("GameModule: Init: module \"" + import +
                          "\" could not be loaded by: " + desc + ", exception:");
                e.PrintToLog();
                return false;
            }
        }
    }

    // Compile a new module //
    ScriptModule* mod = NULL;

    LEVIATHAN_ASSERT(!Scripting, "GameModule may not already have Script instance created");

    Scripting =
        std::shared_ptr<ScriptScript>(new ScriptScript(ScriptExecutor::Get()->CreateNewModule(
            "GameModule(" + Name + ") ScriptModule", LoadedFromFile)));

    // Get the newly created module //
    mod = Scripting->GetModule();

    for(const auto& file : SourceFiles) {
        mod->AddScriptSegmentFromFile(file);
    }

    // Also add imported files
    for(const auto& import : LoadedImportedModules) {
        const auto& fileList = import->GetExportFiles();

        if(fileList.empty()) {

            LOG_WARNING("GameModule: Init: imported module \"" + import->GetName() +
                        "\" doesn't specify any exports");
            continue;
        }

        for(const auto& file : import->GetExportFiles()) {
            mod->AddScriptSegmentFromFile(file);
        }
    }

    // Set access flags //
    if(ExtraAccess != 0)
        mod->AddAccessRight(ExtraAccess);

    // Build the module (by creating a callback list) //
    std::vector<std::shared_ptr<ValidListenerData>> containedlisteners;

    mod->GetListOfListeners(containedlisteners);

    if(mod->GetModule() == nullptr) {
        // Fail to build //
        Logger::Get()->Error("GameModule: Init: failed to build AngelScript module");
        return false;
    }

    RegisterStandardScriptEvents(containedlisteners);

    for(size_t i = 0; i < containedlisteners.size(); i++) {
        // Bind generic event //
        if(containedlisteners[i]->GenericTypeName &&
            containedlisteners[i]->GenericTypeName->size() > 0) {

            // Registered in RegisterStandardScriptEvents
            continue;
        }

        // Skip global events
        EVENT_TYPE etype = ResolveStringToType(*containedlisteners[i]->ListenerName);

        // Skip types that we handle ourselves //
        if(etype == EVENT_TYPE_ERROR)
            etype = GetCommonEventType(*containedlisteners[i]->ListenerName);

        if(etype != EVENT_TYPE_ERROR) {

            continue;
        }

        Logger::Get()->Warning("GameModule: unknown event type " +
                               *containedlisteners[i]->ListenerName +
                               ", did you intent to use Generic type?");
    }

    // Call init callbacks //

    // fire an event //
    Event* initEvent = new Event(EVENT_TYPE_INIT, nullptr);
    OnEvent(initEvent);
    // Release our initial reference
    initEvent->Release();

    IsCurrentlyInitializing = false;
    return true;
}

DLLEXPORT void GameModule::ReleaseScript()
{
    if(Scripting) {
        // Call release callback and destroy everything //
        // fire an event //
        Event* releaseEvent = new Event(EVENT_TYPE_RELEASE, nullptr);
        OnEvent(releaseEvent);
        // Release our initial reference
        releaseEvent->Release();

        // Remove our reference //
        int tmpid = Scripting->GetModule()->GetID();
        Scripting.reset();

        ScriptExecutor::Get()->DeleteModuleIfNoExternalReferences(tmpid);
    }
}
// ------------------------------------ //
DLLEXPORT std::string GameModule::GetDescription(bool full /*= false*/)
{
    return "GameModule(" + Name + (full ? " v" + Version + ") " : ") ") +
           " owned by: " + OwnerName +
           (full ? ", loaded from file: " + LoadedFromFile + "." : ".");
}
// ------------------------------------ //
ScriptRunResult<int> GameModule::_DoCallWithParams(
    ScriptRunningSetup& sargs, Event* event, GenericEvent* event2)
{
    if(event)
        return ScriptExecutor::Get()->RunScript<int>(
            Scripting->GetModuleSafe(), sargs, this, event);
    else
        return ScriptExecutor::Get()->RunScript<int>(
            Scripting->GetModuleSafe(), sargs, this, event2);
}
