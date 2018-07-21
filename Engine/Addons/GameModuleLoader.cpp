// ------------------------------------ //
#include "GameModuleLoader.h"

#include "FileSystem.h"
#include "Logger.h"
#include "ObjectFiles/ObjectFileProcessor.h"

#include <boost/filesystem.hpp>

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT GameModuleLoader::GameModuleLoader() {}

DLLEXPORT GameModuleLoader::~GameModuleLoader()
{
    // We need to be careful about the delete order as the module will unregister from us
    for(auto& [key, value] : LoadedModules) {

        UNUSED(key);

        if(value == nullptr)
            continue;

        // But as we don't keep a reference any still alive modules have outside references
        // This check is quaranteed to be true as the object is alive if it isn't null
        if(value->GetRefCount() > 0) {

            LOG_ERROR("GameModuleLoader: module \"" + value->GetName() +
                      "\" still has outside references. It won't be unloaded!");
        }
    }

    // This isn't really needed
    LoadedModules.clear();
}
// ------------------------------------ //
DLLEXPORT void GameModuleLoader::Init()
{
    GUARD_LOCK();

    const auto modules =
        FileSystem::Get()->FindAllMatchingFiles(FILEGROUP_SCRIPT, ".*", "levgm", false);

    LOG_INFO("GameModuleLoader: detected following module files: ");

    for(const auto& file : modules) {

        std::string error;
        std::string moduleName;

        if(!_LoadInfoFromModuleFile(file->RelativePath, moduleName, error)) {
            LOG_INFO("\t" + file->RelativePath + " which is invalid: " + error);
            continue;
        } else {
            LOG_INFO(
                "\t" + file->RelativePath + " which contains module \"" + moduleName + "\"");
        }

        if(ModuleNameToPath.find(moduleName) != ModuleNameToPath.end()) {

            LOG_ERROR(
                "GameModuleLoader: detected duplicate module name \"" + moduleName +
                "\" at path: " + file->RelativePath +
                " earlier version was loaded from path: " + ModuleNameToPath[moduleName]);
            continue;
        }

        ModuleNameToPath[moduleName] = file->RelativePath;
    }
}

DLLEXPORT GameModule::pointer GameModuleLoader::Load(
    const std::string& moduleorfilename, const char* requiredby)
{
    GUARD_LOCK();

    const auto found = LoadedModules.find(moduleorfilename);

    if(found != LoadedModules.end()) {

        // Just return module if it is still loaded
        // auto locked = found->second.lock();
        auto locked = found->second;

        if(locked) {
            // But detect circular dependencies
            if(locked->IsInitializing()) {

                LOG_ERROR("GameModuleLoader: Load: circular dependency detected, module: " +
                          moduleorfilename +
                          " is being loaded by: " + std::string(requiredby) +
                          ", but it is still currently initializing, returning null");
                throw NotFound("circular loading detected");
            }

            return GameModule::pointer(locked);
        }

        const auto pathIter = ModuleNameToPath.find(moduleorfilename);

        if(pathIter == ModuleNameToPath.end()) {

            LOG_WARNING("GameModuleLoader: no path found for now unloaded module: " +
                        moduleorfilename);
            throw NotFound("unknown path for unloaded module: " + moduleorfilename);
        }

        // Reload module
        LOG_INFO("GameModuleLoader: reloading unloaded module");

        try {
            return _LoadModuleFromFile(ModuleNameToPath[moduleorfilename], requiredby);
        } catch(const NotFound& e) {

            throw NotFound("module file no longer exists: " + moduleorfilename +
                           " from path: " + pathIter->second +
                           ", inner exception: " + e.what());
        }
    }

    // Load it //
    const auto pathIter = ModuleNameToPath.find(moduleorfilename);

    if(pathIter == ModuleNameToPath.end()) {

        // Searching code from GameModule
        // // Find the actual file //
        // file =
        //     FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, modulename, extension,
        //     false);

        // if(file.size() == 0) {

        //     // One more search attempt //
        //     file = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT,
        //         StringOperations::RemoveExtension(modulename), extension, false);
        // }


        LOG_ERROR("GameModuleLoader: module with name doesn't exist: " + moduleorfilename);
        throw NotFound(
            "no module found with name (hint: remove extension): " + moduleorfilename);
    }

    try {
        return _LoadModuleFromFile(ModuleNameToPath[moduleorfilename], requiredby);
    } catch(const NotFound& e) {

        throw NotFound("module failed initial loading: " + moduleorfilename +
                       " from path: " + pathIter->second + ", inner exception: " + e.what());
    }
}
// ------------------------------------ //
void GameModuleLoader::GameModuleReportDestruction(GameModule& module)
{
    GUARD_LOCK();

    const auto iter = LoadedModules.find(module.GetName());

    if(iter == LoadedModules.end()) {

        LOG_WARNING(
            "GameModuleLoader: unknown module reported destruction: " + module.GetName());
        return;
    }

    if(iter->second != &module) {

        LOG_ERROR("GameModuleLoader: module with mismatching name and address reported "
                  "destruction: " +
                  module.GetName() + ". Is there a module with duplicate name?");
        return;
    }

    iter->second = nullptr;
}
// ------------------------------------ //
bool GameModuleLoader::_LoadInfoFromModuleFile(
    const std::string& file, std::string& modulename, std::string& errorstring)
{
    auto ofile = ObjectFileProcessor::ProcessObjectFile(file, Logger::Get());

    if(!ofile) {
        errorstring = "file has invalid syntax";
        return false;
    }

    if(ofile->GetTotalObjectCount() != 1) {
        errorstring = "file doesn't contain just one object";
        return true;
    }

    auto object = ofile->GetObjectFromIndex(0);

    modulename = object->GetName();

    if(modulename.empty()) {

        errorstring = "module object doesn't have a name";
        return false;
    }

    return true;
}
// ------------------------------------ //
GameModule::pointer GameModuleLoader::_LoadModuleFromFile(
    const std::string& filename, const char* requiredby)
{
    if(!boost::filesystem::exists(filename)) {
        throw NotFound("file doesn't exist: " + filename);
    }

    try {

        auto module = GameModule::MakeShared<GameModule>(filename, requiredby, this);

        // At this point the module will report destruction so we must register it before Init,
        // which can fail quite easily due to syntax errors
        LoadedModules[module->GetName()] = module.get();

        if(!module->Init())
            throw Exception("module Init failed");

        // We don't need to keep a reference alive because the module reports to us before it
        // destructs and we want to act like a weak reference
        return module;

    } catch(const Exception& e) {

        throw NotFound("Module failed to load due to exception: " + std::string(e.what()));
    }
}
