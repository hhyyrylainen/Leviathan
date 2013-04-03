#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_LIST
#include "ObjectFileList.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ObjectFileList::ObjectFileList(){
	Variables = new NamedVars();
}
ObjectFileList::ObjectFileList(wstring name){
	Name = name;
	Variables = new NamedVars();
}
ObjectFileList::~ObjectFileList(){
	SAFE_DELETE(Variables);

	SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //
ScriptList* ObjectFileList::AllocateNewListFromData(){
	ScriptList* obj = new ScriptList(Name);

	obj->Variables = new NamedVars(*this->Variables);
	for(unsigned int i = 0; i < Lines.size(); i++){
		obj->Lines.push_back(new wstring(*Lines[i]));
	}
	return obj;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //