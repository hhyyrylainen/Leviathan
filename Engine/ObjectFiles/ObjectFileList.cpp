// ------------------------------------ //
#include "ObjectFileList.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileListProper::ObjectFileListProper(const std::string &name) :
    Name(name)
{

}
// ------------------------------------ //
DLLEXPORT const std::string& Leviathan::ObjectFileListProper::GetName() const{
	return Name;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileListProper::AddVariable(shared_ptr<NamedVariableList> var){
	// Make sure that name is not in use //
	if(Variables.Find(var->GetName()) < Variables.GetVariableCount()){
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
