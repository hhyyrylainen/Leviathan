// ------------------------------------ //
#include "GameModule.h"

#include "Common/StringOperations.h"
#include "FileSystem.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptExecutor.h"

#include <boost/filesystem.hpp>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT GameModule::GameModule(const std::string& modulename, const std::string& ownername,
    const std::string& extension /*= "txt|levgm"*/) :
    EventableScriptObject(nullptr),
    OwnerName(ownername)
{
    std::string file = modulename + ".levgm";

    if(!boost::filesystem::is_regular_file(file)) {
        // Find the actual file //
        file =
            FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, modulename, extension, false);

        if(file.size() == 0) {

            // One more search attempt //
            file = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
                StringOperations::RemoveExtension(modulename), extension, false);
        }

        if(file.size() == 0) {
            // Couldn't find file //

            throw InvalidArgument("File doesn't exist and full search also failed for "
                                  "module '" +
                                  modulename + "'");
        }
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

    // handle the single object //
    ObjectFileList* properties = gmobject->GetListWithName("properties");
    ObjectFileTextBlock* sources = gmobject->GetTextBlockWithName("sourcefiles");

    if(!properties || !sources) {

        throw InvalidArgument("File contains invalid GameModule, properties or "
                              "sourcefiles not found");
    }

    // Copy data //
    if(sources->GetLineCount() < 1) {

        throw InvalidArgument("At least one source file expected in sourcefiles");
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


    LEVIATHAN_ASSERT(!SourceFiles.empty(), "GameModule: empty source files");
}

DLLEXPORT GameModule::~GameModule()
{
    UnRegisterAllEvents();

    if(Scripting) {

        LOG_FATAL("GameModule: 'ReleaseScript' not called before destructor");
    }
}
// ------------------------------------ //
DLLEXPORT bool GameModule::Init()
{
    // Compile a new module //
    ScriptModule* mod = NULL;

    if(!Scripting) {

        Scripting = std::shared_ptr<ScriptScript>(
            new ScriptScript(ScriptExecutor::Get()->CreateNewModule(
                "GameModule(" + Name + ") ScriptModule", LoadedFromFile)));

        // Get the newly created module //
        mod = Scripting->GetModule();

        for(const auto& file : SourceFiles) {
            mod->AddScriptSegmentFromFile(file);
        }

    } else {

        // Get already created module //
        mod = Scripting->GetModule();
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

    return true;
}

DLLEXPORT void GameModule::ReleaseScript()
{
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
