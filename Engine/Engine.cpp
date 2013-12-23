#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ENGINE
#include "Engine.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Application/Application.h"
#include "Entities/GameWorld.h"
#include <boost/thread/future.hpp>

DLLEXPORT Leviathan::Engine::Engine(LeviathanApplication* owner) : Owner(owner), LeapData(NULL), MainConsole(NULL), MainFileHandler(NULL),
	_NewtonManager(NULL), GraphicalEntity1(NULL), PhysMaterials(NULL), _NetworkHandler(NULL), _ThreadingManager(NULL)
{

	// create this here //
	IDDefaultInstance = IDFactory::Get();

	Mainlog = NULL;
	Inited = false;
	Graph = NULL;
	Define = NULL;
	MainRandom = NULL;
	RenderTimer = NULL;

	Sound = NULL;

	TimePassed = 0;
	LastFrame = 0;

	Mainstore = NULL;
	MainScript = NULL;

	TickCount = 0;
	TickTime = 0;
	FrameCount = 0;

	MainEvents = NULL;
	Loader = NULL;
	OutOMemory = NULL;
}

Engine* Leviathan::Engine::instance = NULL;

Engine* Leviathan::Engine::GetEngine(){
	return instance;
}

DLLEXPORT Engine* Leviathan::Engine::Get(){
	return instance;
}
// ------------------------------------ //
bool Leviathan::Engine::Init(AppDef* definition, NetworkClient* networking){
	// get time, for monitoring how long load takes //
	__int64 InitStartTime = Misc::GetTimeMs64();
	// set static access to this object //
	instance = this;
	// store parameters //
	Define = definition;

	// create logger object //
	if(Logger::GetIfExists() != NULL){
		// already exists //
		Mainlog = Logger::Get();
	} else {
		Mainlog = new Logger();
	}

	// create //
	OutOMemory = new OutOfMemoryHandler();

	// create randomizer //
	MainRandom = new Random((int)InitStartTime);
	MainRandom->SetAsMain();

	// Create threading facilities //
	_ThreadingManager = new ThreadingManager();
	if(!_ThreadingManager->Init()){

		Logger::Get()->Error(L"Engine: Init: cannot start threading");
		return false;
	}

	// We want to send a request to the master server as soon as possible //
	_NetworkHandler = new NetworkHandler(networking);

	_NetworkHandler->Init(Define->GetMasterServerInfo());

	// These should be fine to be threaded //

	// data storage //
	boost::promise<bool> MainStoreResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->Mainstore = new DataStore(true);
		if(!engine->Mainstore){

			Logger::Get()->Error(L"Engine: Init: failed to create main data store");
			returnvalue.set_value(false);
			return;
		}

		returnvalue.set_value(true);
	}, boost::ref(MainStoreResult), this))));


	// search data folder for files //
	boost::promise<bool> FileHandlerResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->MainFileHandler = new FileSystem();
		if(!engine->MainFileHandler){

			Logger::Get()->Error(L"Engine: Init: failed to create FileSystem");
			returnvalue.set_value(false);
			return;
		}

		if(!engine->MainFileHandler->Init()){

			Logger::Get()->Error(L"Engine: Init: failed to init FileSystem");
			returnvalue.set_value(false);
			return;
		}

		returnvalue.set_value(true);
	}, boost::ref(FileHandlerResult), this))));

	// file parsing //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([]() -> void{ ObjectFileProcessor::Initialize(); }))));


	// main program wide event dispatcher //
	boost::promise<bool> EventHandlerResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->MainEvents = new EventHandler();
		if(!engine->MainEvents){

			Logger::Get()->Error(L"Engine: Init: failed to create MainEvents");
			returnvalue.set_value(false);
			return;
		}

		if(!engine->MainEvents->Init()){

			Logger::Get()->Error(L"Engine: Init: failed to init MainEvents");
			returnvalue.set_value(false);
			return;
		}

		returnvalue.set_value(true);
	}, boost::ref(EventHandlerResult), this))));

	// create script interface before renderer //
	boost::promise<bool> ScriptInterfaceResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->MainScript = new ScriptInterface();
		if(!engine->MainScript){

			Logger::Get()->Error(L"Engine: Init: failed to create ScriptInterface");
			returnvalue.set_value(false);
			return;
		}

		if(!engine->MainScript->Init()){

			Logger::Get()->Error(L"Engine: Init: failed to init ScriptInterface");
			returnvalue.set_value(false);
			return;
		}

		// create console after script engine //
		engine->MainConsole = new ScriptConsole();
		if(!engine->MainConsole){

			Logger::Get()->Error(L"Engine: Init: failed to create ScriptConsole");
			returnvalue.set_value(false);
			return;
		}

		if(!engine->MainConsole->Init(engine->MainScript)){

			Logger::Get()->Error(L"Engine: Init: failed to initialize Console, continuing anyway");
		}

		returnvalue.set_value(true);
	}, boost::ref(ScriptInterfaceResult), this))));

	// create newton manager before any newton resources are needed //
	boost::promise<bool> NewtonManagerResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->_NewtonManager = new NewtonManager();
		if(!engine->_NewtonManager){

			Logger::Get()->Error(L"Engine: Init: failed to create NewtonManager");
			returnvalue.set_value(false);
			return;
		}

		// next force application to load physical surface materials //
		engine->PhysMaterials = new PhysicsMaterialManager(engine->_NewtonManager);
		if(!engine->PhysMaterials){

			Logger::Get()->Error(L"Engine: Init: failed to create PhysicsMaterialManager");
			returnvalue.set_value(false);
			return;
		}

		engine->Owner->RegisterApplicationPhysicalMaterials(engine->PhysMaterials);

		returnvalue.set_value(true);
	}, boost::ref(NewtonManagerResult), this))));


	ObjectFileProcessor::LoadValueFromNamedVars<int>(Define->GetValues(), L"MaxFPS", FrameLimit, 120, true, L"Graphics: Init:");

	Graph = new Graphics();

	// We need to wait for all current tasks to finish //
	_ThreadingManager->WaitForAllTasksToFinish();

	// Check return values //
	if(!MainStoreResult.get_future().get() || !FileHandlerResult.get_future().get() || !EventHandlerResult.get_future().get() ||
		!ScriptInterfaceResult.get_future().get())
	{

		Logger::Get()->Error(L"Engine: Init: one or more queued tasks failed");
		return false;
	}

	// We can queue some more tasks //
	// create leap controller //
	boost::promise<bool> LeapControllerResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->LeapData = new LeapManager(engine);
		if(!engine->LeapData){
			Logger::Get()->Error(L"Engine: Init: failed to create LeapManager");
			returnvalue.set_value(false);
			return;
		}
		// try here just in case //
		try{
			if(!engine->LeapData->Init()){

				Logger::Get()->Info(L"Engine: Init: No Leap controller found, not using one");
			}
		}
		catch(...){
			// threw something //
			Logger::Get()->Error(L"Engine: Init: Leap threw something, even without leap this shouldn't happen; continuing anyway");
		}

		returnvalue.set_value(true);
	}, boost::ref(LeapControllerResult), this))));


	// sound device //
	boost::promise<bool> SoundDeviceResult;
	// Ref is OK to use since this task finishes before this function //
	_ThreadingManager->QueueTask(shared_ptr<QueuedTask>(new QueuedTask(boost::bind<void>([](boost::promise<bool> &returnvalue, Engine* engine) -> void{

		engine->Sound = new SoundDevice();
		if(!engine->Sound){
			Logger::Get()->Error(L"Engine: Init: failed to create SoundDevice");
			returnvalue.set_value(false);
			return;
		}

		if(!engine->Sound->Init()){

			Logger::Get()->Error(L"Engine: Init: failed to init SoundDevice");
			returnvalue.set_value(false);
			return;
		}

		// make angel script make list of registered stuff //
		engine->MainScript->GetExecutor()->ScanAngelScriptTypes();

		// measuring //
		engine->RenderTimer = new RenderingStatistics();
		if(!engine->RenderTimer){
			Logger::Get()->Error(L"Engine: Init: failed to create RenderingStatistics");
			returnvalue.set_value(false);
			return;
		}

		// create object loader //
		engine->Loader = new ObjectLoader(engine);
		if(!engine->Loader){
			Logger::Get()->Error(L"Engine: Init: failed to create ObjectLoader");
			returnvalue.set_value(false);
			return;
		}

		returnvalue.set_value(true);
	}, boost::ref(SoundDeviceResult), this))));

    if(!Graph){

        Logger::Get()->Error(L"Engine: Init: failed to create instance of Graphics");
        return false;
    }

	// call init //
	if(!Graph->Init(definition)){
		Logger::Get()->Error(L"Failed to init Engine, Init graphics failed! Aborting");
		return false;
	}

	// create window //
	GraphicalEntity1 = new GraphicalInputEntity(Graph, definition);

	if(!LeapControllerResult.get_future().get() || !SoundDeviceResult.get_future().get()){

		Logger::Get()->Error(L"Engine: Init: leap manager or sound device queued tasks failed");
		return false;
	}


	Inited = true;

	PostLoad();

	Logger::Get()->Info(L"Engine init took "+Convert::ToWstring(Misc::GetTimeMs64()-InitStartTime)+L" ms", false);
	return true;
}

