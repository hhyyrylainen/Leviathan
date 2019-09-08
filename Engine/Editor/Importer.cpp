// ------------------------------------ //
#include "Importer.h"

#include "Common/StringOperations.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "Utility/MD5Generator.h"

#include <filesystem>
#include <string_view>

using namespace Leviathan;
using namespace Leviathan::Editor;
// ------------------------------------ //
constexpr auto OPTIONS_FILE_NAME = ".options.json";
constexpr auto FOLDER_OPTIONS_FILE_NAME = "FolderOptions.json";
constexpr auto CACHE_FILE_NAME = "LeviathanImporterCache.json";

Importer::Importer(const std::string& source, const std::string& destination) :
    Source(std::filesystem::absolute(source).string()),
    Destination(std::filesystem::absolute(destination).string())
{
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
    JsonWriter = decltype(JsonWriter)(builder.newStreamWriter());

    builder["commentStyle"] = "All";
    builder["indentation"] = "4";
    PrettyWriter = decltype(PrettyWriter)(builder.newStreamWriter());
}

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

    CacheData = GetCacheStructure();

    // If the source is a file handle it as such
    if(!std::filesystem::is_directory(Source)) {

        InformationCacheFile = "";

        LOG_INFO("Importing single file");
        SourceIsFile = true;
        if(!ImportFile(Source)) {
            result = false;
        }

    } else if(!TargetIsFile) {

        InformationCacheFile = (std::filesystem::path(Destination) / CACHE_FILE_NAME).string();

        LOG_INFO("Import cache file is: " + InformationCacheFile);

        CheckAndLoadCache();

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

        SaveCache();

    } else {
        LOG_ERROR("Importer: cannot import multiple files into a single target file");
        result = false;
    }

    LOG_INFO("Finished import. Success: " + std::to_string(result));
    return result;
}
// ------------------------------------ //
std::string Importer::GetHashOfOptions(const Json::Value& options) const
{
    std::stringstream sstream;
    JsonWriter->write(options, &sstream);

    return MD5(sstream.str()).hexdigest();
}

Json::Value Importer::GetCacheStructure() const
{
    Json::Value obj;
    obj["files"] = Json::Value(Json::ValueType::objectValue);
    return obj;
}

std::string Importer::GetCacheKeyForFile(const std::string& file) const
{
    std::size_t start = 0;

    for(; start < file.size() && start < Source.size(); ++start) {

        if(file[start] != Source[start]) {
            break;
        }
    }

    if(StringOperations::IsCharacterPathSeparator(file[start]))
        ++start;

    auto result = file.substr(start);

    StringOperations::ReplaceSingleCharacterInPlace(result, '\\', '/');
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

        const auto optionsCounterpart = file + OPTIONS_FILE_NAME;

        // Check does this have a corresponding options file
        if(std::filesystem::exists(optionsCounterpart)) {

            // Skip this file if we aren't running on a single file
            if(SourceIsFile) {

                LOG_INFO("Importer: running on single file with an options file, changing "
                         "target to the options file");
                success = ImportWithOptions(optionsCounterpart);

            } else {
                return true;
            }
        }

        const auto type = GetTypeFromExtension(extension);

        if(type == FileType::Invalid) {
            // Ignore unknown extensions
            return true;
        }

        success = ImportTypedFile(file, type, {}, GetFolderOptionsForFile(file), {});
    }

    if(!success) {
        LOG_ERROR("Importer: importing file '" + file + "' failed");
    } else {
        ++ImportedFiles;
    }

    return success;
}

