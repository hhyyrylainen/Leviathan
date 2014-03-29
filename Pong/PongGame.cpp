#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_GAME
#include "PongGame.h"
#endif
#include "add_on/autowrapper/aswrappedcall.h"
#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkClientInterface.h"
using namespace Pong;
using namespace Leviathan;
// ------------------------------------ //
// Put this here, since nowhere else to put it //
BasePongParts* Pong::BasepongStaticAccess = NULL;

Pong::PongGame::PongGame() : GuiManagerAccess(NULL)
#ifdef _WIN32
	, ServerProcessHandle(NULL)
#endif // _WIN32

{
	GameInputHandler = new GameInputController();
	StaticGame = this;
}

Pong::PongGame::~PongGame(){
	GUARD_LOCK_THIS_OBJECT();
	// delete memory //
	SAFE_DELETE(GameInputHandler);
	
	// Wait for the child process to die and close the handle //
#ifdef _WIN32
	// Wait for the server to close for 5 seconds //
#ifndef _DEBUG
	WaitForSingleObject(ServerProcessHandle, 5000);

	TerminateProcess(ServerProcessHandle, -1);
#endif

	CloseHandle(ServerProcessHandle);
	ServerProcessHandle = NULL;
#else


#endif // _WIN32

}

PongGame* Pong::PongGame::Get(){
	return StaticGame;
}

PongGame* Pong::PongGame::StaticGame = NULL;
// ------------------------------------ //
std::wstring Pong::PongGame::GenerateWindowTitle(){
	return wstring(L"Pong version " GAME_VERSIONS L" Leviathan " LEVIATHAN_VERSIONS);
}
// ------------------------------------ //
// This avoids using pointer to pointer (pointer/reference to shared_ptr) //
struct TmpPassTaskObject{
	shared_ptr<Leviathan::SentNetworkThing> PossibleRequest;
	boost::unique_future<bool> WaitForIt;
};

