#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_WINDOW
#include "Window.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Application.h"
LRESULT CALLBACK DefWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	LRESULT result = 0;

	if (message == WM_CREATE){

	} else {
		bool wasHandled = false;
		switch (message)
		{
		case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);

				(LeviathanApplication::GetApp())->OnResize(width, height);

			}
			result = 0;
			wasHandled = true;
			break;

		case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
			result = 0;
			wasHandled = true;
			break;

		case WM_PAINT:
			{
				(LeviathanApplication::GetApp())->Render();	
			}
			result = 0;
			wasHandled = true;
			break;
		case WM_SETFOCUS:
			{
				LeviathanApplication::GetApp()->GainFocus();
			}
		break;
		case WM_KILLFOCUS:
			{
				LeviathanApplication::GetApp()->LoseFocus();
			}
		break;
		}
		if (!wasHandled){
			result = (LeviathanApplication::GetApp())->GetEngine()->HandleWindowCallBack(message,wParam,lParam);
			if(!result)
				result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	return result;
}
// ------------------------------------ //

Window::Window(){
	Windowed = true;
	Width = -1;
	Height = -1;
	m_hwnd = NULL;

	CursorHidden = false;
}
void Window::CloseDown(){
	if(!IsWindow(m_hwnd))
		return;
	if(!Windowed){
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_hwnd);
	m_hwnd = NULL;
}
void Window::Init(HWND hwnd, int width, int height){
	m_hwnd = hwnd;
	Width = width;
	Height = height;

	
}
bool Window::Init(HINSTANCE hInstance, WNDPROC proc, wstring tittle, int width, int height, HICON hIcon, bool windowed){
	HRESULT hr = S_OK;
	Width = width;
	Height = height;
	// get data from config //


	WNDCLASSEX wcex = { sizeof(wcex) };
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = proc;
	wcex.cbWndExtra    = sizeof(LONG_PTR);
	wcex.hInstance     = hInstance;
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = CLASSNAME;
	wcex.hIcon         = hIcon;
	if(!RegisterClassEx(&wcex)){
		Logger::Get()->Error(L"failed to register window class",GetLastError());
		return false;
	}
	// variable dumb //
	int Xpos = CW_USEDEFAULT, Ypos = CW_USEDEFAULT;
	int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	DEVMODE dmScreenSettings;

	if(!windowed){
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		Xpos = Ypos = 0;
		m_hwnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			CLASSNAME,
			tittle.c_str(),
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
			0,
			0,
			screenWidth,
			screenHeight,
			NULL,
			NULL,
			hInstance,
			this
			);
	} else {
		//Xpos = (GetSystemMetrics(SM_CXSCREEN) - screenWidth)  / 2;
		//Ypos = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
		Xpos = screenWidth/2 - width/2;
		Ypos = screenHeight/2 - height/2;
		////calculate size
		RECT wr = {0,0,width,height};
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
		width = wr.right - wr.left;
		height = wr.bottom - wr.top;
	}


	// Create the application window.
	m_hwnd = CreateWindow(
		CLASSNAME,
		tittle.c_str(),
		WS_BORDER | WS_CAPTION | WS_SYSMENU,
		Xpos,
		Ypos,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		this
		);

	//Width=width;
	//Height=height;

	hr = IsWindow(m_hwnd) ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		try{
			if(!ShowWindow(m_hwnd, SW_SHOWNORMAL)){
				// fail? //
				Logger::QueueErrorMessage(L"Window: ShowWindow failed, trying to continue");
				return S_OK;
			}
			if(!UpdateWindow(m_hwnd)){
				// fail? //
				Logger::QueueErrorMessage(L"Window: UpdateWindow failed, trying to continue");
				return S_OK;
			}
			SetFocus(m_hwnd);
		}
		catch (...){
#ifdef _DEBUG
			__debugbreak();
#endif // _DEBUG
			throw;
		}
	}
		

	return SUCCEEDED(hr);
}
bool Window::Init(HINSTANCE hInstance){
	HRESULT hr = S_OK;
	WNDCLASSEX wcex = { sizeof(wcex) };
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = DefWndProc;
	wcex.cbWndExtra    = sizeof(LONG_PTR);
	wcex.hInstance     = hInstance;
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = CLASSNAME;
	wcex.hIcon         = NULL;

	RegisterClassEx(&wcex);
	////calculate size
	RECT wr = {0,0,800,600};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	// Create the application window.
	m_hwnd = CreateWindow(
		CLASSNAME,
		L"Leviathan window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		this
		);

	Width=800;
	Height=600;

	hr = m_hwnd ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		UpdateWindow(m_hwnd);
	}
	return SUCCEEDED(hr);
}

void Window::SetNewSize(int width, int height){
	Width = width;
	Height = height;
}
bool Window::ResizeWin32Window(int newwidth, int newheight, bool resizetocenter){
	SetNewSize(newwidth, newheight);
	int x,y;
	RECT wndsize;

	//Width = newwidth;
	//Height = newheight;

	GetWindowRect(m_hwnd, &wndsize);

	RECT wr = {0,0,newwidth,newheight};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	// calculate actual width and height //
	int realwidth = wr.right-wr.left;
	int realheight = wr.bottom-wr.top;

	if(!resizetocenter){
		// get old x and y first to preserve them //

		x = wndsize.left;
		y = wndsize.top;
	} else {
		// get screen size and set window coordinates to center //
		x = (GetSystemMetrics(SM_CXSCREEN) - realwidth)  / 2;
		y = (GetSystemMetrics(SM_CYSCREEN) - realheight) / 2;
	}


	// call window resize //
	MoveWindow(m_hwnd, x, y, realwidth, realheight, FALSE);
	return true;
}

void Window::GetRelativeMouse(int& x, int& y){
	POINT p;
	GetCursorPos(&p);

	ScreenToClient(m_hwnd, &p);

	x = p.x;
	y = p.y;
}
void Window::SetMouseToCenter(){
	POINT p;
	p.x = Width/2;
	p.y = Height/2;

	ClientToScreen(m_hwnd, &p);

	SetCursorPos(p.x, p.y);

}
void Window::SetHideCursor(bool toset){
	if(toset == CursorHidden)
		return;
	CursorHidden = toset;
	if(CursorHidden){
		ShowCursor(FALSE);
	} else {
		//CursorHidden = true;
		ShowCursor(TRUE);
	}
}

void Window::LoseFocus(){
	if(CursorHidden){
		CursorHidden = false;
		ShowCursor(TRUE);
	}
}
void Window::GainFocus(){
	//if(CursorHidden)
	//	ShowCursor(FALSE);
}



float Leviathan::Window::GetAspectRatio() const{
	return ((float)Width)/Height;
}
