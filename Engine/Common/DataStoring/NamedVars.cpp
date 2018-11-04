// ------------------------------------ //
#include "NamedVars.h"

#include "../../Iterators/StringIterator.h"
#include "FileSystem.h"
#include <cstdint>
#include <limits.h>

using namespace Leviathan;
using namespace std;
// ------------------------------------ //
NamedVariableList::NamedVariableList() : Datas(0), Name("") {}

DLLEXPORT NamedVariableList::NamedVariableList(const std::string& name) : Datas(0), Name(name)
{}

DLLEXPORT NamedVariableList::NamedVariableList(
    const std::string& name, VariableBlock* value1) :
    Datas(1),
    Name(name)
{
    // set value //
    Datas[0] = value1;
}

DLLEXPORT NamedVariableList::NamedVariableList(
    const std::string& name, const VariableBlock& val) :
    Datas(1),
    Name(name)
{
    // set value //
    Datas[0] = new VariableBlock(val);
}

#ifdef LEVIATHAN_USING_ANGELSCRIPT
DLLEXPORT NamedVariableList::NamedVariableList(ScriptSafeVariableBlock* const data) :
    Datas(1), Name(data->GetName())
{
    // Copy value //
    Datas[0] = new VariableBlock(*data);
}
#endif // LEVIATHAN_USING_ANGELSCRIPT

DLLEXPORT NamedVariableList::NamedVariableList(
    const std::string& name, vector<VariableBlock*> values_willclear) :
    Datas(values_willclear.size()),
    Name(name)
{
    // set values //
    for(size_t i = 0; i < values_willclear.size(); i++) {
        Datas[i] = values_willclear[i];
    }
}

DLLEXPORT NamedVariableList::NamedVariableList(const NamedVariableList& other) :
    Datas(other.Datas.size()), Name(other.Name)
{

    // copy value over //
    for(size_t i = 0; i < other.Datas.size(); i++) {

        Datas[i] = new VariableBlock(*other.Datas[i]);
    }
}

DLLEXPORT NamedVariableList::NamedVariableList(const std::string& line,
    LErrorReporter* errorreport, map<std::string, std::shared_ptr<VariableBlock>>* predefined
    /*= NULL*/)
{
    // using StringIterator makes this shorter //
    StringIterator itr(&line);

    auto name = itr.GetUntilEqualityAssignment<std::string>(EQUALITYCHARACTER_TYPE_ALL);

    if(!name) {
        // no name //
#ifdef ALTERNATIVE_EXCEPTIONS_FATAL
        errorreport->Error(std::string("invalid data on line (invalid name)"));
        return;
#else
        throw InvalidArgument("invalid data on line (invalid name)");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }

    Name = *name;

    // skip whitespace //
    itr.SkipWhiteSpace();

    // get last part of it //
    auto tempvar = itr.GetUntilNextCharacterOrAll<std::string>(L';');

    if(!tempvar || tempvar->size() < 1) {
        // no variable //
#ifdef ALTERNATIVE_EXCEPTIONS_FATAL
        Name = "";
        errorreport->Error(std::string("invalid data on line (no variable data)"));
        return;
#else
        throw InvalidArgument("invalid data on line (no variable data)");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }

    if(!ConstructValuesForObject(*tempvar, errorreport, predefined)) {
#ifdef ALTERNATIVE_EXCEPTIONS_FATAL
        Name = "";
        errorreport->Error(std::string("invalid variable string, parsing failed"));
        return;
#else
        throw InvalidArgument("invalid variable string, parsing failed");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }
}

DLLEXPORT NamedVariableList::NamedVariableList(const std::string& name,
    const std::string& valuestr, LErrorReporter* errorreport,
    map<string, std::shared_ptr<VariableBlock>>* predefined /*= NULL*/)
{
    // We already have the name provided for us //
    Name = name;

    // The value needs to be parsed //
    if(!ConstructValuesForObject(valuestr, errorreport, predefined)) {
#ifdef ALTERNATIVE_EXCEPTIONS_FATAL
        Name = "";
        errorreport->Error(std::string("invalid variable string, parsing failed"));
        return;
#else
        throw InvalidArgument("invalid variable string, parsing failed");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }
}

DLLEXPORT bool NamedVariableList::RecursiveParseList(std::vector<VariableBlock*>& resultvalues,
    std::unique_ptr<std::string> expression, LErrorReporter* errorreport,
    std::map<std::string, std::shared_ptr<VariableBlock>>* predefined)
{
    // Empty brackets //
    if(!expression) {

        resultvalues.push_back(new VariableBlock(new StringBlock(new string())));
        return true;
    }

    StringIterator itr(expression.get());

    itr.SkipWhiteSpace();

    // TODO: allow commas inside brackets without quoting them
    while(auto value = itr.GetUntilNextCharacterOrAll<string>(',')) {

        StringIterator itr2(value.get());

        itr2.SkipWhiteSpace();

        if(itr2.IsOutOfBounds()) {

            continue;
        }

        // Parameter is wrapped in brackets //
        if(itr2.GetCharacter() == '[') {

            auto firstvalue = itr2.GetStringInBracketsRecursive<string>();

            std::vector<VariableBlock*> morevalues;

            if(!RecursiveParseList(morevalues, move(firstvalue), errorreport, predefined)) {
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
                throw InvalidArgument("Sub expression parsing failed");
#else
                errorreport->Error(std::string("Sub expression parsing failed"));
                return false;
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
            }

            if(morevalues.size() > 1) {

                SAFE_DELETE_VECTOR(morevalues);
                morevalues.clear();

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
                throw InvalidArgument("NamedVars recursive parsing is not done");
#else
                LEVIATHAN_ASSERT(0, "NamedVars recursive parsing is not done");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL

            } else {

                // Just a single or no values where wrapped in extra brackets //
                for(auto ptr : morevalues) {
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

        std::unique_ptr<VariableBlock> tmpcreated;

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        try {
            tmpcreated = std::make_unique<VariableBlock>(*valuestr, predefined);
        } catch(const InvalidArgument&) {

            // Rethrow the exception //
            SAFE_DELETE_VECTOR(resultvalues);
            throw;
        }
#else
        tmpcreated = std::make_unique<VariableBlock>(*valuestr, predefined);
#endif // ALTERNATIVE_EXCEPTIONS_FATAL

        if(!tmpcreated || !tmpcreated->IsValid()) {

            SAFE_DELETE_VECTOR(resultvalues);
            errorreport->Error(std::string("VariableBlock invalid value: " + *valuestr));
            return false;
        }

        resultvalues.push_back(tmpcreated.release());
    }

    return true;
}

DLLEXPORT bool NamedVariableList::ConstructValuesForObject(const std::string& variablestr,
    LErrorReporter* errorreport,
    std::map<std::string, std::shared_ptr<VariableBlock>>* predefined)
{
    if(variablestr.size() == 0) {
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("invalid variable string, 0 length");
#else
        errorreport->Error(std::string("invalid variable string, 0 length"));
        return false;
#endif
    }

    // check does it have brackets (and need to be processed like so) //
    if(variablestr[0] == L'[') {

        // Needs to be split into values //
        StringIterator itr(variablestr);

        auto firstlevel = itr.GetStringInBracketsRecursive<string>();

        std::vector<VariableBlock*> parsedvalues;

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL

        try {
            if(!RecursiveParseList(
                   parsedvalues, std::move(firstlevel), errorreport, predefined)) {

                throw InvalidArgument("NamedVariableList could not parse top level bracket "
                                      "expression");
            }
        } catch(const InvalidArgument&) {

            throw;
        }
#else
        if(!RecursiveParseList(parsedvalues, std::move(firstlevel), errorreport, predefined)) {

            errorreport->Error(
                std::string("NamedVariableList could not parse top level bracket "
                            "expression"));
            return false;
        }

#endif // ALTERNATIVE_EXCEPTIONS_FATAL

        for(auto iter = Datas.begin(); iter != Datas.end(); ++iter) {

            SAFE_DELETE(*iter);
        }

        Datas.resize(parsedvalues.size());

        // Add the final values //
        for(size_t i = 0; i < Datas.size(); i++) {

            Datas[i] = parsedvalues[i];
        }

        parsedvalues.clear();

        return true;
    }

    // just one value //

    // try to create new VariableBlock //
    // it should always have one element //
    std::unique_ptr<VariableBlock> tmpcreated;

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
    try {
        tmpcreated = std::make_unique<VariableBlock>(variablestr, predefined);
    } catch(const InvalidArgument&) {

        // Rethrow the exception //
        SAFE_DELETE_VECTOR(Datas);
        throw;
    }
#else
    tmpcreated = std::make_unique<VariableBlock>(variablestr, predefined);
#endif // ALTERNATIVE_EXCEPTIONS_FATAL

    if(!tmpcreated || !tmpcreated->IsValid()) {

        SAFE_DELETE_VECTOR(Datas);
        return false;
    }

    Datas.push_back(tmpcreated.release());
    return true;
}
#ifdef SFML_PACKETS
// ------------------ Handling passing to packets ------------------ //
DLLEXPORT NamedVariableList::NamedVariableList(sf::Packet& packet)
{
    // Unpack the data from the packet //
    packet >> Name;

    // First get the size //
    int tmpsize = 0;

    // Thousand is considered here the maximum number of elements //
    if(!(packet >> tmpsize) || tmpsize > 1000 || tmpsize < 0) {

        throw InvalidArgument("invalid packet format");
    }

    // Reserve enough space //
    Datas.reserve((size_t)tmpsize);

    // Loop and get the data //
    for(int i = 0; i < tmpsize; i++) {

        Datas.push_back(new VariableBlock(packet));
    }
}

DLLEXPORT void NamedVariableList::AddDataToPacket(sf::Packet& packet) const
{
    // Start adding data to the packet //
    packet << Name;

    // The vector passing //
    int truncsize = (int)Datas.size();

    if(truncsize > 1000) {

        // That's an error //
        Logger::Get()->Error("NamedVariableList: AddToPacket: too many elements (sane maximum "
                             "is 1000 values), got " +
                             Convert::ToString(truncsize) +
                             " values, truncated to first 1000");

        truncsize = 1000;
    }

    packet << truncsize;

    // Pass that number of elements //
    for(int i = 0; i < truncsize; i++) {

        Datas[i]->AddDataToPacket(packet);
    }
}
#endif // SFML_PACKETS

DLLEXPORT NamedVariableList::~NamedVariableList()
{

    SAFE_DELETE_VECTOR(Datas);
}
// ------------------------------------ //
DLLEXPORT void NamedVariableList::SetValue(const VariableBlock& value1)
{
    // clear old //
    SAFE_DELETE_VECTOR(Datas);

    // assign value //

    // create new //
    Datas.push_back(new VariableBlock(value1));
}

DLLEXPORT void NamedVariableList::SetValue(VariableBlock* value1)
{
    // clear old //
    SAFE_DELETE_VECTOR(Datas);

    // put value to vector //
    Datas.push_back(value1);
}

DLLEXPORT void NamedVariableList::SetValue(const int& nindex, const VariableBlock& valuetoset)
{
    // check do we need to allocate new //
    if(Datas.size() <= (size_t)nindex) {

        // resize to have enough space //
        Datas.resize(nindex + 1, NULL);
        Datas[nindex] = new VariableBlock(valuetoset);
    } else {

        if(Datas[nindex] != NULL) {
            // assign to existing value //
            *Datas[nindex] = valuetoset;
        } else {
            // new value needed //
            Datas[nindex] = new VariableBlock(valuetoset);
        }
    }
}

DLLEXPORT void NamedVariableList::SetValue(const int& nindex, VariableBlock* valuetoset)
{
    // check do we need to allocate new //
    if(Datas.size() <= (size_t)nindex) {

        // resize to have enough space //
        Datas.resize(nindex + 1, NULL);
        // just copy the pointer //
        Datas[nindex] = valuetoset;
    } else {

        if(Datas[nindex] != NULL) {
            // existing value needs to be deleted //
            SAFE_DELETE(Datas[nindex]);
        }
        // set pointer //
        Datas[nindex] = valuetoset;
    }
}

DLLEXPORT void NamedVariableList::SetValue(const vector<VariableBlock*>& values)
{
    // delete old //
    SAFE_DELETE_VECTOR(Datas);

    // copy vector (will copy pointers and steal them) //
    Datas = values;
}
DLLEXPORT void NamedVariableList::PushValue(std::unique_ptr<VariableBlock>&& value)
{
    Datas.push_back(value.release());
}
// ------------------------------------ //
DLLEXPORT VariableBlock& NamedVariableList::GetValue()
{
    // uses vector operator to get value, might throw something //
    return *Datas[0];
}

DLLEXPORT VariableBlock& NamedVariableList::GetValue(size_t nindex)
{
    // uses vector operator to get value, might throw or something //
    return *Datas[nindex];
}

DLLEXPORT void NamedVariableList::GetName(std::string& name) const
{
    // return name in a reference //
    name = Name;
}

void NamedVariableList::SetName(const std::string& name)
{
    Name = name;
}

bool NamedVariableList::CompareName(const std::string& name) const
{
    // just default comparison //
    return Name.compare(name) == 0;
}
DLLEXPORT std::string Leviathan::NamedVariableList::ToText(
    int WhichSeparator /*= 0*/, bool AddAllBrackets /*= false*/) const
{

    string stringifiedval = Name;

    switch(WhichSeparator) {
    default:
    case 0: stringifiedval += " = "; break;
    case 1: stringifiedval += ": "; break;
    }

    // convert value to string //

    const bool WrapInBrackets = AddAllBrackets ? true : Datas.size() != 1;

    // starting bracket //
    if(WrapInBrackets)
        stringifiedval += "[";

    // reserve some space //
    stringifiedval.reserve(Datas.size() * 4);

    for(size_t i = 0; i < Datas.size(); i++) {

        if(i != 0)
            stringifiedval += ", ";

        // Check if type is a string type //
        int blocktype = Datas[i]->GetBlockConst()->Type;

        if(blocktype == DATABLOCK_TYPE_STRING || blocktype == DATABLOCK_TYPE_WSTRING ||
            blocktype == DATABLOCK_TYPE_CHAR) {
            // Output in quotes //
            if(AddAllBrackets)
                stringifiedval += "[\"" + Datas[i]->operator string() + "\"]";
            else
                stringifiedval += "\"" + Datas[i]->operator string() + "\"";

        } else if(blocktype == DATABLOCK_TYPE_BOOL) {

            // Use true/false for this //
            if(AddAllBrackets) {

                stringifiedval +=
                    "[" + (Datas[i]->operator bool() ? string("true") : string("false")) + "]";

            } else {

                stringifiedval += Datas[i]->operator bool() ? string("true") : string("false");
            }

        } else {

            // check is conversion allowed //
            if(!Datas[i]->IsConversionAllowedNonPtr<string>()) {
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
                // no choice but to throw exception //
                throw InvalidType("value cannot be cast to string");
#else
                LEVIATHAN_ASSERT(0, "value cannot be cast to string");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
            }
            if(AddAllBrackets)
                stringifiedval += "[" + Datas[i]->operator string() + "]";
            else
                stringifiedval += "" + Datas[i]->operator string() + "";
        }
    }

    // add ending bracket and done //
    if(WrapInBrackets)
        stringifiedval += "];";
    else
        stringifiedval += ";";

    return stringifiedval;
}

DLLEXPORT NamedVariableList& NamedVariableList::operator=(const NamedVariableList& other)
{
    // copy values //
    Name = other.Name;

    SAFE_DELETE_VECTOR(Datas);
    Datas.resize(other.Datas.size());
    // copy values over //
    for(size_t i = 0; i < other.Datas.size(); i++) {

        Datas[i] = new VariableBlock(*other.Datas[i]);
    }

    // return this as result //
    return *this;
}

DLLEXPORT bool NamedVariableList::operator==(const NamedVariableList& other) const
{
    // Make sure that names are the same //
    if(Name != other.Name)
        return false;

    // Check variables //
    if(Datas.size() != other.Datas.size())
        return false;

    // Compare data in the DataBlocks //
    for(size_t i = 0; i < Datas.size(); i++) {

        if(*Datas[i] != *other.Datas[i])
            return false;
    }

    // They truly are the same //
    return true;
}
DLLEXPORT bool Leviathan::NamedVariableList::operator!=(const NamedVariableList& other) const
{

    return !(*this == other);
}
// ----------------- process functions ------------------- //
DLLEXPORT bool NamedVariableList::ProcessDataDump(const std::string& data,
    std::vector<std::shared_ptr<NamedVariableList>>& vec, LErrorReporter* errorreport,
    std::map<std::string, std::shared_ptr<VariableBlock>>* predefined /*= NULL*/)
{
    // Split to lines //
    std::vector<std::shared_ptr<std::string>> lines;

    StringIterator itr(data);

    // Use string iterator to get the lines that are separated by ; //
    std::unique_ptr<std::string> curLine;
    size_t lineLength = 0;

    do {
        curLine = itr.GetUntilNextCharacterOrNothing<std::string>(';');

        if(!curLine)
            break;

        lineLength = curLine->size();

        lines.push_back(std::shared_ptr<string>(curLine.release()));

    } while(lineLength != 0);


    if(lines.empty()) {
        // No lines //
        return false;
    }

    // Make space for values //
    // let's reserve space //
    vec.reserve(lines.size());

    // Fill values //
    for(size_t i = 0; i < lines.size(); ++i) {
        // Skip empty lines //
        if(lines[i]->empty())
            continue;

            // Create a named var //
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        try {
            auto var =
                std::make_shared<NamedVariableList>(*lines[i], Logger::Get(), predefined);

            if(!var || !var->IsValid()) {
                // Invalid value //
                continue;
            }

            vec.push_back(var);

        } catch(const InvalidArgument& e) {
            // exception throws, must be invalid line //

            errorreport->Error("NamedVar: ProcessDataDump: contains invalid line, "
                               "line (with only ASCII characters): " +
                               // This should remove null characters from the string //
                               Convert::ToString(*lines[i]) + "\nEND");

            // Print to log //
            e.Print(errorreport);

            continue;
        }

#else

        std::shared_ptr<NamedVariableList> var(
            new NamedVariableList(*lines[i], errorreport, predefined));

        if(!var || !var->IsValid()) {
            // Invalid value //
            continue;
        }

        vec.push_back(var);

#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }

    return true;
}

DLLEXPORT void NamedVariableList::SwitchValues(
    NamedVariableList& receiver, NamedVariableList& donator)
{
    // only overwrite name if there is one //
    if(donator.Name.size() > 0)
        receiver.Name = donator.Name;


    SAFE_DELETE_VECTOR(receiver.Datas);
    // resize to match sizes to avoid excess resizing //
    receiver.Datas.resize(donator.Datas.size());

    for(size_t i = 0; i < donator.Datas.size(); i++) {

        receiver.Datas[i] = donator.Datas[i];
    }
    // clear donator data //
    donator.Datas.clear();
}

DLLEXPORT VariableBlock* NamedVariableList::GetValueDirect()
{
    // return first element //
    return Datas.size() ? Datas[0] : NULL;
}

DLLEXPORT VariableBlock* NamedVariableList::GetValueDirect(size_t nindex)
{

    if(nindex >= Datas.size())
        return nullptr;

    return Datas[nindex];
}

DLLEXPORT size_t NamedVariableList::GetVariableCount() const
{
    return Datas.size();
}

DLLEXPORT int NamedVariableList::GetCommonType() const
{
    // if all have a common type return it //
    if(Datas.size() == 0)
        // no common type //
        return DATABLOCK_TYPE_ERROR;

    int lasttype = Datas[0]->GetBlock()->Type;

    for(size_t i = 1; i < Datas.size(); i++) {

        if(lasttype != Datas[i]->GetBlock()->Type) {
            // not same type //
            return DATABLOCK_TYPE_ERROR;
        }
    }
    // there is a common type //
    return lasttype;
}

DLLEXPORT int NamedVariableList::GetVariableType() const
{
    // get variable type of first index //
    return Datas.size() ? Datas[0]->GetBlock()->Type : DATABLOCK_TYPE_ERROR;
}

DLLEXPORT int NamedVariableList::GetVariableType(const int& nindex) const
{

    return Datas[nindex]->GetBlock()->Type;
}

DLLEXPORT VariableBlock& NamedVariableList::operator[](const int& nindex)
{
    // will allow to throw any exceptions the vector wants //
    return *Datas[nindex];
}

DLLEXPORT vector<VariableBlock*>& NamedVariableList::GetValues()
{
    return Datas;
}
// ---------------------------- NamedVars --------------------------------- //
NamedVars::NamedVars() : Variables()
{
    // nothing to initialize //
}
NamedVars::NamedVars(const NamedVars& other)
{
    // deep copy is required here //
    Variables.reserve(other.Variables.size());
    for(size_t i = 0; i < other.Variables.size(); i++) {
        Variables.push_back(
            shared_ptr<NamedVariableList>(new NamedVariableList(*other.Variables[i])));
    }
}

DLLEXPORT NamedVars::NamedVars(NamedVars* stealfrom) : Variables(stealfrom->Variables)
{
    stealfrom->Variables.clear();
}

DLLEXPORT NamedVars::NamedVars(const std::string& datadump, LErrorReporter* errorreport) :
    Variables()
{

    // load data directly to vector //
    if(!NamedVariableList::ProcessDataDump(datadump, Variables, errorreport, NULL)) {

        // error happened //
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("datadump processing failed");
#else
        StateIsInvalid = true;
        return;
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }
}

DLLEXPORT NamedVars::NamedVars(const vector<shared_ptr<NamedVariableList>>& variables) :
    Variables(variables)
{}

DLLEXPORT NamedVars::NamedVars(shared_ptr<NamedVariableList> variable) : Variables(1)
{
    // store the single variable //
    Variables[0] = variable;
}

DLLEXPORT NamedVars::NamedVars(NamedVariableList* takevariable) : Variables(1)
{
    Variables[0] = std::shared_ptr<NamedVariableList>(takevariable);
}

NamedVars::~NamedVars()
{
    // no need to release due to smart pointers //
}
// ------------------------------------ //
#ifdef SFML_PACKETS
DLLEXPORT NamedVars::NamedVars(sf::Packet& packet)
{
    // First get the size //
    int isize;

    if(!(packet >> isize)) {
#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("packet has invalid format");
#else
        StateIsInvalid = true;
        return;
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }

    // Reserve space //
    Variables.reserve(isize);

    for(int i = 0; i < isize; i++) {

        shared_ptr<NamedVariableList> newvalue(new NamedVariableList(packet));

        if(!newvalue || !newvalue->IsValid())
            continue;

        Variables.push_back(newvalue);
    }
}

DLLEXPORT void NamedVars::AddDataToPacket(sf::Packet& packet) const
{
    GUARD_LOCK();

    // First write size //
    int isize = (int)Variables.size();

    packet << isize;

    // Write each individual variable //
    for(int i = 0; i < isize; i++) {

        Variables[i]->AddDataToPacket(packet);
    }
}
#endif // SFML_PACKETS
// ------------------------------------ //
DLLEXPORT bool NamedVars::Add(std::shared_ptr<NamedVariableList> value)
{
    GUARD_LOCK();

    auto index = Find(guard, value->Name);
    // index check //
    if(index >= Variables.size()) {

        Variables.push_back(value);
        return true;
    }

    Variables[index] = value;
    return false;
}

DLLEXPORT bool NamedVars::SetValue(const std::string& name, const VariableBlock& value1)
{
    GUARD_LOCK();
    auto index = Find(name);

    if(index >= Variables.size())
        return false;


    Variables[index]->SetValue(value1);
    return true;
}

DLLEXPORT bool NamedVars::SetValue(const std::string& name, VariableBlock* value1)
{
    GUARD_LOCK();
    auto index = Find(guard, name);

    if(index >= Variables.size())
        return false;


    Variables[index]->SetValue(value1);
    return true;
}

DLLEXPORT bool NamedVars::SetValue(
    const std::string& name, const vector<VariableBlock*>& values)
{
    GUARD_LOCK();
    auto index = Find(name);

    if(index >= Variables.size())
        return false;


    Variables[index]->SetValue(values);
    return true;
}

DLLEXPORT bool NamedVars::SetValue(NamedVariableList& nameandvalues)
{
    GUARD_LOCK();
    auto index = Find(nameandvalues.Name);
    // index check //
    if(index >= Variables.size()) {

        Variables.push_back(
            shared_ptr<NamedVariableList>(new NamedVariableList(nameandvalues)));
        return true;
    }

    nameandvalues.Name.clear();
    // set values with "swap" //
    NamedVariableList::SwitchValues(*Variables[index].get(), nameandvalues);
    return true;
}

DLLEXPORT VariableBlock& NamedVars::GetValueNonConst(const std::string& name)
{
    GUARD_LOCK();
    auto index = Find(guard, name);

    if(index >= Variables.size()) {

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("value not found");
#else
        LEVIATHAN_ASSERT(0, "NamedVars name not found");
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }

    return Variables[index]->GetValue();
}

DLLEXPORT const VariableBlock* NamedVars::GetValue(const std::string& name) const
{
    GUARD_LOCK();

    auto index = Find(guard, name);

    if(index >= Variables.size()) {

#ifndef ALTERNATIVE_EXCEPTIONS_FATAL
        throw InvalidArgument("value not found");
#else
        return nullptr;
#endif // ALTERNATIVE_EXCEPTIONS_FATAL
    }

    return Variables[index]->GetValueDirect();
}

DLLEXPORT bool NamedVars::GetValue(const std::string& name, VariableBlock& receiver) const
{
    GUARD_LOCK();

    auto index = Find(guard, name);
    // index check //
    if(index >= Variables.size()) {
        return false;
    }
    // specific operator wanted here //
    receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue());
    return true;
}

DLLEXPORT bool NamedVars::GetValue(
    const std::string& name, const int& nindex, VariableBlock& receiver) const
{
    GUARD_LOCK();

    auto index = Find(guard, name);

    // index check //
    if(index >= Variables.size()) {
        return false;
    }

    // specific operator wanted here //
    receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue(nindex));
    return true;
}

DLLEXPORT bool NamedVars::GetValue(const int& index, VariableBlock& receiver) const
{
    GUARD_LOCK();

    // index check //
    if(index >= static_cast<int>(Variables.size())) {
        return false;
    }

    // specific operator wanted here //
    receiver = const_cast<const VariableBlock&>(Variables[index]->GetValue(0));
    return true;
}

DLLEXPORT size_t NamedVars::GetValueCount(const std::string& name) const
{
    GUARD_LOCK();

    auto index = Find(guard, name);
    // index check //
    if(index >= Variables.size()) {
        return 0;
    }

    return Variables[index]->GetVariableCount();
}

DLLEXPORT vector<VariableBlock*>* NamedVars::GetValues(const std::string& name)
{
    GUARD_LOCK();
    auto index = Find(guard, name);
    // index check //
    if(index >= Variables.size()) {
        return NULL;
    }

    return &Variables[index]->GetValues();
}

DLLEXPORT bool NamedVars::GetValues(
    const std::string& name, vector<const VariableBlock*>& receiver) const
{
    GUARD_LOCK();
    auto index = Find(guard, name);
    // index check //
    if(index >= Variables.size()) {
        return false;
    }
    vector<VariableBlock*>& tmpvals = Variables[index]->GetValues();

    vector<const VariableBlock*> tmpconsted(tmpvals.size());

    for(size_t i = 0; i < tmpconsted.size(); i++) {

        tmpconsted[i] = const_cast<const VariableBlock*>(tmpvals[i]);
    }

    receiver = tmpconsted;
    return true;
}

DLLEXPORT std::shared_ptr<NamedVariableList> NamedVars::GetValueDirect(
    const std::string& name) const
{
    GUARD_LOCK();
    auto index = Find(guard, name);
    // index check //
    if(index >= Variables.size()) {
        return NULL;
    }
    return Variables[index];
}

DLLEXPORT NamedVariableList* NamedVars::GetValueDirectRaw(const std::string& name) const
{
    GUARD_LOCK();
    auto index = Find(guard, name);
    // index check //
    if(index >= Variables.size()) {
        return NULL;
    }

    return Variables[index].get();
}

DLLEXPORT NamedVariableList* Leviathan::NamedVars::GetValueDirectRaw(size_t index) const
{
    if(index >= Variables.size()) {
        return nullptr;
    }

    return Variables[index].get();
}

DLLEXPORT std::string Leviathan::NamedVars::Serialize(
    const std::string& lineprefix /*= ""*/) const
{
    std::string result;

    for(const auto& variable : Variables) {

        result += lineprefix + variable->ToText(0) + "\n";
    }

    return result;
}

// ------------------------------------ //
DLLEXPORT int NamedVars::GetVariableType(const std::string& name) const
{
    GUARD_LOCK();
    // call overload //
    return GetVariableType(guard, Find(guard, name));
}

DLLEXPORT int NamedVars::GetVariableType(Lock& guard, size_t index) const
{
    return Variables[index]->GetVariableType();
}

DLLEXPORT int NamedVars::GetVariableTypeOfAll(const std::string& name) const
{
    GUARD_LOCK();
    // call overload //
    return GetVariableTypeOfAll(guard, Find(guard, name));
}

DLLEXPORT int NamedVars::GetVariableTypeOfAll(Lock& guard, size_t index) const
{
    return Variables[index]->GetCommonType();
}
// ------------------------------------ //
string NamedVars::GetName(size_t index)
{
    GUARD_LOCK();

    return Variables[index]->GetName();
}

DLLEXPORT bool NamedVars::GetName(size_t index, string& name) const
{
    GUARD_LOCK();

    Variables[index]->GetName(name);
    return true;
}

void NamedVars::SetName(Lock& guard, size_t index, const std::string& name)
{

    Variables[index]->SetName(name);
}

void NamedVars::SetName(const std::string& oldname, const std::string& name)
{
    GUARD_LOCK();
    // call overload //
    SetName(guard, Find(guard, oldname), name);
}

bool NamedVars::CompareName(size_t index, const std::string& name) const
{
    GUARD_LOCK();

    return Variables[index]->CompareName(name);
}
// ------------------------------------ //
DLLEXPORT void NamedVars::AddVar(shared_ptr<NamedVariableList> values)
{
    GUARD_LOCK();
    RemoveIfExists(values->GetName(), guard);
    // just add to vector //
    Variables.push_back(values);
}

DLLEXPORT void NamedVars::AddVar(NamedVariableList* newvaluetoadd)
{
    GUARD_LOCK();
    RemoveIfExists(newvaluetoadd->GetName(), guard);
    // create new smart pointer and push back //
    Variables.push_back(shared_ptr<NamedVariableList>(newvaluetoadd));
}

DLLEXPORT void NamedVars::AddVar(const std::string& name, VariableBlock* valuetosteal)
{
    GUARD_LOCK();
    RemoveIfExists(name, guard);
    // create new smart pointer and push back //
    Variables.push_back(
        shared_ptr<NamedVariableList>(new NamedVariableList(name, valuetosteal)));
}
// ------------------------------------ //
void NamedVars::Remove(size_t index)
{
    GUARD_LOCK();

    // smart pointers //
    Variables.erase(Variables.begin() + index);
}

DLLEXPORT void NamedVars::Remove(const std::string& name)
{
    // call overload //
    Remove(Find(name));
}

DLLEXPORT void NamedVars::RemoveIfExists(const std::string& name, Lock& guard)
{
    // Try  to find it //
    size_t index = Find(guard, name);

    if(index >= Variables.size())
        return;


    Variables.erase(Variables.begin() + index);
}
// ------------------------------------ //
bool NamedVars::LoadVarsFromFile(const std::string& file, LErrorReporter* errorreport)
{
    // call datadump loaded with this object's vector //
    return FileSystem::LoadDataDump(file, Variables, errorreport);
}
vector<shared_ptr<NamedVariableList>>* NamedVars::GetVec()
{
    return &Variables;
}
void NamedVars::SetVec(vector<shared_ptr<NamedVariableList>>& vec)
{
    GUARD_LOCK();
    Variables = vec;
}
// ------------------------------------ //
DLLEXPORT size_t NamedVars::Find(Lock& guard, const std::string& name) const
{
    for(size_t i = 0; i < Variables.size(); i++) {
        if(Variables[i]->CompareName(name))
            return i;
    }

    return std::numeric_limits<size_t>::max();
}
// ------------------ Script compatible functions ------------------ //
#ifdef LEVIATHAN_USING_ANGELSCRIPT
ScriptSafeVariableBlock* NamedVars::GetScriptCompatibleValue(const std::string& name)
{
    // Use a try block to not throw exceptions to the script engine //
    try {
        VariableBlock& tmpblock = GetValueNonConst(name);

        // Create script safe version //
        return new ScriptSafeVariableBlock(&tmpblock, name);


    } catch(...) {
        // Something failed, return empty handle //
        return NULL;
    }
}

bool NamedVars::AddScriptCompatibleValue(ScriptSafeVariableBlock* value)
{
    GUARD_LOCK();

    RemoveIfExists(value->GetName(), guard);

    bool success = false;

    try {

        Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(value)));
        success = true;

    } catch(...) {
    }

    value->Release();
    return success;
}
#endif // LEVIATHAN_USING_ANGELSCRIPT

DLLEXPORT size_t NamedVars::GetVariableCount() const
{
    return Variables.size();
}
