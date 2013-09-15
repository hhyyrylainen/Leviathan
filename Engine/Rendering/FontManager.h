#ifndef LEVIATHAN_FILEREPLACENAME
#define LEVIATHAN_FILEREPLACENAME
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
#include "GUI\Components\GuiPositionable.h"
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{ namespace Rendering{

	class OverlayMaster;

	class FontManager : public Object{
	public:
		DLLEXPORT FontManager();
		DLLEXPORT ~FontManager();


		DLLEXPORT bool LoadFontByName(const wstring &name);
		DLLEXPORT float CountTextLength(const wstring &font, const wstring &text, const float &textheight, OverlayMaster* overlay, const int &coordtype = 
			GUI_POSITIONABLE_COORDTYPE_RELATIVE);

	private:

	};

}}
#endif