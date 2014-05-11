#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_LIST
#include "ObjectFileList.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileListProper::ObjectFileListProper(const wstring &name) : Name(name){

}
// ------------------------------------ //
DLLEXPORT const wstring& Leviathan::ObjectFileListProper::GetName() const{
	return Name;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileListProper::AddVariable(shared_ptr<NamedVariableList> var){
	// Make sure that name is not in use //
	if(Variables.Find(var->GetName()) != -1){
		return false;
	}

	// Add it //
	Variables.AddVar(var);
	return true;
}
// ------------------------------------ //
DLLEXPORT NamedVars& Leviathan::ObjectFileListProper::GetVariables(){
	return Variables;
}
// ------------------ ObjectFileList ------------------ //
DLLEXPORT Leviathan::ObjectFileList::~ObjectFileList(){

}
