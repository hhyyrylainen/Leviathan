#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKHANDLER
#include "NetworkHandler.h"
#endif
#include "FileSystem.h"
#include "ObjectFiles\ObjectFileObject.h"
#include "ObjectFiles\ObjectFileProcessor.h"
#include "SFML\Network\Http.hpp"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NetworkClient* clientside) : IsClient(clientside), IsServer(NULL), CloseMasterServerConnection(false),
	StopGetResponsesThread(false)
{
	instance = this;
}

DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NetworkServer* serverside) : IsServer(serverside), IsClient(NULL), CloseMasterServerConnection(false),
	StopGetResponsesThread(false)
{
	instance = this;
}

DLLEXPORT Leviathan::NetworkHandler::~NetworkHandler(){
	instance = NULL;
}

DLLEXPORT NetworkHandler* Leviathan::NetworkHandler::Get(){
	return instance;
}

NetworkHandler* Leviathan::NetworkHandler::instance = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkHandler::Init(const MasterServerInformation &info){
	// Query master server //
	QueryMasterServer(info);

	return true;
}

DLLEXPORT void Leviathan::NetworkHandler::Release(){
	// Notify master server connection kill //
	StopOwnUpdaterThread();
	if(MasterServerConnection){

		MasterServerConnection->ReleaseSocket();
	}

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
	TempGetResponsesThread.join();
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
DLLEXPORT void Leviathan::NetworkHandler::UpdateAllConnections(){
	ObjectLock guard(*this);
	for(auto iter = ConnectionsToUpdate.begin(); iter != ConnectionsToUpdate.end(); ++iter){

		(*iter)->UpdateListening();
	}
}

DLLEXPORT void Leviathan::NetworkHandler::StopOwnUpdaterThread(){
	ObjectLock guard(*this);
	StopGetResponsesThread = true;
}

DLLEXPORT void Leviathan::NetworkHandler::StartOwnUpdaterThread(){
	ObjectLock guard(*this);
	StopGetResponsesThread = false;
	// Start a thread for it //
	TempGetResponsesThread = boost::thread(RunTemporaryUpdateConnections, this);
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_RegisterConnectionInfo(ConnectionInfo* tomanage){
	ObjectLock guard(*this);
	
	ConnectionsToUpdate.push_back(tomanage);
}

void Leviathan::NetworkHandler::_UnregisterConnectionInfo(ConnectionInfo* unregisterme){
	ObjectLock guard(*this);

	for(auto iter = ConnectionsToUpdate.begin(); iter != ConnectionsToUpdate.end(); ++iter){

		if((*iter) == unregisterme){
			// Remove and don't cause iterator problems by returning //
			ConnectionsToUpdate.erase(iter);
			return;
		}

	}
}
// ------------------------------------ //
void Leviathan::RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar){
	// Try to load master server list //

	// Might as well start this thread here //
	instance->StartOwnUpdaterThread();

	// To reduce duplicated code and namespace pollution use a lambda thread for this //
	boost::thread DataUpdaterThread(boost::bind<void>([](NetworkHandler* instance) -> void{

		Logger::Get()->Info(L"NetworkHandler: Fetching new master server list...");
		sf::Http::Request request(Convert::WstringToString(instance->StoredMasterServerInfo.MasterListFetchPage), sf::Http::Request::Get);

		sf::Http httpserver(Convert::WstringToString(instance->StoredMasterServerInfo.MasterListFetchServer));

		sf::Http::Response response = httpserver.sendRequest(request);

		if(response.getStatus() == sf::Http::Response::Ok){

			// It should just be a list of master servers one on each line //
			WstringIterator itr(Convert::StringToWstring(response.getBody()));

			unique_ptr<wstring> data;

			std::vector<shared_ptr<wstring>> tmplist;

			while((data = itr.GetUntilNextCharacterOrAll(L'\n'))->size()){

				tmplist.push_back(shared_ptr<wstring>(data.release()));	
			}

			// Check //
			if(tmplist.size() == 0){

				Logger::Get()->Warning(L"NetworkHandler: retrieved an empty list of master servers, not updated");
				return;
			}

			// Update real list //
			ObjectLock guard(*instance);

			instance->MasterServers.clear();
			instance->MasterServers.reserve(tmplist.size());
			for(auto iter = tmplist.begin(); iter != tmplist.end(); ++iter){

				instance->MasterServers.push_back(*iter);
			}
			

			// Notify successful fetch //
			Logger::Get()->Info(L"NetworkHandler: Successfully fetched master server list:");
			for(auto iter = instance->MasterServers.begin(); iter != instance->MasterServers.end(); ++iter)
				Logger::Get()->Write(L"\t> "+*(*iter).get(), false);

		} else {
			// Fail //
			Logger::Get()->Error(L"NetworkHandler: failed to update master server list, using old list");
		}


	}, instance));

	if(!instance->_LoadMasterServerList()){

		// We need to request a new list before we can do anything //
		Logger::Get()->Info(L"NetworkHandler: no stored list of master servers, waiting for update to finish");
		DataUpdaterThread.join();
	}


	// Try to find a master server to connect to //
	for(size_t i = 0; i < instance->MasterServers.size(); i++){

		shared_ptr<wstring> tmpaddress;

		{
			ObjectLock guard(*instance);

			tmpaddress = instance->MasterServers[i];
		}

		// Try connection //
		shared_ptr<ConnectionInfo> tmpinfo(new ConnectionInfo(tmpaddress, sf::Socket::AnyPort));

		if(!tmpinfo->Init()){

			Logger::Get()->Error(L"NetworkHandler: failed to bind a socket!");
			continue;
		}

		// Create an identification request //
		shared_ptr<NetworkRequest> inforequest(new NetworkRequest(NETWORKREQUESTTYPE_IDENTIFICATION, 850));

		// Send and block until a request (or 5 fails) //
		shared_ptr<NetworkResponse> serverinforesponse = tmpinfo->SendRequestAndBlockUntilDone(inforequest, 3);

		if(!serverinforesponse){
			// Failed to receive something from the server //
			Logger::Get()->Warning(L"NetworkHandler: failed to receive a response from master server("+*tmpaddress+L")");
			// Release connection //
			tmpinfo->ReleaseSocket();
			continue;
		}

		Logger::Get()->Warning(L"NetworkHandler: received a response from master server("+*tmpaddress+L"):");

		wstring identificationstr, gamename, version, leviathanversion;

		// Get data //
		serverinforesponse->DecodeIdentificationStringResponse(identificationstr, gamename, version, leviathanversion);

		// Ensure data validness //
		Logger::Get()->Info(L"NetworkHandler: received a response from master server at"+*tmpaddress);

		// TODO: verify that the data is sane //

		{
			// Set working server //
			ObjectLock guard(*instance);
			instance->MasterServerConnection = tmpinfo;
		}

		// Successfully connected //
		resultvar->set_value(wstring(L"ConnectedToMasterServer = "+*tmpaddress+L";"));
		goto RunGetResponseFromMasterprepareexit;
	}

	// If we got here, we haven't connected to anything //
	Logger::Get()->Warning(L"NetworkHandler: could not connect to any fetched master servers, you can restart to use a new list");

	// We can let whoever is waiting for us to go now, and finish some utility tasks after that //
	resultvar->set_value(wstring(L"Failed to connect to master server"));

RunGetResponseFromMasterprepareexit:


	// This needs to be done here //
	DataUpdaterThread.join();

	// Output the current list //
	instance->_SaveMasterServerList();
}

void Leviathan::RunTemporaryUpdateConnections(NetworkHandler* instance){

	while(!instance->StopGetResponsesThread){

		instance->UpdateAllConnections();
		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}

}




