#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TEXTUREDEFINITION
#include "TextureDefinition.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "MultiFlag.h"

DLLEXPORT Leviathan::TextureDefinition::TextureDefinition(){
	Creator = L"ERROR";
	Name = L"ERROR";
	Width = -1;
	Height = -1;
	HasAnimation = false;
	Type = FLAG_GOBJECT_MODEL_TEXTURETYPE_UNKOWN;
}
DLLEXPORT Leviathan::TextureDefinition::TextureDefinition(const wstring& name, int type, const wstring& creator, const wstring& imagefilext
	, int width, int height, bool animation /*= false*/)
{
	ImageExtension = imagefilext;
	Creator = creator;
	Name = name;
	Width = width;
	Height = height;
	HasAnimation = animation;
	Type = type;
}
DLLEXPORT Leviathan::TextureDefinition::~TextureDefinition(){

}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


