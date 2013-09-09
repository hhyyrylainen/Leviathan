#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILESYSTEM
#include "FileSystem.h"
#endif
#include "OgreResourceGroupManager.h"
using namespace Leviathan;
// ------------------------------------ //
Leviathan::FileSystem::FileSystem(){
	// set static access //
	Staticaccess = this;
	// set default values //
	CurrentFileExtID = 25;

	// index creation flags //
	IsAllIndexed = IsTextureIndexed = IsModelIndexed = IsSoundIndexed = IsScriptIndexed = IsSorted = IsBeingSorted = ShouldSortStop = false;
}

Leviathan::FileSystem::~FileSystem(){
	// release data //
	Release();
}

wstring Leviathan::FileSystem::DataFolder = L"./Data/";
wstring Leviathan::FileSystem::ModelsFolder = L"Models/";
wstring Leviathan::FileSystem::ScriptsFolder = L"Scripts/";
wstring Leviathan::FileSystem::ShaderFolder = L"Shaders/";
wstring Leviathan::FileSystem::TextureFolder = L"Textures/";
wstring Leviathan::FileSystem::FontFolder = L"Fonts/";
wstring Leviathan::FileSystem::SoundFolder = L"Sound/";

FileSystem* Leviathan::FileSystem::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::FileSystem::Init(){

	IsSorted = false;

	// use find files function on data folder and then save results to appropriate vectors //
	vector<wstring> files;
	GetFilesInDirectory(files, L".\\Data\\");

	if(files.size() < 1){

		Logger::Get()->Error(L"FileSystem: SearchFiles: No files inside data folder, cannot possibly work", files.size(), true);
		return false;
	}

	// save to appropriate places //
	for(unsigned int i = 0; i < files.size(); i++){
		// create new object for storing this //
		shared_ptr<FileDefinitionType> tmpptr(new FileDefinitionType(this, files[i]));

		if(files[i].find(L".\\Data\\Textures\\") == 0){
			// add to texture files //
			TextureFiles.push_back((tmpptr));
		} else if(files[i].find(L".\\Data\\Models\\") == 0){
			// add to texture files //
			ModelFiles.push_back(tmpptr);
		} else if(files[i].find(L".\\Data\\Sound\\") == 0){
			// add to texture files //
			SoundFiles.push_back(tmpptr);
		} else if(files[i].find(L".\\Data\\Scripts\\") == 0){
			// add to texture files //
			ScriptFiles.push_back(tmpptr);
		}

		// everything should be in AllFiles vector //
		AllFiles.push_back(tmpptr);
	}
	// print some info //
	Logger::Get()->Info(L"FileSystem: found "+Convert::IntToWstring(AllFiles.size())+L" files in Data folder with "
		+Convert::IntToWstring(FileTypes.size())+L" different types of extensions", false);

	// sort for quick finding //
	__int64 Timestrt = Misc::GetTimeMicro64();
	CreateIndexesForVecs();

	int elapsed = (int)(Misc::GetTimeMicro64()-Timestrt);

	// print info //
	Logger::Get()->Info(L"FileSystem: vectors sorted and indexes created, took "+Convert::IntToWstring(elapsed)+L" micro seconds", true);

	// done, return true signal that everything is OK //
	return true;
}

