#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_INTERFACE
#include "ScriptInterface.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"

Leviathan::ScriptInterface::ScriptInterface() : ScriptRunner(NULL){
	StaticAccess = this;
}
Leviathan::ScriptInterface::~ScriptInterface(){
	StaticAccess = NULL;
}

ScriptInterface* Leviathan::ScriptInterface::StaticAccess = NULL;
ScriptInterface* Leviathan::ScriptInterface::Get(){ return StaticAccess; };
// ------------------------------------ //
bool Leviathan::ScriptInterface::Init(){
	// create script executor //
	ScriptRunner = new ScriptExecutor();
	if(!ScriptRunner->Init()){

		Logger::Get()->Error(L"ScriptInterface: Init: ScriptExecutor init failed",true);
		return false;
	}

	return true;
}
void Leviathan::ScriptInterface::Release(){
	// delete managed scripts //
	while(Managed.size() != 0){
		// release //
		ReleaseScript(0);
	}

	// send event for scripts shutting down //
	// TODO: this

	// script runner needs to be released //
	SAFE_RELEASEDEL(ScriptRunner);
}
// ------------------------------------ //
void Leviathan::ScriptInterface::TakeOwnerShip(ScriptObject* obj){
	// takes the object as manageable //
	Managed.push_back(obj);
}

void Leviathan::ScriptInterface::RemoveOwnerShip(ScriptObject* obj, size_t index){
	if(index != -1){
		// TODO: use script executor to see if on delete exists //
		Managed.erase(Managed.begin()+index);
		return;
	}
	index = SearchScript(L"", obj);
	ARR_INDEX_CHECKINV((unsigned int)index, Managed.size()){

		Logger::Get()->Error(L"ScriptInterface: RemoveOwnerShip: found invalid index", index, true);
		return;
	}
	Managed.erase(Managed.begin()+index);
}

void Leviathan::ScriptInterface::ReleaseScript(size_t index){
	ARR_INDEX_CHECKINV(index, Managed.size()){

		Logger::Get()->Error(L"ScriptInterface: trying to release script with invalid index", index, true);
		return;
	}
	// TODO: use script executor to see if on delete exists //
	SAFE_DELETE(Managed[index]);
	Managed.erase(Managed.begin()+index);
}
// ------------------------------------ //
int Leviathan::ScriptInterface::SearchScript(const wstring &name, ScriptObject* obj){
	if(obj != NULL){
		// search by pointer 
		for(size_t i = 0; i < Managed.size(); i++){
			if(Managed[i] == obj)
				return i;
		}
		return -1;
	}
	for(size_t i = 0; i < Managed.size(); i++){
		if(Managed[i]->Name == name)
			return i;
	}
	return -1;
}
int Leviathan::ScriptInterface::RunManagedScript(size_t index){
	return 0;
}



// ------------------------------------ //
DLLEXPORT shared_ptr<ScriptArguement> Leviathan::ScriptInterface::ExecuteScript(ScriptScript* obj, const wstring &entrypoint, 
	vector<shared_ptr<ScriptNamedArguement>> Parameters, bool FullDecl /*= false*/)
{
	bool exists = false;
	// run script //
	return ScriptRunner->RunScript(obj, Parameters, true, entrypoint, exists, true, FullDecl);
}
DLLEXPORT shared_ptr<ScriptArguement> Leviathan::ScriptInterface::ExecuteScript(ScriptObject* obj, const wstring &entrypoint, 
	vector<shared_ptr<ScriptNamedArguement>> Parameters, bool FullDecl /*= false*/)
{
	// run script //
	return ExecuteScript(obj->Script.get(), entrypoint, Parameters, FullDecl);
}

DLLEXPORT shared_ptr<ScriptArguement> Leviathan::ScriptInterface::ExecuteIfExistsScript(ScriptScript* obj, const wstring &entrypoint, 
	vector<shared_ptr<ScriptNamedArguement>> Parameters, bool &existreceiver, bool FullDecl /*= false*/)
{
	// run script if exists function //
	return ScriptRunner->RunScript(obj, Parameters, true, entrypoint, existreceiver, false, FullDecl);
}
DLLEXPORT shared_ptr<ScriptArguement> Leviathan::ScriptInterface::ExecuteIfExistsScript(ScriptObject* obj, const wstring &entrypoint, 
	vector<shared_ptr<ScriptNamedArguement>> Parameters, bool &existreceiver, bool FullDecl /*= false*/)
{
	// run script if exists function //
	return ExecuteIfExistsScript(obj->Script.get(), entrypoint, Parameters, existreceiver, FullDecl);
}