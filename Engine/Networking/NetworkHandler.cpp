// ------------------------------------ //
#include "NetworkHandler.h"

#include "Application/GameConfiguration.h"
#include "Connection.h"
#include "FileSystem.h"
#include "GameSpecificPacketHandler.h"
#include "Iterators/StringIterator.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "SentNetworkThing.h"
#include "ObjectFiles/ObjectFile.h"
#include "ObjectFiles/ObjectFileProcessor.h"
#include "RemoteConsole.h"
#include "SFML/Network/Http.hpp"
#include "SyncedVariables.h"
#include "Threading/ThreadingManager.h"
#include "Utility/ComplainOnce.h"
#include "NetworkCache.h"
#include "Engine.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NETWORKED_TYPE ntype,
    NetworkInterface* packethandler) 
    : AppType(ntype), CloseMasterServerConnection(false)
{
    // This will assert if we were passed the wrong type
    packethandler->VerifyType(AppType);
    packethandler->SetOwner(this);
    
    if(AppType == NETWORKED_TYPE::Client){

        ClientInterface = dynamic_cast<NetworkClientInterface*>(packethandler);
        
    } else if(AppType == NETWORKED_TYPE::Server){

        ServerInterface = dynamic_cast<NetworkServerInterface*>(packethandler);
    }
    
    // Create the variable sync object //
    VariableSyncer = std::make_unique<SyncedVariables>(this, AppType, packethandler);

    _NetworkCache = std::make_unique<NetworkCache>(AppType);
    
    if(!_NetworkCache || !_NetworkCache->Init(this)){

        LOG_ERROR("NetworkHandler: failed to create NetworkCache");
        LEVIATHAN_ASSERT(0, "NetworkHandler: failed to create NetworkCache");
    }

    // Create the custom packet handler //
    _GameSpecificPacketHandler = std::make_unique<GameSpecificPacketHandler>(
        packethandler);
}

