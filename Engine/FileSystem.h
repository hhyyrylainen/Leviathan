#ifndef LEVIATHAN_FILESYSTEM
#define LEVIATHAN_FILESYSTEM
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NamedVars.h"
#ifndef LEVIATHAN_LOGGER
#include "Logger.h"
#endif
namespace Leviathan{

	enum FILEGROUP{FILEGROUP_MODEL, FILEGROUP_TEXTURE, FILEGROUP_SOUND, FILEGROUP_SCRIPT, FILEGROUP_OTHER};

	struct FileDefinitionType{
		FileDefinitionType();
		FileDefinitionType(wstring &path); // just the path, everything else is worked out by the constructor //

		// helper function //
		//DLLEXPORT bool IsLesserPTR(shared_ptr<FileDefinitionType> first, shared_ptr<FileDefinitionType> second);

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
		DLLEXPORT bool OperatingOnVista();
		DLLEXPORT bool OperatingOnXP();

		//wstring GetLogFolder();
		DLLEXPORT static void ClearFoundFiles();
		DLLEXPORT static bool SearchFiles();
		DLLEXPORT static int RegisterExtension(wstring& extension);
		DLLEXPORT static void ReSearchFiles();
		// loaded file searching functions //
		DLLEXPORT static wstring& SearchForFile(FILEGROUP which, const wstring& name, const wstring& extensions, bool searchall = true);

		DLLEXPORT static void GetExtensionIDS(const wstring& extensions, vector<int>& ids);
		DLLEXPORT static bool DoesExtensionMatch(FileDefinitionType* file, const vector<int>&Ids);
		
		DLLEXPORT static wstring& GetDataFolder();
		DLLEXPORT static wstring GetModelsFolder();
		DLLEXPORT static wstring GetScriptsFolder();
		DLLEXPORT static wstring GetShaderFolder();
		DLLEXPORT static wstring GetTextureFolder();
		DLLEXPORT static wstring GetFontFolder();
		DLLEXPORT static wstring GetSoundFolder();

		// vector sorting //
		DLLEXPORT static void SortFileVectors(int MaxMCR = -1);
		DLLEXPORT static void CreateIndexesForVecs(bool ForceRe = false);


		DLLEXPORT static void GetWindowsFolder(wstring &path);
		DLLEXPORT static void GetSpecialFolder(wstring &path, int specialtype);

		DLLEXPORT static void SetDataFolder( wstring& folder );
		DLLEXPORT static void SetModelsFolder( wstring& folder );
		DLLEXPORT static void SetScriptsFolder( wstring& folder );
		DLLEXPORT static void SetShaderFolder( wstring& folder );
		DLLEXPORT static void SetTextureFolder( wstring& folder );

		// file handling //
		DLLEXPORT static int LoadDataDumb(wstring file, vector<shared_ptr<NamedVar>>& vec);
		DLLEXPORT static wstring GetExtension(const wstring &path);
		DLLEXPORT static wstring ChangeExtension(const wstring& path, wstring newext);

		DLLEXPORT static wstring RemoveExtension(const wstring &file, bool delpath);

		/// file operations
		DLLEXPORT static int GetFileLenght(wstring name);
		DLLEXPORT static bool FileExists(wstring name);
		DLLEXPORT static bool WriteToFile(const string &data, const string &filename);
		DLLEXPORT static bool WriteToFile(const wstring &data, const wstring &filename);
		DLLEXPORT static bool AppendToFile(const wstring &data, const wstring &filepath);
		DLLEXPORT static bool GetFilesInDirectory(vector<wstring> &files, wstring dirpath, wstring pattern = L"*.*", bool recursive = true);

		DLLEXPORT static vector<shared_ptr<FileDefinitionType>>& GetModelFiles();
		DLLEXPORT static vector<shared_ptr<FileDefinitionType>>& GetSoundFiles();
		DLLEXPORT static vector<shared_ptr<FileDefinitionType>>& GetAllFiles();
		DLLEXPORT static vector<shared_ptr<FileDefinitionType>>& GetScriptFiles();

		// utility stuff
		//DLLEXPORT


		// bitmap stuff //
		DLLEXPORT static BYTE* LoadBMP ( int* width, int* height, long* size, LPCTSTR bmpfile );
		DLLEXPORT static BYTE* ConvertBMPToRGBBuffer ( BYTE* Buffer, int width, int height );

	private:
		// private constructors, to prevent creation of this class //
		DLLEXPORT FileSystem();
		DLLEXPORT ~FileSystem();


		// file search funcs //
		static shared_ptr<FileDefinitionType> _SearchForFileInVec(vector<shared_ptr<FileDefinitionType>>& vec, vector<int>& extensions, const wstring& name, bool UseIndexVector, vector<CharWithIndex*>* Index);

		// file holders //
		static vector<IntWstring*> FileTypes; // vector that holds string value of file extension and it's id code //
		static int CurrentFileExtID;

		static vector<shared_ptr<FileDefinitionType>> AllFiles;

		static vector<shared_ptr<FileDefinitionType>> TextureFiles;
		static vector<shared_ptr<FileDefinitionType>> ModelFiles;
		static vector<shared_ptr<FileDefinitionType>> SoundFiles;
		static vector<shared_ptr<FileDefinitionType>> ScriptFiles;

		// index vectors //
		static bool IsAllIndexed;
		static vector<CharWithIndex*> AllIndexes;
		
		static bool IsTextureIndexed;
		static vector<CharWithIndex*> TextureIndexes;

		static bool IsModelIndexed;
		static vector<CharWithIndex*> ModelIndexes;

		static bool IsSoundIndexed;
		static vector<CharWithIndex*> SoundIndexes;

		static bool IsScriptIndexed;
		static vector<CharWithIndex*> ScriptIndexes;

		// vector sorting //
		static bool IsSorted;
		static bool IsBeingSorted;
		static bool ShouldSortStop;

		//static vector<shared_ptr<IntWstring>> 

		static wstring DataFolder;
		static wstring ModelsFolder;
		static wstring ScriptsFolder;
		static wstring ShaderFolder;
		static wstring TextureFolder;
		static wstring FontFolder;
		static wstring SoundFolder;
	};

}

#endif