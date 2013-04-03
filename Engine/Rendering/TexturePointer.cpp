#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERING_TEXTUREPOINTER
#include "TexturePointer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
TexturePointer::TexturePointer(int id, wstring* file, int index){
	P_ID = id;
	if(file != NULL)
		P_File = *file;
	else
		P_File = L"";
	P_Index;
}
TexturePointer::~TexturePointer(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //