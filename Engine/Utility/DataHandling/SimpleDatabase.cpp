// ------------------------------------ //
#include "SimpleDatabase.h"

#include "Common/StringOperations.h"
#include "FileSystem.h"
#include "Iterators/StringIterator.h"
#include "json/json.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::SimpleDatabase::SimpleDatabase(const std::string& databasename) {}

DLLEXPORT Leviathan::SimpleDatabase::~SimpleDatabase() {}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SimpleDatabase::AddValue(
    const std::string& database, std::shared_ptr<SimpleDatabaseRowObject> valuenamesandvalues)
{
    GUARD_LOCK();
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

DLLEXPORT bool Leviathan::SimpleDatabase::RemoveValue(const std::string& database, int row)
{
    GUARD_LOCK();
    // If we are missing the database we shouldn't add it //
    SimpleDatabaseObject::iterator iter = Database.find(database);

    if(iter == Database.end()) {
        // No such database //
        return false;
    }

    // Remove at the specified index if possible //
    if(iter->second->size() <= (size_t)row || row < 0)
        return false;

    // Remove //
    iter->second->erase(iter->second->begin() + (size_t)row);
    // Notify update //


    return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SimpleDatabase::GetRow(std::vector<std::string>& row,
    const std::string& table, int row_index, const std::vector<std::string>& columns)
{
    GUARD_LOCK();
    // If we are missing the database we shouldn't add it //
    SimpleDatabaseObject::iterator iter = Database.find(table);

    if(iter == Database.end()) {
        // No such database //
        return;
    }

    // Validate index //
    if(iter->second->size() <= (size_t)row_index || row_index < 0) {
        return;
    }

    // Valid value //
    const std::map<std::string, std::shared_ptr<VariableBlock>>& datbaseentry =
        *iter->second->at(row_index).get();

    // Copy data //
    for(size_t i = 0; i < columns.size(); i++) {


        auto iter2 = datbaseentry.find(columns[i]);
        if(iter2 != datbaseentry.end()) {
            // Add to result //
            row.push_back(iter2->second->operator std::string());
        }
    }
}

DLLEXPORT size_t Leviathan::SimpleDatabase::GetNumRows(const std::string& table)
{
    GUARD_LOCK();
    // If we are missing the database we shouldn't add it //
    SimpleDatabaseObject::iterator iter = Database.find(table);

    if(iter == Database.end()) {
        // No such database //
        return 0;
    }

    return iter->second->size();
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<VariableBlock> Leviathan::SimpleDatabase::GetValueOnRow(
    const std::string& table, const std::string& valuekeyname,
    const VariableBlock& wantedvalue, const std::string& wantedvaluekey)
{
    GUARD_LOCK();
    // Search the database for matching row and return another value from that row //
    // If we are missing the database we shouldn't add it //
    SimpleDatabaseObject::iterator iter = Database.find(table);

    if(iter == Database.end()) {
        // No such database //
        return NULL;
    }

    // Search the database //
    for(size_t i = 0; i < iter->second->size(); i++) {

        auto finditer = iter->second->at(i)->find(valuekeyname);

        if(finditer == iter->second->at(i)->end())
            continue;

        // Check does the value match //
        if(*finditer->second == wantedvalue) {
            // Found match //
            auto wantedfinder = iter->second->at(i)->find(wantedvaluekey);

            if(wantedfinder != iter->second->at(i)->end()) {

                return wantedfinder->second;
            }
        }
    }

    return NULL;
}


DLLEXPORT bool Leviathan::SimpleDatabase::WriteTableToJson(
    const std::string& tablename, string& receiver, bool humanreadable /*= false*/)
{
    // Holds the data //
    Json::Value root;

    // Add all the values to this //
    Json::Value tablearray;

    {
        GUARD_LOCK();
        // If we are missing the database we shouldn't add it //
        SimpleDatabaseObject::iterator iter = Database.find(tablename);

        if(iter == Database.end()) {
            // No such database //
            return false;
        }

        // Loop all the values on this table //
        for(size_t i = 0; i < iter->second->size(); i++) {
            // Create a value from this row //
            Json::Value rowdata;

            // Loop all values //
            for(auto iter2 = iter->second->at(i)->begin(); iter2 != iter->second->at(i)->end();
                ++iter2) {
                // We want to set it as the native data //
                switch(iter2->second->GetBlockConst()->Type) {
                case DATABLOCK_TYPE_INT:
                    rowdata[(iter2->first)] = iter2->second->operator int();
                    break;
                case DATABLOCK_TYPE_FLOAT:
                    rowdata[(iter2->first)] = iter2->second->operator float();
                    break;
                case DATABLOCK_TYPE_BOOL:
                    rowdata[(iter2->first)] = iter2->second->operator bool();
                    break;
                case DATABLOCK_TYPE_WSTRING:
                    rowdata[(iter2->first)] = iter2->second->operator string();
                    break;
                case DATABLOCK_TYPE_STRING:
                    rowdata[(iter2->first)] = iter2->second->operator string();
                    break;
                case DATABLOCK_TYPE_CHAR:
                    rowdata[(iter2->first)] = iter2->second->operator char();
                    break;
                case DATABLOCK_TYPE_DOUBLE:
                    rowdata[(iter2->first)] = iter2->second->operator double();
                    break;
                default: assert(0 && "unallowed datablock type in SimpleDatabase");
                }
            }

            // Set the value to the array //
            tablearray.append(rowdata);
        }
    }
    // The lock ends here since it is no longer needed //

    // Add it and write //
    root[(tablename)] = tablearray;

    // It succeeded //
    if(!humanreadable) {
        Json::FastWriter writer;
        receiver = writer.write(root);
    } else {
        Json::StyledWriter writer;
        receiver = writer.write(root);
    }

    return true;
}

// ------------------------------------ //
SimpleDatabaseObject::iterator Leviathan::SimpleDatabase::_EnsureTable(const std::string& name)
{
    // Try to find it //
    SimpleDatabaseObject::iterator iter = Database.find(name);

    if(iter != Database.end()) {
        // Valid database //
        return iter;
    }
    // Ensure new database //
    Database[name] = std::shared_ptr<
        std::vector<shared_ptr<std::map<std::string, std::shared_ptr<VariableBlock>>>>>(
        new std::vector<shared_ptr<std::map<std::string, std::shared_ptr<VariableBlock>>>>());

    // Recurse, might want to avoid stack overflow //
    return _EnsureTable(name);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SimpleDatabase::LoadFromFile(const std::string& file)
{
    // The file should be able to be processed as named variable lists //
    // read the file entirely //
    std::string filecontents;

    try {
        FileSystem::ReadFileEntirely(file, filecontents);
    } catch(const InvalidArgument& e) {

        Logger::Get()->Error(
            "SimpleDatabase: LoadFromFile: file could not be read, exception:");
        e.PrintToLog();
        return false;
    }


    // file needs to be split to lines //
    vector<std::string> Lines;

    if(!StringOperations::CutString(filecontents, std::string("\n"), Lines)) {

        Lines.push_back(filecontents);
        Logger::Get()->Warning(
            "ObjectFileProcessor: file seems to be only a single line: " + filecontents);
    }

    GUARD_LOCK();

    SimpleDatabaseObject::iterator insertiter;

    for(size_t i = 0; i < Lines.size(); i++) {
        // skip empty lines //
        if(Lines[i].size() == 0) {
            continue;
        }

        // Check is this new table //
        if(StringOperations::StringStartsWith<std::string>(Lines[i], "TABLE")) {
            // Move to a new table //

            StringIterator itr(&Lines[i]);

            auto tablename = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

            Database[*tablename] =
                std::shared_ptr<std::vector<shared_ptr<SimpleDatabaseRowObject>>>(
                    new std::vector<shared_ptr<SimpleDatabaseRowObject>>());

            // Change the iter //
            insertiter = Database.find(*tablename);
            continue;
        }

        // try to create a named var from this line //
        try {
            shared_ptr<NamedVariableList> namevar(new NamedVariableList(Lines[i]));
            // didn't cause an exception, is valid add //

            insertiter->second->push_back(
                shared_ptr<SimpleDatabaseRowObject>(new SimpleDatabaseRowObject()));

            auto toinsert = insertiter->second->back();

            if(namevar->GetVariableCount() % 2 != 0) {

                Logger::Get()->Warning("SimpleDatabase: LoadFromFile: file: " + file +
                                       ", line: " + Convert::ToString(i) +
                                       " has invalid number of elements");
                continue;
            }

            for(size_t namei = 0; namei < namevar->GetVariableCount(); namei += 2) {

                std::string name;

                namevar->GetValueDirect(namei)->ConvertAndAssingToVariable<std::string>(name);

                string blockdata;

                if(!namevar->GetValueDirect(namei + 1)->ConvertAndAssingToVariable<string>(
                       blockdata)) {
                    Logger::Get()->Warning("SimpleDatabase: LoadFromFile: file: " + file +
                                           ", line: " + Convert::ToString(i) +
                                           " couldn't convert value to string");
                }

                (*toinsert)[name] =
                    std::shared_ptr<VariableBlock>(new VariableBlock(blockdata));
            }
        } catch(...) {
            continue;
        }
    }
    // Is done //
    return true;
}

DLLEXPORT void Leviathan::SimpleDatabase::SaveToFile(const std::string& file)
{

    std::string datastr;
    std::string tmpdata;
    {
        GUARD_LOCK();
        // Just iterate over everything and write them to file //
        for(auto iter = Database.begin(); iter != Database.end(); ++iter) {

            datastr += "TABLE = \"" + iter->first + "\";\n";

            for(auto iter2 = iter->second->begin(); iter2 != iter->second->end(); ++iter2) {

                datastr += "n= [";

                for(auto iter3 = (*iter2)->begin(); iter3 != (*iter2)->end(); ++iter3) {

                    datastr += "[\"" + iter3->first + "\"]";

                    if(!iter3->second->ConvertAndAssingToVariable<std::string>(tmpdata)) {

                        assert(0 && "database has a value that cannot be stored as a string");
                    }
                    datastr += "[\"" + tmpdata + "\"]";
                }

                datastr += "];\n";
            }
        }
    }

    FileSystem::WriteToFile(datastr, file);
}