void Leviathan::Engine::PostLoad(){
	// increase start count //
	int startcounts = 0;

	if(Mainstore->GetValueAndConvertTo<int>(L"StartCount", startcounts)){
		// increase //
		Mainstore->SetValue(L"StartCount", new VariableBlock(new IntBlock(startcounts+1)));
	} else {

		Mainstore->AddVar(new NamedVariableList(L"StartCount", new VariableBlock(1)));
		// set as persistent //
		Mainstore->SetPersistance(L"StartCount", true);
	}

	// get time //
	LastFrame = Misc::GetTimeMs64();
}

void Leviathan::Engine::Release(){
	// Let the game release it's resources //
	Owner->EnginePreShutdown();

	// Close all connections //
	SAFE_RELEASEDEL(_NetworkHandler);

	// Wait for tasks to finish //
	_ThreadingManager->WaitForAllTasksToFinish();

	// destroy worlds //
	while(GameWorlds.size()){

		GameWorlds[0]->Release();
		GameWorlds.erase(GameWorlds.begin());
	}



	if(GraphicalEntity1){
		// make windows clear their stored objects //
		GraphicalEntity1->ReleaseLinked();
	}

	// destroy windows //
	SAFE_DELETE(GraphicalEntity1);

	// release newton //
	SAFE_DELETE(PhysMaterials);
	SAFE_DELETE(_NewtonManager);

	SAFE_RELEASEDEL(LeapData);

	// console needs to be released before script release //
	SAFE_RELEASEDEL(MainConsole);

	SAFE_RELEASEDEL(MainScript);
	// save at this point (just in case it crashes before exiting) //
	Mainlog->Save();

	SAFE_DELETE(Loader);
	SAFE_RELEASEDEL(Graph);
	SAFE_DELETE(RenderTimer);

	SAFE_RELEASEDEL(Sound);
	SAFE_DELETE(Mainstore);

	SAFE_RELEASEDEL(MainEvents);

	SAFE_DELETE(Mainlog);

	// delete randomizer last, for obvious reasons //
	SAFE_DELETE(MainRandom);

	ObjectFileProcessor::Release();
	SAFE_RELEASEDEL(MainFileHandler);

	// Stop threads //
	_ThreadingManager->WaitForAllTasksToFinish();
	SAFE_RELEASEDEL(_ThreadingManager);

	// clears all running timers that might have accidentally been left running //
	TimingMonitor::ClearTimers();

	// safe to delete this here //
	SAFE_DELETE(OutOMemory);

	SAFE_DELETE(IDDefaultInstance);
}
// ------------------------------------ //
void Leviathan::Engine::Tick(){
	// Because this is checked very often we can check for physics update here //
	PhysicsUpdate();
	// We can also update networking //
	_NetworkHandler->UpdateAllConnections();

	// get time since last update //
	__int64 CurTime = Misc::GetTimeMs64();
	TimePassed = (int)(CurTime-LastFrame);

	if((TimePassed < TICKSPEED)){
		// no tick time yet //
		return;
	}

	//LastFrame = CurTime;
	LastFrame += TICKSPEED;
	TickCount++;

	// update input //
	LeapData->OnTick(TimePassed);

	// sound tick //
	Sound->Tick(TimePassed);

	// update windows //
	GraphicalEntity1->Tick(TimePassed);

	// update texture usage times, to allow unused textures to be unloaded //
	//Graph->GetTextureManager()->TimePass(TimePassed);

	// some dark magic here //
	if(TickCount % 25 == 0){
		// update values
		Mainstore->SetTickCount(TickCount);
		Mainstore->SetTickTime(TickTime);


		// send updated rendering statistics //
		RenderTimer->ReportStats(Mainstore);
	}

	// send tick event //
	MainEvents->CallEvent(new Event(EVENT_TYPE_ENGINE_TICK, new int(TickCount)));

	// Call the default app tick //
	Owner->Tick(TimePassed);

	TickTime = (int)(Misc::GetTimeMs64()-LastFrame);
}

