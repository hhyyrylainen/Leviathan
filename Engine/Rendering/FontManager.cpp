#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILEREPLACENAME
#include "FontManager.h"
#endif
#include "OgreFontManager.h"
#include "OgreFont.h"
#include "FileSystem.h"
#include "Exceptions\ExceptionNotFound.h"
#include "OverlayMaster.h"
#include "Common\DataStoring\DataStore.h"
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

// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::FontManager::LoadFontByName(const wstring &name){
	// load font by name //


	// load file matching this font //
	wstring fontgenfile = FileSystem::Get()->SearchForFile(FILEGROUP_OTHER, name, L"ttf", true);

	// look for it in registry //
	if(fontgenfile.size() == 0){
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
	}
	if(!FileSystem::FileExists(fontgenfile)){
		Logger::Get()->Error(L"FontManager:  LoadFontByName: could not find font .ttf file", true);
		return false;
	}

	// create font //
	Ogre::FontPtr tmpptr = Ogre::FontManager::getSingleton().create(Convert::WstringToString(name), 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// always has to be true type //
	tmpptr->setType(Ogre::FT_TRUETYPE);


	// set source //

	Ogre::String str = Convert::WstringToString(fontgenfile);

	// set as a path //
	FileSystem::RegisterOGREResourceLocation(str.substr(0, str.find_last_of('\\')+1));

	// we want to cut down to just the filename //
	tmpptr->setSource(str.substr(str.find_last_of('\\')+1, str.length()-str.find_last_of('\\')));

	// set generated quality //
	tmpptr->setTrueTypeSize(32.f);
	tmpptr->setTrueTypeResolution(96);

	// no idea what characters should be loaded //
	tmpptr->addCodePointRange(Ogre::Font::CodePointRange(33, 255));

	tmpptr->load();

	return true;
}



// ------------------------------------ //
DLLEXPORT float Leviathan::Rendering::FontManager::CountTextLength(const wstring &font, const wstring &text, const float &textheight, 
	OverlayMaster* overlay, const int &coordtype /*= Gui::GUI_POSITIONABLE_COORDTYPE_RELATIVE*/)
{

	Ogre::Font* tmpptr = static_cast<Ogre::Font*>(Ogre::FontManager::getSingleton().getByName(Convert::WstringToString(font)).get());

	float length = 0;

	if(tmpptr){

		//float viewportcoef = DataStore::Get()->GetHeight()/DataStore::Get()->GetWidth();

		for(size_t i = 0; i < text.size(); i++){

			float aspect = tmpptr->getGlyphAspectRatio(text[i]);
			length += aspect*textheight;
		}

	}

	if(coordtype != GUI_POSITIONABLE_COORDTYPE_RELATIVE){
		// convert to pixels //

		length *= DataStore::Get()->GetWidth();
	}

	return length;
}
// ------------------------------------ //