int Pong::PongGame::StartServer(){
	// Start the server process //
	GUARD_LOCK_THIS_OBJECT();

#ifdef _DEBUG
	wstring serverstartname = L"PongServerD";
#else
	wstring serverstartname = L"PongServer";
#endif // _DEBUG

#ifdef _WIN32
	serverstartname += L".exe";
#else
	// Linux type start //
	serverstartname = L"./"+serverstartname;
#endif // _WIN32

	int Tokennmbr = Random::Get()->GetNumber();

	// Create proper arguments for the program //
	wstring args = L"--nogui -RemoteConsole:CloseIfNone -RemoteConsole:OpenTo:\"localhost:"+
		Convert::ToWstring(NetworkHandler::Get()->GetOurPort())+L"\":Token:"+Convert::ToWstring(Tokennmbr);

	// We need to expect this connection //
	RemoteConsole::Get()->ExpectNewConnection(Tokennmbr, L"ServerConsole", true);

	// Start the server //
#ifdef _WIN32
	// Create needed info //
	STARTUPINFOW processstart;
	PROCESS_INFORMATION startedinfo;

	ZeroMemory(&processstart, sizeof(STARTUPINFOW));
	ZeroMemory(&startedinfo, sizeof(PROCESS_INFORMATION));

	processstart.cb = sizeof(STARTUPINFOW);
	//processstart.dwFlags = STARTF_FORCEOFFFEEDBACK;
	//processstart.wShowWindow = SW_SHOWMINIMIZED;

	wstring finalstart = L"\""+serverstartname+L"\" "+args;

	// Use windows process creation //
	//if(!CreateProcessW(serverstartname.c_str(), const_cast<wchar_t*>(args.c_str()), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &processstart, &startedinfo)){
	if(!CreateProcessW(NULL, const_cast<wchar_t*>(finalstart.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &processstart, &startedinfo)){
		// Failed to start the process
		Logger::Get()->Error(L"Failed to start the server process, error code: "+Convert::ToWstring(GetLastError()));
		return -1;
	}

	// Close our handles //
	CloseHandle(startedinfo.hThread);
	ServerProcessHandle = startedinfo.hProcess;


#else
	// Popen should work //

	// Copy data away //
	string* childprocessname = new string(Convert::WstringToString(serverstartname));
	string* childargumentlist = new string(Convert::WstringToString(args));

	// Actually fork might be simpler //
	if(fork() == 0){
		// We are now in the child process //

		string tmpprocessname = *childprocessname;
		delete childprocessname;
		string allpacketargs = *childargumentlist;
		delete childargumentlist;

		execl(tmpprocessname.c_str(), allpacketargs.c_str(), (char*) NULL);
	}


#endif // _WIN32

	// Queue a task for this //

	_Engine->GetThreadingManager()->QueueTask(shared_ptr<Leviathan::QueuedTask>(new Leviathan::ConditionalTask(
		boost::bind<void>([](Leviathan::RemoteConsole* justforperformance) -> void
	{
		// No more waiting connections, see what happened //
		if(justforperformance->GetActiveConnectionCount() == 0){
			// Failed //
			Logger::Get()->Error(L"Failed to receive a remote connection from the server");
			EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
				new NamedVariableList(L"Message", new VariableBlock(string("The server failed to start properly")))))));
			StaticGame->Disconnect("Server failed to start properly");

			// Kill the server //
#ifdef _WIN32
			TerminateProcess(StaticGame->ServerProcessHandle, -1);

#endif // _WIN32

			GUARD_LOCK_OTHER_OBJECT(StaticGame);

#ifdef WIN32
			CloseHandle(StaticGame->ServerProcessHandle);
			StaticGame->ServerProcessHandle = NULL;
#endif // WIN32


			return;
		}

		Leviathan::ConnectionInfo* tmpconnection = RemoteConsole::Get()->GetUnsafeConnectionForRemoteConsoleSession(L"ServerConsole");

		Logger::Get()->Info(L"Server started successfully");
		EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
			new NamedVariableList(L"Message", new VariableBlock(string("Server started, awaiting proper startup")))))));

		// Add repeating timed task that checks if the server is up and properly running //
		Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<Leviathan::QueuedTask>(new Leviathan::RepeatingDelayedTask(boost::bind<void>(
			[](Leviathan::ConnectionInfo* unsafeptr, shared_ptr<TmpPassTaskObject> taskdata) -> void
		{
			// Get a safe pointer //
			auto safeptr = NetworkHandler::Get()->GetSafePointerToConnection(unsafeptr);

			if(!safeptr){
				// Destroy the attempt //
				auto threadspecific = TaskThread::GetThreadSpecificThreadObject();
				auto taskobject = threadspecific->QuickTaskAccess;
				auto tmpptr = dynamic_cast<RepeatingDelayedTask*>(taskobject.get());
				assert(tmpptr != NULL && "this is not what I wanted, passed wrong task object to task");

				tmpptr->SetRepeatStatus(false);

				// Queue disconnect //
				EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
					new NamedVariableList(L"Message", new VariableBlock(string("Server connection closed unexpectedly")))))));

				return;
			}

			// Send state request //
			if(!taskdata->PossibleRequest){
				// Send a new request //

				shared_ptr<Leviathan::NetworkRequest> tmprequest(new NetworkRequest(NETWORKREQUESTTYPE_SERVERSTATUS));

				taskdata->PossibleRequest = safeptr->SendPacketToConnection(tmprequest, 2);

				taskdata->WaitForIt = taskdata->PossibleRequest->WaitForMe->get_future();
				return;
			}

			// Check if the request is ready //
			if(taskdata->WaitForIt.has_value()){

				auto response = taskdata->PossibleRequest->GotResponse;

				if(response){
					// Check what we got //
					if(response->GetTypeOfResponse() == NETWORKRESPONSETYPE_SERVERSTATUS){

						auto responsedata = response->GetResponseDataForServerStatus();

						if(responsedata){
							// Check is it joinable //
							if(responsedata->Joinable){
								// Stop repeating the task //
								auto threadspecific = TaskThread::GetThreadSpecificThreadObject();
								auto taskobject = threadspecific->QuickTaskAccess;
								auto tmpptr = dynamic_cast<RepeatingDelayedTask*>(taskobject.get());
								assert(tmpptr != NULL && "this is not what I wanted, passed wrong task object to task");

								tmpptr->SetRepeatStatus(false);

								EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
									new NamedVariableList(L"Message", new VariableBlock(string("Attempting to connect in 1 second...")))))));


								// Queue a connect to the server //
								Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<Leviathan::QueuedTask>(new Leviathan::DelayedTask(
									boost::bind(&PongGame::Connect, PongGame::Get(), wstring(safeptr->GenerateFormatedAddressString())), MillisecondDuration(1000))));

							} else {
								EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
									new NamedVariableList(L"Message", new VariableBlock(string("Server still starting")))))));
							}

						} else {
							EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
								new NamedVariableList(L"Message", new VariableBlock(string("Invalid packet!")))))));
						}
					}

				} else {

					EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
						new NamedVariableList(L"Message", new VariableBlock(string("Request to server timed out, resending...")))))));
				}

				// Reset the sent request to resend it //
				taskdata->PossibleRequest.reset();

				return;
			}

			// We are waiting for the request //
			EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
				new NamedVariableList(L"Message", new VariableBlock(string("Waiting for the server to respond to our status request")))))));


		}, tmpconnection, shared_ptr<TmpPassTaskObject>(new TmpPassTaskObject())), MillisecondDuration(50))));


	}, Leviathan::RemoteConsole::Get()), boost::bind<bool>([](Leviathan::RemoteConsole* justforperformance) -> bool{
		
		// Check if all remote console connections are timed out / connected // 
		return !justforperformance->IsAwaitingConnections();
		
	}, Leviathan::RemoteConsole::Get()))));




	// succeeded //
	return 1;
}
// ------------------------------------ //
void Pong::PongGame::AllowPauseMenu(){
	// Allow pause menu //
	GuiManagerAccess->SetCollectionAllowEnableState(L"PauseMenu", true);
}

