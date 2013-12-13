#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SIMPLEDATABASE
#include "SimpleDatabase.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SimpleDatabase::SimpleDatabase(const string &rocketname) : Rocket::Controls::DataSource(rocketname.c_str()){

}

DLLEXPORT Leviathan::SimpleDatabase::~SimpleDatabase(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SimpleDatabase::AddValue(const wstring &database, shared_ptr<SimpleDatabaseRowObject> valuenamesandvalues){
	// Using the database object add a new value to correct vector //
	auto iter = _EnsureTable(database);

	// Return false if new table was not able to be added //
	if(iter == Database.end())
		return false;

	// Push back a new row //
	iter->second->push_back(valuenamesandvalues);

	// Notify update //
	NotifyRowAdd(Convert::WstringToString(database).c_str(), iter->second->size()-1, 1);
	return true;
}

DLLEXPORT bool Leviathan::SimpleDatabase::RemoveValue(const wstring &database, int row){
	// If we are missing the database we shouldn't add it //
	SimpleDatabaseObject::iterator iter = Database.find(database);

	if(iter == Database.end()){
		// No such database //
		return false;
	}

	// Remove at the specified index if possible //
	if(iter->second->size() <= (size_t)row || row < 0)
		return false;

	// Remove //
	iter->second->erase(iter->second->begin()+(size_t)row);
	// Notify update //
	NotifyRowRemove(Convert::WstringToString(database).c_str(), row, 1);
	return true;
}
// ------------------------------------ //
void Leviathan::SimpleDatabase::GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns){
	// If we are missing the database we shouldn't add it //
	SimpleDatabaseObject::iterator iter = Database.find(Convert::StringToWstring(table.CString()));

	if(iter == Database.end()){
		// No such database //
		return;
	}

	// Validate index //
	if(iter->second->size() <= (size_t)row_index || row_index < 0){
		return;
	}

	// Valid value //
	const std::map<wstring, shared_ptr<VariableBlock>>& datbaseentry = *iter->second->at(row_index).get();

	// Copy data //
	for(size_t i = 0; i < columns.size(); i++){


		auto iter2 = datbaseentry.find(Convert::StringToWstring(columns[i].CString()));
		if(iter2 != datbaseentry.end()){
			// Add to result //
			row.push_back((iter2->second->operator string()).c_str());
		}
	}


}

int Leviathan::SimpleDatabase::GetNumRows(const Rocket::Core::String& table){
	// If we are missing the database we shouldn't add it //
	SimpleDatabaseObject::iterator iter = Database.find(Convert::StringToWstring(table.CString()));

	if(iter == Database.end()){
		// No such database //
		return 0;
	}

	return iter->second->size();
}


DLLEXPORT shared_ptr<VariableBlock> Leviathan::SimpleDatabase::GetValueOnRow(const wstring &table, const wstring &valuekeyname, const VariableBlock &wantedvalue, const wstring &wantedvaluekey){
	// Search the database for matching row and return another value from that row //
	// If we are missing the database we shouldn't add it //
	SimpleDatabaseObject::iterator iter = Database.find(table);

	if(iter == Database.end()){
		// No such database //
		return NULL;
	}

	// Search the database //
	for(size_t i = 0; i < iter->second->size(); i++){

		auto finditer = iter->second->at(i)->find(valuekeyname);

		if(finditer == iter->second->at(i)->end())
			continue;

		// Check does the value match //
		if(*finditer->second == wantedvalue){
			// Found match //
			auto wantedfinder = iter->second->at(i)->find(wantedvaluekey);

			if(wantedfinder != iter->second->at(i)->end()){

				return wantedfinder->second;
			}
		}
	}

	return NULL;
}
// ------------------------------------ //
SimpleDatabaseObject::iterator Leviathan::SimpleDatabase::_EnsureTable(const wstring &name){
	// Try to find it //
	SimpleDatabaseObject::iterator iter = Database.find(name);

	if(iter != Database.end()){
		// Valid database //
		return iter;
	}
	// Ensure new database //
	Database[name] = shared_ptr<std::vector<shared_ptr<std::map<wstring, shared_ptr<VariableBlock>>>>>(new
		std::vector<shared_ptr<std::map<wstring, shared_ptr<VariableBlock>>>>());

	// Recurse, might want to avoid stack overflow //
	return _EnsureTable(name);
}
// ------------------------------------ //






