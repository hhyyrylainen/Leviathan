#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NAMEDVARS
#include "NamedVars.h"
#endif
#include "FileSystem.h"
#include "Statistics/TimingMonitor.h"
#include "Utility/Iterators/WstringIterator.h"
#include "Exceptions/ExceptionInvalidType.h"
#include "ObjectFiles/LineTokenizer.h"
using namespace Leviathan;
// ------------------------------------ //

Leviathan::NamedVariableList::NamedVariableList() : Datas(1), Name(L""){
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const wstring &name, VariableBlock* value1) : Datas(1), Name(name){
	// set value //
	Datas[0] = value1;
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const wstring &name, const VariableBlock &val) : Datas(1), Name(name){
	// set value //
	Datas[0] = new VariableBlock(val);
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const wstring &name, vector<VariableBlock*> values_willclear) :
	Datas(values_willclear.size()), Name(name)
{
	// set values //
	for(size_t i = 0; i < values_willclear.size(); i++){
		Datas[i] = values_willclear[i];
	}
}

Leviathan::NamedVariableList::NamedVariableList(const NamedVariableList &other) : Datas(other.Datas.size()), Name(other.Name){

	// copy value over //
	for(size_t i = 0; i < other.Datas.size(); i++){

		Datas[i] = new VariableBlock(*other.Datas[i]);
	}
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(wstring &line, map<wstring, shared_ptr<VariableBlock>>* predefined /*= NULL*/) : Datas(1){
	// using WstringIterator makes this shorter //
	WstringIterator itr(&line, false);

	unique_ptr<wstring> name = itr.GetUntilEqualityAssignment(EQUALITYCHARACTER_TYPE_ALL);

	if(name->size() < 1){
		// no name //
		throw ExceptionInvalidArgument(L"invalid data on line (invalid name)", name->size(), __WFUNCSIG__, L"line", line);
	}

	// TODO: verify that this will be destructed if an exception happens //
	Name = *name;

	// skip whitespace //
	itr.SkipWhiteSpace();

	// get last part of it //
	//unique_ptr<wstring> tempvar = itr.GetUntilEnd();
	unique_ptr<wstring> tempvar = itr.GetUntilNextCharacterOrAll(L';');

	if(tempvar->size() < 1){
		// no variable //
		throw ExceptionInvalidArgument(L"invalid data on line (no variable)", tempvar->size(), __WFUNCSIG__, L"line", line);
	}

	// check does it have brackets (and need to be processed like so) //
	//tempvar->find_first_of(L'[') != wstring::npos
	if(tempvar->at(0) == L'['){

		// needs to be tokenized //

		vector<Token*> tokens;
		// split to tokens //
		LineTokeNizer::SplitTokenToRTokens(*tempvar, tokens);

		if(tokens.size() < 2){
			// release tokens to not leak any memory //
			SAFE_DELETE_VECTOR(tokens);

			// might contain the base token, but cannot possibly have any values inside //
			throw ExceptionInvalidArgument(L"invalid data on line (variable tokenization failed)", tokens.size(), __WFUNCSIG__, L"line", line);
		}

		// first should be base token //

		// reserve space //
		Datas.resize(tokens[0]->GetSubTokenCount());

		// iterate sub tokens and create values from them //
		for(int i = 0; i < tokens[0]->GetSubTokenCount(); i++){

			try{
				// try to create new VariableBlock //
				//Datas.push_back(new VariableBlock(new WstringBlock(tokens[0]->GetSubToken(i)->GetData())));
				Datas[i] = new VariableBlock(tokens[0]->GetSubToken(i)->GetChangeableData(), predefined);
			}
			catch (const ExceptionInvalidArgument &e){
				// release memory //
				SAFE_DELETE_VECTOR(tokens);
				SAFE_DELETE_VECTOR(Datas);

				// rethrow the exception //
				if(e.GetInvalidValueAsPtr()->size())
					throw;
			}
		}
		// all variables are now created //

		// release tokens //
		SAFE_DELETE_VECTOR(tokens);



		// don't want to fall to single value processing //
		return;
	}
	// just one value //
	try{
		// try to create new VariableBlock //
		// it should always have one element //
		if(Datas.size() == 0){
			Datas.push_back(new VariableBlock(*tempvar, predefined));
		} else {
			SAFE_DELETE(Datas[0]);
			Datas[0] = new VariableBlock(*tempvar, predefined);
		}
	}
	catch (const ExceptionInvalidArgument &e){
		// release memory //
		//SAFE_DELETE_VECTOR(Datas);

		// rethrow the exception //
		if(e.GetInvalidValueAsPtr()->size())
			throw;
	}

}

//vector<unsigned int> Deletedindexes;
DLLEXPORT Leviathan::NamedVariableList::~NamedVariableList(){

	SAFE_DELETE_VECTOR(Datas);

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
// ------------------------------------ //
DLLEXPORT void Leviathan::NamedVariableList::SetValue(const VariableBlock &value1){
	// clear old //
	_DeleteAllButFirst();
	// assign value //
	if(Datas.size() == 0){
		// create new //
		Datas.push_back(new VariableBlock(value1));

	} else {
		// assign to existing //
		*Datas[0] = value1;
	}
}

DLLEXPORT void Leviathan::NamedVariableList::SetValue(VariableBlock* value1){
	// clear old //
	SAFE_DELETE_VECTOR(Datas);

	// put value to vector //
	Datas.push_back(value1);
}

DLLEXPORT void Leviathan::NamedVariableList::SetValue(const int &nindex, const VariableBlock &valuetoset){
	// check do we need to allocate new //
	if(Datas.size() <= (size_t)nindex){

		// resize to have enough space //
		Datas.resize(nindex+1, NULL);
		Datas[nindex] = new VariableBlock(valuetoset);
	} else {

		if(Datas[nindex] != NULL){
			// assign to existing value //
			*Datas[nindex] = valuetoset;
		} else {
			// new value needed //
			Datas[nindex] = new VariableBlock(valuetoset);
		}
	}
}

DLLEXPORT void Leviathan::NamedVariableList::SetValue(const int &nindex, VariableBlock* valuetoset){
	// check do we need to allocate new //
	if(Datas.size() <= (size_t)nindex){

		// resize to have enough space //
		Datas.resize(nindex+1, NULL);
		// just copy the pointer //
		Datas[nindex] = valuetoset;
	} else {

		if(Datas[nindex] != NULL){
			// existing value needs to be deleted //
			SAFE_DELETE(Datas[nindex]);
		}
		// set pointer //
		Datas[nindex] = valuetoset;
	}
}

DLLEXPORT void Leviathan::NamedVariableList::SetValue(const vector<VariableBlock*> &values){
	// delete old //
	SAFE_DELETE_VECTOR(Datas);

	// copy vector (will copy pointers and steal them) //
	Datas = values;
}

DLLEXPORT VariableBlock& Leviathan::NamedVariableList::GetValue() THROWS{
	// uses vector operator to get value, might throw something //
	return *Datas[0];
}

DLLEXPORT VariableBlock& Leviathan::NamedVariableList::GetValue(const int &nindex) THROWS{
	// uses vector operator to get value, might throw something //
	return *Datas[nindex];
}

wstring& Leviathan::NamedVariableList::GetName(){
	return Name;
}

DLLEXPORT void Leviathan::NamedVariableList::GetName(wstring &name) const{
	// return name in a reference //
	name = Name;
}

void Leviathan::NamedVariableList::SetName(const wstring& name){
	Name = name;
}

bool Leviathan::NamedVariableList::CompareName(const wstring& name) const{
	// just default comparison //
	return Name.compare(name) == 0;
}
DLLEXPORT wstring Leviathan::NamedVariableList::ToText(int WhichSeparator /*= 0*/) const{

	wstring stringifiedval = Name+L" ";

	switch(WhichSeparator){
	case 0: stringifiedval += L"= "; break;
	case 1: stringifiedval += L": "; break;
	default:
		// error //
		QUICK_ERROR_MESSAGE;
		return L"ERROR: NULL";
	}



	// convert value to wstring //
	// starting bracket //
	stringifiedval += L"[";

	// reserve some space //
	stringifiedval.reserve(Datas.size()*4);

	for(size_t i = 0; i < Datas.size(); i++){

		// check is conversion allowed //
		if(!Datas[i]->IsConversionAllowedNonPtr<wstring>()){
			// no choice but to throw exception //
			throw ExceptionInvalidType(L"value cannot be cast to wstring", Datas[i]->GetBlock()->Type, __WFUNCTION__, L"Datas["+
				Convert::ToWstring<int>(i)+L"]", Convert::ToWstring(typeid(Datas[i]->GetBlock()).name()));
		}
		if(i != 0)
			stringifiedval += L",";
		stringifiedval += L"["+Datas[i]->operator wstring()+L"]";
	}


	// add ending bracket and done //
	stringifiedval += L"];";

	return stringifiedval;
}

DLLEXPORT NamedVariableList& Leviathan::NamedVariableList::operator=(const NamedVariableList &other){
	// copy values //
	Name = other.Name;

	SAFE_DELETE_VECTOR(Datas);
	Datas.resize(other.Datas.size());
	// copy values over //
	for(size_t i = 0; i < other.Datas.size(); i++){

		Datas[i] = new VariableBlock(*other.Datas[i]);
	}

	// return this as result //
	return *this;
}

// ----------------- process functions ------------------- //
DLLEXPORT int Leviathan::NamedVariableList::ProcessDataDump(const wstring &data, vector<shared_ptr<NamedVariableList>> &vec,
	map<wstring, shared_ptr<VariableBlock>>* predefined /*= NULL*/)
{
	//QUICKTIME_THISSCOPE;
	// split to lines //
	vector<shared_ptr<wstring>> Lines;

	WstringIterator itr(data);

	// use wstring iterator to get the lines that are separated by ; //
	unique_ptr<wstring> curline;
	int linelength = 0;
	do {
		curline = itr.GetUntilNextCharacterOrNothing(L';');
		linelength = curline->size();

		wstring* tmp = curline.release();

		Lines.push_back(shared_ptr<wstring>(tmp));
	} while(linelength != 0);


	if(Lines.size() < 1){
		// no lines //
		Logger::Get()->Error(L"NamedVar: ProcessDataDump: No lines (even 1 line requires ending ';' to work)", data.length(), false);
#ifdef _DEBUG
		Logger::Get()->Info(L"[DETAILS] data: "+data, true);
#endif // _DEBUG
		return 400;
	}
	// make space for values //
	// let's reserve space //
	vec.reserve(Lines.size());

	// fill values //
	for(unsigned int i = 0; i < Lines.size(); i++){
		// skip empty lines //
		if(Lines[i]->size() == 0)
			continue;

		// create a named var //
		try{
			shared_ptr<NamedVariableList> var(new NamedVariableList(*Lines[i], predefined));
			vec.push_back(var);
		}
		catch (const ExceptionInvalidArgument &e){
			// print to log //
			e.PrintToLog();
			// exception throws, must be invalid line //
			Logger::Get()->Info(L"NamedVar: ProcessDataDump: contains invalid line, line (with only ASCII characters): "+
				Convert::StringToWstring(Convert::WstringToString(*Lines[i]))+L"\nEND", false);
			continue;
		}

		// check is it valid //
		if(vec.back()->GetName().size() == 0 || vec.back()->GetName().size() > 10000){
			// invalid //
			Logger::Get()->Error(L"NamedVar: ProcessDataDump: invalid NamedVar generated for line: "+*Lines[i]+L"\nEND");
			DEBUG_BREAK;
			vec.erase(vec.end());
		}

		continue;
	}

	return 0;
}

DLLEXPORT  void Leviathan::NamedVariableList::SwitchValues(NamedVariableList &receiver, NamedVariableList &donator){
	// only overwrite name if there is one //
	if(donator.Name.size() > 0)
		receiver.Name = donator.Name;


	SAFE_DELETE_VECTOR(receiver.Datas);
	// resize to match sizes to avoid excess resizing //
	receiver.Datas.resize(donator.Datas.size());

	for(size_t i = 0; i < donator.Datas.size(); i++){

		receiver.Datas[i] = donator.Datas[i];

	}
	// clear donator data //
	donator.Datas.clear();
}

DLLEXPORT VariableBlock* Leviathan::NamedVariableList::GetValueDirect(){
	// return first element //
	return Datas.size() ? Datas[0]: NULL;
}

DLLEXPORT VariableBlock* Leviathan::NamedVariableList::GetValueDirect(const int &nindex){
	ARR_INDEX_CHECKINV(nindex, Datas.size()){
		// out of bounds //
		return NULL;
	}
	return Datas[nindex];
}

DLLEXPORT size_t Leviathan::NamedVariableList::GetVariableCount() const{
	return Datas.size();
}

DLLEXPORT int Leviathan::NamedVariableList::GetCommonType() const{
	// if all have a common type return it //
	if(Datas.size() == 0)
		// no common type //
		return DATABLOCK_TYPE_ERROR;

	int lasttype = Datas[0]->GetBlock()->Type;

	for(size_t i = 1; i < Datas.size(); i++){

		if(lasttype != Datas[i]->GetBlock()->Type){
			// not same type //
			return DATABLOCK_TYPE_ERROR;
		}
	}
	// there is a common type //
	return lasttype;
}

DLLEXPORT int Leviathan::NamedVariableList::GetVariableType() const{
	// get variable type of first index //
	return Datas.size() ? Datas[0]->GetBlock()->Type: DATABLOCK_TYPE_ERROR;
}

DLLEXPORT int Leviathan::NamedVariableList::GetVariableType(const int &nindex) const{
	ARR_INDEX_CHECKINV(nindex, Datas.size()){
		// out of bounds //
		return DATABLOCK_TYPE_ERROR;
	}
	return Datas[nindex]->GetBlock()->Type;
}

DLLEXPORT VariableBlock& Leviathan::NamedVariableList::operator[](const int &nindex) THROWS{
	// will allow to throw any exceptions the vector wants //
	return *Datas[nindex];
}

DLLEXPORT vector<VariableBlock*>& Leviathan::NamedVariableList::GetValues(){
	return Datas;
}

// ---------------------------- NamedVars --------------------------------- //
Leviathan::NamedVars::NamedVars() : Variables(){
	// nothing to initialize //
}
Leviathan::NamedVars::NamedVars(const NamedVars& other){
	// deep copy is required here //
	Variables.reserve(other.Variables.size());
	for(unsigned int i = 0; i < other.Variables.size(); i++){
		Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(*other.Variables[i])));
	}
}
DLLEXPORT Leviathan::NamedVars::NamedVars(const wstring &datadump) : Variables(){

	// load data directly to vector //
	if(NamedVariableList::ProcessDataDump(datadump, Variables, NULL) != 0){
		// error happened //
		Logger::Get()->Error(L"NamedVars: Initialize: process datadump failed", true);
	}
}

