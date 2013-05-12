#ifndef LEVIATHAN_INCLUDE
#define LEVIATHAN_INCLUDE
#include <stdio.h>

// visual leak detector //
//#include <vld.h>

//#define ANALYZEBUILD

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <assert.h>

#include <SDKDDKVer.h>

// some special compiler settings //
#ifdef ANALYZEBUILD
// this stop code analyze from breaking //
#define _AFXDLL
#endif

#ifdef ANALYZEBUILD
#pragma warning (disable:6387)
#include <afxcontrolbars.h>
#pragma warning (default:6387)
#endif // ANALYZEBUILD


#include <Windows.h>
#include <Windowsx.h>
#include <wincodec.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>
#include <algorithm>

#include <string>
#include <time.h>
#include <ctime>

#include <vector>

#include <initguid.h>
#include <shlobj.h>

// not required right now //
//#include <regex>

using namespace std;
//#include <d2d1.h>
//#include <d2d1helper.h>
//#include <dwrite.h>

// DirectX SDK includes
#include <D3Dcommon.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dCompiler.h>
#include <d3dx11.h>
#include <d3dx10math.h>
//#include <D3DX10core.h>
//#include <D3D11Shader.h>

#include <d3d9.h>
//#pragma comment( lib, "d3d11.lib" )
//#pragma comment( lib, "d3dx11.lib" )
//#pragma comment( lib, "d3dx10.lib" )
//#pragma comment( lib, "DXGI.lib" )
//#pragma comment( lib, "d3d9.lib" )

// -------------------------- //
#define CLASSNAME	L"LeviathanWindow"
#define VERSION		0.3701f
#define VERSIONS	L"0.3.7.01"
#define LEVIATHAN

#ifndef DLLEXPORT
#ifdef ENGINE_EXPORTS
#define DLLEXPORT   __declspec( dllexport ) 
#else
#define DLLEXPORT 
#endif // ENGINE_EXPORTS
#endif

namespace Leviathan{
	

	class Object;
	

	class EngineComponent;
}

#include "Types.h"
#include "Misc.h"
#include "CommonMath.h"
//#include "IDFactory.h"
//#include "Convert.h"
// exceptions //
#include "ExceptionBase.h"

#endif