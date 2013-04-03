#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NAMEDVARS
#include "NamedVars.h"
#endif
using namespace Leviathan;
#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_FILESYSTEM
#include "FileSystem.h"
#endif
NamedVar::NamedVar(){
	Name = L"uninit";
	wValue = NULL;
	//wValue = L"";
	iValue = -1;
	Isint = TRUE;
}
NamedVar::NamedVar(const NamedVar &other){
	Name = other.Name;
	Isint = other.Isint;
	if(Isint){
		iValue = other.iValue;
		wValue = NULL;
	} else {
		iValue = -1;
		wValue = new wstring(*other.wValue);
	}
}
NamedVar::NamedVar(const wstring& name, int val){
	Name = name;
	wValue = NULL;
	//wValue = L"";
	iValue = val;
	Isint = TRUE;
}
NamedVar::NamedVar(const wstring& name, const wstring& val){
	Name = name;
	wValue = new wstring(val);
	//wValue = val;
	iValue = 0;
	Isint = FALSE;
}

//vector<unsigned int> Deletedindexes;

DLLEXPORT Leviathan::NamedVar::~NamedVar(){
	//SAFE_DELETE(wValue);
	//if(Deletedindexes.size() == 0)
	//	Deletedindexes.reserve(2100);

	//// check is this already on deleted list //
	//for(unsigned int i = 0; i < Deletedindexes.size(); i++){
	//	if(Deletedindexes[i] == (unsigned int) this){
	//		__debugbreak();
	//	}
	//}

	//Deletedindexes.push_back((unsigned int)this);
}


DLLEXPORT wstring Leviathan::NamedVar::ToText(int WhichSeparator /*= 0*/) const{
	switch(WhichSeparator){
	case 0:
		{
			if(this->Isint)
				return Name+L" = "+Convert::IntToWstring(iValue)+L";";
			return Name+L" = "+*wValue+L";";
		}
	break;
	case 1:
		{
			if(this->Isint)
				return Name+L": "+Convert::IntToWstring(iValue)+L";";
			return Name+L": "+*wValue+L";";
		}
		break;

	default:
		// error //
		QUICK_ERROR_MESSAGE;
		return L"ERROR: NULL";
	}
}

// ------------------------------------ //
void NamedVar::SetValue(int val){
	if(this->Isint){
		iValue = val;
		return;
	}
	this->Isint = true;
	// destroy string //
	SAFE_DELETE(wValue);
	wValue = NULL;
	//wValue.clear();
}
void NamedVar::SetValue(const wstring& val){
	this->Isint = false;
	iValue = -1;
	SAFE_DELETE(wValue);
	wValue = new wstring(val);
	//wValue = val;
}
int NamedVar::GetValue(int& val1, wstring& val2) const{
	if(this->Isint){
		val1 = this->iValue;
		return 1;
	}
	val2 = *this->wValue;
	return 0;
}

bool NamedVar::IsIntValue() const{
	return this->Isint;
}

wstring& NamedVar::GetName(){
	return this->Name;
}
void NamedVar::SetName(const wstring& name){
	this->Name = name;
}
bool NamedVar::CompareName(const wstring& name) const{
	for(unsigned int i = 0; (i < name.length() ) && (i < this->Name.length()); i++){
		if(!(name[i] == this->Name[i])){
			return false;
		}
	}
	return true;


//	return !this->Name.compare(name);
}
// ------------------------------------ //
		// process functions //
