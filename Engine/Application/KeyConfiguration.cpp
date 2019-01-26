// ------------------------------------ //
#include "KeyConfiguration.h"

#include "Common/DataStoring/NamedVars.h"
#include "FileSystem.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT KeyConfiguration::KeyConfiguration(const std::string& configfile) :
    KeyStorageFile(configfile)
{}

//! Creates an in-memory only configuration
DLLEXPORT KeyConfiguration::KeyConfiguration() : InMemory(true) {}

DLLEXPORT KeyConfiguration::~KeyConfiguration()
{
    // The keys should already be saved by now //
}
// ------------------------------------ //
DLLEXPORT bool KeyConfiguration::Init(
    std::function<void(Lock& guard, KeyConfiguration* checkfrom)> functocheck)
{
    GUARD_LOCK();

    // Load the values from the file //
    if(!InMemory) {
        std::vector<std::shared_ptr<NamedVariableList>> tmpvalues;

        if(FileSystem::LoadDataDump(KeyStorageFile, tmpvalues, Logger::Get())) {
            // Create keys from the values //

            for(auto iter = tmpvalues.begin(); iter != tmpvalues.end(); ++iter) {
                // Get name for storing //
                auto name = (*iter)->GetName();
                // Try to create a key //
                auto keys = std::make_shared<std::vector<GKey>>();

                std::vector<VariableBlock*>& values = (*iter)->GetValues();

                for(size_t i = 0; i < values.size(); i++) {
                    // Parse a key //
                    if(values[i]->IsConversionAllowedNonPtr<std::string>()) {
                        keys->push_back(
                            GKey::GenerateKeyFromString(values[i]->operator std::string()));
                    }
                }

                // Assign to the map //
                KeyConfigurations[name] = keys;
            }
        }
    }

    // Call the function with a pointer to this for it to verify loaded keys //
    if(functocheck)
        functocheck(guard, this);

    if(Marked)
        Save(guard);

    return true;
}

DLLEXPORT void KeyConfiguration::Release()
{
    GUARD_LOCK();

    // Save all the keys //
    if(Marked)
        Save(guard);
}
// ------------------------------------ //
DLLEXPORT bool KeyConfiguration::AddKeyIfMissing(
    Lock& guard, const std::string& name, const std::vector<std::string>& defaultkeys)
{
    if(defaultkeys.empty()) {
        LOG_WARNING("KeyConfiguration: AddKeyIfMissing: defaultkeys is empty, not adding one "
                    "with name: " +
                    name);
        return false;
    }

    auto iter = KeyConfigurations.find(name);
    if(iter != KeyConfigurations.end())
        return false;

    auto keys = std::make_shared<std::vector<GKey>>();

    for(const auto& key : defaultkeys)
        keys->push_back(GKey::GenerateKeyFromString(key));

    KeyConfigurations[name] = keys;

    Marked = true;
    return true;
}

DLLEXPORT void KeyConfiguration::MarkAsChanged()
{
    Marked = true;
}
// ------------------------------------ //
DLLEXPORT void KeyConfiguration::Save(Lock& guard)
{
    if(InMemory)
        return;

    // Skip if not given a file //
    if(KeyStorageFile.size() == 0)
        return;

    // First generate a string for this //
    std::string savedata = "";

    // Loop through all keys and create string representations from them //
    for(auto iter = KeyConfigurations.begin(); iter != KeyConfigurations.end(); ++iter) {

        // Print the name //
        savedata += iter->first + " = [";

        // Create a list of data //
        for(size_t i = 0; i < iter->second->size(); i++) {

            if(i != 0)
                savedata += ", ";
            auto strrepresentation = iter->second->at(i).GenerateStringFromKey();

            savedata += "[\"" + strrepresentation + "\"]";
        }

        // Add the line end //
        savedata += "];\n";
    }


    // Save the file //
    FileSystem::WriteToFile(savedata, KeyStorageFile);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<std::vector<GKey>> KeyConfiguration::ResolveControlNameToKey(
    const std::string& controlkey)
{
    GUARD_LOCK();
    auto iter = KeyConfigurations.find(controlkey);

    if(iter != KeyConfigurations.end())
        return iter->second;

    return NULL;
}

DLLEXPORT const std::vector<GKey>& KeyConfiguration::ResolveControlNameToKeyVector(
    const std::string& controlkey)
{
    GUARD_LOCK();
    auto iter = KeyConfigurations.find(controlkey);

    if(iter != KeyConfigurations.end()) {

        if(!iter->second)
            throw InvalidArgument("Key with name \"" + controlkey + "\" is nullptr");

        return *iter->second;
    }

    throw InvalidArgument("Key with name \"" + controlkey + "\" not found");
}

DLLEXPORT GKey KeyConfiguration::ResolveControlNameToFirstKey(const std::string& controlkey)
{
    GUARD_LOCK();
    auto iter = KeyConfigurations.find(controlkey);

    if(iter != KeyConfigurations.end())
        return iter->second->at(0);

    throw InvalidArgument("Key with name \"" + controlkey + "\" not found");
}

DLLEXPORT std::string KeyConfiguration::ResolveKeyToControlName(const GKey& key)
{
    GUARD_LOCK();
    // We need to loop through all the keys and see if any of them match //
    for(auto iter = KeyConfigurations.begin(); iter != KeyConfigurations.end(); ++iter) {

        for(size_t i = 0; i < iter->second->size(); i++) {
            // We'll use the partial match function //
            if(iter->second->at(i).Match(key, false))
                return iter->first;
        }
    }
    // Not found, return an empty string //
    return "";
}
// ------------------------------------ //
