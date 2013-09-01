#ifndef LEVIATHAN_TEXTUREDEFINITION
#define LEVIATHAN_TEXTUREDEFINITION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class TextureDefinition : public Object{
	public:
		DLLEXPORT TextureDefinition::TextureDefinition();
		DLLEXPORT TextureDefinition::TextureDefinition(const wstring& name, int type, const wstring& creator, const wstring& imagefilext, int width,
			int height, bool animation = false);
		DLLEXPORT TextureDefinition::~TextureDefinition();


	//protected:
		wstring Creator;
		wstring Name;
		wstring ImageExtension;
		int Width, Height;
		bool HasAnimation;
		int Type;
		// future texture animation object //

	};

}
#endif