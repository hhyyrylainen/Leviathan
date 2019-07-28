// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "json/json.h"

#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "bsfCore/Importer/BsMeshImportOptions.h"
#include "bsfCore/Importer/BsShaderImportOptions.h"
#include "bsfCore/Importer/BsTextureImportOptions.h"


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
        return bs::SPtr<OptionsType>(new OptionsType);
        // This doesn't exist for some reason
        // return bs::ShaderImportOptions::create();
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

    std::string GetTargetPath(const std::string& file, FileType type) const;
    std::string GetTargetWithoutSingleCheck(const std::string& file, FileType type) const;

    static const char* GetSubFolderForType(FileType type);
    static FileType GetTypeFromExtension(const std::string& extension);

protected:
    bool ImportFile(const std::string& file);

    bool ImportTypedFile(const std::string& file, FileType type);
    bool ImportWithOptions(const std::string& optionsfile);

    //! \brief Compares file timestamps to check if file needs to be imported
    bool NeedsImporting(const std::string& file, const std::string& target);

    //! \brief Compares file timestamps to check if file needs to be imported
    bool NeedsImporting(
        const std::string& file, const std::string& optionsfile, const std::string& target);

private:
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

        auto resource = bs::static_resource_cast<T>(resources[resourceCounter].value);

        if(resource) {

            bs::gResources().save(resource, targets[resourceCounter].c_str(), true, Compress);

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
        if(resourceCounter + 1 < targets.size()) {
            LOG_WARNING("Importer: excess targets provided for multi resource import");
        }

        return MultiResourceSaveSingle<T>(resources, resourceCounter, targets);
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
        }

        // Non-multi import
        return ImportAndSaveFile<T>(file, target, importOptions);
    }

protected:
    std::string Source;
    std::string Destination;

    int ImportedFiles = 0;

    bool TargetIsFile = false;
    bool SourceIsFile = false;

    bool Compress = true;
};

}} // namespace Leviathan::Editor
