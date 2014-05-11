#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE
#include "ObjectFile.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFile::ObjectFile(NamedVars &stealfrom) : HeaderVars(&stealfrom){

}

DLLEXPORT Leviathan::ObjectFile::ObjectFile(){

}

DLLEXPORT Leviathan::ObjectFile::~ObjectFile(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::AddNamedVariable(shared_ptr<NamedVariableList> var){
	// Make sure that name is not in use //
	if(HeaderVars.Find(var->GetName()) != -1){
		return false;
	}

	// Add it //
	HeaderVars.AddVar(var);
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::AddObject(shared_ptr<ObjectFileObject> obj){
	// Make sure that the name is not in use //
	if(IsObjectNameInUse(obj->GetName())){

		return false;
	}

	// Add the object //
	DefinedObjects.push_back(obj);
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::IsObjectNameInUse(const wstring &name) const{
	// Try to find an object with the same name //
	for(size_t i = 0; i < DefinedObjects.size(); i++){

		if(DefinedObjects[i]->GetName() == name)
			return true;
	}

	// Check for matching template names //


	// Didn't match anything //
	return false;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::ObjectFile::GetVariables(){
	return &HeaderVars;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ObjectFile::GetTotalObjectCount() const{
	// Add the template objects to actual objects //
	return DefinedObjects.size();
}

DLLEXPORT ObjectFileObject* Leviathan::ObjectFile::GetObjectFromIndex(size_t index) const THROWS{
	// Return from DefinedObjects if it is in range otherwise from the template instances //
	if(index < DefinedObjects.size()){

		return DefinedObjects[index].get();
	}

	// If in range of templates return a template instance //


	// Invalid index //
	throw ExceptionInvalidArgument(L"index is out of range", index, __WFUNCTION__, L"index", Convert::ToWstring(index));
}
// ------------------------------------ //
DLLEXPORT ObjectFileObject* Leviathan::ObjectFile::GetObjectWithType(const wstring &typestr) const{
	// Find the first that matches the type //
	for(size_t i = 0; i < DefinedObjects.size(); i++){

		if(DefinedObjects[i]->GetTypeName() == typestr)
			return DefinedObjects[i].get();
	}

	// Check for templates //
	
	// Nothing found //
	return NULL;
}
