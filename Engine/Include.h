#ifndef LEVIATHAN_INCLUDE
#define LEVIATHAN_INCLUDE

#include <stdio.h>

// we need to disable this warning to stop stdint and instsafe causing errors //
#pragma warning (disable: 4005)

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

// common ogre files //
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

// temporarily have this here to reduce compile errors
#include <D3D11.h>

// pragma include libraries

// AngelAddons use std style names
#define AS_USE_STLNAMES     1

#ifdef ENGINE_EXPORTS
#ifdef _DEBUG
// debug versions //
#pragma comment(lib, "angelscriptd.lib")
#pragma comment(lib, "Leapd.lib")
#pragma comment(lib, "OgreMain_d.lib")
#pragma comment(lib, "OgreOverlay_d.lib")
#pragma comment(lib, "Plugin_CgProgramManager_d.lib")
#pragma comment(lib, "Plugin_OctreeZone_d.lib")
#pragma comment(lib, "Plugin_ParticleFX_d.lib")
#pragma comment(lib, "RenderSystem_Direct3D11_d.lib")
#pragma comment(lib, "RenderSystem_GL_d.lib")
#else
// release libraries
#pragma comment(lib, "angelscript.lib")
#pragma comment(lib, "Leap.lib")
#pragma comment(lib, "OgreMain.lib")
#pragma comment(lib, "OgreOverlay.lib")
#pragma comment(lib, "Plugin_CgProgramManager.lib")
#pragma comment(lib, "Plugin_OctreeZone.lib")
#pragma comment(lib, "Plugin_ParticleFX.lib")
#pragma comment(lib, "RenderSystem_Direct3D11.lib")
#pragma comment(lib, "RenderSystem_GL.lib")
#endif
#else
// include engine lib
#ifdef _DEBUG
#pragma comment(lib, "EngineD.lib")
#else
#pragma comment(lib, "Engine.lib")
#endif

#endif // ENGINE_EXPORTS

using namespace std;
// -------------------------- //
#define CLASSNAME	L"LeviathanWindow"

#define LEVIATHAN_VERSION 0.502
#define LEVIATHAN_VERSIONS L"0.5.0.2"
#define LEVIATHAN_VERSION_ANSIS "0.5.0.2"
/* #undef USE_MYMATH */

#define VERSION		LEVIATHAN_VERSION
#define VERSIONS	LEVIATHAN_VERSIONS

#define LEVIATHAN

#define PI 3.14159265f
#define DEGREES_TO_RADIANS		PI/180.f
#define EPSILON		0.00000001f
#define VAL_NOUPDATE	-1333678
#define OBJECT_SMOOTH	4
#define SHADER_DEBUG 1

#define UNIT_SCALE		1000
// throw on Error message //
//#define THROW_ON_PRINTERROR

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

#define SSLINE2(x) #x
#define SSLINE(x) SSLINE2(x)
#define __SLINE__ SSLINE(__LINE__)


#define __SWLINE__ WIDEN(__SLINE__)



#define __WDATE__ WIDEN(__DATE__)
#define __WTIME__ WIDEN(__TIME__)
#define __WFUNCSIG__ WIDEN(__FUNCSIG__)

#define SAFE_DELETE_VECTOR(x) while((x).size() != 0){if((x)[0]){delete (x)[0];}(x).erase((x).begin());}

#define STDCONTAINERERASELAST_PTR(x) x->erase(x->begin()+x->size()-1)
#define STDCONTAINERERASELAST(x) x.erase(x.begin()+x.size()-1)

#define QUICK_ERROR_MESSAGE {Logger::Get()->Error((L"Undocumented error: " __WFILE__ L" function: "__WFUNCTION__ L" line: " __SWLINE__ ), true);}
#define QUICK_MEMORY_ERROR_MESSAGE {Logger::Get()->Error((L"Out of memory error from: " __WFILE__ L" function: "__WFUNCTION__), true);}



namespace Leviathan{
	

	class Object;
	

	class EngineComponent;
}

#include "Common\ErrorTypes.h"

#include "Logger.h"

#include "Common\Types.h"
#include "Common\Misc.h"
#include "Math\CommonMath.h"
#include "Utility\Convert.h"

#include "Handlers\IDFactory.h"
// exceptions //
#include "Exceptions\ExceptionBase.h"

#include "Statistics\TimingMonitor.h"
// speed up compile times with leap //
#include "leap.h"

#endif
