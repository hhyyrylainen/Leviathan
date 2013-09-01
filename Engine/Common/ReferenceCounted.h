#ifndef LEVIATHAN_REFERENCECOUNTED
#define LEVIATHAN_REFERENCECOUNTED
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions\ExceptionInvalidAccess.h"

#include "boost/thread/thread.hpp"
#include "boost/thread/lockable_adapter.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "boost/thread/strict_lock.hpp"

namespace Leviathan{

	// macro for adding proxies to hopefully work with scripts //
#define REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(classname) void AddRefProxy(){ this->AddRef(); }; void ReleaseProxy(){ this->Release(); };


	class ReferenceCounted : public boost::basic_lockable_adapter<boost::recursive_mutex>{
	public:
		ReferenceCounted();
		virtual ~ReferenceCounted();

		__forceinline void VerifyLock(boost::strict_lock<ReferenceCounted> &guard) throw(...){
			// ensure that lock is for this //
			if(!guard.owns_lock(this))
				throw ExceptionInvalidAccess(L"wrong lock owner", 0, __WFUNCTION__, L"lock", L"mismatching lock and object");
		}

		__forceinline void AddRef(boost::strict_lock<ReferenceCounted> &guard){
			VerifyLock(guard);
			// we are safely locked and can increment the reference count //
			RefCount++;
		}
		// add a reference with internal locking //
		DLLEXPORT inline void AddRef(){
			// we need to lock this object to ensure thread safety //
			boost::strict_lock<ReferenceCounted> guard(*this);
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