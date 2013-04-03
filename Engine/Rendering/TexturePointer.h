#ifndef LEVIATHAN_RENDERING_TEXTUREPOINTER
#define LEVIATHAN_RENDERING_TEXTUREPOINTER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class TexturePointer : public Object{
	public:
		DLLEXPORT TexturePointer::TexturePointer(int id, wstring* file, int index);
        DLLEXPORT TexturePointer::~TexturePointer();

		int P_ID;
		wstring P_File;
		int P_Index;
	};

}
#endif