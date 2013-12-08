#ifndef LEVIATHAN_THREADSAFE
#define LEVIATHAN_THREADSAFE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Exceptions/ExceptionInvalidAccess.h"
#include "boost/thread/lockable_adapter.hpp"
#include "boost/thread/recursive_mutex.hpp"


namespace Leviathan{

	typedef boost::strict_lock<ThreadSafe> ObjectLock;

	class ThreadSafe : public boost::basic_lockable_adapter<boost::recursive_mutex>{
	public:
		DLLEXPORT ThreadSafe();
		DLLEXPORT virtual ~ThreadSafe();

		__forceinline void VerifyLock(ObjectLock &guard) throw(...){
			// ensure that lock is for this //
			if(!guard.owns_lock(this))
				throw ExceptionInvalidAccess(L"wrong lock owner", 0, __WFUNCTION__, L"lock", L"mismatching lock and object");
		}
	};

}
#endif