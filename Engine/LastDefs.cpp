#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LAST
#include "LastDefs.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
LRESULT CALLBACK Default::DefWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
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

				(LeviathanApplication::GetApp())->Resize(width, height);

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

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //