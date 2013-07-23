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
#pragma warning (disable:6387)
#include <afxcontrolbars.h>
#pragma warning (default:6387)
#endif

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
#include <regex>

#include <initguid.h>
#include <shlobj.h>

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
#include <D3DX10core.h>
#include <D3D11Shader.h>

#include <d3d9.h>
// -------------------------- //
#define CLASSNAME	L"LeviathanWindow"
#define VERSION		0.391f
#define VERSIONS	L"0.3.9.1"
#define LEVIATHAN

#define PI 3.14159265
#define EPSILON		0.00000001
#define VAL_NOUPDATE	-1333678
#define UNIT_SCALE		1000
#define UNIT_SCALE_HALF	(UNIT_SCALE/2)
#define OBJECT_SMOOTH	4

#define SHADER_DEBUG 1

// throw on Error message //
//#define THROW_ON_PRINTERROR

//#define NUM_THREADS 4

#ifdef SHADER_DEBUG
#define SHADER_COMPILE_PREFERFLOW 1
#define SHADER_COMPILE_SKIP_OPTIMIZE 1
#define SHADER_COMPILE_DEBUG 1
#endif

#define FORCE_INLINE __forceinline

#ifdef _MSC_VER
#ifdef _DEBUG
#define DEBUG_BREAK __debugbreak();
#else
#define DEBUG_BREAK 
#endif //_DEBUG
#else
#error Debug break won't work
#endif //_MSC_VER

#ifndef DEBUG_OUTPUT
#if defined( DEBUG ) || defined( _DEBUG )
#define DEBUG_OUTPUT(s)	{Logger::SendDebugMessage(s);}
#else
#define DEBUG_OUTPUT(s)   {}
#endif //DEBUG || _DEBUG
#endif

#ifndef DEBUG_OUTPUT_AUTO
#if defined( DEBUG ) || defined( _DEBUG )
//#define DEBUG_OUTPUT_AUTO(s)	{Logger::SendDebugMessage(wstring(L"[INFO] "L#s L"\n"));}
#define DEBUG_OUTPUT_AUTO(s)	{Logger::SendDebugMessage(wstring(L"[INFO] "+s+L"\n"));}
#else
#define DEBUG_OUTPUT_AUTO(s)   {}
#endif //DEBUG || _DEBUG
#endif

#ifndef DEBUG_OUTPUT_AUTOPLAINTEXT
#if defined( DEBUG ) || defined( _DEBUG )
#define DEBUG_OUTPUT_AUTOPLAINTEXT(s)	{Logger::SendDebugMessage(wstring(L"[INFO] "L#s L"\n"));}
#else
#define DEBUG_OUTPUT_AUTOPLAINTEXT(s)   {}
#endif //DEBUG || _DEBUG
#endif


#ifndef DLLEXPORT
#ifdef ENGINE_EXPORTS
#define DLLEXPORT   __declspec( dllexport ) 
#else
#define DLLEXPORT 
#endif // ENGINE_EXPORTS
#endif


#ifdef _DEBUG
#define MONITOR_FUNCTION_TIME_FSTART wstring functiontimemonitorname = __WFUNCTION__ L" time monitor"; TimingMonitor::StartTiming(functiontimemonitorname);
#else
#define MONITOR_FUNCTION_TIME_FSTART {}
#endif
#ifdef _DEBUG
#define MONITOR_FUNCTION_TIME_FSTOP {TimingMonitor::StopTiming(functiontimemonitorname, true);}
#else
#define MONITOR_FUNCTION_TIME_FSTOP {}
#endif

#define SAFE_RELEASE( x ) {if(x){(x)->Release();(x)=NULL;}}
#define SAFE_RELEASEDEL( x ) {if(x){(x)->Release();delete (x);(x)=NULL;}}
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=NULL;}}
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=NULL;}}
#define EXISTS( x ) {( (x) != NULL )}

#define ARR_INDEX_CHECK( x, y) if(((x) >= 0 ) && ((size_t)(x) < (y)))
#define ARR_INDEX_CHECKINV( x, y) if(!(((x) >= 0 ) && ((size_t)(x) < (y))))
#define VECTOR_LAST(x) (x).at((x).size()-1)
#define VECTOR_LASTP(x) (x)->at((x)->size()-1)
#define CLASS_ALLOC_CHECK(x) if(!(x)){Logger::Get()->Error(L"008");return false;}

#define FORCE_POSITIVE(x) {if((x) < 0){ (x)*=-1; }}
#define FORCE_POSITIVEF(x) {if((x) < 0){ (x)*=-1.0f; }}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)
#define __WDATE__ WIDEN(__DATE__)
#define __WTIME__ WIDEN(__TIME__)
#define __WFUNCSIG__ WIDEN(__FUNCSIG__)


#define SAFE_DELETE_VECTOR(x) while((x).size() != 0){if((x)[0]){delete (x)[0];}(x).erase((x).begin());}

#define STDCONTAINERERASELAST_PTR(x) x->erase(x->begin()+x->size()-1)
#define STDCONTAINERERASELAST(x) x.erase(x.begin()+x.size()-1)

#define QUICK_ERROR_MESSAGE {Logger::Get()->Error((L"Undocumented error: " __WFILE__ L" function: "__WFUNCTION__ L" line: "+__LINE__ ), true);}
#define QUICK_MEMORY_ERROR_MESSAGE {Logger::Get()->Error((L"Out of memory error from: " __WFILE__ L" function: "__WFUNCTION__), true);}



namespace Leviathan{
	

	class Object;
	

	class EngineComponent;
}

#include "ErrorTypes.h"

#include "Logger.h"

#include "Types.h"
#include "Misc.h"
#include ".\Math\CommonMath.h"
#include "Convert.h"

#include "IDFactory.h"
// exceptions //
#include "ExceptionBase.h"

#include "TimingMonitor.h"

#endif