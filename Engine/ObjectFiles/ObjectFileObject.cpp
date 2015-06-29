// ------------------------------------ //
#include "ObjectFileObject.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileObjectProper::ObjectFileObjectProper(const std::string &name,
    const std::string &typesname, vector<std::string*> prefix) : 
	Name(name), TName(typesname), Prefixes(prefix)
{
	
}

DLLEXPORT Leviathan::ObjectFileObjectProper::~ObjectFileObjectProper(){
	// Let the script go //
	Script.reset();

	// Release memory //
	SAFE_DELETE_VECTOR(Prefixes);
	SAFE_DELETE_VECTOR(Contents);
	SAFE_DELETE_VECTOR(TextBlocks);
}
// ------------------------------------ //
DLLEXPORT const std::string& Leviathan::ObjectFileObjectProper::GetName() const{
	return Name;
}

DLLEXPORT const std::string& Leviathan::ObjectFileObjectProper::GetTypeName() const{
	return TName;
}

DLLEXPORT std::shared_ptr<ScriptScript> Leviathan::ObjectFileObjectProper::GetScript() const{
	return Script;
}

DLLEXPORT size_t Leviathan::ObjectFileObjectProper::GetPrefixesCount() const{
	return Prefixes.size();
}

DLLEXPORT const std::string& Leviathan::ObjectFileObjectProper::GetPrefix(size_t index) const{
	// Check the index //
	if(index >= Prefixes.size()){

		throw InvalidArgument("index is out of range");
	}

	return *Prefixes[index];
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileObjectProper::AddVariableList(unique_ptr<ObjectFileList> &list){
	// Make sure name is unique //
	for(size_t i = 0; i < Contents.size(); i++){

		if(Contents[i]->GetName() == list->GetName()){
			return false;
		}
	}

	// Add it //
	Contents.push_back(list.release());
	return true;
}

DLLEXPORT bool Leviathan::ObjectFileObjectProper::AddTextBlock(unique_ptr<ObjectFileTextBlock> &tblock){
	// Make sure name is unique //
	for(size_t i = 0; i < TextBlocks.size(); i++){

		if(TextBlocks[i]->GetName() == tblock->GetName()){
			return false;
		}
	}

	// Add it //
	TextBlocks.push_back(tblock.release());
	return true;
}

DLLEXPORT void Leviathan::ObjectFileObjectProper::AddScriptScript(shared_ptr<ScriptScript> script){
	// Warn if we already have a script //
	if(Script){

		Logger::Get()->Warning("ObjectFileObject: already has a script, overwrote old one, name: "
            +Name);
	}

	Script = script;
}
// ------------------------------------ //
DLLEXPORT ObjectFileList* Leviathan::ObjectFileObjectProper::GetListWithName(const std::string &name) const{
	// Loop and compare names //
	for(size_t i = 0; i < Contents.size(); i++){

		if(Contents[i]->GetName() == name)
			return Contents[i];
	}

	// Not found //
	return NULL;
}

DLLEXPORT ObjectFileTextBlock* Leviathan::ObjectFileObjectProper::GetTextBlockWithName(const std::string &name) const{
	// Loop and compare names //
	for(size_t i = 0; i < TextBlocks.size(); i++){

		if(TextBlocks[i]->GetName() == name)
			return TextBlocks[i];
	}

	// Not found //
	return NULL;
}

DLLEXPORT size_t Leviathan::ObjectFileObjectProper::GetListCount() const{
	return Contents.size();
}

DLLEXPORT ObjectFileList* Leviathan::ObjectFileObjectProper::GetList(size_t index) const{
	// Check the index //
	if(index >= Contents.size()){

		throw InvalidArgument("index is out of range");
	}

	return Contents[index];
}

DLLEXPORT size_t Leviathan::ObjectFileObjectProper::GetTextBlockCount() const{
	return TextBlocks.size();
}

DLLEXPORT ObjectFileTextBlock* Leviathan::ObjectFileObjectProper::GetTextBlock(size_t index) const{
	// Check the index //
	if(index >= TextBlocks.size()){

		throw InvalidArgument("index is out of range");
	}

	return TextBlocks[index];
}

DLLEXPORT bool Leviathan::ObjectFileObjectProper::IsThisTemplated() const{
	return false;
}

// ------------------ ObjectFileObject ------------------ //
DLLEXPORT Leviathan::ObjectFileObject::~ObjectFileObject(){

}
