#ifndef LEVIATHAN_FILESYSTEM
#define LEVIATHAN_FILESYSTEM
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common\DataStoring\NamedVars.h"
#include "Exceptions\ExceptionInvalidArguement.h"

namespace Leviathan{

	enum FILEGROUP{FILEGROUP_MODEL, FILEGROUP_TEXTURE, FILEGROUP_SOUND, FILEGROUP_SCRIPT, FILEGROUP_OTHER};

	class FileSystem;

	struct FileDefinitionType{
		FileDefinitionType(FileSystem* instance, const wstring &path); // just the path, everything else is worked out by the constructor //
		~FileDefinitionType();

		// operation for sorting //
		bool operator < (const FileDefinitionType& other) const;

		wstring RelativePath;
		wstring Name;
		int ExtensionID;

	};

	struct FileDefSorter{
		DLLEXPORT bool operator()(shared_ptr<FileDefinitionType>& first, shared_ptr<FileDefinitionType>& second);
	};

	class FileSystem{
	public:
		//// private constructors, to prevent creation of this class //
		DLLEXPORT FileSystem();
		DLLEXPORT ~FileSystem();


		DLLEXPORT bool Init();
		DLLEXPORT void Release();
		DLLEXPORT bool ReSearchFiles();

		// vector sorting //
		DLLEXPORT void SortFileVectors();
		DLLEXPORT void CreateIndexesForVecs(bool ForceRe = false);

		DLLEXPORT int RegisterExtension(const wstring &extension);
		DLLEXPORT void GetExtensionIDS(const wstring& extensions, vector<int>& ids);
		

		// loaded file searching functions //
		DLLEXPORT wstring& SearchForFile(FILEGROUP which, const wstring& name, const wstring& extensions, bool searchall = true);

		DLLEXPORT vector<shared_ptr<FileDefinitionType>> FindAllMatchingFiles(FILEGROUP which, const wstring& regexname, const wstring &extensions,
			bool searchall = true);

		// direct access to files //
		DLLEXPORT vector<shared_ptr<FileDefinitionType>>& GetModelFiles();
		DLLEXPORT vector<shared_ptr<FileDefinitionType>>& GetSoundFiles();
		DLLEXPORT vector<shared_ptr<FileDefinitionType>>& GetAllFiles();
		DLLEXPORT vector<shared_ptr<FileDefinitionType>>& GetScriptFiles();
		// ------------------ Static part ------------------ //
		DLLEXPORT static bool OperatingOnVista();
		DLLEXPORT static bool OperatingOnXP();
		
		
		DLLEXPORT static wstring& GetDataFolder();
		DLLEXPORT static wstring GetModelsFolder();
		DLLEXPORT static wstring GetScriptsFolder();
		DLLEXPORT static wstring GetShaderFolder();
		DLLEXPORT static wstring GetTextureFolder();
		DLLEXPORT static wstring GetFontFolder();
		DLLEXPORT static wstring GetSoundFolder();

		DLLEXPORT static void RegisterOGREResourceGroups();
		DLLEXPORT static void RegisterOGREResourceLocation(const string &location);

		DLLEXPORT static bool DoesExtensionMatch(FileDefinitionType* file, const vector<int>&Ids);

		DLLEXPORT static void GetWindowsFolder(wstring &path);
		DLLEXPORT static void GetSpecialFolder(wstring &path, int specialtype);

		DLLEXPORT static void SetDataFolder(const wstring &folder);
		DLLEXPORT static void SetModelsFolder(const wstring &folder);
		DLLEXPORT static void SetScriptsFolder(const wstring &folder);
		DLLEXPORT static void SetShaderFolder(const wstring &folder);
		DLLEXPORT static void SetTextureFolder(const wstring &folder);

		// file handling //
		DLLEXPORT static int LoadDataDump(const wstring &file, vector<shared_ptr<NamedVariableList>>& vec);
		DLLEXPORT static bool GetFilesInDirectory(vector<wstring> &files, wstring dirpath, wstring pattern = L"*.*", bool recursive = true);

		// extension handling //
		DLLEXPORT static wstring GetExtension(const wstring &path);
		DLLEXPORT static wstring ChangeExtension(const wstring& path, const wstring &newext);
		DLLEXPORT static wstring RemoveExtension(const wstring &file, bool delpath);
		DLLEXPORT static string RemovePath(const string &filepath);

		// file operations //
		DLLEXPORT static int GetFileLength(wstring name);
		DLLEXPORT static bool FileExists(const wstring &name);
		DLLEXPORT static bool FileExists(const string &name);
		DLLEXPORT static bool WriteToFile(const string &data, const string &filename);
		DLLEXPORT static bool WriteToFile(const wstring &data, const wstring &filename);
		DLLEXPORT static bool AppendToFile(const wstring &data, const wstring &filepath);
		DLLEXPORT static void ReadFileEntirely(const wstring &file, wstring &resultreceiver) throw(...);

		// bitmap stuff //
		DLLEXPORT static BYTE* LoadBMP ( int* width, int* height, long* size, LPCTSTR bmpfile );
		DLLEXPORT static BYTE* ConvertBMPToRGBBuffer ( BYTE* Buffer, int width, int height );

		DLLEXPORT static inline FileSystem* Get(){
			return Staticaccess;
		}

	private:
		// file search functions //
		shared_ptr<FileDefinitionType> _SearchForFileInVec(vector<shared_ptr<FileDefinitionType>> &vec, vector<int> &extensions, 
			const wstring &name, bool UseIndexVector, vector<CharWithIndex*>* Index);
		void _SearchForFilesInVec(vector<shared_ptr<FileDefinitionType>>& vec, vector<shared_ptr<FileDefinitionType>>& results, 
			vector<int>& extensions, const basic_regex<wchar_t> &regex);
		void _CreateIndexesIfMissing(vector<shared_ptr<FileDefinitionType>> &vec, vector<CharWithIndex*> &resultvec, bool &indexed, 
			const bool &force /*= false*/);
		// ------------------------------------ //
		// vector that holds string value of file extension and it's id code //
		vector<IntWstring*> FileTypes;
		int CurrentFileExtID;

		// file holders //
		vector<shared_ptr<FileDefinitionType>> AllFiles;

		vector<shared_ptr<FileDefinitionType>> TextureFiles;
		vector<shared_ptr<FileDefinitionType>> ModelFiles;
		vector<shared_ptr<FileDefinitionType>> SoundFiles;
		vector<shared_ptr<FileDefinitionType>> ScriptFiles;

		// index vectors //
		bool IsAllIndexed;
		vector<CharWithIndex*> AllIndexes;
		
		bool IsTextureIndexed;
		vector<CharWithIndex*> TextureIndexes;

		bool IsModelIndexed;
		vector<CharWithIndex*> ModelIndexes;

		bool IsSoundIndexed;
		vector<CharWithIndex*> SoundIndexes;

		bool IsScriptIndexed;
		vector<CharWithIndex*> ScriptIndexes;

		// vector sorting //
		bool IsSorted;
		bool IsBeingSorted;
		bool ShouldSortStop;

		// ------------------------------------ //
		static wstring DataFolder;
		static wstring ModelsFolder;
		static wstring ScriptsFolder;
		static wstring ShaderFolder;
		static wstring TextureFolder;
		static wstring FontFolder;
		static wstring SoundFolder;

		static FileSystem* Staticaccess;
	};

}

#endif