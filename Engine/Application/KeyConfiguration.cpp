// ------------------------------------ //
#include "KeyConfiguration.h"

#include "FileSystem.h"
#include "Common/DataStoring/NamedVars.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::KeyConfiguration::KeyConfiguration(const string &configfile) :
    KeyStorageFile(configfile)
{
	// The file is stored and the map waits for init before loading //
	staticaccess = this;
}

DLLEXPORT Leviathan::KeyConfiguration::~KeyConfiguration(){
	// The keys should already be saved by now //
	// Reset static access //
	staticaccess = NULL;
}

DLLEXPORT KeyConfiguration* Leviathan::KeyConfiguration::Get(){
	return staticaccess;
}

KeyConfiguration* Leviathan::KeyConfiguration::staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::KeyConfiguration::Init(
    std::function<void (KeyConfiguration* checkfrom)> functocheck)
{
	GUARD_LOCK();

	// Skip if not given a file //
	if(KeyStorageFile.size() == 0)
		return true;

	// Load the values from the file //
	std::vector<shared_ptr<NamedVariableList>> tmpvalues;

	if(FileSystem::LoadDataDump(KeyStorageFile, tmpvalues)){
		// Create keys from the values //

		for(auto iter = tmpvalues.begin(); iter != tmpvalues.end(); ++iter){
			// Get name for storing //
			auto name = (*iter)->GetName();
			// Try to create a key //
			shared_ptr<std::vector<GKey>> keys(new std::vector<GKey>);

			std::vector<VariableBlock*>& values = (*iter)->GetValues();

			for(size_t i = 0; i < values.size(); i++){
				// Parse a key //
				if(values[i]->IsConversionAllowedNonPtr<string>()){
					keys->push_back(GKey::GenerateKeyFromString(values[i]->operator string()));

				}
			}

			// Assign to the map //
			KeyConfigurations[name] = keys;
		}
	}


	// Call the function with a pointer to this for it to verify loaded keys //
	functocheck(this);

	return true;
}

DLLEXPORT void Leviathan::KeyConfiguration::Release(){
	GUARD_LOCK();
	// Save all the keys //
	Save();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::KeyConfiguration::Save(){
	// Skip if not given a file //
	if(KeyStorageFile.size() == 0)
		return;

	// First generate a string for this //
	string savedata = "";

	GUARD_LOCK();

	// Loop through all keys and create string representations from them //
	for(auto iter = KeyConfigurations.begin(); iter != KeyConfigurations.end(); ++iter){

		// Print the name //
		savedata += iter->first+" = [";

		// Create a list of data //
		for(size_t i = 0; i < iter->second->size(); i++){

			if(i != 0)
				savedata += ", ";
			auto strrepresentation = iter->second->at(i).GenerateStringFromKey();

			savedata += "[\""+strrepresentation+"\"]";
		}

		// Add the line end //
		savedata += "];\n";
	}


	// Save the file //
	FileSystem::WriteToFile(savedata, KeyStorageFile);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<std::vector<GKey>> Leviathan::KeyConfiguration::ResolveControlNameToKey(
    const string &controlkey)
{
	GUARD_LOCK();
	auto iter = KeyConfigurations.find(controlkey);

	if(iter != KeyConfigurations.end())
		return iter->second;

	return NULL;
}

DLLEXPORT string Leviathan::KeyConfiguration::ResolveKeyToControlName(const GKey &key){
	GUARD_LOCK();
	// We need to loop through all the keys and see if any of them match //
	for(auto iter = KeyConfigurations.begin(); iter != KeyConfigurations.end(); ++iter){

		for(size_t i = 0; i < iter->second->size(); i++){
			// We'll use the partial match function //
			if(iter->second->at(i).Match(key, false))
				return iter->first;
		}
	}
	// Not found, return an empty string //
	return "";
}
// ------------------------------------ //