DLLEXPORT Leviathan::NamedVars::NamedVars(const vector<shared_ptr<NamedVariableList>> &variables) : Variables(variables){

}

DLLEXPORT Leviathan::NamedVars::NamedVars(shared_ptr<NamedVariableList> variable) : Variables(1){
	// store the single variable //
	Variables[0] = variable;
}

Leviathan::NamedVars::~NamedVars(){
	// no need to release due to smart pointers //
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NamedVars::SetValue(const wstring &name, const VariableBlock &value1){
	int index = Find(name);

	ARR_INDEX_CHECKINV(index, Variables.size()){

		return false;
	}

	Variables[index]->SetValue(value1);
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::SetValue(const wstring &name, VariableBlock* value1){
	int index = Find(name);

	ARR_INDEX_CHECKINV(index, Variables.size()){

		return false;
	}

	Variables[index]->SetValue(value1);
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::SetValue(const wstring &name, const vector<VariableBlock*> &values){
	int index = Find(name);

	ARR_INDEX_CHECKINV(index, Variables.size()){

		return false;
	}

	Variables[index]->SetValue(values);
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::SetValue(NamedVariableList &nameandvalues){
	int index = Find(nameandvalues.Name);
	// index check //
	ARR_INDEX_CHECKINV(index, (int)Variables.size()){
		Logger::Get()->Warning(L"NamedVars: SetValue: not found, creating new for: "+nameandvalues.GetName(), false);

		Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(nameandvalues)));
		return true;
	}
	nameandvalues.Name.clear();
	// set values with "swap" //
	NamedVariableList::SwitchValues(*Variables[index].get(), nameandvalues);
	return true;
}

DLLEXPORT VariableBlock& Leviathan::NamedVars::GetValueNonConst(const wstring &name) THROWS{
	int index = Find(name);

	return Variables[index]->GetValue();
}

DLLEXPORT const VariableBlock* Leviathan::NamedVars::GetValue(const wstring &name) const{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return NULL;
	}

	return Variables[index]->GetValueDirect();
}

DLLEXPORT bool Leviathan::NamedVars::GetValue(const wstring &name, VariableBlock &receiver) const{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return false;
	}
	// specific operator wanted here //
	receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue());
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::GetValue(const wstring &name, const int &nindex, VariableBlock &receiver) const{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return false;
	}
	// specific operator wanted here //
	receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue(nindex));
	return true;
}