DLLEXPORT void Leviathan::Engine::PreFirstTick(){
	// On first tick we need to do some cleanup //
	_NetworkHandler->StopOwnUpdaterThread();
}
// ------------------------------------ //
void Leviathan::Engine::RenderFrame(){

	int SinceLastFrame = -1;

	// limit check //
	if(!RenderTimer->CanRenderNow(FrameLimit, SinceLastFrame)){
		// fps would go too high //

		return;
	}

	// since last frame is in microseconds 10^-6 convert to milliseconds //
	// SinceLastFrame is always more than 1000 (always 1 ms or more) //
	SinceLastFrame /= 1000;
	FrameCount++;

	// advanced statistic start monitoring //
	RenderTimer->RenderingStart();

	MainEvents->CallEvent(new Event(EVENT_TYPE_FRAME_BEGIN, new int(SinceLastFrame)));

	// render //
	GraphicalEntity1->Render(SinceLastFrame);
	Graph->Frame();

	MainEvents->CallEvent(new Event(EVENT_TYPE_FRAME_END, new int(FrameCount)));

	// advanced statistics frame has ended //
	RenderTimer->RenderingEnd();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Engine::ExecuteCommandLine(const wstring &commands){

}
void Leviathan::Engine::RunScrCommand(wstring command, wstring params){
#ifdef _DEBUG
	Mainlog->Info(L"[DEBUG] Running script command: "+command+L" with params "+params, false);
#endif

	// pass to console //
	DEBUG_BREAK;

}

DLLEXPORT void Leviathan::Engine::SaveScreenShot(){

	const wstring fileprefix = MainFileHandler->GetDataFolder()+L"Screenshots/Captured_frame_";


	GraphicalEntity1->SaveScreenShot(Convert::WstringToString(fileprefix));
}

DLLEXPORT int Leviathan::Engine::GetWindowOpenCount(){
	int openwindows = 0;

	if(GraphicalEntity1->GetWindow()->IsOpen())
		openwindows++;

	return openwindows;
}

DLLEXPORT shared_ptr<GameWorld> Leviathan::Engine::CreateWorld(){
	shared_ptr<GameWorld> tmp(new GameWorld(Graph->GetOgreRoot()));

	GameWorlds.push_back(tmp);
	return GameWorlds.back();
}

DLLEXPORT void Leviathan::Engine::PhysicsUpdate(){
	// go through all worlds and simulate updates //
	for(size_t i = 0; i < GameWorlds.size(); i++){

		GameWorlds[i]->SimulateWorld();
	}

}

DLLEXPORT void Leviathan::Engine::ResetPhysicsTime(){
	// go through all worlds and set last update time to this moment //
	for(size_t i = 0; i < GameWorlds.size(); i++){

		GameWorlds[i]->ClearSimulatePassedTime();
	}
}
// ------------------------------------ //
void Leviathan::Engine::_NotifyThreadsRegisterOgre(){
	// Register threads to use graphical objects //
	_ThreadingManager->MakeThreadsWorkWithOgre();
}



// ------------------------------------ //

// ------------------------------------ //
