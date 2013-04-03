#ifndef LEVIATHAN_LAST
#define LEVIATHAN_LAST
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Application.h"


namespace Leviathan{

	class Default{
	public:
		
	};
	//{
	//	LRESULT result = 0;

	//	if (message == WM_CREATE)
	//	{

	//	}
	//	else
	//	{
	//		bool wasHandled = false;
	//		switch (message)
	//		{
	//		case WM_SIZE:
	//			{
	//				UINT width = LOWORD(lParam);
	//				UINT height = HIWORD(lParam);

	//				(LeviathanApplication::GetApp())->Resize(width, height);

	//			}
	//			result = 0;
	//			wasHandled = true;
	//			break;

	//		case WM_DISPLAYCHANGE:
	//			{
	//				InvalidateRect(hwnd, NULL, FALSE);
	//			}
	//			result = 0;
	//			wasHandled = true;
	//			break;

	//		case WM_PAINT:
	//			{
	//				(LeviathanApplication::GetApp())->Render();	
	//			}
	//			result = 0;
	//			wasHandled = true;
	//			break;
	//		case WM_SETFOCUS:
	//			{
	//				LeviathanApplication::GetApp()->GainFocus();
	//			}
	//		break;
	//		case WM_KILLFOCUS:
	//			{
	//				LeviathanApplication::GetApp()->LoseFocus();
	//			}
	//		break;
	//		}
	//		if (!wasHandled)
	//		{
	//			result = (LeviathanApplication::GetApp())->GetEngine()->HandleWindowCallBack(message,wParam,lParam);
	//			if(!result)
	//				result = DefWindowProc(hwnd, message, wParam, lParam);
	//		}
	//	}
	//	return result;
	//}
}
#endif
