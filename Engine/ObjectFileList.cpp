#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_LIST
#include "ObjectFileList.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectFileList::ObjectFileList() : Lines(), Name(L""){
}
ObjectFileList::ObjectFileList(const wstring &name) : Lines(), Name(name){
}
ObjectFileList::~ObjectFileList(){
	// release memory //
	//SAFE_DELETE(Variables);
	SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //