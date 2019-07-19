// ------------------------------------ //
#include "FileSystem.h"
#include "Common/StringOperations.h"

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
#include "Exceptions.h"
#endif
#include "TimeIncludes.h"

#include <fstream>
#include <ostream>

#include <boost/filesystem.hpp>

#ifdef __linux__
#include <dirent.h>
#include <sys/stat.h>
#else
#endif
#ifdef _WIN32

#include "WindowsInclude.h"

#endif //_WIN32
#include <iosfwd>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
Leviathan::FileSystem::FileSystem()
{
    // set static access //
    Staticaccess = this;
    // set default values //
    CurrentFileExtID = 25;

    // index creation flags //
    IsAllIndexed = IsTextureIndexed = IsModelIndexed = IsSoundIndexed = IsScriptIndexed =
        IsSorted = IsBeingSorted = ShouldSortStop = false;
}

Leviathan::FileSystem::~FileSystem()
{

    // Helps catch errors with tests etc.
    if(Staticaccess == this)
        Staticaccess = nullptr;

    SAFE_DELETE_VECTOR(FileTypes);

    AllFiles.clear();

    TextureFiles.clear();
    ModelFiles.clear();
    SoundFiles.clear();
    ScriptFiles.clear();

    // Clear indexes //
    IsAllIndexed = false;
    SAFE_DELETE_VECTOR(AllIndexes);

    IsTextureIndexed = false;
    SAFE_DELETE_VECTOR(TextureIndexes);

    IsModelIndexed = false;
    SAFE_DELETE_VECTOR(ModelIndexes);

    IsSoundIndexed = false;
    SAFE_DELETE_VECTOR(SoundIndexes);

    IsScriptIndexed = false;
    SAFE_DELETE_VECTOR(ScriptIndexes);
}

DLLEXPORT FileSystem* FileSystem::Get()
{

    return Staticaccess;
}

string Leviathan::FileSystem::DataFolder = "./Data/";
string Leviathan::FileSystem::ModelsFolder = "Models/";
string Leviathan::FileSystem::ScriptsFolder = "Scripts/";
string Leviathan::FileSystem::ShaderFolder = "Shaders/";
string Leviathan::FileSystem::TextureFolder = "Textures/";
string Leviathan::FileSystem::MaterialFolder = "Materials/";
string Leviathan::FileSystem::FontFolder = "Fonts/";
string Leviathan::FileSystem::SoundFolder = "Sound/";

FileSystem* Leviathan::FileSystem::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::FileSystem::Init(LErrorReporter* errorreport)
{

    ErrorReporter = errorreport;

    IsSorted = false;

    // use find files function on data folder and then save results to appropriate vectors //
    vector<string> files;
#ifdef _WIN32
    GetFilesInDirectory(files, "./Data/");
#else
    GetFilesInDirectory(files, "./Data");
#endif

    if(files.size() < 1) {

        ErrorReporter->Error(
            std::string("FileSystem: SearchFiles: No files inside data folder, "
                        "cannot possibly work"));
        return false;
    }

    // save to appropriate places //
    for(size_t i = 0; i < files.size(); i++) {

        // create new object for storing this //
        auto tmpptr = make_shared<FileDefinitionType>(this, files[i]);

        if(files[i].find("./Data/Textures/") == 0) {

            // add to texture files //
            TextureFiles.push_back((tmpptr));

        } else if(files[i].find("./Data/Models/") == 0) {

            // add to texture files //
            ModelFiles.push_back(tmpptr);

        } else if(files[i].find("./Data/Sound/") == 0) {

            // add to texture files //
            SoundFiles.push_back(tmpptr);

        } else if(files[i].find("./Data/Scripts/") == 0) {

            // add to texture files //
            ScriptFiles.push_back(tmpptr);
        }

        // everything should be in AllFiles vector //
        AllFiles.push_back(tmpptr);
    }
    // print some info //
    ErrorReporter->Info("FileSystem: found " + Convert::ToString(AllFiles.size()) +
                        " files in Data folder with " + Convert::ToString(FileTypes.size()) +
                        " different types of extensions");

    // sort for quick finding //
    auto starttime = Time::GetTimeMicro64();
    CreateIndexesForVecs();

    auto elapsed = Time::GetTimeMicro64() - starttime;

    // print info //
    ErrorReporter->Info("FileSystem: vectors sorted and indexes created, took " +
                        Convert::ToString(elapsed) + " micro seconds");

    return true;
}

