#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKHANDLER
#include "NetworkHandler.h"
#endif
#include "FileSystem.h"
#include "ObjectFiles/ObjectFile.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "SFML/Network/Http.hpp"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "Application/GameConfiguration.h"
#include "Utility/ComplainOnce.h"
#include "ConnectionInfo.h"
#include "RemoteConsole.h"
#include "SyncedVariables.h"
#include "GameSpecificPacketHandler.h"
#include "Iterators/StringIterator.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler) : AppType(ntype), 
	CloseMasterServerConnection(false), StopGetResponsesThread(true)
{
	instance = this;
	interfaceinstance = packethandler;

	// Set our type to the NetworkInteface //
	interfaceinstance->_SetNetworkType(AppType);

	// Create the variable syncer //
	VariableSyncer = new SyncedVariables(this, AppType == NETWORKED_TYPE_SERVER, interfaceinstance);

	// Create the custom packet handler //
	_GameSpecificPacketHandler = new GameSpecificPacketHandler(interfaceinstance);
}

DLLEXPORT Leviathan::NetworkHandler::~NetworkHandler(){
	instance = NULL;

	SAFE_DELETE(VariableSyncer);
	SAFE_DELETE(_GameSpecificPacketHandler);
}

DLLEXPORT NetworkHandler* Leviathan::NetworkHandler::Get(){
	return instance;
}

DLLEXPORT NetworkInterface* Leviathan::NetworkHandler::GetInterface(){
	return interfaceinstance;
}

NetworkHandler* Leviathan::NetworkHandler::instance = NULL;
NetworkInterface* Leviathan::NetworkHandler::interfaceinstance = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkHandler::Init(const MasterServerInformation &info){
	GUARD_LOCK_THIS_OBJECT();

	MasterServerMustPassIdentification = info.MasterServerIdentificationString;

	if(AppType != NETWORKED_TYPE_MASTER){
		// Query master server //
		QueryMasterServer(info);

	} else {
		// We are our own master! //
		
		// Get out port number here //
		GAMECONFIGURATION_GET_VARIABLEACCESS(vars);

		int tmpport = 0;

		if(!vars->GetValueAndConvertTo<int>(L"MasterServerPort", tmpport)){
			// This is quite bad //
			Logger::Get()->Error(L"NetworkHandler: Init: no port configured, config missing 'MasterServerPort' of type int");
		}

		PortNumber = (USHORT)tmpport;
	}

	if(AppType == NETWORKED_TYPE_CLIENT){
		// We can use any port we get //
		PortNumber = sf::Socket::AnyPort;

	} else if(AppType == NETWORKED_TYPE_SERVER){
		// We need to use a specific port //
		GAMECONFIGURATION_GET_VARIABLEACCESS(vars);

		int tmpport = 0;

		if(!vars->GetValueAndConvertTo<int>(L"DefaultServerPort", tmpport)){
			// This is quite bad //
			Logger::Get()->Error(L"NetworkHandler: Init: no port configured, config missing 'ServerPort' of type int");
		}

		PortNumber = (USHORT)tmpport;
	}

	// We want to receive responses //
	if(_Socket.bind(PortNumber) != sf::Socket::Done){

		Logger::Get()->Error(L"NetworkHandler: Init: failed to bind to a port "+Convert::ToWstring(PortNumber));
		return false;
	}

	// Set the socket as non-blocking //
	_Socket.setBlocking(false);

	// Report success //
	Logger::Get()->Info(L"NetworkHandler: running listening socket on port "+Convert::ToWstring(_Socket.getLocalPort()));

	// Might as well start this thread here //
	StartOwnUpdaterThread();

	return true;
}

DLLEXPORT void Leviathan::NetworkHandler::Release(){
	GUARD_LOCK_THIS_OBJECT();
	
	CloseMasterServerConnection = true;
	
	// Kill master server connection //
	//MasterServerConnectionThread.join();
	StopOwnUpdaterThread();
	TempGetResponsesThread.join();

	// Notify master server connection kill //
	if(MasterServerConnection){

		MasterServerConnection->Release();
	}
	// Close all connections //
	for(size_t i = 0; i < AutoOpenedConnections.size(); i++){

		AutoOpenedConnections[i]->Release();
	}
	
	MasterServerConnectionThread.join();
	MasterServerConnection.reset();
	AutoOpenedConnections.clear();

	_ReleaseSocket();
}

