// ------------------------------------ //
#include "Importer.h"

#include "Common/StringOperations.h"
#include "Exceptions.h"

//#include "boost/filesystem.hpp"

#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"

#include <filesystem>
#include <string_view>

using namespace Leviathan;
using namespace Leviathan::Editor;
// ------------------------------------ //
Importer::Importer(const std::string& source, const std::string& destination) :
    Source(source), Destination(destination)
{}

Importer::~Importer() {}
// ------------------------------------ //
bool Importer::Run()
{
    LOG_INFO("Beginning import from " + Source + " to " + Destination);

    if(!std::filesystem::exists(Source)) {
        LOG_ERROR("Importer: source does not exist: " + Source);
        return false;
    }

    Source = std::filesystem::absolute(Source).c_str();

    // If target has an extension treat it as a single file
    if(!StringOperations::GetExtension(StringOperations::RemovePath(Destination)).empty()) {
        TargetIsFile = true;
    }

    bool result = true;

    // If the source is a file handle it as such
    if(!std::filesystem::is_directory(Source)) {

        LOG_INFO("Importing single file");
        if(!ImportFile(Source)) {
            result = false;
        }

    } else if(!TargetIsFile) {

        std::filesystem::recursive_directory_iterator dir(Source), end;
        while(dir != end) {

            if(dir->status().type() == std::filesystem::file_type::regular ||
                dir->status().type() == std::filesystem::file_type::symlink) {

                if(!ImportFile(std::filesystem::absolute(dir->path()).c_str())) {
                    result = false;
                }
            }

            ++dir;
        }
    } else {
        LOG_ERROR("Importer: cannot import multiple files into a single target file");
        result = false;
    }

    LOG_INFO("Finished import. Success: " + std::to_string(result));
    return result;
}
// ------------------------------------ //
bool Importer::ImportFile(const std::string& file)
{
    // Detect type from extension
    const auto extension = StringOperations::GetExtension(file);
    bool success = false;

    if(extension == "png" || extension == "jpg") {

        success = ImportTypedFile(file, FileType::Texture);

    } else if(extension == "bsl") {

        success = ImportTypedFile(file, FileType::Shader);
        return true;
    } else if(extension == "fbx") {

        success = ImportTypedFile(file, FileType::Model);
        return true;
    } else {
        // Ignore unknown extensions
        return true;
    }

    // TODO: bslinc files should be copied

    if(!success) {
        LOG_ERROR("Importer: importing file '" + file + "' failed");
    } else {
        ++ImportedFiles;
    }

    return success;
}

template<class T>
bool ImportAndSaveFile(const std::string& file, const std::string& target, bool compress)
{
    auto resource = bs::gImporter().import<T>(file.c_str());

    if(resource) {

        bs::gResources().save(resource, target.c_str(), true, compress);
        return true;
    }

    return false;
}

bool Importer::ImportTypedFile(const std::string& file, FileType type)
{
    const auto target = GetTargetPath(file, type);

    // Skip if the target is up to date
    if(!NeedsImporting(file, target)) {
        return true;
    }

    LOG_INFO("Importing '" + file + "' to '" + target + "'");

    switch(type) {
    case FileType::Shader: return ImportAndSaveFile<bs::Shader>(file, target, Compress);
    case FileType::Texture: return ImportAndSaveFile<bs::Texture>(file, target, Compress);
    case FileType::Model: return ImportAndSaveFile<bs::Mesh>(file, target, Compress);
    }

    // models need for animations:
    // auto importOptions = bs::MeshImportOptions::create();
    // importOptions->setImportAnimation(true);
    // auto resources = bs::gImporter().importAll("humanAnimated.fbx", importOptions);
    // HAnimationClip animationClip = static_resource_cast<AnimationClip>(resources[1].value);


    LEVIATHAN_ASSERT(false, "should not get here");
}
// ------------------------------------ //
std::string Importer::GetTargetPath(const std::string& file, FileType type) const
{
    if(TargetIsFile)
        return Destination;

    using namespace std::string_view_literals;

    const auto prefixRemoved = StringOperations::RemovePrefix(file, Source);

    std::string_view strippedPath = prefixRemoved;

    if(!strippedPath.empty() && strippedPath[0] == '/')
        strippedPath = strippedPath.substr(1);

    bool needsPrefix = false;

    // Detect common sub folder names that should be erased
    switch(type) {
    case FileType::Shader:
        if(strippedPath.find("Shaders"sv) == std::string_view::npos &&
            strippedPath.find("shaders"sv) == std::string_view::npos)
            needsPrefix = true;
        break;
    case FileType::Texture:
        if(strippedPath.find("Textures"sv) == std::string_view::npos &&
            strippedPath.find("textures"sv) == std::string_view::npos)
            needsPrefix = true;
        break;
    case FileType::Model:
        if(strippedPath.find("Models"sv) == std::string_view::npos &&
            strippedPath.find("models"sv) == std::string_view::npos)
            needsPrefix = true;
        break;
    }

    auto targetPath = std::filesystem::path(Destination);
    if(needsPrefix)
        targetPath = targetPath / std::filesystem::path(GetSubFolderForType(type));

    const auto result =
        targetPath / std::filesystem::path(
                         std::string(strippedPath.data(), strippedPath.size()) + ".asset");

    // Make sure the directory exists
    try {
        std::filesystem::create_directories(result.parent_path());
    } catch(const std::filesystem::filesystem_error& e) {

        LOG_ERROR("Failed to make target folder: " +
                  std::string(result.parent_path().c_str()) + ", error: " + e.what());
        throw InvalidArgument("cannot make target folder");
    }

    return result.c_str();
}
// ------------------------------------ //
const char* Importer::GetSubFolderForType(FileType type)
{
    switch(type) {
    case FileType::Shader: return "Shaders";
    case FileType::Texture: return "Textures";
    case FileType::Model: return "Models";
    }

    LEVIATHAN_ASSERT(false, "should not get here");
    return nullptr;
}
// ------------------------------------ //
bool Importer::NeedsImporting(const std::string& file, const std::string& target)
{
    if(!std::filesystem::exists(target))
        return true;

    return std::filesystem::last_write_time(file) > std::filesystem::last_write_time(target);
}