DLLEXPORT bool Leviathan::FileSystem::ReSearchFiles()
{

    // Reset values //
    CurrentFileExtID = 25;
    IsSorted = false;

    SAFE_DELETE_VECTOR(FileTypes);

    AllFiles.clear();

    TextureFiles.clear();
    ModelFiles.clear();
    SoundFiles.clear();
    ScriptFiles.clear();

    // Clear indexes //
    IsAllIndexed = false;
    SAFE_DELETE_VECTOR(AllIndexes);

    IsTextureIndexed = false;
    SAFE_DELETE_VECTOR(TextureIndexes);

    IsModelIndexed = false;
    SAFE_DELETE_VECTOR(ModelIndexes);

    IsSoundIndexed = false;
    SAFE_DELETE_VECTOR(SoundIndexes);

    IsScriptIndexed = false;
    SAFE_DELETE_VECTOR(ScriptIndexes);

    // Search again //
    return Init(ErrorReporter);
}
// ------------------------------------ //
string Leviathan::FileSystem::GetDataFolder()
{

    return (DataFolder);
}

string Leviathan::FileSystem::GetModelsFolder()
{

    return (DataFolder + ModelsFolder);
}

string Leviathan::FileSystem::GetScriptsFolder()
{

    return (DataFolder + ScriptsFolder);
}

string Leviathan::FileSystem::GetShaderFolder()
{

    return (DataFolder + ShaderFolder);
}

string Leviathan::FileSystem::GetTextureFolder()
{

    return (DataFolder + TextureFolder);
}

string Leviathan::FileSystem::GetFontFolder()
{

    return (DataFolder + FontFolder);
}

