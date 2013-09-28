// ------------------------------------ //
#ifndef SANDBOXIE_APP
#include "App.h"
#endif
//using namespace Leviathan;
using namespace SandBoxie;
// ------------------------------------ //
App::App(){

}
// ------------------------------------ //
void App::CustomizeEnginePostLoad(){
	// load GUI documents //
	Gui::GuiManager::Get()->LoadGUIFile(FileSystem::GetScriptsFolder()+L"MainGui.txt");

	Gui::GuiManager::Get()->SetMouseFile(FileSystem::GetScriptsFolder()+L"cursor.rml");
}
// ------------------------------------ //

//LRESULT CALLBACK App::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	// handle creation message here //
//	if(message == WM_CREATE){
//
//		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
//		WindowPassData* WData = (WindowPassData *)pcs->lpCreateParams;
//
//
//		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(WData));
//
//		return 1;
//	}
//
//	WindowPassData* WData = reinterpret_cast<WindowPassData*>(static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));
//
//	// skip if empty data //
//	if(!WData){
//		return DefWindowProc(hwnd, message, wParam, lParam);
//	}
//
//	switch (message){
//	case WM_SIZE:
//		{
//			UINT width = LOWORD(lParam);
//			UINT height = HIWORD(lParam);
//
//			WData->Appinterface->OnResize(width, height);
//			return 0;
//		}
//	case WM_DISPLAYCHANGE:
//		{
//			InvalidateRect(hwnd, NULL, FALSE);
//			return 0;
//		}
//	case WM_MENUCHAR:
//		{
//			return MNC_CLOSE << 16;
//		}
//	case WM_QUIT: case WM_CLOSE: case WM_DESTROY:
//		{
//			// close the application //
//			WData->Appinterface->StartRelease();
//			// close passed data //
//			SAFE_DELETE(WData);
//			// set back so that it won't be used anymore //
//			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(WData));
//
//			return 0;
//		}
//		break;
//	case WM_SETFOCUS:
//		{
//			WData->Appinterface->GainFocus();
//			return 0;
//		}
//	case WM_KILLFOCUS:
//		{
//			WData->Appinterface->LoseFocus();
//			return 0;
//		}
//	case WM_KEYDOWN:
//		switch(wParam){
//		case VK_ESCAPE:
//			{
//				WData->Appinterface->StartRelease();
//				return 0;
//			}
//		}
//	break;
//	}
//
//	// got here, tell engine to handle, or resort to default window procedure //
//	if(!WData->Appinterface->GetEngine()->HandleWindowCallBack(message, wParam, lParam))
//		return DefWindowProc(hwnd, message, wParam, lParam);
//
//	return 0;
//}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //