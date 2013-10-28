#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_ENGINE
#include "Engine.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Application\Application.h"
#include "Rendering\TextureManager.h"
#include "Entities\GameWorld.h"

DLLEXPORT Leviathan::Engine::Engine(LeviathanApplication* owner) : Owner(owner), LeapData(NULL), MainConsole(NULL), MainFileHandler(NULL), 
	_NewtonManager(NULL), GraphicalEntity1(NULL), PhysMaterials(NULL)
{

	// create this here //
	IDDefaultInstance = IDFactory::Get();

	Mainlog = NULL;
	Inited = false;
	Graph = NULL;
	Define = NULL;
	MTimer = NULL;
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
bool Leviathan::Engine::Init(AppDef* definition){
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

	// data storage //
	Mainstore = new DataStore(true);
	CLASS_ALLOC_CHECK(Mainstore);


	// search data folder for files //
	MainFileHandler = new FileSystem();
	CLASS_ALLOC_CHECK(MainFileHandler);
	MainFileHandler->Init();

	// file parsing //
	ObjectFileProcessor::Initialize();

	// main program wide event dispatcher //
	MainEvents = new EventHandler();
	CLASS_ALLOC_CHECK(MainEvents);
	if(!MainEvents->Init()){
		Logger::Get()->Error(L"Engine: Init: Init EventHandler failed!");
		return false;
	}

	// timing object //
	MTimer = new Timer();

	// create script interface before renderer //
	MainScript = new ScriptInterface();
	if(!MainScript){

		Logger::Get()->Error(L"Engine: 008");
		return false;
	}
	if(!MainScript->Init()){
		Logger::Get()->Error(L"Failed to init Engine, Init ScriptInterface failed!");
		return false;
	}

	// create console after script engine //
	MainConsole = new ScriptConsole();
	CLASS_ALLOC_CHECK(MainConsole);

	if(!MainConsole->Init(MainScript)){

		Logger::Get()->Error(L"Engine: Init: failed to initialize Console, continuing anyway");
	}

	ObjectFileProcessor::LoadValueFromNamedVars<int>(Define->GetValues(), L"MaxFPS", FrameLimit, 120, true, L"Graphics: Init:");

	Graph = new Graphics();
	CLASS_ALLOC_CHECK(Graph);

	// call init //
	if(!Graph->Init(definition)){
		Logger::Get()->Error(L"Failed to init Engine, Init graphics failed! Aborting");
		return false;
	}

	// create newton manager before any newton resources are needed //
	_NewtonManager = new NewtonManager();

	// next force application to load physical surface materials //
	PhysMaterials = new PhysicsMaterialManager(_NewtonManager);



	// create window //
	GraphicalEntity1 = new GraphicalInputEntity(Graph, definition);


	// make angel script make list of registered stuff //
	MainScript->GetExecutor()->ScanAngelScriptTypes();


	// create leap controller //
	LeapData = new LeapManager(this);
	if(!LeapData){
		Logger::Get()->Error(L"Engine: 008");
		return false;
	}
	// try here just in case //
	try{
		if(!LeapData->Init()){

			Logger::Get()->Info(L"Engine: Init: No Leap controller found, not using one");
		}
	}
	catch(...){
		// threw something //
		Logger::Get()->Error(L"Engine: Init: Leap threw something, even without leap this shouldn't happen; continuing anyway");
	}

	// sound device //
	Sound = new SoundDevice();
	CLASS_ALLOC_CHECK(Sound);
	if(!Sound->Init()){

		Logger::Get()->Error(L"Failed to init Engine, sound init failed");
		return false;
	}

	// create object loader //
	Loader = new ObjectLoader(this);
	CLASS_ALLOC_CHECK(Loader);

	// measuring //
	RenderTimer = new RenderingStatistics();
	CLASS_ALLOC_CHECK(RenderTimer);

	Inited = true;

	PostLoad();

	Logger::Get()->Info(L"Engine init took "+Convert::ToWstring(Misc::GetTimeMs64()-InitStartTime)+L" ms", false);

	// let's send a debug message telling engine initialized //
	Logger::Get()->Info(L"Engine initialized", true);
	return true;
}

void Leviathan::Engine::PostLoad(){

	// get time //
	LastFrame = Misc::GetTimeMs64();

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
}

void Leviathan::Engine::Release(){

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
	SAFE_RELEASEDEL(PhysMaterials);
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


	SAFE_DELETE(MTimer);
	SAFE_DELETE(Mainlog);

	// delete randomizer last, for obvious reasons //
	SAFE_DELETE(MainRandom);

	ObjectFileProcessor::Release();
	SAFE_RELEASEDEL(MainFileHandler);

	// clears all running timers that might have accidentally been left running //
	TimingMonitor::ClearTimers();

	// safe to delete this here //
	SAFE_DELETE(OutOMemory);

	SAFE_DELETE(IDDefaultInstance);
}
// ------------------------------------ //
void Leviathan::Engine::Tick(){
	// get time since last update //
	__int64 CurTime = Misc::GetTimeMs64();
	TimePassed = (int)(CurTime-LastFrame);

	if((TimePassed < TICKSPEED)){
		// no tick time yet //
		return;
	}

	// update focus state //


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
	Graph->GetTextureManager()->TimePass(TimePassed);

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

	TickTime = (int)(Misc::GetTimeMs64()-LastFrame);
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

	const wstring fileprefix = MainFileHandler->GetDataFolder()+L"Screenshots\\Captured_frame_";


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

// ------------------------------------ //

// ------------------------------------ //
