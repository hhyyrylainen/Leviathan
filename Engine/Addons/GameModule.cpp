// ------------------------------------ //
#include "GameModule.h"

#include "Common/StringOperations.h"
#include "FileSystem.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptExecutor.h"

#include <boost/filesystem.hpp>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::GameModule::GameModule(const std::string& modulename,
    const std::string& ownername, const std::string& extension /*= "txt|levgm"*/) :
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

    LEVIATHAN_ASSERT(!SourceFiles.empty(), "GameModule: empty source files");
}

DLLEXPORT Leviathan::GameModule::~GameModule()
{
    UnRegisterAllEvents();
    
    if(Scripting) {

        LOG_FATAL("GameModule: 'ReleaseScript' not called before destructor");
    }
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::GameModule::Init()
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

    // Build the module (by creating a callback list) //
    std::vector<std::shared_ptr<ValidListenerData>> containedlisteners;

    mod->GetListOfListeners(containedlisteners);

    if(mod->GetModule() == nullptr) {
        // Fail to build //
        Logger::Get()->Error("GameModule: Init: failed to build AngelScript module");
        return false;
    }

    for(size_t i = 0; i < containedlisteners.size(); i++) {
        // Bind generic event //
        if(containedlisteners[i]->GenericTypeName &&
            containedlisteners[i]->GenericTypeName->size() > 0) {

            // custom event listener //
            RegisterForEvent(*containedlisteners[i]->GenericTypeName);
            continue;
        }

        // look for global events //
        EVENT_TYPE etype = ResolveStringToType(*containedlisteners[i]->ListenerName);
        if(etype != EVENT_TYPE_ERROR) {

            RegisterForEvent(etype);
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

DLLEXPORT void Leviathan::GameModule::ReleaseScript()
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
DLLEXPORT std::string Leviathan::GameModule::GetDescription(bool full /*= false*/)
{
    return "GameModule(" + Name + (full ? " v" + Version + ") " : ") ") +
           " owned by: " + OwnerName +
           (full ? ", loaded from file: " + LoadedFromFile + "." : ".");
}
// ------------------------------------ //
void Leviathan::GameModule::_CallScriptListener(Event* event, GenericEvent* event2)
{

    ScriptModule* mod = Scripting->GetModule();

    if(event) {
        // Get the listener name from the event type //
        std::string listenername = GetListenerNameFromType(event->GetType());

        // check does the script contain right listeners //
        if(mod->DoesListenersContainSpecificListener(listenername)) {

            ScriptRunningSetup ssetup;
            ssetup.SetEntrypoint(mod->GetListeningFunctionName(listenername));

            // run the script //
            if(Scripting) {
                ScriptExecutor::Get()->RunScript<void>(
                    Scripting->GetModuleSafe(), ssetup, this, event);
                // do something with result //
            }

            // Do something with the result //
        }
    } else {
        // generic event is passed //
        if(mod->DoesListenersContainSpecificListener("", event2->GetTypePtr())) {

            ScriptRunningSetup ssetup;
            ssetup.SetEntrypoint(mod->GetListeningFunctionName("", event2->GetTypePtr()));

            // run the script //
            if(Scripting) {
                ScriptExecutor::Get()->RunScript<void>(
                    Scripting->GetModuleSafe(), ssetup, this, event2);
                // do something with result //
            }
        }
    }
}
// ------------------ Being an actual module ------------------ //
DLLEXPORT std::shared_ptr<VariableBlock> Leviathan::GameModule::ExecuteOnModule(
    const std::string& entrypoint,
    std::vector<std::shared_ptr<NamedVariableBlock>>& otherparams, bool& existed,
    bool passself, bool fulldeclaration /*= false*/)
{
    // TODO: move over to new script call type
    DEBUG_BREAK;
    // Add this as parameter //
    if(passself) {
        otherparams.insert(otherparams.begin(),
            std::shared_ptr<NamedVariableBlock>(
                new NamedVariableBlock(new VoidPtrBlock(this), "GameModule")));

        // we are returning ourselves so increase refcount
        AddRef();
    }

    ScriptRunningSetup setup;
    setup.SetArguments(otherparams)
        .SetEntrypoint(entrypoint)
        .SetUseFullDeclaration(fulldeclaration);

    if(!Scripting)
        return nullptr;

    auto result = ScriptExecutor::Get()->RunSetUp(Scripting->GetModule(), &setup);

    existed = setup.ScriptExisted;

    return result;
}
// ------------------ Script proxies ------------------ //
