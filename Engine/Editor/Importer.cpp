// ------------------------------------ //
#include "Importer.h"

#include "Common/StringOperations.h"
#include "Exceptions.h"

#include <filesystem>
#include <string_view>

using namespace Leviathan;
using namespace Leviathan::Editor;
// ------------------------------------ //
constexpr auto OPTIONS_FILE_NAME = ".options.json";

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

    Source = std::filesystem::absolute(Source).string();

    // If target has an extension treat it as a single file
    if(!StringOperations::GetExtension(StringOperations::RemovePath(Destination)).empty()) {
        TargetIsFile = true;
    }

    bool result = true;

    // If the source is a file handle it as such
    if(!std::filesystem::is_directory(Source)) {

        LOG_INFO("Importing single file");
        SourceIsFile = true;
        if(!ImportFile(Source)) {
            result = false;
        }

    } else if(!TargetIsFile) {

        std::filesystem::recursive_directory_iterator dir(Source), end;
        while(dir != end) {

            if(dir->status().type() == std::filesystem::file_type::regular ||
                dir->status().type() == std::filesystem::file_type::symlink) {

                if(!ImportFile(std::filesystem::absolute(dir->path()).string())) {
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

    // Options files
    if(extension == "json" &&
        StringOperations::StringEndsWith<std::string>(file, OPTIONS_FILE_NAME)) {

        success = ImportWithOptions(file);

    } else {

        const auto optionsCoutnerpart = file + OPTIONS_FILE_NAME;

        // Check does this have a corresponding options file
        if(std::filesystem::exists(optionsCoutnerpart)) {

            // Skip this file if we aren't running on a single file
            if(SourceIsFile) {

                LOG_INFO("Importer: running on single file with an options file, changing "
                         "target to the options file");
                success = ImportWithOptions(optionsCoutnerpart);

            } else {
                return true;
            }
        }

        const auto type = GetTypeFromExtension(extension);

        if(type == FileType::Invalid) {
            // Ignore unknown extensions
            return true;
        }

        success = ImportTypedFile(file, type);
    }

    if(!success) {
        LOG_ERROR("Importer: importing file '" + file + "' failed");
    } else {
        ++ImportedFiles;
    }

    return success;
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
    case FileType::Invalid: throw InvalidArgument("trying to import invalid file");
    }

    LEVIATHAN_ASSERT(false, "should not get here");
}

bool Importer::ImportWithOptions(const std::string& optionsfile)
{
    std::ifstream fileReader(optionsfile);

    Json::CharReaderBuilder builder;
    Json::Value value;
    JSONCPP_STRING errs;

    if(!parseFromStream(builder, fileReader, &value, &errs))
        throw InvalidArgument("invalid json:" + errs);

    std::string baseFile;

    if(value["baseFile"]) {
        baseFile = value["baseFile"].asString();
    } else {
        baseFile = optionsfile.substr(0, optionsfile.size() - std::strlen(OPTIONS_FILE_NAME));
    }

    if(!std::filesystem::exists(baseFile)) {
        LOG_ERROR("Importer: base file for options doesn't exist: " + baseFile);
        return false;
    }

    const auto extension = StringOperations::GetExtension(baseFile);


    const auto type = GetTypeFromExtension(extension);

    if(type == FileType::Invalid) {
        LOG_ERROR("Importer: options base file has unknown type");
        return true;
    }

    std::string target;

    if(value["target"]) {
        target = GetTargetPath(value["target"].asString(), type);
    } else {
        target = GetTargetPath(baseFile, type);
    }

    LOG_INFO("Importing with options from '" + optionsfile + "' to '" + target + "'");

    switch(type) {
    case FileType::Shader:
        return ImportAndSaveWithOptions<bs::Shader>(baseFile, target, Compress, value);
    case FileType::Texture:
        return ImportAndSaveWithOptions<bs::Texture>(baseFile, target, Compress, value);
    case FileType::Model:
        return ImportAndSaveWithOptions<bs::Mesh>(baseFile, target, Compress, value);
    case FileType::Invalid: throw InvalidArgument("trying to import invalid file");
    }

    LEVIATHAN_ASSERT(false, "should not get here");

    return true;
}
// ------------------------------------ //
std::string Importer::GetTargetPath(const std::string& file, FileType type) const
{
    if(TargetIsFile)
        return Destination;
    return GetTargetWithoutSingleCheck(file, type);
}

std::string Importer::GetTargetWithoutSingleCheck(const std::string& file, FileType type) const
{
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
    case FileType::Invalid: break;
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
                  std::string(result.parent_path().string()) + ", error: " + e.what());
        throw InvalidArgument("cannot make target folder");
    }

    return result.string();
}
// ------------------------------------ //
const char* Importer::GetSubFolderForType(FileType type)
{
    switch(type) {
    case FileType::Shader: return "Shaders";
    case FileType::Texture: return "Textures";
    case FileType::Model: return "Models";
    case FileType::Invalid:
        throw InvalidArgument("cannot get sub folder for invalid file type");
    }

    LEVIATHAN_ASSERT(false, "should not get here");
    return nullptr;
}

Importer::FileType Importer::GetTypeFromExtension(const std::string& extension)
{
    if(extension == "png" || extension == "jpg")
        return FileType::Texture;

    if(extension == "bsl" || extension == "bslinc")
        return FileType::Shader;


    if(extension == "fbx")
        return FileType::Model;

    // throw InvalidArgument("Extension (" + extension + ") is not a valid type");
    return FileType::Invalid;
}
// ------------------------------------ //
bool Importer::NeedsImporting(const std::string& file, const std::string& target)
{
    if(!std::filesystem::exists(target))
        return true;

    return std::filesystem::last_write_time(file) > std::filesystem::last_write_time(target);
}

bool Importer::NeedsImporting(
    const std::string& file, const std::string& optionsfile, const std::string& target)
{
    if(NeedsImporting(file, target))
        return true;

    return std::filesystem::last_write_time(optionsfile) >
           std::filesystem::last_write_time(target);
}