void Leviathan::NetworkHandler::_ReleaseSocket(){
	// This might cause the game to hang... //

	bool blockunbind = false;
	
	{
		GAMECONFIGURATION_GET_VARIABLEACCESS(variables);
		variables->GetValueAndConvertTo<bool>(L"DisableSocketUnbind", blockunbind);
	}

	// This should do the trick //
	if(!blockunbind){
		_Socket.unbind();
	} else {
		Logger::Get()->Info(L"NetworkHandler: _ReleaseSocket: blocked unbind");
	}
}
// ------------------------------------ //
DLLEXPORT shared_ptr<boost::promise<wstring>> Leviathan::NetworkHandler::QueryMasterServer(const MasterServerInformation &info){
	// Might as well lock here //
	GUARD_LOCK_THIS_OBJECT();
	// Copy the data //
	StoredMasterServerInfo = info;

	shared_ptr<boost::promise<wstring>> resultvalue(new boost::promise<wstring>());

	// Make sure it doesn't die instantly //
	CloseMasterServerConnection = false;
	
	// Run the task async //
	MasterServerConnectionThread = boost::thread(RunGetResponseFromMaster, this, resultvalue);

	return resultvalue;
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_SaveMasterServerList(){
	// Set up the values //
	vector<VariableBlock*> vals;
	vals.reserve(MasterServers.size());
	{
		// Only this scope requires locking //
		GUARD_LOCK_THIS_OBJECT();

		for(size_t i = 0; i < MasterServers.size(); i++){
			vals.push_back(new VariableBlock(*MasterServers[i].get()));
		}
	}

	NamedVars headervars(new NamedVariableList(L"MasterServers", vals));
	
	ObjectFile file(headervars);
	

	ObjectFileProcessor::WriteObjectFile(file, StoredMasterServerInfo.StoredListFile);
}

bool Leviathan::NetworkHandler::_LoadMasterServerList(){

	// Skip if there is no file defined //
	if(!StoredMasterServerInfo.StoredListFile.size())
		return false;

	auto fileobj = ObjectFileProcessor::ProcessObjectFile(StoredMasterServerInfo.StoredListFile);

	if(!fileobj)
		return false;

	// Try to get the right variable with the name //
	auto foundvar = fileobj->GetVariables()->GetValueDirectRaw(L"MasterServers");

	if(!foundvar)
		return false;

	// Found value, load //
	size_t maxval = foundvar->GetVariableCount();
	MasterServers.reserve(maxval);

	// We need locking for this add //
	GUARD_LOCK_THIS_OBJECT();

	for(size_t i = 0; i < maxval; i++){
		MasterServers.push_back(unique_ptr<wstring>(new wstring(foundvar->GetValueDirect(i)->ConvertAndReturnVariable<wstring>())));
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT wstring Leviathan::NetworkHandler::GetServerAddressPartOfAddress(const wstring &fulladdress, const wstring &regextouse /*= L"http://.*?/"*/){
	// Create a regex //
	boost::wregex findaddressregex(regextouse, boost::regex_constants::icase);

	boost::match_results<const wchar_t*> addressmatch;

	// Return the match //
	boost::regex_search(fulladdress.c_str(), addressmatch, findaddressregex);


	return wstring(addressmatch[1]);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::UpdateAllConnections(){
	GUARD_LOCK_THIS_OBJECT();

	// Remove closed connections //
	RemoveClosedConnections(guard);
	
	// Update remote console sessions if they exist //
	auto rconsole = RemoteConsole::Get();
	if(rconsole)
		rconsole->UpdateStatus();

	// Let's listen for things //
	sf::Packet receivedpacket;

	sf::IpAddress sender;
	USHORT sentport;

	// Loop through all received packets //
	while(_Socket.receive(receivedpacket, sender, sentport) == sf::Socket::Done){
		// Process packet //
		//Logger::Get()->Info(L"NetworkHandler: received a packet! from "+Convert::StringToWstring(sender.toString()));

		// Pass to a connection //
		bool Passed = false;

		for(size_t i = 0; i < ConnectionsToUpdate.size(); i++){
			// Keep passing until somebody handles it //
			if(ConnectionsToUpdate[i]->IsThisYours(receivedpacket, sender, sentport)){
				Passed = true;
				break;
			}
		}

		if(Passed)
			continue;

		shared_ptr<ConnectionInfo> tmpconnect;

		// \todo Check is it a close or a keepalive packet //



		// We might want to open a new connection to this client //
		Logger::Get()->Info(L"Received a new connection from "+Convert::StringToWstring(sender.toString())+L":"+Convert::ToWstring(sentport));

		if(AppType != NETWORKED_TYPE_CLIENT){
			// Accept the connection //
			Logger::Get()->Info(L"\t> Connection accepted");

			tmpconnect = shared_ptr<ConnectionInfo>(new ConnectionInfo(sender, sentport));

		} else if(RemoteConsole::Get()->IsAwaitingConnections()){
			// We might allow a remote start remote console session //
			Logger::Get()->Info(L"\t> Connection accepted for remote console receive");

			tmpconnect = shared_ptr<ConnectionInfo>(new ConnectionInfo(sender, sentport));
			// We need a special restriction for this connection //
			tmpconnect->SetRestrictionMode(CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE);
			
		} else {
			// Deny the connection //
			Logger::Get()->Info(L"\t> Dropping connection due to not being a server (and not expecting a remote console session)");

		}

		if(tmpconnect){
			// Try to handle with the new connection //
			// We need to initialize the new connection first //
			if(!tmpconnect->Init()){
				// This should never happen //
				assert(0 && "connection init function should never fail");
			}

			AutoOpenedConnections.push_back(tmpconnect);

			// Try to handle the packet //
			if(!tmpconnect->IsThisYours(receivedpacket, sender, sentport)){
				// That's an error //
				Logger::Get()->Error(L"NetworkHandler: UpdateAllConnections: new connection refused to process it's packet from"
					+Convert::StringToWstring(sender.toString())+L":"+Convert::ToWstring(sentport));
			}
		}

	}
	// Time-out requests //
	for(size_t i = 0; i < ConnectionsToUpdate.size(); i++){
		//// This needs to be unlocked to avoid deadlocking //
		// The callee needs to make sure to use the right locking function to not deadlock //
		ConnectionsToUpdate[i]->UpdateListening();
	}

	// Interface might want to do something //
	interfaceinstance->TickIt();
}

DLLEXPORT void Leviathan::NetworkHandler::StopOwnUpdaterThread(){
	GUARD_LOCK_THIS_OBJECT();
	StopGetResponsesThread = true;
}

DLLEXPORT void Leviathan::NetworkHandler::StartOwnUpdaterThread(){
	GUARD_LOCK_THIS_OBJECT();
	// Check if already running //
	if(StopGetResponsesThread == false)
		return;
	StopGetResponsesThread = false;
	// Start a thread for it //
	TempGetResponsesThread = boost::thread(RunTemporaryUpdateConnections, this);
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_RegisterConnectionInfo(ConnectionInfo* tomanage){
	GUARD_LOCK_THIS_OBJECT();

	ConnectionsToUpdate.push_back(tomanage);
}

void Leviathan::NetworkHandler::_UnregisterConnectionInfo(ConnectionInfo* unregisterme){
	GUARD_LOCK_THIS_OBJECT();

	for(auto iter = ConnectionsToUpdate.begin(); iter != ConnectionsToUpdate.end(); ++iter){

		if((*iter) == unregisterme){
			// Remove and don't cause iterator problems by returning //
			ConnectionsToUpdate.erase(iter);
			return;
		}

	}
}

shared_ptr<boost::strict_lock<boost::basic_lockable_adapter<boost::recursive_mutex>>> Leviathan::NetworkHandler::LockSocketForUse(){
	return shared_ptr<boost::strict_lock<boost::basic_lockable_adapter<boost::recursive_mutex>>>(
		new boost::strict_lock<boost::basic_lockable_adapter<boost::recursive_mutex>>(SocketMutex));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::SafelyCloseConnectionTo(ConnectionInfo* to){
	GUARD_LOCK_THIS_OBJECT();

	// Make sure that it isn't there already //
	for(auto iter = ConnectionsToTerminate.begin(); iter != ConnectionsToTerminate.end(); ++iter){
		// Return if we found a match //
		if(*iter == to)
			return;
	}


	// Add to the queue //
	ConnectionsToTerminate.push_back(to);
}

DLLEXPORT void Leviathan::NetworkHandler::RemoveClosedConnections(ObjectLock &guard){
	VerifyLock(guard);

	if(ConnectionsToUpdate.size() == 0 || ConnectionsToTerminate.size() == 0)
		return;

	// Go through the removed connection list and remove them //
	for(size_t i = 0; i < ConnectionsToTerminate.size(); i++){
		// Send a close packet //
		ConnectionsToTerminate[i]->SendCloseConnectionPacket();

		// Close it //
		ConnectionsToTerminate[i]->Release();
		// The connection will automatically remove itself from the vector //

		// But if we have opened it we need to delete our pointer //
		for(size_t a = 0; a < AutoOpenedConnections.size(); a++){
			if(AutoOpenedConnections[a].get() == ConnectionsToTerminate[i]){
				AutoOpenedConnections.erase(AutoOpenedConnections.begin()+a);
				break;
			}
		}
	}

	// All are handled, clear them //
	ConnectionsToTerminate.clear();
}
// ------------------------------------ //
DLLEXPORT USHORT Leviathan::NetworkHandler::GetOurPort(){
	GUARD_LOCK_THIS_OBJECT();
	return _Socket.getLocalPort();
}

DLLEXPORT NETWORKED_TYPE Leviathan::NetworkHandler::GetNetworkType(){
	return AppType;
}
// ------------------------------------ //
DLLEXPORT shared_ptr<ConnectionInfo> Leviathan::NetworkHandler::OpenConnectionTo(const wstring &targetaddress, ObjectLock &guard){
	VerifyLock(guard);
	// Create object //
	shared_ptr<ConnectionInfo> tmpconnection(new ConnectionInfo(targetaddress));

	// Initialize the connection //
	if(!tmpconnection || !tmpconnection->Init()){
		// Failed //
		return NULL;
	}

	// If succeeded add to the automatically managed connections //
	AutoOpenedConnections.push_back(tmpconnection);

	return tmpconnection;
}

DLLEXPORT shared_ptr<ConnectionInfo> Leviathan::NetworkHandler::GetSafePointerToConnection(ConnectionInfo* unsafeptr){
	GUARD_LOCK_THIS_OBJECT();

	for(auto iter = AutoOpenedConnections.begin(); iter != AutoOpenedConnections.end(); ++iter){
		if(iter->get() == unsafeptr)
			return *iter;
	}

	return NULL;
}

DLLEXPORT shared_ptr<ConnectionInfo> Leviathan::NetworkHandler::GetOrCreatePointerToConnection(const wstring &address){
	GUARD_LOCK_THIS_OBJECT();

	for(auto iter = AutoOpenedConnections.begin(); iter != AutoOpenedConnections.end(); ++iter){
		if((*iter)->GenerateFormatedAddressString() == address)
			return *iter;
	}

	// We need to open a new connection //
	return OpenConnectionTo(address, guard);
}
// ------------------------------------ //
void Leviathan::RunGetResponseFromMaster(NetworkHandler* instance, shared_ptr<boost::promise<wstring>> resultvar){
	// Try to load master server list //

	// To reduce duplicated code and namespace pollution use a lambda thread for this //
	boost::thread DataUpdaterThread(boost::bind<void>([](NetworkHandler* instance) -> void{

		Logger::Get()->Info(L"NetworkHandler: Fetching new master server list...");
		sf::Http::Request request(Convert::WstringToString(instance->StoredMasterServerInfo.MasterListFetchPage), sf::Http::Request::Get);

		sf::Http httpserver(Convert::WstringToString(instance->StoredMasterServerInfo.MasterListFetchServer));

		sf::Http::Response response = httpserver.sendRequest(request, sf::seconds(2.f));
		
		if(instance->CloseMasterServerConnection)
			return;
		
		if(response.getStatus() == sf::Http::Response::Ok){

			// It should just be a list of master servers one on each line //
			StringIterator itr(response.getBody());

			unique_ptr<string> data;

			std::vector<unique_ptr<string>> tmplist;


			while((data = itr.GetUntilNextCharacterOrAll<string>(L'\n')) && (data->size())){

				tmplist.push_back(move(data));
			}

			// Check //
			if(tmplist.size() == 0){

				Logger::Get()->Warning(L"NetworkHandler: retrieved an empty list of master servers, not updated");
				return;
			}

			if(instance->CloseMasterServerConnection)
				return;
					
			// Update real list //
			GUARD_LOCK_OTHER_OBJECT(instance);

			instance->MasterServers.clear();
			instance->MasterServers.reserve(tmplist.size());
			for(auto iter = tmplist.begin(); iter != tmplist.end(); ++iter){

				instance->MasterServers.push_back(shared_ptr<wstring>(new wstring(Convert::StringToWstring(*(*iter)))));
			}


			// Notify successful fetch //
			auto tmpget = Logger::Get();
			if(tmpget)
				tmpget->Info(L"NetworkHandler: Successfully fetched a master server list:");
			for(auto iter = instance->MasterServers.begin(); iter != instance->MasterServers.end(); ++iter)
				if(tmpget)
				Logger::Get()->Write(L"\t> "+*(*iter).get(), false);

		} else {
			// Fail //
			Logger::Get()->Error(L"NetworkHandler: failed to update the master server list, using the old list");
		}


	}, instance));

	if(!instance->_LoadMasterServerList()){

		// We need to request a new list before we can do anything //
		Logger::Get()->Info(L"NetworkHandler: no stored list of master servers, waiting for the update to finish");
		DataUpdaterThread.join();
	}

	if(instance->CloseMasterServerConnection)
		return;


	// Try to find a master server to connect to //
	for(size_t i = 0; i < instance->MasterServers.size(); i++){

		shared_ptr<wstring> tmpaddress;

		{
			GUARD_LOCK_OTHER_OBJECT(instance);

			tmpaddress = instance->MasterServers[i];
		}

		// We might want to try to connect to localhost //
		{
			GAMECONFIGURATION_GET_VARIABLEACCESS(variables);

			bool uselocalhost = false;
			if(variables && variables->GetValueAndConvertTo<bool>(L"MasterServerForceLocalhost", uselocalhost) && uselocalhost){

				// We might want to warn about this //
				ComplainOnce::PrintWarningOnce(L"MasterServerForceLocalhostOn", L"Master server list forced to use localhost as address, might not be what you want");

				StringIterator itr(tmpaddress.get());

				itr.GetUntilNextCharacterOrAll<wstring>(L':');

				// Right now the part we don't want is retrieved //
				auto tmpres = itr.GetUntilEnd<wstring>();

				// The result now should have the ':' character and the possible port //

				tmpaddress = shared_ptr<wstring>(new wstring(L"localhost"+*tmpres.get()));
			}
		}


		if(instance->CloseMasterServerConnection)
			return;
		
		
		// Try connection //
		shared_ptr<ConnectionInfo> tmpinfo(new ConnectionInfo(*tmpaddress));

		if(!tmpinfo->Init()){

			Logger::Get()->Error(L"NetworkHandler: failed to open a connection to target");
			continue;
		}

		// Create an identification request //
		shared_ptr<NetworkRequest> inforequest(new NetworkRequest(NETWORKREQUESTTYPE_IDENTIFICATION, 1500, PACKAGE_TIMEOUT_STYLE_TIMEDMS));

		// Send and block until a request (or 5 fails) //
		shared_ptr<NetworkResponse> serverinforesponse = tmpinfo->SendRequestAndBlockUntilDone(inforequest, 5);

		if(!serverinforesponse){
			// Failed to receive something from the server //
			Logger::Get()->Warning(L"NetworkHandler: failed to receive a response from master server("+*tmpaddress+L")");
			// Release connection //
			tmpinfo->Release();
			continue;
		}

		Logger::Get()->Warning(L"NetworkHandler: received a response from master server("+*tmpaddress+L"):");

		// Get data //
		NetworkResponseDataForIdentificationString* tmpresponse = serverinforesponse->GetResponseDataForIdentificationString();

		if(!tmpresponse){
			// Failed to receive valid response //
			Logger::Get()->Warning(L"NetworkHandler: received an invalid response from master server("+*tmpaddress+L")");
			// Release connection //
			tmpinfo->Release();
			continue;
		}

		// Ensure data validness //

		// \todo verify that the data is sane //

		{
			// Set working server //
			GUARD_LOCK_OTHER_OBJECT(instance);
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
		try{
			boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		} catch(...){
			continue;
		}
	}

}