DLLEXPORT  int Leviathan::NamedVar::ProcessDataDumb(wstring& data, vector<shared_ptr<NamedVar>>& vec, vector<IntWstring*>* specialintvalues /*= NULL*/){
	// split to lines //
	vector<wstring> Lines;

	if(Misc::CutWstring(data, L";", Lines) != 0){
		// no lines //
		Logger::Get()->Info(L"Tried to ProcessDataDumb with empty string", true);
		return 400;
	}
	// make space for values //
	vec.clear();



	// fill values //	// skip last
	for(unsigned int i = 0; i < Lines.size(); i++){
		wstring cur = Lines.at(i);
		if(cur.length() == 0)
			continue;
		wstring name = L"";
		wstring tempvar = L"";

		wstring possiv = L"";
		//int possii = 0;

		wchar_t SplitChar = L':';
		for(unsigned int a = 1; a < cur.length(); a++){
			if(cur[a] == L'"')
				break;
			// check if = is used as name and variable separator //
			if(cur[a] == L'='){
				if(a-1 < 0)
					continue;
				if(cur[a-1] != L'\\'){
					SplitChar = L'=';
					break;
				}

			}

		}

		// split name/ variable //
		bool skip = false;
		unsigned int e = 0;
		bool IsNextSpecial = false;
		while((cur[e] != SplitChar) && (!IsNextSpecial)){
			if(((cur[e] < 32) | (cur[e] == L' ')) && (e+1 < cur.length())){
				e++;
				continue;
			}
			if(cur[e] == L'\\'){
				if(!IsNextSpecial){
					IsNextSpecial = true;
					continue;
				}
			} else {
				if(IsNextSpecial)
					continue;
			}
			name += cur[e];
			e++;
			if(e >= cur.length()){
				// no variable! //
				//Logger::Get()->Info(L"ProcessDataDumb name split no variable, error: 404", true);
				skip = true;
				break;
			}
		}
		// skip : //
		e++;
		if(skip)
			continue;
		// skip spaces //
		while(cur[e] == L' '){
			e++;
			if(e >= cur.length()){
				// no variable! //
				Logger::Get()->Info(L"ProcessDataDumb skip spaces no variable, error: 404", true);
				skip = true;
				break;
			}
		}
		if(skip)
			continue;
		// get temp var //
		for(e; e < cur.length(); e++){
			tempvar += cur[e];
		}
		// check is it a int value //
		bool isspecial = false;
		int specialval = 0;
		if(specialintvalues != NULL){
			for(unsigned int indexed = 0; indexed < specialintvalues->size(); indexed++){
				wstring* tempcompare = specialintvalues->at(indexed)->GetString();
				if(tempcompare != NULL){
					if(*tempcompare == tempvar){
						isspecial = true;
						specialval = specialintvalues->at(indexed)->GetValue();
						break;
					}
				}
			}

			if(isspecial){
				// is int //
				vec.push_back(shared_ptr<NamedVar>(new NamedVar(name, specialval)));
				continue;
			}
		}

		// check type //
		if(Misc::WstringTypeCheck(tempvar, 0 /* check int */) == 1){
			// is int //
			vec.push_back(shared_ptr<NamedVar>(new NamedVar(name, Convert::WstringToInt(tempvar))));
			continue;
		}
		if(Misc::WstringTypeCheck(tempvar, 3 /* check bool */) == 1){
			// is int //
			vec.push_back(shared_ptr<NamedVar>(new NamedVar(name, Convert::WstringFromBoolToInt(tempvar))));
			continue;
		}
		// is string/other //

		if(tempvar[0] == L'"'){
			// remove " marks //
			tempvar.erase(tempvar.begin());
			tempvar.erase(tempvar.begin()+tempvar.size()-1);
		}


		vec.push_back(shared_ptr<NamedVar>(new NamedVar(name, tempvar)));
		continue;
	}

	

	return 0;
}


// ---------------------------- NamedVars --------------------------------- //
NamedVars::NamedVars(){
	vector<NamedVar> variables = vector<NamedVar>();
}
NamedVars::NamedVars(const NamedVars& other){
	// deep copy is required here //
	variables.reserve(other.variables.size());
	for(unsigned int i = 0; i < other.variables.size(); i++){
		variables.push_back(shared_ptr<NamedVar>(new NamedVar(*other.variables[i])));
	}
}
NamedVars::~NamedVars(){

	// clear values //
	//SAFE_DELETE_VECTOR(variables);
	variables.clear();

}
		// ------------------------------------ //
void NamedVars::SetValue(const wstring &name, int val){
	int index = Find(name);
	if((index < 0 ) || (index > (int)variables.size())){
		Logger::Get()->Error(L"NamedVars: SetValue(find) out of range, error: 006",index);
		return;
	}
	return variables[index]->SetValue(val);
}