DLLEXPORT size_t Leviathan::NamedVars::GetValueCount(const wstring &name) const{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return 0;
	}
	return Variables[index]->GetVariableCount();
}

DLLEXPORT vector<VariableBlock*>* Leviathan::NamedVars::GetValues(const wstring &name) THROWS{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return NULL;
	}
	return &Variables[index]->GetValues();
}

DLLEXPORT bool Leviathan::NamedVars::GetValues(const wstring &name, vector<const VariableBlock*> &receiver) const{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return false;
	}
	vector<VariableBlock*> &tmpvals = Variables[index]->GetValues();

	vector<const VariableBlock*> tmpconsted(tmpvals.size());

	for(size_t i = 0; i < tmpconsted.size(); i++){

		tmpconsted[i] = const_cast<const VariableBlock*>(tmpvals[i]);
	}

	receiver = tmpconsted;
	return true;
}

DLLEXPORT shared_ptr<NamedVariableList> Leviathan::NamedVars::GetValueDirect(const wstring &name) const{
	int index = Find(name);
	// index check //
	ARR_INDEX_CHECKINV(index, Variables.size()){
		return NULL;
	}
	return Variables[index];
}

// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT int Leviathan::NamedVars::GetVariableType(const wstring &name) const{
	// call overload //
	return GetVariableType((size_t)Find(name));
}

