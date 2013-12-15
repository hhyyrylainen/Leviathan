#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILEREPLACENAME
#include "FontManager.h"
#endif
#include "FileSystem.h"
#include "Exceptions/ExceptionNotFound.h"
#include "OverlayMaster.h"
#include "Common/DataStoring/DataStore.h"
#include <Rocket/Core.h>
using namespace Leviathan;
using namespace Leviathan::Rendering;
// ------------------------------------ //
DLLEXPORT Leviathan::Rendering::FontManager::FontManager(){
	//// at least load arial font //

}

DLLEXPORT Leviathan::Rendering::FontManager::~FontManager(){
	// could add font unloading here //
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::FontManager::LoadFontByName(const wstring &name){
	// load font by name //


	// load file matching this font //
	wstring fontgenfile = FileSystem::Get()->SearchForFile(FILEGROUP_OTHER, name, L"ttf|otf", true);

	// look for it in registry //
	if(fontgenfile.size() == 0){
#ifdef _WIN32
		// set name to arial because we can't find other fonts //
		fontgenfile = L"Arial";

		if(fontgenfile == L"Arial"){
			HKEY hKey;
			LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ,
				&hKey);

			if(lRes != ERROR_SUCCESS){
				Logger::Get()->Error(L"FontGenerator: could not locate Arial font, OpenKey failed", true);
				return false;
			}

			WCHAR szBuffer[512];
			DWORD dwBufferSize;

			RegQueryValueEx(hKey, L"Arial (TrueType)", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);

			//fontgenfile = L"C:\\Windows\\Fonts\\";
			FileSystem::GetWindowsFolder(fontgenfile);
			fontgenfile += L"Fonts\\";
			fontgenfile += szBuffer;
		}
#else
		// This may or may not work on linux //
		fontgenfile += L"/usr/share/fonts/Arial.ttf";


#endif // _WIN32
	}
	if(!FileSystem::FileExists(fontgenfile)){
		Logger::Get()->Error(L"FontManager:  LoadFontByName: could not find font .ttf file", true);
		return false;
	}

	// register rocket font //
	Rocket::Core::FontDatabase::LoadFontFace(Convert::WstringToString(fontgenfile).c_str());

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Rendering::FontManager::LoadAllFonts(){
	// load arial //
	LoadFontByName(L"Arial");


	// get all font files and load them //
	std::vector<shared_ptr<FileDefinitionType>> files = FileSystem::Get()->FindAllMatchingFiles(FILEGROUP_OTHER, L".*", L"ttf|otf", false);

	// load them all //
	for(size_t i = 0; i < files.size(); i++){

		LoadFontByName(files[i]->Name);
	}
}