void NamedVars::SetValue(const wstring &name, wstring& val){
	int index = Find(name);
	if( index == -1)
		return;
	if((index < 0 ) && (index > (int)variables.size())){
		Logger::Get()->Error(L"NamedVars: SetValue(find) out of range, error: 006",index);
		return;
	}
	return variables[index]->SetValue(val);
}

int NamedVars::GetValue(const wstring &name, int& val1, wstring& val2) const{
	int index = Find(name);
	if(index == -1)
		return 404;
	if((index < 0 ) && (index > (int)variables.size())){
		Logger::Get()->Error(L"NamedVars: GetValue(find) out of range, error: 006",index);
		return 5;
	}
	return variables[index]->GetValue(val1, val2);
}

int NamedVars::GetValue(const wstring &name, int& val1) const{
	int index = Find(name);
	if(index == -1)
		return 404;
	if((index < 0 ) && (index > (int)variables.size())){
		Logger::Get()->Error(L"NamedVars: GetValue(find) out of range, error: 006",index);
		return 5;
	}
	wstring temp = L"";
	return variables[index]->GetValue(val1, temp);
}


bool NamedVars::IsIntValue(const wstring &name) const{
	int index = Find(name);
	if((index < 0 ) && (index > (int)variables.size())){
		Logger::Get()->Error(L"NamedVars: IsIntValue(find) out of range, error: 006",index);
		return false;
	}
	return variables[index]->IsIntValue();
}
bool NamedVars::IsIntValue(unsigned int index){
	if((index < 0 ) && (index > variables.size())){
		Logger::Get()->Error(L"NamedVars: IsIntValue out of range, error: 006",index);
		return false;
	}
	return variables[index]->IsIntValue();
}

wstring& NamedVars::GetName(unsigned int index) const{
	if((index < 0 ) && (index > variables.size())){
		Logger::Get()->Error(L"NamedVars: GetName out of range, error: 006",index);
		//wstring errstr = Misc::GetErrString();
		return Misc::GetErrString();
	}
	return variables[index]->GetName();
}
void NamedVars::SetName(unsigned int index, const wstring &name){
	if((index < 0 ) && (index > variables.size())){
		Logger::Get()->Error(L"NamedVars: SetName out of range, error: 006",index);
		return;
	}
	variables[index]->SetName(name);
}
void NamedVars::SetName(const wstring &oldname, const wstring &name){
	int index = Find(oldname);
	if((index < 0 ) && (index > (int)variables.size())){
		Logger::Get()->Error(L"NamedVars: SetName(find) out of range, error: 006",index);
		return;
	}
	variables[index]->SetName(name);
}
bool NamedVars::CompareName(unsigned int index, const wstring &name) const{
	return variables[index]->CompareName(name);
}
bool NamedVars::CompareName(const wstring &name1, const wstring &name2) const{
	if(name1.compare(name2) == 0)
		return true;
	return false;
}
void NamedVars::AddVar(const wstring &name, int val, const wstring &wval, bool isint){
	if(isint){
		variables.push_back(shared_ptr<NamedVar>(new NamedVar(name, val)));
	} else {
		variables.push_back(shared_ptr<NamedVar>(new NamedVar(name, wval)));
	}
}
void NamedVars::Remove(unsigned int index){
	// delete allocated memory //
	//SAFE_DELETE(variables[index]);
	// smart pointers //
	variables.erase(variables.begin()+index);
}

		// ------------------------------------ //

int NamedVars::LoadVarsFromFile(const wstring &file){
	return FileSystem::LoadDataDumb(file, variables);
}
vector<shared_ptr<NamedVar>>* NamedVars::GetVec(){
	return &variables;
}
void NamedVars::SetVec(vector<shared_ptr<NamedVar>>& vec){
	variables = vec;
}

		// ------- //
int NamedVars::Find(const wstring &name) const{
	for(unsigned int i = 0; i < variables.size(); i++){
		if(variables[i]->CompareName(name))
			return i;
	}
	return -1;
}
