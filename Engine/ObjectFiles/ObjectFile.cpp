#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE
#include "ObjectFile.h"
#endif
#include "ObjectFileTemplates.h"
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
	throw InvalidArgument("index is out of range");
}
// ------------------------------------ //
DLLEXPORT ObjectFileObject* Leviathan::ObjectFile::GetObjectWithType(const wstring &typestr) const{
	// Find the first that matches the type //
	for(size_t i = 0; i < DefinedObjects.size(); i++){

		if(DefinedObjects[i]->GetTypeName() == typestr)
			return DefinedObjects[i].get();
	}

	// Nothing found //
	return NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFile::GenerateTemplatedObjects(){
	// Create template instances from the templates //
	for(size_t i = 0; i < TemplateInstantiations.size(); i++){

		// Find a template definition for this //
		auto tmpldef = FindTemplateDefinition(TemplateInstantiations[i]->GetNameOfParentTemplate());

		if(!tmpldef){

			Logger::Get()->Error(L"ObjectFile: could not find template definition with name: "
				+Convert::StringToWstring(TemplateInstantiations[i]->GetNameOfParentTemplate()));
			return false;
		}

		// Try to generate it //
		auto resultobj = tmpldef->CreateInstanceFromThis(*TemplateInstantiations[i]);

		if(!resultobj){

			Logger::Get()->Error(L"ObjectFile: could not instantiate template (parameter count probably didn't match), name: "
				+Convert::StringToWstring(TemplateInstantiations[i]->GetNameOfParentTemplate()));
			return false;
		}

		shared_ptr<ObjectFileObject> tmpobj(resultobj.release());

#ifdef _DEBUG
		assert(tmpobj->IsThisTemplated() && "ObjectFile template created object must return true from IsThisTemplated");
#endif // _DEBUG

		// Add it to us //
		if(!AddObject(tmpobj)){
			
			Logger::Get()->Error(L"ObjectFile: template instance's result was an object whose name is already in use, template name: "
				+Convert::StringToWstring(TemplateInstantiations[i]->GetNameOfParentTemplate())+L", result object: "+tmpobj->GetName());
			return false;
		}
	}


	// No errors occurred //
	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ObjectFile::AddTemplateInstance(shared_ptr<ObjectFileTemplateInstance> tiobj){
	// Template instances may have names that are not present yet, the instance can be before the definition //
	TemplateInstantiations.push_back(tiobj);
}

DLLEXPORT bool Leviathan::ObjectFile::AddTemplate(shared_ptr<ObjectFileTemplateDefinition> templatedef){
	// Make sure the name is unique //
	for(size_t i = 0; i < TemplateDefinitions.size(); i++){

		if(TemplateDefinitions[i]->GetName() == templatedef->GetName()){

			// Conflicting name //
			return false;
		}
	}

	// Add it //
	TemplateDefinitions.push_back(templatedef);
	return true;
}

DLLEXPORT shared_ptr<ObjectFileTemplateDefinition> Leviathan::ObjectFile::FindTemplateDefinition(const string &name) const{
	// Try to find one with the exact name //
	for(size_t i = 0; i < TemplateDefinitions.size(); i++){

		if(TemplateDefinitions[i]->GetName() == name){

			// Found a match //
			return TemplateDefinitions[i];
		}
	}

	// Nothing found //
	return NULL;
}
