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


	template<class TypeName>
	struct SharedPtrReleaseDeleter{

		static void DoRelease(TypeName* obj){
			obj->Release();
		}
	};


	// \todo implement generic calling convention and make this virtually inherited //
	class ReferenceCounted : /*virtual*/ public ThreadSafe{
	public:
		ReferenceCounted();
		virtual ~ReferenceCounted();

		FORCE_INLINE void AddRef(ObjectLock &guard){
			VerifyLock(guard);
			// we are safely locked and can increment the reference count //
			RefCount++;
		}
		//! add a reference with internal locking //
		DLLEXPORT inline void AddRef(){
			// we need to lock this object to ensure thread safety //
			ObjectLock guard(*this);
			AddRef(guard);
		}
		//! removes a reference and deletes the object if reference count reaches zero //
		DLLEXPORT void Release();


		//! \brief Sets the reference count to a specific value
		//! \warning Only use this if you know you have the only reference
		//! \note This should only be used with AngelScript functions that increase reference, but isn't wanted
		DLLEXPORT void UnsafeSetReferences(int value);

	protected:
		// flag used to stop release if we are already being deleted //
		bool ToDelete : 1;
		volatile __int32 RefCount;
	};

}
#endif
