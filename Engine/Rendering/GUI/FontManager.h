#ifndef LEVIATHAN_FILEREPLACENAME
#define LEVIATHAN_FILEREPLACENAME
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{ namespace Rendering{

	class OverlayMaster;

	class FontManager : public Object{
	public:
		DLLEXPORT FontManager();
		DLLEXPORT ~FontManager();

		DLLEXPORT void LoadAllFonts();

		DLLEXPORT bool LoadFontByName(const wstring &name);

	private:

	};

}}
#endif