void Pong::PongGame::CustomizedGameEnd(){
	GUARD_LOCK_THIS_OBJECT();
	// Stop sending pointless input //
	GameInputHandler->UnlinkPlayers();
	GameInputHandler->SetBlockState(true);

	// Disable pause menu //
	GuiManagerAccess->SetCollectionState(L"PauseMenu", false);
	GuiManagerAccess->SetCollectionAllowEnableState(L"PauseMenu", true);
}

void Pong::PongGame::StartInputHandling(){
	GUARD_LOCK_THIS_OBJECT();
	GameInputHandler->StartReceivingInput(PlayerList);
	GameInputHandler->SetBlockState(false);
}
// ------------------------------------ //
void Pong::PongGame::CheckGameConfigurationVariables(GameConfiguration* configobj){
	// Check for various variables //

	GUARD_LOCK_OTHER_OBJECT_NAME(configobj, lockit);
	NamedVars* vars = configobj->AccessVariables(lockit);

	// Master server force localhost //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<bool>(L"MasterServerForceLocalhost")){
		// Add new //
		vars->AddVar(L"MasterServerForceLocalhost", new VariableBlock(false));
		configobj->MarkModified();
	}

	// Game configuration database //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<wstring>(L"GameDatabase")){
		// Add new //
		vars->AddVar(L"GameDatabase", new VariableBlock(wstring(L"PongGameDatabase.txt")));
		configobj->MarkModified();
	}

	// Default server port //
	if(vars->ShouldAddValueIfNotFoundOrWrongType<int>(L"DefaultServerPort")){
		// Add new //
		vars->AddVar(L"DefaultServerPort", new VariableBlock(int(53221)));
		configobj->MarkModified();
	}

}

void Pong::PongGame::CheckGameKeyConfigVariables(KeyConfiguration* keyconfigobj){

}
// ------------------------------------ //
void Pong::PongGame::MoveBackToLobby(){
	GUARD_LOCK_THIS_OBJECT();
	DEBUG_BREAK;
}

