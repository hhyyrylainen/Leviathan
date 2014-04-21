#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILEREPLACENAME
#include "FontManager.h"
#endif
#include "FileSystem.h"
#include "Exceptions/ExceptionNotFound.h"
#include "Common/DataStoring/DataStore.h"
#include "CEGUI/FontManager.h"
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
DLLEXPORT void Leviathan::Rendering::FontManager::LoadAllFonts(){
	// This may be quite slow so timing this will help //
	ScopeTimer timethis(L"Font loading");

	// Get all font files and load them //
	std::vector<shared_ptr<FileDefinitionType>> files = FileSystem::Get()->FindAllMatchingFiles(FILEGROUP_OTHER, L".*", L"font", true);

	// load them all //
	for(size_t i = 0; i < files.size(); i++){

		// Register the font //
		CEGUI::FontManager::getSingleton().createFromFile(Convert::WstringToString(files[i]->GetNameWithExtension()));
	}
}