DLLEXPORT void Leviathan::FileSystem::Release(){
	// set some values to defaults //
	CurrentFileExtID = 25;
	IsSorted = false;

	// file types need to be deleted because no smart pointers are used there //
	SAFE_DELETE_VECTOR(FileTypes);

	AllFiles.clear();

	TextureFiles.clear();
	ModelFiles.clear();
	SoundFiles.clear();
	ScriptFiles.clear();

	// clear indexes //
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

DLLEXPORT bool Leviathan::FileSystem::ReSearchFiles(){
	// just clear files and call search again //
	Release();
	return Init();
}
// ------------------------------------ //
bool Leviathan::FileSystem::OperatingOnVista(){

	OSVERSIONINFO osver;
	memset(&osver, 0, sizeof(OSVERSIONINFO));
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if(!GetVersionEx(&osver))
		return FALSE;

	if((osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion >= 6  ))
		return TRUE;

	return FALSE;
}

bool Leviathan::FileSystem::OperatingOnXP(){

	OSVERSIONINFO osver;
	memset(&osver, 0, sizeof(OSVERSIONINFO));
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (!GetVersionEx(&osver))
		return FALSE;

	if ((osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion >= 5))
		return TRUE;

	return FALSE;
}
// ------------------------------------ //
wstring& Leviathan::FileSystem::GetDataFolder(){

	return( DataFolder );
}

wstring Leviathan::FileSystem::GetModelsFolder(){

	return(DataFolder + ModelsFolder);
}

wstring Leviathan::FileSystem::GetScriptsFolder(){

	return(DataFolder + ScriptsFolder);
}

wstring Leviathan::FileSystem::GetShaderFolder(){

	return(DataFolder + ShaderFolder);
}

wstring Leviathan::FileSystem::GetTextureFolder(){

	return(DataFolder + TextureFolder);
}

wstring Leviathan::FileSystem::GetFontFolder(){

	return(DataFolder + FontFolder);
}

wstring Leviathan::FileSystem::GetSoundFolder(){

	return(DataFolder + SoundFolder);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::FileSystem::DoesExtensionMatch(FileDefinitionType* file, const vector<int>&Ids){
	// check does file contain an extension id that is in the vector //
	for(size_t i = 0; i < Ids.size(); i++){
		if(file->ExtensionID == Ids[i]){
			// match found //
			return true; 
		}
	}
	return false;
}

// ------------------------------------ //
void Leviathan::FileSystem::GetWindowsFolder(wstring &path){
	wchar_t winddir[MAX_PATH];
	GetWindowsDirectoryW(winddir, MAX_PATH);
	path = winddir;
	if(path.back() != L'\\')
		path += L'\\';
}

void Leviathan::FileSystem::GetSpecialFolder(wstring &path, int specialtype){
	wchar_t directory[MAX_PATH];
	SHGetSpecialFolderPathW(NULL, directory, specialtype, FALSE);
	path = directory;
	if(path.back() != L'\\')
		path += L'\\';
}
// ------------------------------------ //
DLLEXPORT void Leviathan::FileSystem::SetDataFolder(const wstring &folder){

	DataFolder = folder;
}

void Leviathan::FileSystem::SetModelsFolder(const wstring &folder){

	ModelsFolder = folder;
}

void Leviathan::FileSystem::SetScriptsFolder(const wstring &folder){

	ScriptsFolder = folder;
}

void Leviathan::FileSystem::SetShaderFolder(const wstring &folder){

	ShaderFolder = folder;
}

void Leviathan::FileSystem::SetTextureFolder(const wstring &folder){

	TextureFolder = folder;
}
// ------------------ File handling ------------------ //
DLLEXPORT int Leviathan::FileSystem::LoadDataDump(const wstring &file, vector<shared_ptr<NamedVariableList>>& vec){
	wstring filecontents = L"";
	wstring construct = L"";
	int Length = 0;
	// get data //
	wifstream stream;
	stream.open(file, ios::in);
	if(!stream.good()){
		// no file ! //
		Logger::Get()->Error(L"FileSystem: LoadDataDumb: Failed to read file: "+file, false);
		return 404;
	}
	// count length //
	stream.seekg(0, ios::end);
	Length = (int)stream.tellg();
	stream.seekg(0, ios::beg);
	if(Length == 0){
		// empty file ! //
		Logger::Get()->Warning(L"FileSystem: LoadDataDumb: Empty file: "+file, false);
		return 0;
	}
	unique_ptr<wchar_t> Buff(new wchar_t[Length+1]);
	// set null terminator, just in case
	(Buff.get())[Length] = '\0';

	stream.read(Buff.get(), Length);

	stream.close();

	filecontents = Buff.get();

	// create values //
	int retvalue = NamedVariableList::ProcessDataDump(filecontents, vec);
	if(retvalue != 0){
		// error happened //
		Logger::Get()->Error(L"FileSystem: LoadDataDumb: call to ProcessDataDumb failed", retvalue, true);
		return 5;
	}
	return 0;
}

DLLEXPORT bool Leviathan::FileSystem::GetFilesInDirectory(vector<wstring> &files, wstring dirpath, wstring pattern /*= L"*.*"*/, bool recursive /*= true*/){
	wstring FilePath;
	wstring Pattern;
	HANDLE hFile;          
	WIN32_FIND_DATA FileInfo;

	Pattern = dirpath + pattern;

	hFile = ::FindFirstFile(Pattern.c_str(), &FileInfo);
	if(hFile != INVALID_HANDLE_VALUE){
		do{

			if(FileInfo.cFileName[0] != '.'){
				FilePath.erase();
				FilePath = dirpath + FileInfo.cFileName+L"\\";

				if(FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){

					if(recursive){
						// call self to search subdirectory
						int retr = GetFilesInDirectory(files, FilePath, pattern, recursive);
						if(!retr)
							break; // failed //
					}
				} else {
					// save file
					files.push_back(dirpath+FileInfo.cFileName);
				}
			}
		}while(::FindNextFile(hFile, &FileInfo) == TRUE);

		// close handle //
		FindClose(hFile);
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT wstring Leviathan::FileSystem::GetExtension(const wstring &path){
	wstring extension = L"";

	bool found = false;
	for(int i = (int)path.length()-1; (i > -1) && (i < (int)path.length()); ){
		if(!found){
			i--;
			if(path[i] == L'.'){
				found = true;
				i++;
			}
			continue;
		}
		extension += path[i];
		i++;
	}

	return extension;
}

DLLEXPORT  wstring Leviathan::FileSystem::ChangeExtension(const wstring& path, const wstring &newext){
	// get last dot in string //
	size_t lastdot = path.find_last_of(L'.');

	if(lastdot == wstring::npos){
		// no dot!, just append to end and return //
		return path+L'.'+newext;
	}

	// get substring from begin to dot and add new extension //
	return path.substr(0, lastdot+1)+newext;
}

wstring Leviathan::FileSystem::RemoveExtension(const wstring &file, bool delpath){
	wstring returnstr = L"";
	
	bool found = false;
	for(int i = (int)file.length()-1; (i > -1) && (i < (int)file.length()); ){
		if(!found){
			i--;
			if(!(i > -1)){
				return file;
			}
			if(file[i] == L'.'){
				found = true;
				//i--;
			}
			continue;
		}
		i--;
		if(!(i > -1 && i < (int)file.length())){
			break;
		}
		if(((file[i] == L'/') | (file[i] == L'\\')) && (delpath))
			break;
		returnstr += file[i];
	}
	// turn around //
	wstring final = L"";
	for(int i = (int)returnstr.size()-1; i > -1; i--){
		final += returnstr[i];
	}

	return final;
}
// ------------------ File operations ------------------ //
int Leviathan::FileSystem::GetFileLength(wstring name){
	wifstream file(name);
	if(file.good()){
		file.seekg(0, ios::end);
		int returnval = (int)file.tellg();
		file.close();
		return returnval;
	}

	return -1; // not found
}

bool Leviathan::FileSystem::FileExists(wstring name){
	bool Ex=FALSE;
	wifstream file(name);
	if(file.is_open()){
		Ex=TRUE;
	}
	file.close();
	return Ex;
}

bool Leviathan::FileSystem::WriteToFile(const string &data, const string &filename){
	ofstream file(filename);
	if (file.is_open()){
		file << data;

		file.close();
		return true;
	}

	file.close();
	return false;
}

bool Leviathan::FileSystem::WriteToFile(const wstring &data, const wstring &filename){
	wofstream file(filename);
	if (file.is_open()){
		file << data;

		file.close();
		return true;
	}

	file.close();
	return false;
}

bool Leviathan::FileSystem::AppendToFile(const wstring &data, const wstring &filepath){
	wofstream file(filepath, wofstream::app);

	if (file.is_open()){
		file << data;

		file.close();
		return true;
	}

	file.close();
	return false;
}

DLLEXPORT  void Leviathan::FileSystem::ReadFileEntirely(const wstring &file, wstring &resultreceiver) throw(...){
	wifstream reader(file, ios::in);
	if(reader){

		// go to end to count length //
		reader.seekg(0, ios::end);

		streamoff rpos = reader.tellg();


		// cannot be loaded //
		assert(SIZE_T_MAX >= rpos);

		resultreceiver.resize((UINT)rpos);
		// back to start //
		reader.seekg(0, ios::beg);
		// read the actual data //
		reader.read(&resultreceiver[0], resultreceiver.size());

		// done, cleanup //
		reader.close();
		return;
	}
	throw ExceptionInvalidArguement(L"cannot read given file", GetLastError(), __WFUNCTION__, L"file", file);
}
// ------------------ Bitmap operations ------------------ //
BYTE* Leviathan::FileSystem::LoadBMP( int* Width, int* Height, long* Size, LPCTSTR bmpfile ){
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	DWORD bytesread;

	HANDLE file = CreateFile ( bmpfile , GENERIC_READ, FILE_SHARE_READ,
		 NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if ( NULL == file )
		return NULL;

	if ( ReadFile ( file, &bmpheader, sizeof ( BITMAPFILEHEADER ), 
		&bytesread, NULL ) == false )
	{
		CloseHandle ( file );
		return NULL;
	}

	if ( ReadFile ( file, &bmpinfo, sizeof ( BITMAPINFOHEADER ), 
		&bytesread, NULL ) == false )
	{
		CloseHandle ( file );
		return NULL;
	}

	if ( bmpheader.bfType != 'MB' )
	{
		CloseHandle ( file );
		return NULL;
	}

	if ( bmpinfo.biCompression != BI_RGB )
	{
		CloseHandle ( file );
		return NULL;
	}

	if ( bmpinfo.biBitCount != 24 )
	{
		CloseHandle ( file );
		return NULL;
	}

	*Width = bmpinfo.biWidth;
	*Height = bmpinfo.biHeight;

	*Size = bmpheader.bfSize - bmpheader.bfOffBits;

	BYTE* Buffer = new BYTE[ *Size ];

	SetFilePointer ( file, bmpheader.bfOffBits, NULL, FILE_BEGIN );

	if ( ReadFile ( file, Buffer, *Size, &bytesread, NULL ) == false )
	{
		delete [] Buffer;
		CloseHandle ( file );
		return NULL;
	}
	CloseHandle ( file );
	return Buffer;

}
BYTE* Leviathan::FileSystem::ConvertBMPToRGBBuffer ( BYTE* Buffer, int width, int height ){
	if ( ( NULL == Buffer ) || ( width == 0 ) || ( height == 0 ) )
		return NULL;

	int padding = 0;
	int scanlinebytes = width * 3;
	while ( ( scanlinebytes + padding ) % 4 != 0 )
		padding++;
		
	int psw = scanlinebytes + padding;
	BYTE* newbuf = new BYTE[width*height*3];

	long bufpos = 0;   
	long newpos = 0;
	for ( int y = 0; y < height; y++ )
		for ( int x = 0; x < 3 * width; x+=3 )
		{
			newpos = y * 3 * width + x;     
			bufpos = ( height - y - 1 ) * psw + x;

			newbuf[newpos] = Buffer[bufpos + 2];       
			newbuf[newpos + 1] = Buffer[bufpos+1]; 
			newbuf[newpos + 2] = Buffer[bufpos];     
		}

	return newbuf;

}
// ------------------ Non static part ------------------ //
DLLEXPORT void Leviathan::FileSystem::SortFileVectors(){
	// check if already sorted //
	if(IsSorted)
		return;

	ShouldSortStop = false;
	// call sort on the vectors //
	IsBeingSorted = true;

	// looping so that other thread can cancel the action before it is finished //
	for(int i = 0; i < 5; i++){
		// switch on index and call std sort //
		switch(i){
		case 0: sort(AllFiles.begin(), AllFiles.end(), FileDefSorter()); break;
		case 1: sort(TextureFiles.begin(), TextureFiles.end(), FileDefSorter()); break;
		case 2: sort(ModelFiles.begin(), ModelFiles.end(), FileDefSorter()); break;
		case 3: sort(SoundFiles.begin(), SoundFiles.end(), FileDefSorter()); break;
		case 4: sort(ScriptFiles.begin(), ScriptFiles.end(), FileDefSorter()); break;
		}

		// check for end
		if(ShouldSortStop){
			// asked to stop //
			goto end;
		}
	}
	// sort done
	IsSorted = true;

end:
	IsBeingSorted = false;
}

DLLEXPORT void Leviathan::FileSystem::CreateIndexesForVecs(bool ForceRe /*= false*/){
	// check are vectors sorted, if not call sort //
	if(!IsSorted){

		SortFileVectors();
	}

	_CreateIndexesIfMissing(AllFiles, AllIndexes, IsAllIndexed, ForceRe);
	_CreateIndexesIfMissing(TextureFiles, TextureIndexes, IsTextureIndexed, ForceRe);
	_CreateIndexesIfMissing(ModelFiles, ModelIndexes, IsModelIndexed, ForceRe);
	_CreateIndexesIfMissing(SoundFiles, SoundIndexes, IsSoundIndexed, ForceRe);
	_CreateIndexesIfMissing(ScriptFiles, ScriptIndexes, IsScriptIndexed, ForceRe);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::FileSystem::RegisterExtension(const wstring &extension){
	// check does it exist //
	for(unsigned int i = 0; i < FileTypes.size(); i++){
		if(Misc::WstringCompareInsensitiveRefs(*FileTypes[i]->Wstr, extension))
			return FileTypes[i]->Value;
	}

	// add //
	CurrentFileExtID++;
	FileTypes.push_back(new IntWstring(extension, CurrentFileExtID));
	return CurrentFileExtID;
}

void Leviathan::FileSystem::GetExtensionIDS(const wstring& extensions, vector<int>& ids){
	// generate info about the extensions //
	vector<wstring> Exts;
	Misc::CutWstring(extensions, L"|", Exts);
	if(Exts.size() == 0){
		// just one extension //
		ids.push_back(RegisterExtension(extensions));
		return;
		//Logger::Get()->Info(L"FileSystem: GetExtensionIDS: warning extensions provided in invalid format should be separated by | not space."
		//	L" Received: "+extensions+L" HINT: IT WORKED");
	}

	for(unsigned int i = 0; i < Exts.size(); i++){
		ids.push_back(RegisterExtension(Exts[i]));
	}
}
// ------------------------------------ //
DLLEXPORT wstring& Leviathan::FileSystem::SearchForFile(FILEGROUP which, const wstring& name, const wstring& extensions, bool searchall /*= true*/){
	// generate info about the search file //
	vector<int> ExtensionIDS;
	GetExtensionIDS(extensions, ExtensionIDS);

	switch(which){
	case FILEGROUP_MODEL:
		{
			shared_ptr<FileDefinitionType> result = _SearchForFileInVec(ModelFiles, ExtensionIDS, name, IsModelIndexed, &ModelIndexes);
			if(result.get() != NULL){
				// found //
				return result.get()->RelativePath;
			}
#ifdef _DEBUG
			Logger::Get()->Info(L"SearchForFile: Not found in primary vector "+Convert::IntToWstring((int)which), true);
#endif // _DEBUG
		}
	break;
	case FILEGROUP_TEXTURE:
		{
			shared_ptr<FileDefinitionType> result = _SearchForFileInVec(TextureFiles, ExtensionIDS, name, IsTextureIndexed, &TextureIndexes);
			if(result.get() != NULL){
				// found //
				return result.get()->RelativePath;
			}
#ifdef _DEBUG
			Logger::Get()->Info(L"SearchForFile: Not found in primary vector "+Convert::IntToWstring((int)which), true);
#endif // _DEBUG
		}
		break;
	case FILEGROUP_SOUND:
		{
			shared_ptr<FileDefinitionType> result = _SearchForFileInVec(SoundFiles, ExtensionIDS, name, IsSoundIndexed, &SoundIndexes);
			if(result.get() != NULL){
				// found //
				return result.get()->RelativePath;
			}
#ifdef _DEBUG
			Logger::Get()->Info(L"SearchForFile: Not found in primary vector "+Convert::IntToWstring((int)which), true);
#endif // _DEBUG
		}
		break;
	case FILEGROUP_SCRIPT:
		{
			shared_ptr<FileDefinitionType> result = _SearchForFileInVec(ScriptFiles, ExtensionIDS, name, IsScriptIndexed, &ScriptIndexes);
			if(result.get() != NULL){
				// found //
				return result.get()->RelativePath;
			}
#ifdef _DEBUG
			Logger::Get()->Info(L"SearchForFile: Not found in primary vector "+Convert::IntToWstring((int)which), true);
#endif // _DEBUG
		}
	break;
	case FILEGROUP_OTHER:
		{
			shared_ptr<FileDefinitionType> result = _SearchForFileInVec(AllFiles, ExtensionIDS, name, IsAllIndexed, &AllIndexes);
			if(result.get() != NULL){
				// found //
				return result.get()->RelativePath;
			}
#ifdef _DEBUG
			Logger::Get()->Info(L"SearchForFile: Not found in primary vector "+Convert::IntToWstring((int)which), true);
#endif // _DEBUG
		}
	break;
	}


	// still not found, if searchall specified search all files vector //
	if(searchall){
		shared_ptr<FileDefinitionType> result = _SearchForFileInVec(AllFiles, ExtensionIDS, name, IsAllIndexed, &AllIndexes);
		if(result.get() != NULL){
			// found //
			return result.get()->RelativePath;
		}
#ifdef _DEBUG
		Logger::Get()->Info(L"SearchForFile: Not found in AllFiles vector ", true);
#endif // _DEBUG

	}
	// not found return empty and if debug build warn //
//#ifdef _DEBUG
	Logger::Get()->Error(L"FileSystem: File not found: "+name+L"."+extensions, (bool)true);
//#endif
	return Misc::EmptyString;
}
// ------------------------------------ //
vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetModelFiles(){
	return ModelFiles;
}

vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetSoundFiles(){
	return SoundFiles;
}

vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetAllFiles(){
	return AllFiles;
}

vector<shared_ptr<FileDefinitionType>>& Leviathan::FileSystem::GetScriptFiles(){
	return ScriptFiles;
}
// ------------------------------------ //
shared_ptr<FileDefinitionType> Leviathan::FileSystem::_SearchForFileInVec(vector<shared_ptr<FileDefinitionType>>& vec, vector<int>& extensions, const wstring& name, bool UseIndexVector, vector<CharWithIndex*>* Index){
	int StartSpot = 0;

	// use index to get start spot for faster searching //
	if(UseIndexVector){
		wchar_t startchar = name[0];
		bool Found = false;

		// find matching char //
		for(unsigned int i = 0; i < Index->size(); i++){
			if(Index->at(i)->Char == startchar){
				Found = true;
				StartSpot = Index->at(i)->Index;
				break;
			}
		}
		// if character that starts the word wasn't found it can't be there, exit function //
		if(!Found)
			return NULL;
	}

	for(size_t i = StartSpot; i < vec.size(); i++){
		// if no extension specified skip checking them //
		if(extensions.size() > 0){
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

void Leviathan::FileSystem::_CreateIndexesIfMissing(vector<shared_ptr<FileDefinitionType>> &vec, vector<CharWithIndex*> &resultvec, bool &indexed, 
	const bool &force /*= false*/)
{
	// we'll need to delete old ones if index creation is forced //
	if(force){
		indexed = false;
		SAFE_DELETE_VECTOR(resultvec);
	}
	// if they are valid we can just return //
	if(indexed)
		return;

	// now that the file vector is sorted we can loop through it and every time first character changes add it to index //
	wchar_t curchar = L'!';

	for(size_t i = 0; i < vec.size(); i++){
		if(vec[i]->Name[0] != curchar){
			// beginning character changed, push to indexes //
			curchar = vec[i]->Name[0];
			resultvec.push_back(new CharWithIndex(curchar, (int)i));
		}
	}

	// done //
	indexed = true;
}

DLLEXPORT void Leviathan::FileSystem::RegisterOGREResourceGroups(){
	// get the resource managing singleton //
	Ogre::ResourceGroupManager& manager = Ogre::ResourceGroupManager::getSingleton();


	// Models folder //
	manager.createResourceGroup("MainModelsFolder");

	Ogre::String folder = Convert::WstringToString(DataFolder+ModelsFolder);

	manager.addResourceLocation(folder, "FileSystem", "MainModelsFolder", true);


	// Textures folder //
	manager.createResourceGroup("MainTexturesFolder");

	folder = Convert::WstringToString(DataFolder+TextureFolder);

	manager.addResourceLocation(folder, "FileSystem", "MainTexturesFolder", true);

	// Terrain group //
	manager.createResourceGroup("Terrain");

	folder = Convert::WstringToString(DataFolder+L"Cache\\Terrain\\");

	manager.addResourceLocation(folder, "FileSystem", "Terrain", true);


	// possibly register addon folders //


	// initialize the groups //
	manager.initialiseAllResourceGroups();

	// load the groups //
	manager.loadResourceGroup("MainModelsFolder");
}

// ------------------ FileDefinitionType ------------------ //
Leviathan::FileDefinitionType::FileDefinitionType(FileSystem* instance, const wstring &path) : RelativePath(path){
	// get extension //
	wstring tempexpt = FileSystem::GetExtension(path);

	// register extension and store id //
	ExtensionID = instance->RegisterExtension(tempexpt);

	// save name //
	Name = FileSystem::RemoveExtension(path, true);
}

bool Leviathan::FileDefinitionType::operator<(const FileDefinitionType& other) const{
	int isfirst = Misc::IsWstringBeforeInAlphabet(this->Name, other.Name);

	return (isfirst > 0);
}

Leviathan::FileDefinitionType::~FileDefinitionType(){

}
// ------------------ FileDefSorter ------------------ //
bool Leviathan::FileDefSorter::operator()(shared_ptr<FileDefinitionType>& first, shared_ptr<FileDefinitionType>& second){
	return (*first.get()) < *(second).get();
}