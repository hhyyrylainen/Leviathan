#ifndef LEVIATHAN_REFERENCECOUNTED
#define LEVIATHAN_REFERENCECOUNTED
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ThreadSafe.h"

namespace Leviathan{

	// macro for adding proxies to hopefully work with scripts //
#define REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(classname) void AddRefProxy(){ this->AddRef(); }; void ReleaseProxy(){ this->Release(); };

	// TODO: implement generic calling convention and make this virtually inherited //
	class ReferenceCounted : /*virtual*/ public ThreadSafe{
	public:
		ReferenceCounted();
		virtual ~ReferenceCounted();

		__forceinline void AddRef(ObjectLock &guard){
			VerifyLock(guard);
			// we are safely locked and can increment the reference count //
			RefCount++;
		}
		// add a reference with internal locking //
		DLLEXPORT inline void AddRef(){
			// we need to lock this object to ensure thread safety //
			ObjectLock guard(*this);
			AddRef(guard);
		}
		// removes a reference and deletes the object if reference count reaches zero //
		DLLEXPORT void Release();


	protected:
		// flag used to stop release if we are already being deleted //
		bool ToDelete : 1;
		volatile __int32 RefCount;
	};

}
#endif