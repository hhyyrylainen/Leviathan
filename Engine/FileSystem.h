// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Common/ThreadSafe.h"

#include "ErrorReporter.h"
#include <regex>


namespace Leviathan {

enum FILEGROUP {
    FILEGROUP_MODEL,
    FILEGROUP_TEXTURE,
    FILEGROUP_SOUND,
    FILEGROUP_SCRIPT,
    FILEGROUP_OTHER
};

//! \brief Helper class for indexing
class CharWithIndex {
public:
    CharWithIndex();
    CharWithIndex(char character, size_t index);

    char Char;
    size_t Index;
};

//! \brief File type
class FileTypeHolder {
public:
    FileTypeHolder(int id, const std::string& name);

    int ID;
    std::string Name;
};


struct FileDefinitionType {
    // just the path, everything else is worked out by the constructor //
    FileDefinitionType(FileSystem* instance, const std::string& path);
    ~FileDefinitionType();

    //! \return The name in this format: "file.extension"
    std::string GetNameWithExtension() const;

    // operation for sorting //
    bool operator<(const FileDefinitionType& other) const;

    std::string RelativePath;
    std::string Name;
    int ExtensionID;
};

struct FileDefSorter {
    DLLEXPORT bool operator()(const std::shared_ptr<FileDefinitionType>& first,
        const std::shared_ptr<FileDefinitionType>& second);
};

//! \brief Class for indexing and searching game data directory
class FileSystem {
public:
    DLLEXPORT FileSystem();
    DLLEXPORT ~FileSystem();

    //! \brief Runs the indexing and sorting
    DLLEXPORT bool Init(LErrorReporter* errorreport);

    //! \brief Destroys the current index and recreates it
    DLLEXPORT bool ReSearchFiles();

    DLLEXPORT void SortFileVectors();
    DLLEXPORT void CreateIndexesForVecs(bool forcerecreation = false);

    //! \brief Reserves a number for an extension string
    DLLEXPORT int RegisterExtension(const std::string& extension);

    //! \brief Returns list of matching ids to the extensions
    //! \param extensions '|' separated list of extensions, example: "png|jpg|txt"
    DLLEXPORT void GetExtensionIDS(const std::string& extensions, std::vector<int>& ids);

    //! \brief Retrieves the name of the extension from the id
    //! \exception NotFound when extension with id not registered
    DLLEXPORT const std::string& GetExtensionName(int id) const;


    //! \brief Searches for a file
    //! \return Full path to the file or an empty string if not found
    DLLEXPORT std::string SearchForFile(FILEGROUP which, const std::string& name,
        const std::string& extensions, bool searchall = true);

    //! \brief Returns all matching files
    DLLEXPORT std::vector<std::shared_ptr<FileDefinitionType>> FindAllMatchingFiles(
        FILEGROUP which, const std::string& regexname, const std::string& extensions,
        bool searchall = true);

    // direct access to files //
    DLLEXPORT std::vector<std::shared_ptr<FileDefinitionType>>& GetModelFiles();
    DLLEXPORT std::vector<std::shared_ptr<FileDefinitionType>>& GetSoundFiles();
    DLLEXPORT std::vector<std::shared_ptr<FileDefinitionType>>& GetAllFiles();
    DLLEXPORT std::vector<std::shared_ptr<FileDefinitionType>>& GetScriptFiles();
    // ------------------ Static part ------------------ //

    DLLEXPORT static std::string GetDataFolder();
    DLLEXPORT static std::string GetModelsFolder();
    DLLEXPORT static std::string GetScriptsFolder();
    DLLEXPORT static std::string GetShaderFolder();
    DLLEXPORT static std::string GetTextureFolder();
    DLLEXPORT static std::string GetFontFolder();
    DLLEXPORT static std::string GetSoundFolder();

    DLLEXPORT static bool DoesExtensionMatch(
        FileDefinitionType* file, const std::vector<int>& Ids);

#ifdef _WIN32
    DLLEXPORT static void GetWindowsFolder(std::wstring& path);
    DLLEXPORT static void GetSpecialFolder(std::wstring& path, int specialtype);
#endif

    DLLEXPORT static void SetDataFolder(const std::string& folder);
    DLLEXPORT static void SetModelsFolder(const std::string& folder);
    DLLEXPORT static void SetScriptsFolder(const std::string& folder);
    DLLEXPORT static void SetShaderFolder(const std::string& folder);
    DLLEXPORT static void SetTextureFolder(const std::string& folder);

    // file handling //
    DLLEXPORT static bool LoadDataDump(const std::string& file,
        std::vector<std::shared_ptr<NamedVariableList>>& vec, LErrorReporter* errorreport);

    // Warning: \todo linux version ignores the defined pattern //
    DLLEXPORT static bool GetFilesInDirectory(std::vector<std::string>& files,
        const std::string& dirpath, const std::string& pattern = "*.*", bool recursive = true);

    // file operations //
    DLLEXPORT static size_t GetFileLength(const std::string& name);
    DLLEXPORT static bool FileExists(const std::string& name);
    DLLEXPORT static bool WriteToFile(const std::string& data, const std::string& filename);
    DLLEXPORT static bool WriteToFile(const std::wstring& data, const std::wstring& filename);
    DLLEXPORT static bool AppendToFile(const std::string& data, const std::string& filepath);

    DLLEXPORT static bool ReadFileEntirely(
        const std::string& file, std::string& resultreceiver);

    DLLEXPORT static bool ReadFileEntirely(
        const std::wstring& file, std::wstring& resultreceiver);


    DLLEXPORT static FileSystem* Get();

private:
    // file search functions //
    std::shared_ptr<FileDefinitionType> _SearchForFileInVec(
        std::vector<std::shared_ptr<FileDefinitionType>>& vec, std::vector<int>& extensions,
        const std::string& name, bool UseIndexVector, std::vector<CharWithIndex*>* Index);

    void _SearchForFilesInVec(std::vector<std::shared_ptr<FileDefinitionType>>& vec,
        std::vector<std::shared_ptr<FileDefinitionType>>& results,
        std::vector<int>& extensions, const std::regex& regex);

    void _CreateIndexesIfMissing(std::vector<std::shared_ptr<FileDefinitionType>>& vec,
        std::vector<CharWithIndex*>& resultvec, bool& indexed, const bool& force /*= false*/);

    // ------------------------------------ //
    // vector that holds string value of file extension and it's id code //
    std::vector<FileTypeHolder*> FileTypes;
    int CurrentFileExtID;

    // file holders //
    std::vector<std::shared_ptr<FileDefinitionType>> AllFiles;

    std::vector<std::shared_ptr<FileDefinitionType>> TextureFiles;
    std::vector<std::shared_ptr<FileDefinitionType>> ModelFiles;
    std::vector<std::shared_ptr<FileDefinitionType>> SoundFiles;
    std::vector<std::shared_ptr<FileDefinitionType>> ScriptFiles;

    // index vectors //
    bool IsAllIndexed;
    std::vector<CharWithIndex*> AllIndexes;

    bool IsTextureIndexed;
    std::vector<CharWithIndex*> TextureIndexes;

    bool IsModelIndexed;
    std::vector<CharWithIndex*> ModelIndexes;

    bool IsSoundIndexed;
    std::vector<CharWithIndex*> SoundIndexes;

    bool IsScriptIndexed;
    std::vector<CharWithIndex*> ScriptIndexes;

    // vector sorting //
    bool IsSorted;
    bool IsBeingSorted;
    bool ShouldSortStop;

    LErrorReporter* ErrorReporter = nullptr;

    // ------------------------------------ //
    static std::string DataFolder;
    static std::string ModelsFolder;
    static std::string ScriptsFolder;
    static std::string ShaderFolder;
    static std::string TextureFolder;
    static std::string MaterialFolder;
    static std::string FontFolder;
    static std::string SoundFolder;

    static FileSystem* Staticaccess;
};

} // namespace Leviathan
