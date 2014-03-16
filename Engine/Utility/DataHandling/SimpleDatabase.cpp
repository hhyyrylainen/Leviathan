#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SIMPLEDATABASE
#include "SimpleDatabase.h"
#endif
#include "FileSystem.h"
#include "Common/StringOperations.h"
#include "../Iterators/WstringIterator.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SimpleDatabase::SimpleDatabase(const wstring &databasename){

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


	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SimpleDatabase::GetRow(std::vector<wstring> &row, const wstring &table, int row_index, const std::vector<wstring> &columns){
	// If we are missing the database we shouldn't add it //
	SimpleDatabaseObject::iterator iter = Database.find(table);

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


		auto iter2 = datbaseentry.find(columns[i]);
		if(iter2 != datbaseentry.end()){
			// Add to result //
			row.push_back(iter2->second->operator wstring());
		}
	}


}

DLLEXPORT int Leviathan::SimpleDatabase::GetNumRows(const wstring &table){
	// If we are missing the database we shouldn't add it //
	SimpleDatabaseObject::iterator iter = Database.find(table);

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
DLLEXPORT bool Leviathan::SimpleDatabase::LoadFromFile(const wstring &file){
	// The file should be able to be processed as named variable lists //
	// read the file entirely //
	wstring filecontents;

	try{
		FileSystem::ReadFileEntirely(file, filecontents);
	}
	catch(const ExceptionInvalidArgument &e){

		Logger::Get()->Error(L"SimpleDatabase: LoadFromFile: file could not be read, exception:");
		e.PrintToLog();
		return false;
	}


	// file needs to be split to lines //
	vector<wstring> Lines;

	if(!StringOperations::CutString(filecontents, wstring(L"\n"), Lines)){

		Lines.push_back(filecontents);
		Logger::Get()->Warning(L"ObjectFileProcessor: file seems to be only a single line: "+filecontents);
	}

	// remove excess spaces //
	for(size_t i = 0; i < Lines.size(); i++){

		WstringIterator::StripPreceedingAndTrailingWhitespaceComments(Lines[i]);
	}

	SimpleDatabaseObject::iterator insertiter;

	for(size_t i = 0; i < Lines.size(); i++){
		// skip empty lines //
		if(Lines[i].size() == 0){
			continue;
		}

		// Check is this new table //
		if(StringOperations::StringStartsWith<wstring>(Lines[i], L"TABLE")){
			// Move to a new table //

			WstringIterator itr(&Lines[i], false);

			auto tablename = itr.GetStringInQuotes(QUOTETYPE_BOTH);

			Database[*tablename] = shared_ptr<std::vector<shared_ptr<SimpleDatabaseRowObject>>>(new std::vector<shared_ptr<SimpleDatabaseRowObject>>());

			// Change the iter //
			insertiter = Database.find(*tablename);
			continue;
		}

		// try to create a named var from this line //
		try{
			shared_ptr<NamedVariableList> namevar(new NamedVariableList(Lines[i], NULL));
			// didn't cause an exception, is valid add //

			insertiter->second->push_back(shared_ptr<SimpleDatabaseRowObject>(new SimpleDatabaseRowObject()));

			auto toinsert = insertiter->second->back();

			if(namevar->GetVariableCount() % 2 != 0){

				Logger::Get()->Warning(L"SimpleDatabase: LoadFromFile: file: "+file+L", line: "+Convert::ToWstring(i)+L" has invalid number of elements");
				continue;
			}

			for(size_t namei = 0; namei < namevar->GetVariableCount(); namei += 2){

				wstring name;

				namevar->GetValueDirect(namei)->ConvertAndAssingToVariable<wstring>(name);

				string blockdata;

				if(!namevar->GetValueDirect(namei+1)->ConvertAndAssingToVariable<string>(blockdata)){
					Logger::Get()->Warning(L"SimpleDatabase: LoadFromFile: file: "+file+L", line: "+Convert::ToWstring(i)+L" couldn't convert value to string");
				}

				(*toinsert)[name] = shared_ptr<VariableBlock>(new VariableBlock(blockdata));
			}
		}
		catch(...){
			continue;
		}
	}
	// Is done //
	return true;
}

DLLEXPORT void Leviathan::SimpleDatabase::SaveToFile(const wstring &file){

	wstring datastr;
	wstring tmpdata;

	// Just iterate over everything and write them to file //
	for(auto iter = Database.begin(); iter != Database.end(); ++iter){

		datastr += L"TABLE = \""+iter->first+L"\";\n";

		for(auto iter2 = iter->second->begin(); iter2 != iter->second->end(); ++iter2){

			datastr += L"n= [";

			for(auto iter3 = (*iter2)->begin(); iter3 != (*iter2)->end(); ++iter3){

				datastr += L"[\""+iter3->first+L"\"]";

				if(!iter3->second->ConvertAndAssingToVariable<wstring>(tmpdata)){

					assert(0 && "database has a value that cannot be stored as a string");
				}
				datastr += L"[\""+tmpdata+L"\"]";
			}
			datastr += L"];\n";
		}
	}

	FileSystem::WriteToFile(datastr, file);
}

