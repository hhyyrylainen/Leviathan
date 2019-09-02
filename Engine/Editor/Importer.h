// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/StringOperations.h"

#include "json/json.h"

#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "bsfCore/Importer/BsMeshImportOptions.h"
#include "bsfCore/Importer/BsShaderImportOptions.h"
#include "bsfCore/Importer/BsTextureImportOptions.h"
#include "bsfCore/Mesh/BsMesh.h"
#include "bsfUtility/Reflection/BsRTTIType.h"

#include <optional>

namespace Leviathan { namespace Editor {

template<class T>
struct OptionsCreator {};

template<>
struct OptionsCreator<bs::Mesh> {
    using OptionsType = bs::MeshImportOptions;

    static auto Create()
    {
        return OptionsType::create();
    }
};

template<>
struct OptionsCreator<bs::Shader> {
    using OptionsType = bs::ShaderImportOptions;

    static auto Create()
    {
        return OptionsType::create();
    }
};

template<>
struct OptionsCreator<bs::Texture> {
    using OptionsType = bs::TextureImportOptions;

    static auto Create()
    {
        return OptionsType::create();
    }
};


//! \brief Handles importing resources
class Importer {
    enum class FileType { Texture, Shader, Model, Invalid };

public:
    Importer(const std::string& source, const std::string& destination);
    ~Importer();

    //! \brief Runs the import
    //! \returns False on failure
    bool Run();

    //! \returns Folder level settings if exists
    std::optional<Json::Value> GetFolderOptionsForFile(const std::string& file) const;

    //! \returns A cache key for a input file
    //!
    //! This basically strips the source path from the file. This is so that on different
    //! computers the cache works
    std::string GetCacheKeyForFile(const std::string& file) const;

    std::string GetTargetPath(const std::string& file, FileType type) const;
    std::string GetTargetWithoutSingleCheck(const std::string& file, FileType type) const;

    //! \returns A hash of options
    std::string GetHashOfOptions(const Json::Value& options) const;

    Json::Value GetCacheStructure() const;

    static const char* GetSubFolderForType(FileType type);
    static FileType GetTypeFromExtension(const std::string& extension);

protected:
    //! Helper for passing the file to the proper function for handling
    bool ImportFile(const std::string& file);

    //! Handles calling the right template method for type
    //! \param originalfile is used for the cache. Specify if there is an options file with an
    //! overridden base file
    bool ImportTypedFile(const std::string& file, FileType type,
        std::optional<std::string> target, std::optional<Json::Value> options,
        std::optional<std::string> originalfile);

    //! Called for files that have a matching options file
    bool ImportWithOptions(const std::string& optionsfile);

    //! \brief Compares file timestamps and cache data to check if file needs to be imported
    //! \param hash If the timestamps were compared, this contains the hash of file
    bool NeedsImporting(const std::string& file, const std::string& target,
        const Json::Value& options, std::string& hash);

    void CheckAndLoadCache();
    void SaveCache();



private:
    //! Prefer calling ImportAndSaveWithOptions
    template<class T>
    bool ImportAndSaveFile(const std::string& file, const std::string& target,
        bs::SPtr<typename OptionsCreator<T>::OptionsType> options =
            OptionsCreator<T>::Create())
    {
        auto resource = bs::gImporter().import<T>(file.c_str(), options);

        if(resource) {

            bs::gResources().save(resource, target.c_str(), true, Compress);
            return true;
        }

        return false;
    }

    template<class T>
    bool MultiResourceSaveSingle(const bs::Vector<bs::SubResource>& resources,
        size_t& resourceCounter, const std::vector<std::string>& targets)
    {
        if(resourceCounter >= resources.size()) {
            LOG_ERROR("Importer: ran out of resources in multi resource import");
            return false;
        }

        if(resourceCounter >= targets.size()) {
            LOG_ERROR("Importer: ran out of targets in multi resource import");
            return false;
        }

        if(!bs::rtti_is_of_type<T>(resources[resourceCounter].value.get())) {
            LOG_ERROR("Importer: multi import resource failed at index: " +
                      std::to_string(resourceCounter) +
                      ", is the wrong type, name: " + resources[resourceCounter].name.c_str());
            return false;
        }

        auto resource = bs::static_resource_cast<T>(resources[resourceCounter].value);

        if(resource) {

            // Name replace
            if(targets[resourceCounter].find("$NAME") != std::string::npos) {

                bs::gResources().save(resource,
                    StringOperations::Replace<std::string>(targets[resourceCounter], "$NAME",
                        resources[resourceCounter].name.c_str())
                        .c_str(),
                    true, Compress);
            } else {

                bs::gResources().save(
                    resource, targets[resourceCounter].c_str(), true, Compress);
            }

        } else {
            LOG_ERROR("Importer: converting multi import resource failed, index: " +
                      std::to_string(resourceCounter) +
                      ", name: " + resources[resourceCounter].name.c_str());
            return false;
        }

        ++resourceCounter;
        return true;
    }

    template<class T>
    bool MultiResourceSaveHelper(const bs::Vector<bs::SubResource>& resources,
        size_t& resourceCounter, const std::vector<std::string>& targets)
    {
        bool ran = false;

        // The last resource type consumes all the rest of the targets
        while(resourceCounter < targets.size()) {
            ran = true;

            if(!MultiResourceSaveSingle<T>(resources, resourceCounter, targets))
                return false;
        }

        return ran;
    }

