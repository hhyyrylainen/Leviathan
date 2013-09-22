#ifndef LEVIATHAN_FILEREPLACENAME
#define LEVIATHAN_FILEREPLACENAME
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
#include "GUI\Components\GuiPositionable.h"
// ------------------------------------ //
// ---- includes ---- //
#include "OgreFont.h"


namespace Leviathan{ namespace Rendering{

	class OverlayMaster;

	class FontManager : public Object{
	public:
		DLLEXPORT FontManager();
		DLLEXPORT ~FontManager();

		DLLEXPORT static Ogre::Font* GetFontPtrFromName(const string &name);

		DLLEXPORT void LoadAllFonts();

		DLLEXPORT bool LoadFontByName(const wstring &name);
		DLLEXPORT float CountTextLength(const wstring &font, const wstring &text, const float &textheight, OverlayMaster* overlay, const int &coordtype = 
			GUI_POSITIONABLE_COORDTYPE_RELATIVE);

		// salvaged from the old TextRenderer/RenderingFont and made to work with Ogre overlay text //
		DLLEXPORT static bool AdjustTextSizeToFitBox(Ogre::Font* font, const Float2 &BoxToFit, const wstring &text, 
			int CoordType, size_t &Charindexthatfits, float &EntirelyFitModifier, float &HybridScale, Float2 &Finallength, float scaletocutfrom);
		DLLEXPORT static float CalculateTextLengthAndLastFitting(Ogre::Font* font, float TextSize, int CoordType, const wstring &text, 
			const float &fitlength, size_t & Charindexthatfits, float delimiterlength);
		DLLEXPORT static float CalculateDotsSizeAtScale(Ogre::Font* font, const float &scale);

	private:

	};

}}
#endif