bool Importer::ImportTypedFile(const std::string& file, FileType type,
    std::optional<std::string> target, std::optional<Json::Value> options,
    std::optional<std::string> originalfile)
{
    if(!options)
        options = Json::Value(Json::objectValue);

    if(!target)
        target = GetTargetPath(file, type);

    if(!originalfile)
        originalfile = file;

    std::string hash;

    // TODO: with the options files (when originalfile was specified when entering this) also
    // the hash of the base file should be used

    // Skip if the target is up to date
    if(!NeedsImporting(*originalfile, *target, *options, hash)) {
        return true;
    }

    LOG_INFO("Importing '" + file + "' to '" + *target + "'");

    bool result = false;

    switch(type) {
    case FileType::Shader:
        result = ImportAndSaveWithOptions<bs::Shader>(file, *target, *options);
        break;
    case FileType::Texture:
        result = ImportAndSaveWithOptions<bs::Texture>(file, *target, *options);
        break;
    case FileType::Model:
        result = ImportAndSaveWithOptions<bs::Mesh>(file, *target, *options);
        break;
    case FileType::Invalid: throw InvalidArgument("trying to import invalid file");
    }

    // Write to cache
    if(result) {
        Json::Value obj;
        obj["optionsHash"] = GetHashOfOptions(*options);
        obj["fileHash"] = hash;

        CacheData["files"][GetCacheKeyForFile(*originalfile)] = obj;
    }

    return result;
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
    std::optional<std::string> originalFile;

    if(value["baseFile"]) {
        baseFile = value["baseFile"].asString();
        originalFile = optionsfile;
        // TODO: detect incorrectly specifying the base file if it is the same as the automatic
        // one
    } else {
        baseFile = optionsfile.substr(0, optionsfile.size() - std::strlen(OPTIONS_FILE_NAME));
    }

    try {
        if(!std::filesystem::exists(baseFile)) {
            // Try relative
            baseFile = (std::filesystem::path(optionsfile).parent_path() / baseFile).string();
        }
    } catch(const std::filesystem::filesystem_error& e) {
        LOG_WARNING(
            "Importer: error when building relative basefile path: " + std::string(e.what()));
    }

    if(!std::filesystem::exists(baseFile)) {
        LOG_ERROR("Importer: base file for options doesn't exist: " + baseFile);
        return false;
    }

    const auto extension = StringOperations::GetExtension(baseFile);

    const auto type = GetTypeFromExtension(extension);

    if(type == FileType::Invalid) {
        LOG_ERROR("Importer: options base file has unknown type");
        return false;
    }

    std::string target;

    if(value["target"]) {
        target = GetTargetPath(value["target"].asString(), type);
    } else {
        target = GetTargetPath(baseFile, type);
    }

    // Merge folder and file options
    auto finalOptions = GetFolderOptionsForFile(baseFile);

    if(finalOptions) {
        for(const auto& key : value.getMemberNames()) {
            // TODO: recursive merge
            finalOptions.value()[key] = value[key];
        }


    } else {
        finalOptions = value;
    }

    return ImportTypedFile(baseFile, type, target, finalOptions, originalFile);
}
// ------------------------------------ //
std::optional<Json::Value> Importer::GetFolderOptionsForFile(const std::string& file) const
{
    const auto optionsFile =
        std::filesystem::path(file).parent_path() / FOLDER_OPTIONS_FILE_NAME;

    if(!std::filesystem::exists(optionsFile)) {
        return {};
    }

    std::ifstream reader(optionsFile.string());

    Json::CharReaderBuilder builder;
    Json::Value value;
    JSONCPP_STRING errs;

    if(!parseFromStream(builder, reader, &value, &errs))
        throw InvalidState("invalid folder options json:" + errs);

    return value;
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

    if(!strippedPath.empty() && StringOperations::IsCharacterPathSeparator(strippedPath[0]))
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
bool NeedImportTimestampHelper(const std::string& file, const std::string& target)
{
    return std::filesystem::last_write_time(file) > std::filesystem::last_write_time(target);
}

bool NeedImportHashHelper(const std::string& file, std::string& hash)
{
    std::string fileContents;
    if(!FileSystem::ReadFileEntirely(file, fileContents))
        throw InvalidArgument("Cannot read file: " + file + " for hash calculation");

    hash = MD5(fileContents).hexdigest();
    return true;
}


bool Importer::NeedsImporting(const std::string& file, const std::string& target,
    const Json::Value& options, std::string& hash)
{
    // If missing from cache, always import
    const auto key = GetCacheKeyForFile(file);

    try {
        if(!CacheData["files"].isMember(key))
            return NeedImportHashHelper(file, hash);
    } catch(const Json::Exception& e) {
        throw InvalidState("importer cache is corrupted: " + std::string(e.what()));
    }

    std::string oldOptions;
    std::string oldFile;

    try {
        const auto data = CacheData["files"][key];

        oldOptions = data["optionsHash"].asString();
        oldFile = data["fileHash"].asString();

    } catch(const Json::Exception& e) {
        LOG_ERROR(
            "Importer: cache entry for (" + file + ") is invalid, json error: " + e.what());
        return true;
    }

    // Exist check
    if(!std::filesystem::exists(target))
        return NeedImportHashHelper(file, hash);

    // Options hash overrides the timestamp
    const auto optionsHash = GetHashOfOptions(options);

    if(optionsHash != oldOptions)
        return true;

    // Timestamp check
    if(!NeedImportTimestampHelper(file, target))
        return false;

    // File hash check
    NeedImportHashHelper(file, hash);

    if(hash == oldFile) {
        LOG_INFO("File (" + file +
                 ") is newer than imported file, but hash and options are the same, "
                 "touching the "
                 "file to skip it in the future");

        std::filesystem::last_write_time(target, std::filesystem::last_write_time(file));

        return false;
    }

    return true;
}
// ------------------------------------ //
void Importer::CheckAndLoadCache()
{
    if(!std::filesystem::exists(InformationCacheFile)) {
        LOG_INFO("Importer cache file does not exist: " + InformationCacheFile);
        return;
    }

    std::ifstream file(InformationCacheFile);

    try {
        if(!file.good())
            throw InvalidAccess("can't read the cache file");

        Json::CharReaderBuilder builder;
        Json::Value value;
        JSONCPP_STRING errs;

        if(!parseFromStream(builder, file, &value, &errs))
            throw InvalidState("invalid json:" + errs);

        // Full paths are naturally different on different computers...
        // if(value["source"].asString() != Source ||
        //     value["destination"].asString() != Destination)
        //     throw Exception("cache has different source or destination, cannot use it");

        CacheData = value;

    } catch(const std::exception& e) {
        LOG_ERROR("Importer failed to load cache (" + InformationCacheFile +
                  ") due to exception: " + e.what());
    }
}

void Importer::SaveCache()
{
    std::ofstream file(InformationCacheFile);
    JsonWriter->write(CacheData, &file);
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