DLLEXPORT NetworkHandler::~NetworkHandler(){

    _NetworkCache.reset();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkHandler::Init(const MasterServerInformation &info){

    MasterServerMustPassIdentification = info.MasterServerIdentificationString;

    if(AppType != NETWORKED_TYPE::Master){
        // Query master server //
        QueryMasterServer(info);

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

    if(AppType == NETWORKED_TYPE::Client){
        // We can use any port we get //
        PortNumber = sf::Socket::AnyPort;

    } else if(AppType == NETWORKED_TYPE::Server){
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

        Logger::Get()->Error("NetworkHandler: Init: failed to bind to port "+
            Convert::ToString(PortNumber));
        return false;
    }

    PortNumber = _Socket.getLocalPort();

    // Set the socket as blocking //
    if(BlockingMode) {

        _Socket.setBlocking(true);

        // Run the listening thread //
        ListenerThread = std::thread(std::bind(&NetworkHandler::_RunListenerThread, this));
    } else {

        _Socket.setBlocking(false);
    }

    // Report success //
    Logger::Get()->Info("NetworkHandler: running listening socket on port "+Convert::ToString(
            _Socket.getLocalPort()));

    return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkHandler::Init(uint16_t port /*= 0*/) {

    if(_Socket.bind(port) != sf::Socket::Done) {

        LOG_ERROR("NetworkHandler: Init: failed to bind to port " +
            Convert::ToString(PortNumber));
        return false;
    }

    PortNumber = _Socket.getLocalPort();

    _Socket.setBlocking(false);

    return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::Release() {
    
    {
        GUARD_LOCK();

        CloseMasterServerConnection = true;

        // Kill master server connection //

        // Notify master server connection kill //
        if(MasterServerConnection){

            MasterServerConnection->Release();
        }

        // Close all connections //
        for(auto& connection : OpenConnections){

            connection->Release();
        }

        OpenConnections.clear();
        

    }

    if(MasterServerConnectionThread.joinable()){
        MasterServerConnectionThread.join();
        MasterServerConnection.reset();
    }
    
    _ReleaseSocket();

    if(BlockingMode){
        // This thread is blocked in infinite read //
        //ListenerThread.join();
        if(ListenerThread.joinable())
            ListenerThread.detach();
    }
}

DLLEXPORT void NetworkHandler::DisconnectInterface(){

    ServerInterface = nullptr;
    ClientInterface = nullptr;
}

void Leviathan::NetworkHandler::_ReleaseSocket(){
    // This might cause the game to hang... //

    bool blockunbind = false;

    {
        GAMECONFIGURATION_GET_VARIABLEACCESS(variables);
        variables->GetValueAndConvertTo<bool>("DisableSocketUnbind", blockunbind);
    }

    auto lock = LockSocketForUse();

    // This should do the trick //
    if(!blockunbind || !BlockingMode){
        
        _Socket.unbind();

    } else {
        Logger::Get()->Info("NetworkHandler: _ReleaseSocket: blocked unbind");
    }
}

bool Leviathan::NetworkHandler::_RunUpdateOnce(Lock &guard)
{
    sf::Packet receivedpacket;

    sf::IpAddress sender;
    unsigned short sentport;

    RemoteConsole* rcon = Engine::Get()->GetRemoteConsole();

    sf::Socket::Status status;

    while (true) {

        guard.unlock();

        {
            auto lock = LockSocketForUse();
            status = _Socket.receive(receivedpacket, sender, sentport);
        }

        guard.lock();

        if(status != sf::Socket::Done)
            break;

        // Process packet //

        // Pass to a connection //
        bool Passed = false;


        for (size_t i = 0; i < OpenConnections.size(); i++) {
            // Keep passing until somebody handles it //
            if(OpenConnections[i]->IsThisYours(sender, sentport)) {

                auto curconnection = OpenConnections[i];

                // Prevent deaclocks, TODO: make NetworkHandler only usable by the main thread
                guard.unlock();
                curconnection->HandlePacket(receivedpacket);
                guard.lock();                

                Passed = true;
                break;
            }
        }

        if(Passed)
            continue;

        shared_ptr<Connection> tmpconnect;

        // TODO: Check is it a close or a keep alive packet //

        // We might want to open a new connection to this client //
        Logger::Get()->Info("Received a new connection from " + sender.toString() + ":" +
            Convert::ToString(sentport));

        // \todo Make sure that the console won't be deleted between this and the actual check

        if(AppType != NETWORKED_TYPE::Client) {
            // Accept the connection //
            LOG_WRITE("\t> Connection accepted");

            tmpconnect = OpenConnectionTo(guard, sender, sentport);

        } else if(rcon && rcon->IsAwaitingConnections()) {

            // We might allow a remote start remote console session //
            LOG_WRITE("\t> Connection accepted for remote console receive");

            tmpconnect = OpenConnectionTo(guard, sender, sentport);

            // We need a special restriction for this connection //
            if(tmpconnect)
                tmpconnect->SetRestrictionMode(CONNECTION_RESTRICTION::ReceiveRemoteConsole);

        } else {
            // Deny the connection //
            LOG_WRITE("\t> Dropping connection due to not being a server "
                "(and not expecting anything)");
            continue;
        }

        if(!tmpconnect) {

            LOG_WRITE("\t> Failed to create connection object");
            continue;
        }

        // Try to handle the packet //
        if(!tmpconnect->IsThisYours(sender, sentport)) {
            // That's an error //
            Logger::Get()->Error("NetworkHandler: UpdateAllConnections: new connection "
                "refused to process its packet from " + sender.toString() + ":" +
                Convert::ToString(sentport));
            CloseConnection(*tmpconnect);
        } else {

            tmpconnect->HandlePacket(receivedpacket);
        }
    }

    return (status != sf::Socket::Error) && (status != sf::Socket::Disconnected);
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<std::promise<string>> Leviathan::NetworkHandler::QueryMasterServer(
    const MasterServerInformation &info) {
    // Copy the data //
    StoredMasterServerInfo = info;

    shared_ptr<std::promise<string>> resultvalue(new std::promise<string>());

    // Make sure it doesn't die instantly //
    CloseMasterServerConnection = false;

    // Run the task async //
    MasterServerConnectionThread = std::thread(RunGetResponseFromMaster, this, resultvalue);

    return resultvalue;
}

DLLEXPORT bool Leviathan::NetworkHandler::IsConnectionValid(Connection &connection) const {

    GUARD_LOCK();

    bool found = false;

    for (auto& connectioniter : OpenConnections) {

        if(connectioniter.get() == &connection) {

            found = true;
            break;
        }
    }

    if(!found)
        return false;

    return connection.IsValidForSend();
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

    NamedVars headervars;
    headervars.Add(std::make_shared<NamedVariableList>("MasterServers", vals));

    ObjectFile file(headervars);

    ObjectFileProcessor::WriteObjectFile(file, StoredMasterServerInfo.StoredListFile,
        Logger::Get());
}

bool Leviathan::NetworkHandler::_LoadMasterServerList(){

    // Skip if there is no file defined //
    if(!StoredMasterServerInfo.StoredListFile.size())
        return false;

    auto fileobj = ObjectFileProcessor::ProcessObjectFile(
        StoredMasterServerInfo.StoredListFile, Logger::Get());

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
        MasterServers.push_back(unique_ptr<string>(new std::string(
                    foundvar->GetValueDirect(i)->ConvertAndReturnVariable<std::string>())));
    }

    return true;
}

DLLEXPORT void Leviathan::NetworkHandler::ShutdownCache() {

    if(_NetworkCache)
        _NetworkCache->Release();
}

DLLEXPORT void Leviathan::NetworkHandler::ReleaseInputHandler() {

}
// ------------------------------------ //
DLLEXPORT string Leviathan::NetworkHandler::GetServerAddressPartOfAddress(
    const string &fulladdress, const string &regextouse /*= "http://.*?/"*/)
{
    // Create a regex //
    regex findaddressregex(regextouse, regex_constants::ECMAScript | regex_constants::icase);

    match_results<const char*> addressmatch;

    // Return the match //
    regex_search(fulladdress.c_str(), addressmatch, findaddressregex);


    return string(addressmatch[1]);
}

DLLEXPORT void Leviathan::NetworkHandler::_RegisterConnection(
    std::shared_ptr<Connection> connection) 
{
    // Skip duplicates //
    if(GetConnection(connection.get()))
        return;

    GUARD_LOCK();

    OpenConnections.push_back(connection);
}

// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkHandler::UpdateAllConnections(){

    // Update remote console sessions if they exist //
    auto rconsole = Engine::Get()->GetRemoteConsole();
    if(rconsole)
        rconsole->UpdateStatus();
    {
        GUARD_LOCK();

        // Remove closed connections //
        RemoveClosedConnections(guard);

        // Do listening //
        if(!BlockingMode) {

            _RunUpdateOnce(guard);
        }

        // Time-out requests //
        for (auto& connection : OpenConnections) {

            connection->UpdateListening();
        }
    }
    
    // Interface might want to do something //
    GetInterface()->TickIt();
}
// ------------------------------------ //
Lock Leviathan::NetworkHandler::LockSocketForUse(){
    
    return Lock((SocketMutex));
}
// ------------------------------------ //
DLLEXPORT void NetworkHandler::CloseConnection(Connection &to){

    Lock lock(ConnectionsToTerminateMutex);

    // Return if it is already queued //
    for(auto& connection : ConnectionsToTerminate){
        if(connection == &to)
            return;
    }

    ConnectionsToTerminate.push_back(&to);
}

DLLEXPORT void Leviathan::NetworkHandler::RemoveClosedConnections(Lock &guard)
{
    Lock terminatelock(ConnectionsToTerminateMutex);

    // Go through the removed connection list and remove closed connections //
    for (size_t a = 0; a < OpenConnections.size(); ) {

        bool close = false;

        if(!OpenConnections[a]->IsValidForSend()) {

            close = true;

        } else {

            for (size_t i = 0; i < ConnectionsToTerminate.size(); i++) {

                if(OpenConnections[a].get() == ConnectionsToTerminate[i]) {

                    close = true;
                    break;
                }
            }
        }

        if(close) {

            auto& connection = OpenConnections[a];

            // Send a close packet //
            connection->SendCloseConnectionPacket();

            // Close it //
            connection->Release();

            connection.reset();

            OpenConnections.erase(OpenConnections.begin() + a);

        } else {

            a++;
        }
    }

    ConnectionsToTerminate.clear();
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<Connection> NetworkHandler::GetConnection(Connection* directptr)
    const
{
    GUARD_LOCK();

    for(auto& connection : OpenConnections){

        if(connection.get() == directptr)
            return connection;
    }

    return nullptr;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<Connection> Leviathan::NetworkHandler::OpenConnectionTo(
    const string &targetaddress)
{
    if(targetaddress.empty())
        return nullptr;
    
    GUARD_LOCK();

    // Find existing one //
    for (auto& connection : OpenConnections) {

        if(connection->GetRawAddress() == targetaddress)
            return connection;
    }

    // Create new //
    auto newconnection = std::make_shared<Connection>(targetaddress);

    // Initialize the connection //
    if(!newconnection->Init(this)){
        
        return nullptr;
    }

    OpenConnections.push_back(newconnection);

    return newconnection;
}

DLLEXPORT std::shared_ptr<Leviathan::Connection> Leviathan::NetworkHandler::OpenConnectionTo(
    Lock &guard, const sf::IpAddress &targetaddress, unsigned short port) 
{
    // Find existing one //
    for (auto& connection : OpenConnections) {

        if(connection->IsThisYours(targetaddress, port))
            return connection;
    }

    // Create new //
    auto newconnection = std::make_shared<Connection>(targetaddress, port);

    // Initialize the connection //
    if(!newconnection->Init(this)) {

        return nullptr;
    }

    OpenConnections.push_back(newconnection);

    return newconnection;
}

// ------------------------------------ //
void Leviathan::NetworkHandler::_RunListenerThread(){

    // Let's listen for things //
    Logger::Get()->Info("NetworkHandler: running listening thread");
    {

        GUARD_LOCK();
        while (_RunUpdateOnce(guard)) {

        }
    }

    Logger::Get()->Info("NetworkHandler: listening socket thread quitting");
}
// ------------------------------------ //
void Leviathan::RunGetResponseFromMaster(NetworkHandler* instance,
    std::shared_ptr<std::promise<string>> resultvar)
{
    // Try to load master server list //

    // To reduce duplicated code and namespace pollution use a lambda thread for this //
    std::thread DataUpdaterThread(std::bind<void>([](NetworkHandler* instance)
        -> void
    {

        Logger::Get()->Info("NetworkHandler: Fetching new master server list...");
        sf::Http::Request request(instance->StoredMasterServerInfo.MasterListFetchPage,
            sf::Http::Request::Get);

        sf::Http httpserver(instance->StoredMasterServerInfo.MasterListFetchServer);

        sf::Http::Response response = httpserver.sendRequest(request, sf::seconds(2.f));

        if(instance->CloseMasterServerConnection)
            return;

        if(response.getStatus() == sf::Http::Response::Ok) {

            // It should just be a list of master servers one on each line //
            StringIterator itr(response.getBody());

            unique_ptr<string> data;

            std::vector<unique_ptr<string>> tmplist;


            while ((data = itr.GetUntilNextCharacterOrAll<string>(L'\n')) && (data->size())) {

                tmplist.push_back(move(data));
            }

            // Check //
            if(tmplist.size() == 0) {

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
            for (auto iter = tmplist.begin(); iter != tmplist.end(); ++iter) {

                instance->MasterServers.push_back(shared_ptr<string>(new string((*(*iter)))));
            }


            // Notify successful fetch //
            auto tmpget = Logger::Get();
            if(tmpget)
                tmpget->Info("NetworkHandler: Successfully fetched a master server list:");

            for (auto iter = instance->MasterServers.begin();
                iter != instance->MasterServers.end();
                ++iter)
            {
                if(tmpget)
                    Logger::Get()->Write("\t> " + *(*iter).get());
            }

        } else {
            // Fail //
            Logger::Get()->Error("NetworkHandler: failed to update the master server list, "
                "using the old list");
        }


    }, instance));

    if(!instance->_LoadMasterServerList()){

        // We need to request a new list before we can do anything //
        Logger::Get()->Info("NetworkHandler: no stored list of master servers, "
            "waiting for the update to finish");

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
    for (size_t i = 0; i < instance->MasterServers.size(); i++) {

        shared_ptr<string> tmpaddress;

        {
            GUARD_LOCK_OTHER(instance);

            tmpaddress = instance->MasterServers[i];
        }

        if(uselocalhost) {

            // We might want to warn about this //
            ComplainOnce::PrintWarningOnce("MasterServerForceLocalhostOn",
                "Master server list forced to use localhost as address, "
                "might not be what you want");

            StringIterator itr(tmpaddress.get());

            itr.GetUntilNextCharacterOrAll<string>(':');

            // Right now the part we don't want is retrieved //
            auto tmpres = itr.GetUntilEnd<string>();

            if(!tmpres){

                // Got nothing //
                LOG_WARNING("GetMasterServerList: got an empty response");
                continue;
            }

            // The result now should have the ':' character and the possible port //

            tmpaddress = std::shared_ptr<string>(new string("localhost" + *tmpres.get()));
        }

        if(instance->CloseMasterServerConnection)
            return;

        // Try connection //
        auto tmpinfo = instance->OpenConnectionTo(*tmpaddress);

        if(!tmpinfo) {

            Logger::Get()->Error("NetworkHandler: failed to open a connection to target");
            continue;
        }

        // Create an identification request //
        auto serverinforesponse = tmpinfo->SendPacketToConnection(
            std::make_shared<RequestIdentification>(),
                RECEIVE_GUARANTEE::Critical);

        // a task that'll wait for the response //
        serverinforesponse->SetCallbackFunc(
            [=](bool succeeded, SentNetworkThing &sentthing) -> void {
                
                // Report this for easier debugging //
                Logger::Get()->Info("Master server check with \"" + *tmpaddress +
                    "\" completed");
                    
                // Quit if the instance is gone //
                if(instance->CloseMasterServerConnection)
                    return;
                    
                auto actualresponse = serverinforesponse->GotResponse;
                

                if(!actualresponse) {
                    // Failed to receive something from the server //
                    Logger::Get()->Warning("NetworkHandler: failed to receive a "
                        "response from master server(" + *tmpaddress + ")");
                        
                    // Release connection //
                    tmpinfo->Release();
                    return;
                }
                    
                Logger::Get()->Warning("NetworkHandler: received a response from "
                    "master server(" + *tmpaddress + "):");
            
                // Get data //
                ResponseIdentification* tmpresponse = static_cast<ResponseIdentification*>(
                    actualresponse.get());

                if(actualresponse->GetType() != NETWORK_RESPONSE_TYPE::Identification) {
                    // Failed to receive valid response //
                    Logger::Get()->Warning("NetworkHandler: received an invalid response "
                        "from master server(" + *tmpaddress + ")");
                    // Release connection //
                    tmpinfo->Release();
                    return;
                }

                // Ensure data validness //

                // \todo verify that the data is sane //

                {
                    // Set a working server //
                    GUARD_LOCK_OTHER(instance);
                    instance->MasterServerConnection = tmpinfo;
                }

                // Successfully connected //
                resultvar->set_value(string("ConnectedToMasterServer = " + *tmpaddress + ";"));
            });
    }

    // TODO: run this only when all the tasks fail
    if(false){
        // If we got here, we haven't connected to anything //
        Logger::Get()->Warning("NetworkHandler: could not connect to any fetched master "
            "servers, you can restart to use a new list");
        
        // We can let whoever is waiting for us to go now, and finish some utility
        // tasks after that
        resultvar->set_value(std::string("Failed to connect to master server"));
    }

    // This needs to be done here //
    if(DataUpdaterThread.joinable())
        DataUpdaterThread.join();

    // Output the current list //
    instance->_SaveMasterServerList();
}


