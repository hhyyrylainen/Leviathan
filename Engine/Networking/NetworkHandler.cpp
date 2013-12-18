#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKHANDLER
#include "NetworkHandler.h"
#endif
#include "FileSystem.h"
#include "ObjectFiles\ObjectFileObject.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "SFML\Network\Http.hpp"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NetworkClient* clientside) : IsClient(clientside), IsServer(NULL), CloseMasterServerConnection(false){

}

DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NetworkServer* serverside) : IsServer(serverside), IsClient(NULL){

}

DLLEXPORT Leviathan::NetworkHandler::~NetworkHandler(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkHandler::Init(const MasterServerInformation &info){
	// Query master server //
	QueryMasterServer(info);

	return true;
}

DLLEXPORT void Leviathan::NetworkHandler::Release(){
	// Notify master server connection kill //

	// Close all connections //
	if(IsClient){

		IsClient->Release();
		IsClient = NULL;
	}
	if(IsServer){

		IsServer->Release();
		IsServer = NULL;
	}

	// Kill master server connection //
	MasterServerConnectionThread.join();
}
// ------------------------------------ //
DLLEXPORT shared_ptr<boost::promise<wstring>> Leviathan::NetworkHandler::QueryMasterServer(const MasterServerInformation &info){
	// Might as well lock here //
	ObjectLock guard(*this);
	// Copy the data //
	StoredMasterServerInfo = info;

	shared_ptr<boost::promise<wstring>> resultvalue(new boost::promise<wstring>());

	// Run the task async //
	MasterServerConnectionThread = boost::thread(RunGetResponseFromMaster, this, resultvalue);
	
	return resultvalue;
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_SaveMasterServerList(){
	// Set up the values //
	vector<shared_ptr<ObjectFileObject>> objects;
	vector<shared_ptr<NamedVariableList>> Values;

	vector<VariableBlock*> vals;
	vals.reserve(MasterServers.size());
	{
		// Only this scope requires locking //
		ObjectLock guard(*this);

		for(size_t i = 0; i < MasterServers.size(); i++){
			vals.push_back(new VariableBlock(*MasterServers[i].get()));
		}
	}
	

	Values.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"MasterServers", vals)));

	ObjectFileProcessor::WriteObjectFile(objects, StoredMasterServerInfo.StoredListFile, Values, false);
}

bool Leviathan::NetworkHandler::_LoadMasterServerList(){

	vector<shared_ptr<NamedVariableList>> Values;
	ObjectFileProcessor::ProcessObjectFile(StoredMasterServerInfo.StoredListFile, Values);

	// Load the values from the value //
	for(size_t fi = 0; fi < Values.size(); fi++){
		if(Values[fi]->CompareName(L"MasterServers")){
			// Found value, load //
			size_t maxval = Values[fi]->GetVariableCount();
			MasterServers.reserve(maxval);

			// We need locking for this add //
			ObjectLock guard(*this);

			for(size_t i = 0; i < maxval; i++){
				MasterServers.push_back(unique_ptr<wstring>(new wstring(Values[fi]->GetValueDirect(i)->ConvertAndReturnVariable<wstring>())));
			}
				
			return true;
		}
	}
	return false;
}
// ------------------------------------ //
DLLEXPORT wstring Leviathan::NetworkHandler::GetServerAddressPartOfAddress(const wstring &fulladdress, const wstring &regextouse /*= L"http://.*?/"*/){
	// Create a regex //
	wregex findaddressregex(regextouse, regex_constants::icase); 

	match_results<const wchar_t*> addressmatch;

	// Return the match //
	regex_search(fulladdress.c_str(), addressmatch, findaddressregex);


	return wstring(addressmatch[1]);
}
// ------------------------------------ //
void Leviathan::RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar){
	// Try to load master server list //

	if(!instance->_LoadMasterServerList()){

		// We need to request a new list before we can do anything //
		sf::Http::Request request(Convert::WstringToString(instance->StoredMasterServerInfo.MasterListFetchPage), sf::Http::Request::Get);

		sf::Http httpserver(Convert::WstringToString(instance->StoredMasterServerInfo.MasterListFetchServer));

		sf::Http::Response response = httpserver.sendRequest(request);

		if(response.getStatus() == sf::Http::Response::Ok){

			// It should just be a list of master servers one on each line //
			WstringIterator itr(Convert::StringToWstring(response.getBody()));

			ObjectLock guard(*instance);

			unique_ptr<wstring> data;

			while((data = itr.GetUntilNextCharacterOrAll(L'\n'))->size()){

				instance->MasterServers.push_back(shared_ptr<wstring>(data.release()));
			}

			// Notify successful fetch //
			Logger::Get()->Info(L"NetworkHandler: Successfully fetched master server list:");
			for(auto iter = instance->MasterServers.begin(); iter != instance->MasterServers.end(); ++iter)
				Logger::Get()->Write(L"\t> "+*(*iter).get(), false);

		} else {
			// Fail //
			Logger::Get()->Error(L"NetworkHandler: failed to update master server list, using old list");
		}
	}
	// We can just update the master server list in another thread //



	resultvar->set_value(wstring(L"Not done!"));

	// Output the current list //
	instance->_SaveMasterServerList();
}




