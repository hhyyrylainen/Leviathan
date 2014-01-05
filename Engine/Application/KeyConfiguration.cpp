#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_KEYCONFIGURATION
#include "KeyConfiguration.h"
#endif
#include "FileSystem.h"
#include "Common/DataStoring/NamedVars.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::KeyConfiguration::KeyConfiguration(const wstring &configfile) : KeyStorageFile(configfile){
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
DLLEXPORT bool Leviathan::KeyConfiguration::Init(boost::function<void (KeyConfiguration* checkfrom)> functocheck){
	ObjectLock guard(*this);

	// Skip if not given a file //
	if(KeyStorageFile.size() == 0)
		return true;

	// Load the values from the file //
	std::vector<shared_ptr<NamedVariableList>> tmpvalues;

	if(FileSystem::LoadDataDump(KeyStorageFile, tmpvalues) != 0){

		Logger::Get()->QueueErrorMessage(L"KeyConfiguration: could not load file: "+KeyStorageFile);
	} else {
		// Create keys from the values //

		for(auto iter = tmpvalues.begin(); iter != tmpvalues.end(); ++iter){
			// Get name for storing //
			const wstring& name = (*iter)->GetName();
			// Try to create a key //
			shared_ptr<std::vector<GKey>> keys(new std::vector<GKey>);

			std::vector<VariableBlock*>& values = (*iter)->GetValues();

			for(size_t i = 0; i < values.size(); i++){
				// Parse a key //
				if(values[i]->IsConversionAllowedNonPtr<wstring>()){
					keys->push_back(GKey::GenerateKeyFromString(values[i]->operator wstring()));

				} else {
					// Warn about this //
					Logger::Get()->Warning(L"KeyConfiguration: Warning key type is not convertible to wstring! name: "+name);
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
	ObjectLock guard(*this);
	// Save all the keys //
	Save();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::KeyConfiguration::Save(){
	// Skip if not given a file //
	if(KeyStorageFile.size() == 0)
		return;

	// First generate a string for this //
	wstring savedata = L"";

	ObjectLock guard(*this);

	// Loop through all keys and create string representations from them //
	for(auto iter = KeyConfigurations.begin(); iter != KeyConfigurations.end(); ++iter){

		// Print the name //
		savedata += iter->first+L" = [";

		// Create a list of data //
		for(size_t i = 0; i < iter->second->size(); i++){

			if(i != 0)
				savedata += L", ";
			wstring strrepresentation = iter->second->at(i).GenerateWstringFromKey();

			savedata += L"[\""+strrepresentation+L"\"]";
		}

		// Add the line end //
		savedata += L"];\n";
	}


	// Save the file //
	FileSystem::WriteToFile(savedata, KeyStorageFile);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<std::vector<GKey>> Leviathan::KeyConfiguration::ResolveControlNameToKey(const wstring &controlkey){
	ObjectLock guard(*this);
	auto iter = KeyConfigurations.find(controlkey);

	if(iter != KeyConfigurations.end())
		return iter->second;

	return NULL;
}

DLLEXPORT wstring Leviathan::KeyConfiguration::ResolveKeyToControlName(const GKey &key){
	ObjectLock guard(*this);
	// We need to loop through all the keys and see if any of them match //
	for(auto iter = KeyConfigurations.begin(); iter != KeyConfigurations.end(); ++iter){

		for(size_t i = 0; i < iter->second->size(); i++){
			// We'll use the partial match function //
			if(iter->second->at(i).Match(key, false))
				return iter->first;
		}
	}
	// Not found, return an empty string //
	return L"";
}
// ------------------------------------ //
