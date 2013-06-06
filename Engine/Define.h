#ifndef LEVIATHAN_DEFINE
#define LEVIATHAN_DEFINE
#ifndef LEVIATHAN_INCLUDE
#include "Include.h"
#endif



#define SAFE_RELEASE( x ) {if(x){(x)->Release();(x)=NULL;}}
#define SAFE_RELEASEDEL( x ) {if(x){(x)->Release();delete (x);(x)=NULL;}}
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=NULL;}}
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=NULL;}}
#define EXISTS( x ) {( (x) != NULL )}

#define ARR_INDEX_CHECK( x, y) if(((x) >= 0 ) && ((x) < (y)))
#define ARR_INDEX_CHECKINV( x, y) if(!(((x) >= 0 ) && ((x) < (y))))
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

#define QUICK_ERROR_MESSAGE {Logger::Get()->Error((L"Undocumented error: " __WFILE__ L" function: "__WFUNCTION__ L" line: "+__LINE__ ), true);}
#define QUICK_MEMORY_ERROR_MESSAGE {Logger::Get()->Error((L"Out of memory error from: " __WFILE__ L" function: "__WFUNCTION__), true);}

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
#define DEBUG_OUTPUT_AUTO(s)	{Logger::SendDebugMessage(wstring(L"[INFO] "L#s L"\n"));}
//#define DEBUG_OUTPUT_AUTO(s)	{Logger::SendDebugMessage(wstring(L"[INFO] \n"));}
#else
#define DEBUG_OUTPUT_AUTO(s)   {}
#endif //DEBUG || _DEBUG
#endif

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)	{}
#endif //DEBUG || _DEBUG
#endif

//#define NUM_THREADS 4

// throw on Error message //
//#define THROW_ON_PRINTERROR


#define PI 3.14159265
#define EPSILON		0.00000001
#define VAL_NOUPDATE	-1333678
#define UNIT_SCALE		1000
#define UNIT_SCALE_HALF	(UNIT_SCALE/2)
#define OBJECT_SMOOTH	4



#define SHADER_DEBUG 1

#ifdef SHADER_DEBUG
#define SHADER_COMPILE_PREFERFLOW 1
#define SHADER_COMPILE_SKIP_OPTIMIZE 1
#define SHADER_COMPILE_DEBUG 1
#endif

// some macros //
//#define mGETVALD(xtype,x) (xtype) Get(x)( );
//#define mGETVAL(classn,xtype,x) (xtype) classn::Get(x)( ){ return this->x; };

#define SAFE_DELETE_VECTOR(x) while((x).size() != 0){if((x)[0]){delete (x)[0];}(x).erase((x).begin());}

#define STDCONTAINERERASELAST_PTR(x) x->erase(x->begin()+x->size()-1)
#define STDCONTAINERERASELAST(x) x.erase(x.begin()+x.size()-1)

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

namespace Leviathan{
	
	template<class T>
	void SafeReleaser(T* obj){
		SAFE_RELEASE(obj);
	}
	template<class T>
	void SafeReleaseDeleter(T* obj){
		SAFE_RELEASEDEL(obj);
	}

	class Object{
	public:
		DLLEXPORT Object();;
		DLLEXPORT virtual ~Object();
		DLLEXPORT virtual bool IsThis(Object* compare);

	protected:

	};
	

	class EngineComponent : public Object{
	public:
		DLLEXPORT EngineComponent();

		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual bool Release(bool all);
		DLLEXPORT virtual bool IsInited();
	protected:
		bool Inited;

	};
}
#include "ErrorTypes.h"

#include "Convert.h"
#include "Logger.h"
#include "IDFactory.h"

#endif