DLLEXPORT int Leviathan::NamedVars::GetVariableType(unsigned int index) const{
	ARR_INDEX_CHECKINV(index, Variables.size()){
		Logger::Get()->Error(L"NamedVars: GetVariableType: out of range", index);
		return false;
	}
	return Variables[index]->GetVariableType();
}

DLLEXPORT int Leviathan::NamedVars::GetVariableTypeOfAll(const wstring &name) const{
	// call overload //
	return GetVariableType((size_t)Find(name));
}

DLLEXPORT int Leviathan::NamedVars::GetVariableTypeOfAll(unsigned int index) const{
	ARR_INDEX_CHECKINV(index, Variables.size()){
		Logger::Get()->Error(L"NamedVars: GetVariableTypeOfAll: out of range", index);
		return false;
	}
	return Variables[index]->GetCommonType();
}
// ------------------------------------ //
wstring& Leviathan::NamedVars::GetName(unsigned int index){
	ARR_INDEX_CHECKINV(index, Variables.size()){
		Logger::Get()->Error(L"NamedVars: GetName: out of range", index);
		// "clever" way to avoid exceptions //
		return Misc::GetErrString();
	}

	return Variables[index]->GetName();
}

DLLEXPORT bool Leviathan::NamedVars::GetName(unsigned int index, wstring &name) const{
	ARR_INDEX_CHECKINV(index, Variables.size()){
		Logger::Get()->Error(L"NamedVars: GetName: out of range", index);
		return false;
	}
	// fetch name to variables //
	Variables[index]->GetName(name);
	return true;
}

