#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_APPLICATION
#include "Application.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "FileSystem.h"

LeviathanApplication* GlobalLevApPtr = NULL;

LeviathanApplication::LeviathanApplication(){
	Quit = false;
	engine = NULL;// = new Engine();
	m_wind = new Window();
	Defvals = NULL;
	if(Curapp != NULL){
		Logger::Get()->Error(L"Application already existed, overwrote old!");
	}
	Curapp = this;
	GlobalLevApPtr = Curapp;
	Windowed = true;
	Quitted = false;
}
LeviathanApplication* LeviathanApplication::Curapp = NULL;
LeviathanApplication* LeviathanApplication::GetApp(){
	return Curapp;
}



AppDef* LeviathanApplication::GetAppDef(){
	return Curapp->Defvals;
}
// ------------------------------------ //
bool LeviathanApplication::Initialize(HINSTANCE hinstance){
	hInstance = hinstance;
	m_wind->Init(hinstance);
	// create definitions //
	ConstructDefinition();
	InternalInit();

	// init engine //
	engine = new Engine();
	return engine->InitEngine(m_wind, Windowed, Defvals);
}
bool LeviathanApplication::Initialize(HINSTANCE hinstance, HWND hwnd, /*int width, int height,*/ bool windowed){
	// create definitions //
	hInstance = hinstance;
	ConstructDefinition();

	// config windowed override reading //
	if(Defvals->GetValues()->Find(L"Windowed") != -1){
		// override value //
		int temp = 0;
		Defvals->GetValues()->GetValue(L"Windowed", temp);
		windowed = (temp != 0);
	}

	Windowed = windowed;
	

	// get size from configs //
	int width = 800, height = 600;
	Defvals->GetValues()->GetValue(L"WindowWidth", width);
	Defvals->GetValues()->GetValue(L"WindowHeight", height);

	InternalInit();

	// init engine //
	m_wind->Init(hwnd, width, height);
	engine = new Engine();
	return engine->InitEngine(m_wind, Windowed, Defvals);
}	
bool LeviathanApplication::Initialize(HINSTANCE hinstance,  WNDPROC proc, wstring tittle, /*int width, int height,*/ HICON hIcon, bool windowed){
	// create definitions //
	hInstance = hinstance;
	ConstructDefinition();

	// config windowed override reading //
	if(Defvals->GetValues()->Find(L"Windowed") != -1){
		// override value //
		int temp = 0;
		Defvals->GetValues()->GetValue(L"Windowed", temp);
		windowed = (temp != 0);
	}

	Windowed = windowed;
	

	// get size from configs //
	int width = 800, height = 600;
	Defvals->GetValues()->GetValue(L"WindowWidth", width);
	Defvals->GetValues()->GetValue(L"WindowHeight", height);

	InternalInit();

	// init engine //
	m_wind->Init(hinstance, proc, tittle, width, height, hIcon, Windowed);
	engine = new Engine();
	return engine->InitEngine(m_wind, Windowed, Defvals);
}
void LeviathanApplication::ConstructDefinition(){
	this->Defvals = new AppDef(true);

	// store this pointer //

	if(!Defvals){
		Logger::Get()->Error(L"Application: failed to generate default configuration values", true);
		return;
	}
	this->Defvals->GetValues()->LoadVarsFromFile(L".\\EngineConf.conf");

	//Defvals = AppDef::GetDefault();
	//FileSystem::LoadDataDumb(L".\\EngineConf.conf", *Defvals->GetValues()->GetVec());

}
void LeviathanApplication::InternalInit(){


}
// ------------------------------------ //
void LeviathanApplication::Close(){
	if(!Quitted){
		engine->ShutDownEngine();
		delete engine;
		engine = NULL;
		m_wind->CloseDown();
		delete m_wind;
		m_wind = NULL;
		Quitted = true;
	}


		
	Quit = true;
}
// ------------------------------------ //
void LeviathanApplication::LoseFocus(){
	if(engine)
		engine->LoseFocus();
}
void LeviathanApplication::GainFocus(){
	if(engine)
		engine->GainFocus();
}
// ------------------------------------ //
void LeviathanApplication::Render(){
	engine->RenderFrame();
}

void Leviathan::LeviathanApplication::OnResize(UINT width, UINT height){
	// let the engine handle this //
	if(engine == NULL){
		// something is terribly wrong OR
		// this is also called when window is created, should be ignored (window is created before engine) //
		return;
	}
	engine->OnResize(width,height);
}

void Leviathan::LeviathanApplication::ResizeWindow(int newwidth, int newheight){
	// tell engine to resize window //
	engine->DoWindowResize(newwidth, newheight);
}

int LeviathanApplication::RunMessageLoop(){
	MSG Msg;
//	BOOL bRet;
	for (int GameLoopCount=0; ; GameLoopCount++){
		while(PeekMessage(&Msg, this->m_wind->GetHandle(),NULL,NULL,PM_REMOVE)){
			TranslateMessage(&Msg); 
			DispatchMessage(&Msg); 
			if(Msg.message == WM_QUIT){
				this->Close();
				break;
			}
		}
		//while(GetQueueStatus(QS_ALLINPUT) !=0 ){
		//		bRet = GetMessage( &Msg, NULL, 0, 0 );
		//	if (bRet <= 0){
		//		Quit=1;
		//		break;
		//	} else {
		//		TranslateMessage(&Msg); 
		//		DispatchMessage(&Msg); 
		//		if(Msg.message == WM_QUIT){
		//			Quit = true;
		//		}
		//	}
		//}
		if(Quit){
			this->Close();
			Msg.wParam = 0;
			break;
		}

		// engine tick //
		engine->Tick(false);
		Render();

	}
	return Msg.wParam; 
}
// ------------------------------------ //
 void LeviathanApplication::PassCommandLine(wstring params){
	 engine->ExecuteCommandLine(params);
 }