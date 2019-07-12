// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

namespace Leviathan { namespace Editor {

//! \brief Handles importing resources
class Importer {
    enum class FileType { Texture, Shader };

public:
    Importer(const std::string& source, const std::string& destination);
    ~Importer();

    //! \brief Runs the import
    //! \returns False on failure
    bool Run();

    std::string GetTargetPath(const std::string& file, FileType type) const;

    static const char* GetSubFolderForType(FileType type);

protected:
    bool ImportFile(const std::string& file);

    bool ImportTypedFile(const std::string& file, FileType type);

    //! \brief Compares file timestamps to check if file needs to be imported
    bool NeedsImporting(const std::string& file, const std::string& target);

protected:
    std::string Source;
    std::string Destination;

    int ImportedFiles = 0;

    bool TargetIsFile = false;

    bool Compress = true;
};

}} // namespace Leviathan::Editor