void Leviathan::NamedVars::SetName(unsigned int index, const wstring &name){
	ARR_INDEX_CHECKINV(index, Variables.size()){
		Logger::Get()->Error(L"NamedVars: SetName: out of range", index);
		return;
	}

	Variables[index]->SetName(name);
}
void Leviathan::NamedVars::SetName(const wstring &oldname, const wstring &name){
	// call overload //
	SetName(Find(oldname), name);
}
bool Leviathan::NamedVars::CompareName(unsigned int index, const wstring &name) const{
	ARR_INDEX_CHECK(index, Variables.size())
		return Variables[index]->CompareName(name);
	// maybe throw an exception here //
	DEBUG_BREAK;
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NamedVars::AddVar(shared_ptr<NamedVariableList> values){
	// just add to vector //
	Variables.push_back(values);
}

DLLEXPORT void Leviathan::NamedVars::AddVar(NamedVariableList* newvaluetoadd){
	// create new smart pointer and push back //
	Variables.push_back(shared_ptr<NamedVariableList>(newvaluetoadd));
}

DLLEXPORT void Leviathan::NamedVars::AddVar(const wstring &name, VariableBlock* valuetosteal){
	// create new smart pointer and push back //
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(name, valuetosteal)));
}

void Leviathan::NamedVars::Remove(unsigned int index){
	ARR_INDEX_CHECKINV(index, Variables.size()){
		Logger::Get()->Error(L"NamedVars: Remove: out of range", index);
		return;
	}
	// smart pointers //
	Variables.erase(Variables.begin()+index);
}

