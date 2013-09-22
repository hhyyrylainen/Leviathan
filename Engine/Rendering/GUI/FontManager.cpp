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
#include <Rocket\Core.h>
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
DLLEXPORT Ogre::Font* Leviathan::Rendering::FontManager::GetFontPtrFromName(const string &name){
	return static_cast<Ogre::Font*>((Ogre::FontManager::getSingleton().getByName(name)).get());
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Rendering::FontManager::LoadFontByName(const wstring &name){
	// load font by name //


	// load file matching this font //
	wstring fontgenfile = FileSystem::Get()->SearchForFile(FILEGROUP_OTHER, name, L"ttf|otf", true);

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

	// register rocket font //
	Rocket::Core::FontDatabase::LoadFontFace(Convert::WstringToString(fontgenfile).c_str());

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

	Ogre::Font* tmpptr = GetFontPtrFromName(Convert::WstringToString(font));

	float length = 0;

	if(tmpptr){

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
DLLEXPORT bool Leviathan::Rendering::FontManager::AdjustTextSizeToFitBox(Ogre::Font* font, const Float2 &BoxToFit, const wstring &text, 
	int CoordType, size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom)
{
	// calculate theoretical max height //
	float TMax = BoxToFit.Y;

	// calculate length of 3 dots //
	float dotslength = CalculateDotsSizeAtScale(font, TMax);

	float CalculatedTotalLength = CalculateTextLengthAndLastFitting(font, TMax, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

	// check did all fit //
	if(Charindexthatfits == text.size()-1){
		// everything fits at the maximum size //
		// set data and return //
		EntirelyFitModifier = HybridScale = TMax;
		Finallength = Float2(CalculatedTotalLength, TMax);

		return true;
	}
	// finding a scale at which the text could fit entirely //
	float MinWantedScale = TMax*scaletocutfrom;

	float LowVal = TMax*(scaletocutfrom/3);
	// how well the length needs to match //
	const float threshold = 0.02f;

	// looping variables //
	bool Stop = false;
	int itrs = 0;
	// loop until we have narrowed down to under threshold //
	while((TMax - LowVal) > threshold){
		// calculate size to test this on //
		float TestSize = (TMax+LowVal)/2;

		// check is scale too low //
		if(TestSize <= MinWantedScale){
			// calculate values at the lowest possibly wanted value //
			TestSize = MinWantedScale;
			// and break afterwards //
			Stop = true;
			// update return value //
			HybridScale = LowVal = TestSize;
		}

		// update dots length //
		dotslength = CalculateDotsSizeAtScale(font, TestSize);
		CalculatedTotalLength = CalculateTextLengthAndLastFitting(font, TestSize, CoordType, text, BoxToFit.X, Charindexthatfits, dotslength);

		if(Charindexthatfits < text.size()-1 || CalculatedTotalLength >= BoxToFit.X){
			// adjust max size //
			TMax = TestSize;
		} else {
			// too small text, adjust minimum size //
			LowVal = TestSize;
		}
		// Use Low value so that we undershoot rather than overshoot //
		HybridScale = LowVal;

		// update possible return value before looping //
		Finallength = Float2(CalculatedTotalLength, HybridScale);

		// we can return here if size will go too low //
		if(Stop)
			break;
		itrs++;
	}

	// everything should be done now //
	return true;
}

DLLEXPORT float Leviathan::Rendering::FontManager::CalculateTextLengthAndLastFitting(Ogre::Font* font, float TextSize, int CoordType, 
	const wstring &text, const float &fitlength, size_t & Charindexthatfits, float delimiterlength)
{
	// calculated length after done //
	float CalculatedTotalLength = 0.f;

	// used in checking if character would fit to the box //
	float curneededlength = 0;


	// calculate length using the provided size //
	for(size_t i = 0; i < text.size(); i++){

		// increase length //
		float aspect = font->getGlyphAspectRatio(text[i]);
		CalculatedTotalLength += aspect*TextSize;


		i+1 >= text.length() ? curneededlength = CalculatedTotalLength: curneededlength = CalculatedTotalLength+delimiterlength;

		// check would all this stuff fit //
		if(curneededlength <= fitlength){
			// this character would fit with truncation to the box (or is last character and would fit on its own) //
			Charindexthatfits = i;

		}
	}

	// update total length if not relative //
	if(CoordType != GUI_POSITIONABLE_COORDTYPE_RELATIVE){

		CalculatedTotalLength *= DataStore::Get()->GetWidth();
	}

	return CalculatedTotalLength;
}

DLLEXPORT float Leviathan::Rendering::FontManager::CalculateDotsSizeAtScale(Ogre::Font* font, const float &scale){

	return (font->getGlyphAspectRatio(L'.')*scale)*3;
}

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




