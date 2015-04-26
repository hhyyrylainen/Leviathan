// ------------------------------------ //
#include "FontManager.h"

#include "../../FileSystem.h"
#include "../../Statistics/TimingMonitor.h"
#include "CEGUI/FontManager.h"
using namespace Leviathan;
using namespace Rendering;
using namespace std;
// ------------------------------------ //
DLLEXPORT FontManager::FontManager(){
	//// at least load arial font //

}

DLLEXPORT FontManager::~FontManager(){
	// could add font unloading here //
}
// ------------------------------------ //
DLLEXPORT void FontManager::LoadAllFonts(){
    
	// This may be quite slow so timing this will help //
	ScopeTimer timethis("Font loading");

	// Get all font files and load them //
	std::vector<shared_ptr<FileDefinitionType>> files =
        FileSystem::Get()->FindAllMatchingFiles(FILEGROUP_OTHER, ".*", "font", true);

	// load them all //
	for(size_t i = 0; i < files.size(); i++){

		// Register the font //
		CEGUI::FontManager::getSingleton().createFromFile(files[i]->GetNameWithExtension());
	}
}