DLLEXPORT void Leviathan::NamedVars::Remove(const wstring &name){
	// call overload //
	Remove(Find(name));
}
// ------------------------------------ //
int Leviathan::NamedVars::LoadVarsFromFile(const wstring &file){
	// call datadump loaded with this object's vector //
	return FileSystem::LoadDataDump(file, Variables);
}
vector<shared_ptr<NamedVariableList>>* Leviathan::NamedVars::GetVec(){
	return &Variables;
}
void Leviathan::NamedVars::SetVec(vector<shared_ptr<NamedVariableList>>& vec){
	Variables = vec;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::NamedVars::Find(const wstring &name) const{
	for(unsigned int i = 0; i < Variables.size(); i++){
		if(Variables[i]->CompareName(name))
			return i;
	}
	return -1;
}
// ------------------ Script compatible functions ------------------ //
ScriptSafeVariableBlock* Leviathan::NamedVars::GetScriptCompatibleValue(string name){
	// Use a try block to not throw exceptions to the script engine //
	try{
		wstring wstrname = Convert::StringToWstring(name);
		VariableBlock& tmpblock = GetValueNonConst(wstrname);

		// Create script safe version //
		return new ScriptSafeVariableBlock(&tmpblock, wstrname);


	} catch(...){
		// Something failed, return empty handle //
		return NULL;
	}
}