    template<class T, class SecondT, class... ExtraT>
    bool MultiResourceSaveHelper(const bs::Vector<bs::SubResource>& resources,
        size_t& resourceCounter, const std::vector<std::string>& targets)
    {
        if(!MultiResourceSaveSingle<T>(resources, resourceCounter, targets))
            return false;

        return MultiResourceSaveHelper<SecondT, ExtraT...>(
            resources, resourceCounter, targets);
    }

    template<class T, class... AdditionalTypes>
    bool ImportMultiResource(const std::string& file, const std::vector<std::string>& targets,
        bs::SPtr<typename OptionsCreator<T>::OptionsType> options)
    {
        auto resources = bs::gImporter().importAll(file.c_str(), options);

        if(resources->entries.size() == 0)
            return false;

        size_t resourceCounter = 0;

        return MultiResourceSaveHelper<T, AdditionalTypes...>(
            resources->entries, resourceCounter, targets);
    }

    template<class T>
    bool ImportAndSaveWithOptions(
        const std::string& file, const std::string& target, const Json::Value& options)
    {
        auto importOptions = OptionsCreator<T>::Create();

        std::vector<std::string> targets;
        targets.push_back(target);

        // Check for options and perform multi-imports
        if constexpr(std::is_same_v<T, bs::Mesh>) {
            if(options["animation"]) {

                importOptions->importAnimation = true;
                importOptions->reduceKeyFrames = true;
                bool valid = false;

                for(const auto& animation : options["animation"]) {

                    // TODO: error checking
                    const auto type = animation["type"].asString();
                    const auto target = Importer::GetTargetWithoutSingleCheck(
                        animation["target"].asString(), Importer::FileType::Model);

                    targets.push_back(target);

                    if(type == "skin") {
                        importOptions->importSkin = true;

                    } else {
                        LOG_ERROR("Importer: unknown animation type: " + type);
                        return false;
                    }

                    // importOptions->setImportBlendShapes(true);

                    LOG_INFO("Animation target is '" + target + "'");
                    valid = true;
                }


                if(!valid) {
                    LOG_ERROR("Importer: animation definition is invalid");
                    return false;
                }

                return ImportMultiResource<T, bs::AnimationClip>(file, targets, importOptions);
            }
        } else if constexpr(std::is_same_v<T, bs::Texture>) {
            // TODO: texture mipmaps

            if(options["cubemap"]) {

                importOptions->cubemap = true;

                if(options["cubemap"]["type"]) {
                    const auto type = options["cubemap"]["type"].asString();

                    if(type == "single") {

                        importOptions->cubemapSourceType = bs::CubemapSourceType::Single;

                    } else if(type == "cylindrical") {

                        importOptions->cubemapSourceType = bs::CubemapSourceType::Cylindrical;

                    } else if(type == "faces") {

                        importOptions->cubemapSourceType = bs::CubemapSourceType::Faces;

                    } else if(type == "spherical") {

                        importOptions->cubemapSourceType = bs::CubemapSourceType::Spherical;

                    } else {
                        LOG_ERROR("Importer: unknown cubemap type: " + type);
                        return false;
                    }
                }
            }

            if(options["pixelFormat"]) {

                const auto selectedFormat = options["pixelFormat"].asString();

                // TODO: rest of the pixel formats, this should also probably be moved to some
                // other file
                if(selectedFormat == "RGBA8") {
                    // Default format
                    importOptions->format = bs::PixelFormat::PF_RGBA8;
                } else if(selectedFormat == "RGB8") {

                    importOptions->format = bs::PixelFormat::PF_RGB8;
                } else if(selectedFormat == "BC1") {

                    importOptions->format = bs::PixelFormat::PF_BC1;
                } else if(selectedFormat == "BC2") {

                    importOptions->format = bs::PixelFormat::PF_BC2;
                } else if(selectedFormat == "BC3") {

                    // Better version of BC2.
                    importOptions->format = bs::PixelFormat::PF_BC3;
                } else if(selectedFormat == "BC7") {

                    // Higher decompress overhead
                    importOptions->format = bs::PixelFormat::PF_BC7;
                } else if(selectedFormat == "RG11B10F") {

                    // For skyboxes
                    importOptions->format = bs::PixelFormat::PF_RG11B10F;
                } else {
                    LOG_ERROR("Importer: unknown pixel format: " + selectedFormat);
                    return false;
                }
            }
        }

        // Non-multi import
        return ImportAndSaveFile<T>(file, target, importOptions);
    }

protected:
    std::string Source;
    std::string Destination;

    //! Stores things like file hashes and options used to skip unneeded imports
    std::string InformationCacheFile;
    Json::Value CacheData;

    std::unique_ptr<Json::StreamWriter> JsonWriter;
    std::unique_ptr<Json::StreamWriter> PrettyWriter;

    int ImportedFiles = 0;

    bool TargetIsFile = false;
    bool SourceIsFile = false;

    bool Compress = true;
};

}} // namespace Leviathan::Editor
