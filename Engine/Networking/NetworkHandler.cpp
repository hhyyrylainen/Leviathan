// ------------------------------------ //
#include "NetworkHandler.h"

#include "Application/GameConfiguration.h"
#include "ConnectionInfo.h"
#include "FileSystem.h"
#include "GameSpecificPacketHandler.h"
#include "Iterators/StringIterator.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "ObjectFiles/ObjectFile.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "RemoteConsole.h"
#include "SFML/Network/Http.hpp"
#include "SyncedVariables.h"
#include "Threading/ThreadingManager.h"
#include "Utility/ComplainOnce.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NETWORKED_TYPE ntype, NetworkInterface* packethandler) 
	: AppType(ntype), CloseMasterServerConnection(false), UpdaterThreadStop(false)
{
	instance = this;
	interfaceinstance = packethandler;

	// Set our type to the NetworkInteface //
    if(interfaceinstance)
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

    GUARD_LOCK();

	MasterServerMustPassIdentification = info.MasterServerIdentificationString;

	if(AppType != NETWORKED_TYPE_MASTER){
		// Query master server //
		QueryMasterServer(guard, info);

	} else {
		// We are our own master! //

		// Get out port number here //
		GAMECONFIGURATION_GET_VARIABLEACCESS(vars);

		int tmpport = 0;

		if(!vars->GetValueAndConvertTo<int>("MasterServerPort", tmpport)){
			// This is quite bad //
			Logger::Get()->Error("NetworkHandler: Init: no port configured, config missing "
                "'MasterServerPort' of type int");
		}

		PortNumber = (unsigned short)tmpport;
	}

	if(AppType == NETWORKED_TYPE_CLIENT){
		// We can use any port we get //
		PortNumber = sf::Socket::AnyPort;

	} else if(AppType == NETWORKED_TYPE_SERVER){
		// We need to use a specific port //
		GAMECONFIGURATION_GET_VARIABLEACCESS(vars);

		int tmpport = 0;

		if(!vars->GetValueAndConvertTo<int>("DefaultServerPort", tmpport)){
			// This is quite bad //
			Logger::Get()->Error("NetworkHandler: Init: no port configured, config missing "
                "'ServerPort' of type int");
		}

		PortNumber = (unsigned short)tmpport;
	}

	// We want to receive responses //
	if(_Socket.bind(PortNumber) != sf::Socket::Done){

		Logger::Get()->Error("NetworkHandler: Init: failed to bind to a port "+
            Convert::ToString(PortNumber));
		return false;
	}

	// Set the socket as blocking //
	_Socket.setBlocking(true);

    // Run the listening thread //
    ListenerThread = std::thread(std::bind(&NetworkHandler::_RunListenerThread, this));

	// Report success //
	Logger::Get()->Info("NetworkHandler: running listening socket on port "+Convert::ToString(
            _Socket.getLocalPort()));

    // Run temporary update thread //
    TemporaryUpdateThread = std::thread(std::bind(&NetworkHandler::_RunTemporaryUpdaterThread,
            this));

	return true;
}

DLLEXPORT void Leviathan::NetworkHandler::Release(){
    
    {
        GUARD_LOCK();

        CloseMasterServerConnection = true;

        // Kill master server connection //

        // Notify master server connection kill //
        if(MasterServerConnection){

            MasterServerConnection->Release();
        }

        {
            {
                Lock lock(AutoOpenedConnectionsMutex);
        
                // Close all connections //
                for(size_t i = 0; i < AutoOpenedConnections.size(); i++){

                    AutoOpenedConnections[i]->Release();
                }

                AutoOpenedConnections.clear();
            }

            MasterServerConnectionThread.join();
            MasterServerConnection.reset();
        }
    
        // This might have been left on by accident
        StopOwnUpdaterThread(guard);

        // Set the static instance to nothing //
        instance = NULL;

    }
    
    _ReleaseSocket();

    // This thread is blocked in infinite read //
    //ListenerThread.join();
    ListenerThread.detach();
}

