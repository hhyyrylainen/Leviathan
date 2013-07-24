#ifndef LEVIATHAN_DEFINE
#define LEVIATHAN_DEFINE
#ifndef LEVIATHAN_INCLUDE
#include "Include.h"
#endif

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)	{}
#endif //DEBUG || _DEBUG
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
	// has no virtual destructor, objects may not be pointed by this base class //
	class EngineComponent : public Object{
	public:
		DLLEXPORT EngineComponent();

		DLLEXPORT virtual bool Init();
		DLLEXPORT virtual void Release();
		DLLEXPORT inline bool IsInited(){
			return Inited;
		}
	protected:
		bool Inited;

	};
}




#endif