string Leviathan::FileSystem::GetSoundFolder()
{

    return (DataFolder + SoundFolder);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::FileSystem::DoesExtensionMatch(
    FileDefinitionType* file, const vector<int>& Ids)
{
    // check does file contain an extension id that is in the vector //
    for(size_t i = 0; i < Ids.size(); i++) {
        if(file->ExtensionID == Ids[i]) {
            // match found //
            return true;
        }
    }
    return false;
}

// ------------------------------------ //
#ifdef _WIN32
void Leviathan::FileSystem::GetWindowsFolder(wstring& path)
{
    wchar_t winddir[MAX_PATH];
    if(GetWindowsDirectoryW(winddir, MAX_PATH) > 0)
        path = winddir;
    if(path.back() != L'/')
        path += L'/';
}

void Leviathan::FileSystem::GetSpecialFolder(wstring& path, int specialtype)
{
    wchar_t directory[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, directory, specialtype, false);
    path = directory;
    if(path.back() != L'/')
        path += L'/';
}
#endif
// ------------------------------------ //
DLLEXPORT void Leviathan::FileSystem::SetDataFolder(const string& folder)
{

    DataFolder = folder;
}

void Leviathan::FileSystem::SetModelsFolder(const string& folder)
{

    ModelsFolder = folder;
}

void Leviathan::FileSystem::SetScriptsFolder(const string& folder)
{

    ScriptsFolder = folder;
}

void Leviathan::FileSystem::SetShaderFolder(const string& folder)
{

    ShaderFolder = folder;
}

void Leviathan::FileSystem::SetTextureFolder(const string& folder)
{

    TextureFolder = folder;
}
// ------------------ File handling ------------------ //
DLLEXPORT bool FileSystem::LoadDataDump(const string& file,
    vector<shared_ptr<NamedVariableList>>& vec, LErrorReporter* errorreport)
{
    // Get data //
    ifstream stream(file);

    if(!stream.good()) {
        // no file ! //
        errorreport->Error("FileSystem: LoadDataDump: Failed to read file: " + file);
        return false;
    }

    // count length //
    stream.seekg(0, ios::end);
    auto length = stream.tellg();
    stream.seekg(0, ios::beg);

    if(length == std::streampos(0)) {

        // empty file ! //
        return false;
    }

    // TODO: directly copy into the string
    unique_ptr<char[]> Buff(new char[(size_t)length + 1]);

    // set null terminator, just in case
    (Buff.get())[length] = '\0';

    stream.read(Buff.get(), length);

    stream.close();

    string filecontents = Buff.get();

    // Create values //
    return NamedVariableList::ProcessDataDump(filecontents, vec, errorreport);
}

#ifdef _WIN32
DLLEXPORT bool Leviathan::FileSystem::GetFilesInDirectory(vector<string>& files,
    const string& dirpath, const string& pattern, bool recursive /*= true*/)
{
    string FilePath;
    string Pattern;
    HANDLE hFile;
    WIN32_FIND_DATAA FileInfo;

    Pattern = dirpath + pattern;

    hFile = ::FindFirstFileA(Pattern.c_str(), &FileInfo);
    if(hFile != INVALID_HANDLE_VALUE) {
        do {

            if(FileInfo.cFileName[0] != '.') {
                FilePath.erase();
                FilePath = dirpath + FileInfo.cFileName + "/";

                if(FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                    if(recursive) {
                        // call self to search subdirectory
                        int retr = GetFilesInDirectory(files, FilePath, pattern, recursive);
                        if(!retr)
                            break; // failed //
                    }
                } else {
                    // save file
                    files.push_back(dirpath + FileInfo.cFileName);
                }
            }
        } while(::FindNextFileA(hFile, &FileInfo) == 1);

        // close handle //
        FindClose(hFile);
    }

    return true;
}
#else
DLLEXPORT bool Leviathan::FileSystem::GetFilesInDirectory(vector<string>& files,
    const string& dirpath, const string& pattern /*= "*.*"*/, bool recursive /*= true*/)
{
    dirent* ent;
    struct stat st;

    // Start searching //
    DIR* dir = opendir(dirpath.c_str());

    if(!dir) {

        // Non-existant directory
        return false;
    }

    while((ent = readdir(dir)) != NULL) {
        const string file_name = ent->d_name;

        // Ignore if starts with a '.' //
        if(file_name[0] == '.')
            continue;

        const string full_file_name = dirpath + "/" + file_name;

        // Get info to determine if it is a dirpath //
        if(stat(full_file_name.c_str(), &st) == -1)
            continue;

        // Check if it is a dirpath //
        if((st.st_mode & S_IFDIR) != 0) {
            // Go into dirpath if recursive search //
            if(recursive) {
                // \todo fix performance //
                GetFilesInDirectory(files, full_file_name, pattern, recursive);
            }
            continue;
        }

        files.push_back(full_file_name);
    }

    closedir(dir);

    return true;
}
#endif
// ------------------ File operations ------------------ //
size_t Leviathan::FileSystem::GetFileLength(const string& name)
{

    ifstream file(name, ios::binary);

    if(file.good()) {

        file.seekg(0, ios::end);
        auto returnval = file.tellg();
        file.close();
        return returnval;
    }

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
    throw InvalidArgument("Cannot determine file size it doesn't exist");
#else
    return 0;
#endif
}

DLLEXPORT bool Leviathan::FileSystem::FileExists(const string& name)
{
    return boost::filesystem::exists(name);
}

bool Leviathan::FileSystem::WriteToFile(const string& data, const string& filename)
{
    ofstream file(filename, ios::binary);
    if(file.is_open()) {

        file << data;

        file.close();
        return true;
    }

    file.close();
    return false;
}

bool Leviathan::FileSystem::WriteToFile(const wstring& data, const wstring& filename)
{
#ifdef _WIN32
    wofstream file(filename, ios::binary);
#else
    wofstream file(Convert::Utf16ToUtf8(filename), ios::binary);
#endif
    if(file.is_open()) {
        file << data;

        file.close();
        return true;
    }

    file.close();
    return false;
}

bool Leviathan::FileSystem::AppendToFile(const string& data, const string& filepath)
{

    ofstream file(filepath, ofstream::app | ios::binary);

    if(file.is_open()) {

        file << data;

        file.close();
        return true;
    }

    file.close();
    return false;
}

DLLEXPORT bool Leviathan::FileSystem::ReadFileEntirely(
    const wstring& file, wstring& resultreceiver)
{
#ifdef _WIN32
    wifstream reader(file, ios::in | ios::binary);
#else
    wifstream reader(Convert::Utf16ToUtf8(file), ios::in | ios::binary);
#endif
    if(reader) {

        // go to end to count length //
        reader.seekg(0, ios::end);

        streamoff rpos = reader.tellg();

        // cannot be loaded //
        LEVIATHAN_ASSERT(std::numeric_limits<streamoff>::max() >= rpos, "file is too large");

        resultreceiver.resize(static_cast<size_t>(rpos));
        // back to start //
        reader.seekg(0, ios::beg);
        // read the actual data //
        reader.read(&resultreceiver[0], resultreceiver.size());

        // done, cleanup //
        reader.close();
        return true;
    }

    return false;
}

DLLEXPORT bool Leviathan::FileSystem::ReadFileEntirely(
    const string& file, string& resultreceiver)
{

    ifstream reader(file, ios::in | ios::binary);

    if(reader) {

        // go to end to count length //
        reader.seekg(0, ios::end);

        streamoff rpos = reader.tellg();


        // cannot be loaded //
        LEVIATHAN_ASSERT(std::numeric_limits<streamoff>::max() >= rpos, "file is too large");

        resultreceiver.resize(static_cast<size_t>(rpos));
        // back to start //
        reader.seekg(0, ios::beg);
        // read the actual data //
        reader.read(&resultreceiver[0], resultreceiver.size());

        // done, cleanup //
        reader.close();
        return true;
    }

    return false;
}
// ------------------ Non static part ------------------ //
DLLEXPORT void Leviathan::FileSystem::SortFileVectors()
{
    // check if already sorted //
    if(IsSorted)
        return;

    ShouldSortStop = false;
    // call sort on the vectors //
    IsBeingSorted = true;

    // looping so that other thread can cancel the action before it is finished //
    for(int i = 0; i < 5; i++) {
        // switch on index and call std sort //
        switch(i) {
        case 0: sort(AllFiles.begin(), AllFiles.end(), FileDefSorter()); break;
        case 1: sort(TextureFiles.begin(), TextureFiles.end(), FileDefSorter()); break;
        case 2: sort(ModelFiles.begin(), ModelFiles.end(), FileDefSorter()); break;
        case 3: sort(SoundFiles.begin(), SoundFiles.end(), FileDefSorter()); break;
        case 4: sort(ScriptFiles.begin(), ScriptFiles.end(), FileDefSorter()); break;
        }

        // check for end
        if(ShouldSortStop) {
            // asked to stop //
            goto end;
        }
    }
    // sort done
    IsSorted = true;

end:
    IsBeingSorted = false;
}

DLLEXPORT void Leviathan::FileSystem::CreateIndexesForVecs(bool forcerecreation /*= false*/)
{
    // check are vectors sorted, if not call sort //
    if(!IsSorted) {

        SortFileVectors();
    }

    _CreateIndexesIfMissing(AllFiles, AllIndexes, IsAllIndexed, forcerecreation);
    _CreateIndexesIfMissing(TextureFiles, TextureIndexes, IsTextureIndexed, forcerecreation);
    _CreateIndexesIfMissing(ModelFiles, ModelIndexes, IsModelIndexed, forcerecreation);
    _CreateIndexesIfMissing(SoundFiles, SoundIndexes, IsSoundIndexed, forcerecreation);
    _CreateIndexesIfMissing(ScriptFiles, ScriptIndexes, IsScriptIndexed, forcerecreation);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::FileSystem::RegisterExtension(const string& extension)
{
    // check does it exist //
    for(size_t i = 0; i < FileTypes.size(); i++) {
        if(StringOperations::CompareInsensitive(FileTypes[i]->Name, extension))
            return FileTypes[i]->ID;
    }

    // add //
    CurrentFileExtID++;
    FileTypes.push_back(new FileTypeHolder(CurrentFileExtID, extension));

    return CurrentFileExtID;
}

void Leviathan::FileSystem::GetExtensionIDS(const string& extensions, vector<int>& ids)
{
    // generate info about the extensions //
    vector<string> Exts;
    StringOperations::CutString(extensions, string("|"), Exts);
    if(Exts.size() == 0) {
        // just one extension //
        ids.push_back(RegisterExtension(extensions));
        return;
    }

    for(size_t i = 0; i < Exts.size(); i++) {
        ids.push_back(RegisterExtension(Exts[i]));
    }
}

DLLEXPORT const string& Leviathan::FileSystem::GetExtensionName(int id) const
{
    // Look for it //
    for(size_t i = 0; i < FileTypes.size(); i++) {
        if(FileTypes[i]->ID == id)
            return FileTypes[i]->Name;
    }

    // Not found //
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
    throw NotFound("No extension corresponds with id");
#else
    LEVIATHAN_ASSERT(0, "No extension corresponds with id");
    return FileTypes[0]->Name;
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
}
// ------------------------------------ //
DLLEXPORT string Leviathan::FileSystem::SearchForFile(
    FILEGROUP which, const string& name, const string& extensions, bool searchall /*= true*/)
{
    if(name.empty())
        return "";

    // generate info about the search file //
    vector<int> ExtensionIDS;
    GetExtensionIDS(extensions, ExtensionIDS);

    switch(which) {
    case FILEGROUP_MODEL: {
        shared_ptr<FileDefinitionType> result =
            _SearchForFileInVec(ModelFiles, ExtensionIDS, name, IsModelIndexed, &ModelIndexes);
        if(result.get() != NULL) {
            // found //
            return result.get()->RelativePath;
        }
    } break;
    case FILEGROUP_TEXTURE: {
        shared_ptr<FileDefinitionType> result = _SearchForFileInVec(
            TextureFiles, ExtensionIDS, name, IsTextureIndexed, &TextureIndexes);

        if(result.get() != NULL) {
            // found //
            return result.get()->RelativePath;
        }
    } break;
    case FILEGROUP_SOUND: {
        shared_ptr<FileDefinitionType> result =
            _SearchForFileInVec(SoundFiles, ExtensionIDS, name, IsSoundIndexed, &SoundIndexes);
        if(result.get() != NULL) {
            // found //
            return result.get()->RelativePath;
        }
    } break;
    case FILEGROUP_SCRIPT: {
        shared_ptr<FileDefinitionType> result = _SearchForFileInVec(
            ScriptFiles, ExtensionIDS, name, IsScriptIndexed, &ScriptIndexes);
        if(result.get() != NULL) {
            // found //
            return result.get()->RelativePath;
        }
    } break;
    case FILEGROUP_OTHER: {
        shared_ptr<FileDefinitionType> result =
            _SearchForFileInVec(AllFiles, ExtensionIDS, name, IsAllIndexed, &AllIndexes);
        if(result.get() != NULL) {
            // found //
            return result.get()->RelativePath;
        }
    } break;
    }


    // still not found, if searchall specified search all files vector //
    if(searchall) {
        shared_ptr<FileDefinitionType> result =
            _SearchForFileInVec(AllFiles, ExtensionIDS, name, IsAllIndexed, &AllIndexes);
        if(result.get() != NULL) {
            // found //
            return result.get()->RelativePath;
        }
    }
    // not found return empty and if debug build warn //

    ErrorReporter->Error("FileSystem: File not found: " + name + "." + extensions);

    return "";
}

DLLEXPORT vector<shared_ptr<FileDefinitionType>> Leviathan::FileSystem::FindAllMatchingFiles(
    FILEGROUP which, const string& regexname, const string& extensions,
    bool searchall /*= true*/)
{
    // generate info about the search file //
    vector<int> ExtensionIDS;
    GetExtensionIDS(extensions, ExtensionIDS);

    // create regex //
    regex usedregex(regexname, regex_constants::ECMAScript | regex_constants::icase);

    vector<shared_ptr<FileDefinitionType>> foundfiles;

    if(searchall) {

        _SearchForFilesInVec(AllFiles, foundfiles, ExtensionIDS, usedregex);

    } else {
        // specific vector //
        vector<shared_ptr<FileDefinitionType>>* targetvector = NULL;

        switch(which) {
        case FILEGROUP_MODEL: {
            targetvector = &ModelFiles;
        } break;
        case FILEGROUP_TEXTURE: {
            targetvector = &TextureFiles;
        } break;
        case FILEGROUP_SOUND: {
            targetvector = &SoundFiles;
        } break;
        case FILEGROUP_SCRIPT: {
            targetvector = &ScriptFiles;
        } break;
        case FILEGROUP_OTHER: {
            targetvector = &AllFiles;
        } break;
        }

        _SearchForFilesInVec(*targetvector, foundfiles, ExtensionIDS, usedregex);
    }

    // return what we found //
    return foundfiles;
}


// ------------------------------------ //
vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetModelFiles()
{
    return ModelFiles;
}

vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetSoundFiles()
{
    return SoundFiles;
}

vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetAllFiles()
{
    return AllFiles;
}

vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetScriptFiles()
{
    return ScriptFiles;
}
// ------------------------------------ //
shared_ptr<FileDefinitionType> Leviathan::FileSystem::_SearchForFileInVec(
    vector<shared_ptr<FileDefinitionType>>& vec, vector<int>& extensions, const string& name,
    bool UseIndexVector, vector<CharWithIndex*>* Index)
{
    size_t StartSpot = 0;

    // use index to get start spot for faster searching //
    if(UseIndexVector) {
        char startchar = name[0];
        bool Found = false;

        // find matching char //
        for(unsigned int i = 0; i < Index->size(); i++) {
            if(Index->at(i)->Char == startchar) {
                Found = true;
                StartSpot = Index->at(i)->Index;
                break;
            }
        }
        // if character that starts the word wasn't found it can't be there, exit function //
        if(!Found)
            return NULL;
    }

    for(size_t i = StartSpot; i < vec.size(); i++) {
        // if no extension specified skip checking them //
        if(extensions.size() > 0) {
            // check does extension(s) match //
            if(!DoesExtensionMatch(vec[i].get(), extensions))
                continue;
        }
        // extensions match check name //
        if((vec[i]->Name != name))
            continue;

        // match //
        return vec[i];
    }
    // nothing //
    return NULL;
}

void Leviathan::FileSystem::_SearchForFilesInVec(vector<shared_ptr<FileDefinitionType>>& vec,
    vector<shared_ptr<FileDefinitionType>>& results, vector<int>& extensions,
    const regex& regex)
{
    for(size_t i = 0; i < vec.size(); i++) {
        // if no extension specified skip checking them //
        if(extensions.size() > 0) {
            // check does extension(s) match //
            if(!DoesExtensionMatch(vec[i].get(), extensions)) {
                continue;
            }
        }
        // extensions match check name //
        if(!regex_match(vec[i]->Name, regex)) {
            continue;
        }

        // match //
        results.push_back(vec[i]);
    }
}

void Leviathan::FileSystem::_CreateIndexesIfMissing(
    vector<shared_ptr<FileDefinitionType>>& vec, vector<CharWithIndex*>& resultvec,
    bool& indexed, const bool& force /*= false*/)
{
    // we'll need to delete old ones if index creation is forced //
    if(force) {
        indexed = false;
        SAFE_DELETE_VECTOR(resultvec);
    }
    // if they are valid we can just return //
    if(indexed)
        return;

    // now that the file vector is sorted we can loop through it and every time first character
    // changes add it to index
    char curchar = '!';

    for(size_t i = 0; i < vec.size(); i++) {
        if(vec[i]->Name[0] != curchar) {
            // beginning character changed, push to indexes //
            curchar = vec[i]->Name[0];
            resultvec.push_back(new CharWithIndex(curchar, i));
        }
    }

    // done //
    indexed = true;
}
// ------------------ FileDefinitionType ------------------ //
Leviathan::FileDefinitionType::FileDefinitionType(FileSystem* instance, const string& path) :
    RelativePath(path)
{
    // get extension //
    string tempexpt = StringOperations::GetExtension<std::string>(path);

    // register extension and store id //
    ExtensionID = instance->RegisterExtension(tempexpt);

    // save name //
    Name = StringOperations::RemoveExtension<std::string>(path, true);
}

bool Leviathan::FileDefinitionType::operator<(const FileDefinitionType& other) const
{
    return this->Name < other.Name;
}

Leviathan::FileDefinitionType::~FileDefinitionType() {}

std::string Leviathan::FileDefinitionType::GetNameWithExtension() const
{
    // Add the extension text to the end of the name //
    return Name + "." + FileSystem::Get()->GetExtensionName(ExtensionID);
}
// ------------------ FileDefSorter ------------------ //
bool Leviathan::FileDefSorter::operator()(const std::shared_ptr<FileDefinitionType>& first,
    const std::shared_ptr<FileDefinitionType>& second)
{
    return (*first.get()) < *(second).get();
}
// ------------------ CharWithIndex ------------------ //
CharWithIndex::CharWithIndex()
{
    Char = L' ';
    Index = -1;
}

CharWithIndex::CharWithIndex(char character, size_t index) : Char(character), Index(index) {}
// ------------------ FileTypeHolder ------------------ //
FileTypeHolder::FileTypeHolder(int id, const std::string& name) : ID(id), Name(name) {}