void Leviathan::NetworkHandler::_ReleaseSocket(){
	// This might cause the game to hang... //

	bool blockunbind = false;

	{
		GAMECONFIGURATION_GET_VARIABLEACCESS(variables);
		variables->GetValueAndConvertTo<bool>("DisableSocketUnbind", blockunbind);
	}

    auto lock = std::move(LockSocketForUse());

	// This should do the trick //
	if(!blockunbind){
        
		_Socket.unbind();

	} else {
		Logger::Get()->Info("NetworkHandler: _ReleaseSocket: blocked unbind");
	}
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<std::promise<string>> Leviathan::NetworkHandler::QueryMasterServer(
    Lock &guard, const MasterServerInformation &info)
{
	// Copy the data //
	StoredMasterServerInfo = info;

	shared_ptr<std::promise<string>> resultvalue(new std::promise<string>());

	// Make sure it doesn't die instantly //
	CloseMasterServerConnection = false;

	// Run the task async //
	MasterServerConnectionThread = std::thread(RunGetResponseFromMaster, this, resultvalue);

	return resultvalue;
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_SaveMasterServerList(){

	// Set up the values //
	vector<VariableBlock*> vals;
    {
        vals.reserve(MasterServers.size());

		// Only this scope requires locking //
		GUARD_LOCK();

		for(size_t i = 0; i < MasterServers.size(); i++){
			vals.push_back(new VariableBlock(*MasterServers[i].get()));
		}
	}

	NamedVars headervars(new NamedVariableList("MasterServers", vals));

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
	auto foundvar = fileobj->GetVariables()->GetValueDirectRaw("MasterServers");

	if(!foundvar)
		return false;

	// Found value, load //
	size_t maxval = foundvar->GetVariableCount();

	// We need locking for this add //
	GUARD_LOCK();

	MasterServers.reserve(maxval);

	for(size_t i = 0; i < maxval; i++){
		MasterServers.push_back(unique_ptr<string>(new string(
                    foundvar->GetValueDirect(i)->ConvertAndReturnVariable<string>())));
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT string Leviathan::NetworkHandler::GetServerAddressPartOfAddress(const string &fulladdress,
    const string &regextouse /*= "http://.*?/"*/)
{
	// Create a regex //
	regex findaddressregex(regextouse, regex_constants::ECMAScript | regex_constants::icase);

	match_results<const char*> addressmatch;

	// Return the match //
	regex_search(fulladdress.c_str(), addressmatch, findaddressregex);


	return string(addressmatch[1]);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::UpdateAllConnections(){

    // Remove closed connections //
    RemoveClosedConnections();

    // Update remote console sessions if they exist //
    auto rconsole = RemoteConsole::Get();
    if(rconsole)
        rconsole->UpdateStatus();
        

    {
        Lock lock(ConnectionsToUpdateMutex);
        
        // Time-out requests //
        for(size_t i = 0; i < ConnectionsToUpdate.size(); i++){

            auto connection = ConnectionsToUpdate[i];

            GUARD_LOCK_OTHER(connection);

            connection->UpdateListening(guard);
        }
    }

	// Interface might want to do something //
	interfaceinstance->TickIt();
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_RegisterConnectionInfo(ConnectionInfo* tomanage){

    Lock lock(ConnectionsToUpdateMutex);
	ConnectionsToUpdate.push_back(tomanage);
}

void Leviathan::NetworkHandler::_UnregisterConnectionInfo(ConnectionInfo* unregisterme){

    Lock lock(ConnectionsToUpdateMutex);
    
	for(auto iter = ConnectionsToUpdate.begin(); iter != ConnectionsToUpdate.end(); ++iter){
		if((*iter) == unregisterme){

			ConnectionsToUpdate.erase(iter);
			return;
		}
	}
}

Lock Leviathan::NetworkHandler::LockSocketForUse(){
    
	return std::move(Lock((SocketMutex)));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::SafelyCloseConnectionTo(ConnectionInfo* to){

    Lock lock(ConnectionsToTerminateMutex);

    // Return if it is already queued //
	for(auto& connection : ConnectionsToTerminate){
		if(connection == to)
			return;
	}

	ConnectionsToTerminate.push_back(to);
}

DLLEXPORT void Leviathan::NetworkHandler::RemoveClosedConnections(){

    Lock terminatelock(ConnectionsToTerminateMutex);

	if(ConnectionsToTerminate.empty())
		return;

	// Go through the removed connection list and remove them //
	for(size_t i = 0; i < ConnectionsToTerminate.size(); i++){

        auto connection = ConnectionsToTerminate[i];

        ConnectionsToTerminate.erase(ConnectionsToTerminate.begin()+i);

        terminatelock.unlock();
        
		// Send a close packet //
		connection->SendCloseConnectionPacket();

		// Close it //
        // The connection will automatically remove itself from the vector //
		connection->Release();

        {
            Lock openedlock(AutoOpenedConnectionsMutex);
        
            // But if we have opened it we need to delete our pointer //
            auto end = AutoOpenedConnections.end();
            for(auto iter = AutoOpenedConnections.begin(); iter != end; ++iter){
            
                if((*iter).get() == connection){
                
                    AutoOpenedConnections.erase(iter);
                    break;
                }
            }
        }

        terminatelock.lock();
	}
}
// ------------------------------------ //
DLLEXPORT unsigned short Leviathan::NetworkHandler::GetOurPort(){
	auto lock = std::move(LockSocketForUse());
	return _Socket.getLocalPort();
}

DLLEXPORT NETWORKED_TYPE Leviathan::NetworkHandler::GetNetworkType() const{
	return AppType;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<ConnectionInfo> Leviathan::NetworkHandler::OpenConnectionTo(const string &targetaddress)
{
	// Create object //
	shared_ptr<ConnectionInfo> tmpconnection(new ConnectionInfo(targetaddress));

	// Initialize the connection //
	if(!tmpconnection || !tmpconnection->Init()){
        
		return NULL;
	}

    Lock autolock(AutoOpenedConnectionsMutex);
    
	// If succeeded add to the automatically managed connections //
	AutoOpenedConnections.push_back(tmpconnection);

	return tmpconnection;
}

DLLEXPORT std::shared_ptr<ConnectionInfo> Leviathan::NetworkHandler::GetSafePointerToConnection(ConnectionInfo* unsafeptr){

    Lock autolock(AutoOpenedConnectionsMutex);
    
	for(auto iter = AutoOpenedConnections.begin(); iter != AutoOpenedConnections.end(); ++iter){
		if(iter->get() == unsafeptr)
			return *iter;
	}

	return NULL;
}

DLLEXPORT std::shared_ptr<ConnectionInfo> Leviathan::NetworkHandler::GetOrCreatePointerToConnection(const string &address){

    {
        Lock autolock(AutoOpenedConnectionsMutex);
    
        for(auto iter = AutoOpenedConnections.begin(); iter != AutoOpenedConnections.end(); ++iter){
            if((*iter)->GenerateFormatedAddressString() == address)
                return *iter;
        }

    }

	// We need to open a new connection //
	return OpenConnectionTo(address);
}
// ------------------------------------ //
void Leviathan::NetworkHandler::_RunListenerThread(){

    // Let's listen for things //
	sf::Packet receivedpacket;

	sf::IpAddress sender;
	unsigned short sentport;

    Logger::Get()->Info("NetworkHandler: running listening thread");

	// Loop through all received packets //
	while(_Socket.receive(receivedpacket, sender, sentport) == sf::Socket::Done){

        // Quit if no longer valid //
        if(Get() != this)
            break;
        
		// Process packet //
        
		// Pass to a connection //
		bool Passed = false;

        Lock connectionlock(ConnectionsToUpdateMutex);
        
		for(size_t i = 0; i < ConnectionsToUpdate.size(); i++){
			// Keep passing until somebody handles it //
			if(ConnectionsToUpdate[i]->IsThisYours(sender, sentport)){

                auto curconnection = ConnectionsToUpdate[i];
                
                // Handle it //
                connectionlock.unlock();
                
                curconnection->HandlePacket(receivedpacket);
                
                connectionlock.lock();
                
				Passed = true;
				break;
			}
		}

		if(Passed)
			continue;

		shared_ptr<ConnectionInfo> tmpconnect;

		// \todo Check is it a close or a keepalive packet //



		// We might want to open a new connection to this client //
		Logger::Get()->Info("Received a new connection from "+sender.toString()+":"+Convert::ToString(sentport));

        // \todo Make sure that the console won't be deleted between this and the actual check
        RemoteConsole* rcon = RemoteConsole::Get();

		if(AppType != NETWORKED_TYPE_CLIENT){
			// Accept the connection //
			Logger::Get()->Info("\t> Connection accepted");

			tmpconnect = std::shared_ptr<ConnectionInfo>(new ConnectionInfo(sender, sentport));

		} else if(rcon && rcon->IsAwaitingConnections()){
            
			// We might allow a remote start remote console session //
			Logger::Get()->Info("\t> Connection accepted for remote console receive");

			tmpconnect = std::shared_ptr<ConnectionInfo>(new ConnectionInfo(sender, sentport));
			// We need a special restriction for this connection //
			tmpconnect->SetRestrictionMode(CONNECTION_RESTRICTION_RECEIVEREMOTECONSOLE);

		} else {
			// Deny the connection //
			Logger::Get()->Info("\t> Dropping connection due to not being a server (and not expecting anything)");

		}

		if(tmpconnect){

            // This doesn't need relocking as it will be recreated next loop //
            connectionlock.unlock();
            
			// Try to handle with the new connection //
			// We need to initialize the new connection first //
			if(!tmpconnect->Init()){
				// This should never happen //
				assert(0 && "connection init function should never fail");
			}

            {
                Lock autolock(AutoOpenedConnectionsMutex);
            
                AutoOpenedConnections.push_back(tmpconnect);

            }

			// Try to handle the packet //
			if(!tmpconnect->IsThisYours(sender, sentport)){
				// That's an error //
				Logger::Get()->Error("NetworkHandler: UpdateAllConnections: new connection refused to process "
                    "its packet from "+sender.toString()+":"+Convert::ToString(sentport));
			} else {

                tmpconnect->HandlePacket(receivedpacket);
            }
		}
	}

    Logger::Get()->Info("NetworkHandler: listening socket thread quitting");
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::StopOwnUpdaterThread(Lock &guard){
    
    // Tell the thread to stop //
    UpdaterThreadStop = true;

    // Make it stop waiting for faster quitting //
    NotifyTemporaryUpdater.notify_all();

    // Wait for the thread to die //
    guard.unlock();
    
    // TODO: check that this doesn't deadlock //
    if(TemporaryUpdateThread.joinable())
        TemporaryUpdateThread.join();

    guard.lock();
}

void Leviathan::NetworkHandler::_RunTemporaryUpdaterThread(){

    GUARD_LOCK_NAME(lockit);

    // Run until told to stop and update connections when enough time has elapsed //
    while(!UpdaterThreadStop){

        lockit.unlock();

        // This whole object doesn't need to be locked in here //
        UpdateAllConnections();

        lockit.lock();
        
        NotifyTemporaryUpdater.wait_for(lockit, std::chrono::milliseconds(50));
    }
}
// ------------------------------------ //
void Leviathan::RunGetResponseFromMaster(NetworkHandler* instance,
    std::shared_ptr<std::promise<string>> resultvar)
{
	// Try to load master server list //

	// To reduce duplicated code and namespace pollution use a lambda thread for this //
	std::thread DataUpdaterThread(std::bind<void>([](NetworkHandler* instance) -> void{

		Logger::Get()->Info("NetworkHandler: Fetching new master server list...");
		sf::Http::Request request(instance->StoredMasterServerInfo.MasterListFetchPage,
            sf::Http::Request::Get);

		sf::Http httpserver(instance->StoredMasterServerInfo.MasterListFetchServer);

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

				Logger::Get()->Warning("NetworkHandler: retrieved an empty list of master "
                    "servers, not updated");
				return;
			}

			if(instance->CloseMasterServerConnection)
				return;

			// Update real list //
			GUARD_LOCK_OTHER(instance);

			instance->MasterServers.clear();
			instance->MasterServers.reserve(tmplist.size());
			for(auto iter = tmplist.begin(); iter != tmplist.end(); ++iter){

				instance->MasterServers.push_back(shared_ptr<string>(new string((*(*iter)))));
			}


			// Notify successful fetch //
			auto tmpget = Logger::Get();
			if(tmpget)
				tmpget->Info("NetworkHandler: Successfully fetched a master server list:");
			for(auto iter = instance->MasterServers.begin(); iter != instance->MasterServers.end();
                ++iter)
            {
				if(tmpget)
				Logger::Get()->Write("\t> "+*(*iter).get());
            }

		} else {
			// Fail //
			Logger::Get()->Error("NetworkHandler: failed to update the master server list, using the old list");
		}


	}, instance));

	if(!instance->_LoadMasterServerList()){

		// We need to request a new list before we can do anything //
		Logger::Get()->Info("NetworkHandler: no stored list of master servers, waiting for the update to finish");

        if(!DataUpdaterThread.joinable())
            throw Exception("This should be joinable");
		DataUpdaterThread.join();
	}

	if(instance->CloseMasterServerConnection)
		return;

	bool uselocalhost = false;

	// We might want to try to connect to localhost //
	{
		GAMECONFIGURATION_GET_VARIABLEACCESS(variables);

		if(variables)
			variables->GetValueAndConvertTo<bool>("MasterServerForceLocalhost", uselocalhost);
	}

	// Try to find a master server to connect to //
	for(size_t i = 0; i < instance->MasterServers.size(); i++){

		shared_ptr<string> tmpaddress;

		{
			GUARD_LOCK_OTHER(instance);

			tmpaddress = instance->MasterServers[i];
		}

		if(uselocalhost){

			// We might want to warn about this //
			ComplainOnce::PrintWarningOnce("MasterServerForceLocalhostOn",
                "Master server list forced to use localhost as address, might not be what you want");

			StringIterator itr(tmpaddress.get());

			itr.GetUntilNextCharacterOrAll<string>(L':');

			// Right now the part we don't want is retrieved //
			auto tmpres = itr.GetUntilEnd<string>();

			// The result now should have the ':' character and the possible port //

			tmpaddress = std::shared_ptr<string>(new string("localhost"+*tmpres.get()));
		}

		if(instance->CloseMasterServerConnection)
			return;

		// Try connection //
		shared_ptr<ConnectionInfo> tmpinfo(new ConnectionInfo(*tmpaddress));

		if(!tmpinfo->Init()){

			Logger::Get()->Error("NetworkHandler: failed to open a connection to target");
			continue;
		}

		// Create an identification request //
		shared_ptr<NetworkRequest> inforequest(new NetworkRequest(NETWORKREQUESTTYPE_IDENTIFICATION, 1500,
                PACKET_TIMEOUT_STYLE_TIMEDMS));

		// Send the request and create a task that'll wait for the response //
		shared_ptr<SentNetworkThing> serverinforesponse = tmpinfo->SendPacketToConnection(inforequest, 5);


        std::shared_ptr<std::promise<bool>> sendpromise(new std::promise<bool>());


		// Now we'll just wait until it is done //
		ThreadingManager::Get()->QueueTask(new ConditionalTask(std::bind<void>([](
                        std::shared_ptr<SentNetworkThing> response, NetworkHandler* instance,
                        std::shared_ptr<std::promise<bool>> result, std::shared_ptr<ConnectionInfo> currentconnection,
                        std::shared_ptr<string> tmpaddress, std::shared_ptr<std::promise<string>> resultvar) -> void
				{
					// Report this for easier debugging //
					Logger::Get()->Info("Master server check with \""+*tmpaddress+
										"\" completed");

					// Quit if the instance is gone //
					if(instance->CloseMasterServerConnection)
						return;
					
					auto actualresponse = response->GotResponse;
					
					
					if(!actualresponse){
						// Failed to receive something from the server //
						Logger::Get()->Warning("NetworkHandler: failed to receive a response from master server("+
                            *tmpaddress+")");
						// Release connection //
						currentconnection->Release();
						return;
					}
					
					Logger::Get()->Warning("NetworkHandler: received a response from master server("+*tmpaddress+
                        "):");
					
					// Get data //
					
					NetworkResponseDataForIdentificationString* tmpresponse =
                        actualresponse->GetResponseDataForIdentificationString();
					
					if(!tmpresponse){
						// Failed to receive valid response //
						Logger::Get()->Warning("NetworkHandler: received an invalid response from master server("+
                            *tmpaddress+")");
						// Release connection //
						currentconnection->Release();
						return;
					}
					
					// Ensure data validness //
					
					// \todo verify that the data is sane //
					
					{
						// Set a working server //
						GUARD_LOCK_OTHER(instance);
						instance->MasterServerConnection = currentconnection;
					}
						
					// Successfully connected //
					resultvar->set_value(string("ConnectedToMasterServer = "+*tmpaddress+";"));
					
						
					// Tell all our friends of the result //
					result->set_value(true);
						
						
				}, serverinforesponse, instance, sendpromise, tmpinfo, tmpaddress, resultvar),
                std::bind<bool>([](std::shared_ptr<SentNetworkThing> response) -> bool
					{
						
						return response->IsFinalized();
					}, serverinforesponse)));
	}
	

	// TODO: run this only when all the tasks fail
	if(false){
		// If we got here, we haven't connected to anything //
		Logger::Get()->Warning("NetworkHandler: could not connect to any fetched master servers, "
            "you can restart to use a new list");
		
		// We can let whoever is waiting for us to go now, and finish some utility
        // tasks after that
		resultvar->set_value(string("Failed to connect to master server"));
	}


	// This needs to be done here //
    if(DataUpdaterThread.joinable())
        DataUpdaterThread.join();

	// Output the current list //
	instance->_SaveMasterServerList();
}


