#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_TEXTBLOCK
#include "ObjectFileTextBlock.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectFileTextBlock::ObjectFileTextBlock(){
}
ObjectFileTextBlock::ObjectFileTextBlock(const wstring &name){
	Name = name;
}
ObjectFileTextBlock::~ObjectFileTextBlock(){
	SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //