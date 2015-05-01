// ------------------------------------ //
#include "NamedVars.h"

#include "FileSystem.h"
#include "Statistics/TimingMonitor.h"
#include "../../Iterators/StringIterator.h"
#include "Exceptions.h"
#include <cstdint>
#include <limits.h>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
Leviathan::NamedVariableList::NamedVariableList() : Datas(0), Name(""){
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const string &name, VariableBlock* value1) :
    Datas(1), Name(name)
{
	// set value //
	Datas[0] = value1;
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const string &name, const VariableBlock &val) :
    Datas(1), Name(name)
{
	// set value //
	Datas[0] = new VariableBlock(val);
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(ScriptSafeVariableBlock* const data) :
    Datas(1), Name(data->GetName())
{
	// Copy value //
	Datas[0] = new VariableBlock(*data);
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const string &name, vector<VariableBlock*> values_willclear)
    : Datas(values_willclear.size()), Name(name)
{
	// set values //
	for(size_t i = 0; i < values_willclear.size(); i++){
		Datas[i] = values_willclear[i];
	}
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const NamedVariableList &other) :
    Datas(other.Datas.size()), Name(other.Name)
{

	// copy value over //
	for(size_t i = 0; i < other.Datas.size(); i++){

		Datas[i] = new VariableBlock(*other.Datas[i]);
	}
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const string &line,
    map<string, std::shared_ptr<VariableBlock>>* predefined /*= NULL*/)
{
	// using StringIterator makes this shorter //
	StringIterator itr(&line);

	auto name = itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_ALL);

	if(!name){
		// no name //
		throw InvalidArgument("invalid data on line (invalid name)");
	}

	Name = *name;

	// skip whitespace //
	itr.SkipWhiteSpace();

	// get last part of it //
	//unique_ptr<string> tempvar = itr.GetUntilEnd();
	auto tempvar = itr.GetUntilNextCharacterOrAll<string>(L';');

	if(!tempvar || tempvar->size() < 1){
		// no variable //
		throw InvalidArgument("invalid data on line (no variable)");
	}

	ConstructValuesForObject(*tempvar, predefined);
}

DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(const string &name, const string &valuestr, map<string,
    std::shared_ptr<VariableBlock>>* predefined /*= NULL*/)
{
	// We already have the name provided for us //
	Name = name;

	// The value needs to be parsed //
	ConstructValuesForObject(valuestr, predefined);
}

DLLEXPORT bool NamedVariableList::RecursiveParseList(std::vector<VariableBlock*> &resultvalues,
    std::unique_ptr<std::string> expression,
    std::map<std::string, std::shared_ptr<VariableBlock>>* predefined)
{
    // Empty brackets //
    if(!expression){

        resultvalues.push_back(new VariableBlock(new StringBlock(new string())));
        return true;
    }

    StringIterator itr(expression.get());

    itr.SkipWhiteSpace();

    // TODO: allow commas inside brackets without quoting them
    while(auto value = itr.GetUntilNextCharacterOrAll<string>(',')){

        StringIterator itr2(value.get());

        itr2.SkipWhiteSpace();

        if(itr2.IsOutOfBounds()){

            continue;
        }

        // Parameter is wrapped in brackets //
        if(itr2.GetCharacter() == '['){

            auto firstvalue = itr2.GetStringInBracketsRecursive<string>();

            std::vector<VariableBlock*> morevalues;

            if(!RecursiveParseList(morevalues, move(firstvalue), predefined)){

                throw InvalidArgument("Sub expression parsing failed");
            }

            if(morevalues.size() > 1){
                
                SAFE_DELETE_VECTOR(morevalues);
                morevalues.clear();
                throw InvalidArgument("NamedVars recursive parsing is not done");
                
            } else {

                // Just a single or no values where wrapped in extra brackets //
                for(auto ptr : morevalues){
                    resultvalues.push_back(ptr);
                }

                morevalues.clear();
            }
            
            continue;
        }

        // Parse value //
        auto valuestr = itr2.GetUntilEnd<string>();

        if(!valuestr)
            continue;
        
        try{
            resultvalues.push_back(new VariableBlock(*valuestr, predefined));
        } catch(const InvalidArgument&){

            SAFE_DELETE_VECTOR(resultvalues);
            throw;
        }
    }
    
    return true;
}

DLLEXPORT void Leviathan::NamedVariableList::ConstructValuesForObject(const string &variablestr, map<string,
    std::shared_ptr<VariableBlock>>* predefined)
{
	if(variablestr.size() == 0){
        
		throw InvalidArgument("invalid variable string, 0 length");
	}
    
	// check does it have brackets (and need to be processed like so) //
	if(variablestr[0] == L'['){

		// Needs to be split into values //
        StringIterator itr(variablestr);

        auto firstlevel = itr.GetStringInBracketsRecursive<string>();

        std::vector<VariableBlock*> parsedvalues;
        try{
            if(!RecursiveParseList(parsedvalues, move(firstlevel), predefined)){
            
                throw InvalidArgument("NamedVariableList could not parse top level bracket "
                    "expression");
            }
        } catch(const InvalidArgument &e){

            throw;
        }

        for(auto iter = Datas.begin(); iter != Datas.end(); ++iter){

            SAFE_DELETE(*iter);
        }

        Datas.resize(parsedvalues.size());

        // Add the final values //
        for(size_t i = 0; i < Datas.size(); i++){

            Datas[i] = parsedvalues[i];
        }

        parsedvalues.clear();
        
		return;
	}

	// just one value //
	try{
		// try to create new VariableBlock //
		// it should always have one element //
        Datas.push_back(new VariableBlock(variablestr, predefined));


	}
	catch (const InvalidArgument &e){

		// Rethrow the exception //
        SAFE_DELETE_VECTOR(Datas);
        throw;
	}
}
// ------------------ Handling passing to packets ------------------ //
DLLEXPORT Leviathan::NamedVariableList::NamedVariableList(sf::Packet &packet){
	// Unpack the data from the packet //
	packet >> Name;

	// First get the size //
	int tmpsize = 0;

	// Thousand is considered here the maximum number of elements //
	if(!(packet >> tmpsize) || tmpsize > 1000 || tmpsize < 0){

		throw InvalidArgument("invalid packet format");
	}

	// Reserve enough space //
	Datas.reserve((size_t)tmpsize);

	// Loop and get the data //
	for(int i = 0; i < tmpsize; i++){

		Datas.push_back(new VariableBlock(packet));
	}
}

DLLEXPORT void Leviathan::NamedVariableList::AddDataToPacket(sf::Packet &packet) const{
	// Start adding data to the packet //
	packet << Name;

	// The vector passing //
	int truncsize = (int)Datas.size();

	if(truncsize > 1000){

		// That's an error //
		Logger::Get()->Error("NamedVariableList: AddToPacket: too many elements (sane maximum is 1000 values), got "+
            Convert::ToString(truncsize)+
			" values, truncated to first 1000");
        
		truncsize = 1000;
	}

	packet << truncsize;

	// Pass that number of elements //
	for(int i = 0; i < truncsize; i++){

		Datas[i]->AddDataToPacket(packet);
	}
}

DLLEXPORT Leviathan::NamedVariableList::~NamedVariableList(){

	SAFE_DELETE_VECTOR(Datas);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NamedVariableList::SetValue(const VariableBlock &value1){
	// clear old //
	SAFE_DELETE_VECTOR(Datas);

	// assign value //

	// create new //
	Datas.push_back(new VariableBlock(value1));
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

DLLEXPORT VariableBlock& Leviathan::NamedVariableList::GetValue(){
	// uses vector operator to get value, might throw something //
	return *Datas[0];
}

DLLEXPORT VariableBlock& Leviathan::NamedVariableList::GetValue(const int &nindex){
	// uses vector operator to get value, might throw something //
	return *Datas[nindex];
}

string& Leviathan::NamedVariableList::GetName(){
	return Name;
}

DLLEXPORT void Leviathan::NamedVariableList::GetName(string &name) const{
	// return name in a reference //
	name = Name;
}

void Leviathan::NamedVariableList::SetName(const string& name){
	Name = name;
}

bool Leviathan::NamedVariableList::CompareName(const string& name) const{
	// just default comparison //
	return Name.compare(name) == 0;
}
DLLEXPORT string Leviathan::NamedVariableList::ToText(int WhichSeparator /*= 0*/) const{

	string stringifiedval = Name;

	switch(WhichSeparator){
	case 0: stringifiedval += " = "; break;
	case 1: stringifiedval += ": "; break;
	default:
		// error //
        throw Exception("Invalid separator type");
	}

	// convert value to string //
	// starting bracket //
	stringifiedval += "[";

	// reserve some space //
	stringifiedval.reserve(Datas.size()*4);

	for(size_t i = 0; i < Datas.size(); i++){

		if(i != 0)
			stringifiedval += ",";
        
		// Check if type is a string type //
		int blocktype = Datas[i]->GetBlockConst()->Type;

		if(blocktype == DATABLOCK_TYPE_STRING || blocktype == DATABLOCK_TYPE_WSTRING ||
            blocktype == DATABLOCK_TYPE_CHAR)
        {
			// Output in quotes //
			stringifiedval += "[\""+Datas[i]->operator string()+"\"]";
            
		} else if(blocktype == DATABLOCK_TYPE_BOOL){
            
			// Use true/false for this //
			stringifiedval += "["+(Datas[i]->operator bool() ? string("true"):
                string("false"))+"]";

		} else {

            // check is conversion allowed //
            if(!Datas[i]->IsConversionAllowedNonPtr<string>()){
                // no choice but to throw exception //
                throw InvalidType("value cannot be cast to string");
            }

			stringifiedval += "["+Datas[i]->operator string()+"]";
		}
	}


	// add ending bracket and done //
	stringifiedval += "];";

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

DLLEXPORT bool Leviathan::NamedVariableList::operator==(const NamedVariableList &other) const{
	// Make sure that names are the same //
	if(Name != other.Name)
		return false;

	// Check variables //
	if(Datas.size() != other.Datas.size())
		return false;

	// Compare data in the DataBlocks //
	for(size_t i = 0; i < Datas.size(); i++){

		if(*Datas[i] != *other.Datas[i])
			return false;
	}

	// They truly are the same //
	return true;
}
// ----------------- process functions ------------------- //
DLLEXPORT bool Leviathan::NamedVariableList::ProcessDataDump(const string &data,
    vector<shared_ptr<NamedVariableList>> &vec, map<string,
    std::shared_ptr<VariableBlock>>* predefined /*= NULL*/)
{
	// split to lines //
	vector<shared_ptr<string>> Lines;

	StringIterator itr(data);

	// use string iterator to get the lines that are separated by ; //
	unique_ptr<string> curline;
	int linelength = 0;
	do {
		curline = itr.GetUntilNextCharacterOrNothing<string>(';');
		if(!curline)
			break;
		
		linelength = curline->size();

		string* tmp = curline.release();

		Lines.push_back(shared_ptr<string>(tmp));
	} while(linelength != 0);


	if(Lines.size() < 1){
		// no lines //
		Logger::Get()->Error("NamedVar: ProcessDataDump: No lines (even 1 line requires "
            "ending ';' to work)");

		return 400;
	}
	// make space for values //
	// let's reserve space //
	vec.reserve(Lines.size());

	// fill values //
	for(size_t i = 0; i < Lines.size(); i++){
		// skip empty lines //
		if(Lines[i]->size() == 0)
			continue;

		// create a named var //
		try{
			shared_ptr<NamedVariableList> var(new NamedVariableList(*Lines[i], predefined));
			vec.push_back(var);
		}
		catch (const InvalidArgument &e){
			// print to log //
			e.PrintToLog();
			// exception throws, must be invalid line //

            // This should remove null characters from the string //
            
			Logger::Get()->Info("NamedVar: ProcessDataDump: contains invalid line, line (with only ASCII characters): "
                +Convert::ToString(Lines[i])+"\nEND");
            
			continue;
		}

		// check is it valid //
		if(vec.back()->GetName().size() == 0 || vec.back()->GetName().size() > 10000){
			// invalid //
			Logger::Get()->Error("NamedVar: ProcessDataDump: invalid NamedVar generated for line: "+
                Convert::ToString(Lines[i])+"\nEND");

			vec.erase(vec.end());
		}

		continue;
	}

	return true;
}

DLLEXPORT  void Leviathan::NamedVariableList::SwitchValues(NamedVariableList &receiver,
    NamedVariableList &donator)
{
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

    if(nindex >= Datas.size())
        return nullptr;
    
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

	return Datas[nindex]->GetBlock()->Type;
}

DLLEXPORT VariableBlock& Leviathan::NamedVariableList::operator[](const int &nindex){
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
	for(size_t i = 0; i < other.Variables.size(); i++){
		Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(*other.Variables[i])));
	}
}

DLLEXPORT Leviathan::NamedVars::NamedVars(NamedVars* stealfrom) : Variables(stealfrom->Variables){
	stealfrom->Variables.clear();
}

DLLEXPORT Leviathan::NamedVars::NamedVars(const string &datadump) : Variables(){

	// load data directly to vector //
	if(!NamedVariableList::ProcessDataDump(datadump, Variables, NULL)){
        
		// error happened //
        throw InvalidArgument("datadump processing failed");
	}
}

DLLEXPORT Leviathan::NamedVars::NamedVars(const vector<shared_ptr<NamedVariableList>> &variables) : Variables(variables){

}

DLLEXPORT Leviathan::NamedVars::NamedVars(shared_ptr<NamedVariableList> variable) : Variables(1){
	// store the single variable //
	Variables[0] = variable;
}

DLLEXPORT Leviathan::NamedVars::NamedVars(NamedVariableList* takevariable) : Variables(1){
	Variables[0] = std::shared_ptr<NamedVariableList>(takevariable);
}

Leviathan::NamedVars::~NamedVars(){
	// no need to release due to smart pointers //
}
// ------------------------------------ //
DLLEXPORT Leviathan::NamedVars::NamedVars(sf::Packet &packet){
	// First get the size //
	int isize;

	if(!(packet >> isize)){

		throw InvalidArgument("packet has invalid format");
	}

	// Reserve space //
	Variables.reserve(isize);

	for(int i = 0; i < isize; i++){

		Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(packet)));
	}


}

DLLEXPORT void Leviathan::NamedVars::AddDataToPacket(sf::Packet &packet) const{
	GUARD_LOCK();
	// First write size //
	int isize = (int)Variables.size();

	packet << isize;

	// Write each individual variable //
	for(int i = 0; i < isize; i++){

		Variables[i]->AddDataToPacket(packet);
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NamedVars::SetValue(const string &name, const VariableBlock &value1){
	GUARD_LOCK();
	auto index = Find(name);

	if(index >= Variables.size())
        return false;
    

	Variables[index]->SetValue(value1);
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::SetValue(const string &name, VariableBlock* value1){
	GUARD_LOCK();
	auto index = Find(guard, name);

	if(index >= Variables.size())
        return false;

    
	Variables[index]->SetValue(value1);
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::SetValue(const string &name, const vector<VariableBlock*> &values){
	GUARD_LOCK();
	auto index = Find(name);

	if(index >= Variables.size())
        return false;
    

	Variables[index]->SetValue(values);
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::SetValue(NamedVariableList &nameandvalues){
	GUARD_LOCK();
	auto index = Find(nameandvalues.Name);
	// index check //
	if(index >= Variables.size()){

		Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(nameandvalues)));
		return true;
	}
    
	nameandvalues.Name.clear();
	// set values with "swap" //
	NamedVariableList::SwitchValues(*Variables[index].get(), nameandvalues);
	return true;
}

DLLEXPORT VariableBlock& Leviathan::NamedVars::GetValueNonConst(const string &name){
	GUARD_LOCK();
	auto index = Find(name);

	if(index >= Variables.size()){

		throw InvalidArgument("value not found");
	}

	return Variables[index]->GetValue();
}

DLLEXPORT const VariableBlock* Leviathan::NamedVars::GetValue(const string &name) const{
	GUARD_LOCK();
	auto index = Find(guard, name);

	if(index >= Variables.size()){

		throw InvalidArgument("value not found");
	}

	return Variables[index]->GetValueDirect();
}

DLLEXPORT bool Leviathan::NamedVars::GetValue(const string &name, VariableBlock &receiver) const{
	GUARD_LOCK();
	auto index = Find(guard, name);
	// index check //
	if(index >= Variables.size()){
		return false;
	}
	// specific operator wanted here //
	receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue());
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::GetValue(const string &name, const int &nindex, VariableBlock &receiver) const{
	GUARD_LOCK();

	auto index = Find(guard, name);
    
	// index check //
	if(index >= Variables.size()){
		return false;
	}
    
	// specific operator wanted here //
	receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue(nindex));
	return true;
}

DLLEXPORT bool Leviathan::NamedVars::GetValue(const int &index, VariableBlock &receiver) const{
	GUARD_LOCK();
	// index check //
	if(index >= Variables.size()){
		return false;
	}
    
	// specific operator wanted here //
	receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue(0));
	return true;
}

DLLEXPORT size_t Leviathan::NamedVars::GetValueCount(const string &name) const{
	GUARD_LOCK();
	auto index = Find(guard, name);
	// index check //
	if(index >= Variables.size()){
		return 0;
	}
    
	return Variables[index]->GetVariableCount();
}

DLLEXPORT vector<VariableBlock*>* Leviathan::NamedVars::GetValues(const string &name){
	GUARD_LOCK();
	auto index = Find(guard, name);
	// index check //
	if(index >= Variables.size()){
		return NULL;
	}
    
	return &Variables[index]->GetValues();
}

DLLEXPORT bool Leviathan::NamedVars::GetValues(const string &name, vector<const VariableBlock*> &receiver) const{
	GUARD_LOCK();
	auto index = Find(guard, name);
	// index check //
	if(index >= Variables.size()){
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

DLLEXPORT std::shared_ptr<NamedVariableList> Leviathan::NamedVars::GetValueDirect(const string &name) const{
	GUARD_LOCK();
	auto index = Find(guard, name);
	// index check //
	if(index >= Variables.size()){
		return NULL;
	}
	return Variables[index];
}

DLLEXPORT NamedVariableList* Leviathan::NamedVars::GetValueDirectRaw(const string &name) const{
	GUARD_LOCK();
	auto index = Find(guard, name);
	// index check //
	if(index >= Variables.size()){
		return NULL;
	}

	return Variables[index].get();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::NamedVars::GetVariableType(const string &name) const{
	GUARD_LOCK();
	// call overload //
	return GetVariableType(guard, Find(guard, name));
}

DLLEXPORT int Leviathan::NamedVars::GetVariableType(Lock &guard, size_t index) const{

	return Variables[index]->GetVariableType();
}

DLLEXPORT int Leviathan::NamedVars::GetVariableTypeOfAll(const string &name) const{
	GUARD_LOCK();
	// call overload //
	return GetVariableTypeOfAll(guard, Find(guard, name));
}

DLLEXPORT int Leviathan::NamedVars::GetVariableTypeOfAll(Lock &guard, size_t index) const{

	return Variables[index]->GetCommonType();
}
// ------------------------------------ //
string Leviathan::NamedVars::GetName(size_t index){
	GUARD_LOCK();

	return Variables[index]->GetName();
}

DLLEXPORT bool Leviathan::NamedVars::GetName(size_t index, string &name) const{
	GUARD_LOCK();

	Variables[index]->GetName(name);
	return true;
}

void Leviathan::NamedVars::SetName(Lock &guard, size_t index, const string &name){

	Variables[index]->SetName(name);
}

void Leviathan::NamedVars::SetName(const string &oldname, const string &name){
	GUARD_LOCK();
	// call overload //
	SetName(guard, Find(guard, oldname), name);
}

bool Leviathan::NamedVars::CompareName(size_t index, const string &name) const{
	GUARD_LOCK();
    
    return Variables[index]->CompareName(name);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NamedVars::AddVar(shared_ptr<NamedVariableList> values){
	GUARD_LOCK();
	RemoveIfExists(values->GetName(), guard);
	// just add to vector //
	Variables.push_back(values);
}

DLLEXPORT void Leviathan::NamedVars::AddVar(NamedVariableList* newvaluetoadd){
	GUARD_LOCK();
	RemoveIfExists(newvaluetoadd->GetName(), guard);
	// create new smart pointer and push back //
	Variables.push_back(shared_ptr<NamedVariableList>(newvaluetoadd));
}

DLLEXPORT void Leviathan::NamedVars::AddVar(const string &name, VariableBlock* valuetosteal){
	GUARD_LOCK();
	RemoveIfExists(name, guard);
	// create new smart pointer and push back //
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(name, valuetosteal)));
}
// ------------------------------------ //
void Leviathan::NamedVars::Remove(size_t index){
	GUARD_LOCK();
    
	// smart pointers //
	Variables.erase(Variables.begin()+index);
}

DLLEXPORT void Leviathan::NamedVars::Remove(const string &name){
	// call overload //
	Remove(Find(name));
}

DLLEXPORT void Leviathan::NamedVars::RemoveIfExists(const string &name, Lock &guard){
	// Try  to find it //
	size_t index = Find(guard, name);

    if(index >= Variables.size())
        return;


    Variables.erase(Variables.begin()+index);
}
// ------------------------------------ //
bool Leviathan::NamedVars::LoadVarsFromFile(const string &file){
	// call datadump loaded with this object's vector //
	return FileSystem::LoadDataDump(file, Variables);
}
vector<shared_ptr<NamedVariableList>>* Leviathan::NamedVars::GetVec(){
	return &Variables;
}
void Leviathan::NamedVars::SetVec(vector<shared_ptr<NamedVariableList>>& vec){
	GUARD_LOCK();
	Variables = vec;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::NamedVars::Find(Lock &guard, const string &name) const{
	for(size_t i = 0; i < Variables.size(); i++){
		if(Variables[i]->CompareName(name))
			return i;
	}
    
	return SIZE_MAX;
}
// ------------------ Script compatible functions ------------------ //
ScriptSafeVariableBlock* Leviathan::NamedVars::GetScriptCompatibleValue(const string &name){
	// Use a try block to not throw exceptions to the script engine //
	try{
		VariableBlock& tmpblock = GetValueNonConst(name);

		// Create script safe version //
		return new ScriptSafeVariableBlock(&tmpblock, name);


	} catch(...){
		// Something failed, return empty handle //
		return NULL;
	}
}

bool Leviathan::NamedVars::AddScriptCompatibleValue(ScriptSafeVariableBlock* value){

    GUARD_LOCK();

    RemoveIfExists(value->GetName(), guard);

    try{
        
        Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(value)));
    
    } catch(...){
        return false;
    }

    return true;
}

DLLEXPORT size_t Leviathan::NamedVars::GetVariableCount() const{
	return Variables.size();
}