void Pong::PongGame::Disconnect(const string &reasonstring){
	GUARD_LOCK_THIS_OBJECT();

}
// ------------------------------------ //
void Pong::PongGame::DoSpecialPostLoad(){


	shared_ptr<ViewerCameraPos> MainCamera;

	_Engine->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](shared_ptr<ViewerCameraPos>* MainCamera, PongGame* game) -> void{
		// camera //
		*MainCamera = shared_ptr<ViewerCameraPos>(new ViewerCameraPos());
		(*MainCamera)->SetPos(Leviathan::Float3(0.f, 22.f*BASE_ARENASCALE, 0.f));

		// camera should always point down towards the play field //
		(*MainCamera)->SetRotation(Leviathan::Float3(0.f, -90.f, 0.f));

		// sound listening camera //
		(*MainCamera)->BecomeSoundPerceiver();

	}, &MainCamera, this))));

	// Wait for everything to finish //
	_Engine->GetThreadingManager()->WaitForAllTasksToFinish();

#ifdef _DEBUG

	// We are probably in text-only mode //
	if(Engine::Get()->GetGraphics() == NULL)
		return;

#endif // _DEBUG

	// load GUI documents //
	GuiManagerAccess = Engine::GetEngine()->GetWindowEntity()->GetGUI();

	GuiManagerAccess->LoadGUIFile(L"./Data/Scripts/GUI/PongMenus.txt");

	// set skybox to have some sort of visuals //
	WorldOfPong->SetSkyBox("NiceDaySky");

	// link world and camera to a window //
	GraphicalInputEntity* window1 = Engine::GetEngine()->GetWindowEntity();

	window1->LinkObjects(MainCamera, WorldOfPong);

	// link window input to game logic //
	window1->GetInputController()->LinkReceiver(GameInputHandler);
}
// ------------------------------------ //
void Pong::PongGame::MoreCustomScriptTypes(asIScriptEngine* engine){

	if(engine->RegisterObjectType("PongGame", 0, asOBJ_REF | asOBJ_NOCOUNT) < 0){
		SCRIPT_REGISTERFAIL;
	}


	if(engine->RegisterGlobalFunction("PongGame@ GetPongGame()", WRAP_FN(PongGame::Get), asCALL_GENERIC) < 0){
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PongGame", "int StartServer()", WRAP_MFN(PongGame, StartServer), asCALL_GENERIC) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}

	if(engine->RegisterObjectMethod("PongGame", "void MoveBackToLobby()", WRAP_MFN(PongGame, MoveBackToLobby), asCALL_GENERIC) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("PongGame", "void Disconnect(const string &in statusmessage)", WRAP_MFN(PongGame, Disconnect), asCALL_GENERIC) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
	if(engine->RegisterObjectMethod("PongGame", "void Connect(const string &in address)", WRAP_MFN(PongGame, ConnectProxy), asCALL_GENERIC) < 0)
	{
		SCRIPT_REGISTERFAIL;
	}
}

void Pong::PongGame::MoreCustomScriptRegister(asIScriptEngine* engine, std::map<int, wstring> &typeids){
	typeids.insert(make_pair(engine->GetTypeIdByDecl("PongGame"), L"PongGame"));
}

void Pong::PongGame::Connect(const wstring &address){
	Logger::Get()->Info(L"About to connect to address "+address);

	// Get a connection to use //
	auto tmpconnection = Leviathan::NetworkHandler::Get()->GetOrCreatePointerToConnection(address);

	if(!tmpconnection){

		EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
			new NamedVariableList(L"Message", new VariableBlock(string("Tried to connect to an invalid address, ")+Convert::WstringToString(address)))))));
		return;
	}

	EventHandler::Get()->CallEvent(new Leviathan::GenericEvent(L"ConnectStatusMessage", Leviathan::NamedVars(shared_ptr<NamedVariableList>(
		new NamedVariableList(L"Message", new VariableBlock(string("Opening connection to server at ")+Convert::WstringToString(address)))))));

	// We are a client and we can use our interface to handle the server connection functions //

	dynamic_cast<NetworkClientInterface*>(Leviathan::NetworkHandler::GetInterface())->JoinServer(tmpconnection);
	// The function automatically reports any errors //


	// Now it should be fine, waiting for messages //
}
