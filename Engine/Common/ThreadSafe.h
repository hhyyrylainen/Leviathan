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

	typedef boost::strict_lock<boost::recursive_mutex> ObjectLock;
	typedef boost::unique_lock<boost::recursive_mutex> UniqueObjectLock;

#define GUARD_LOCK_THIS_OBJECT()			ObjectLock guard(this->ObjectsLock);
#define GUARD_LOCK_OTHER_OBJECT(x)			ObjectLock guard(x->ObjectsLock);
#define GUARD_LOCK_OTHER_OBJECT_NAME(x,y)	ObjectLock y(x->ObjectsLock);
#define UNIQUE_LOCK_OBJECT(x)				UniqueObjectLock lockit(x->ObjectsLock);

	//! \brief Allows the inherited object to be locked
	class ThreadSafe{
	public:
		DLLEXPORT ThreadSafe();
		DLLEXPORT virtual ~ThreadSafe();

		FORCE_INLINE void VerifyLock(ObjectLock &guard) const THROWS{
			// ensure that lock is for this //
			if(!guard.owns_lock(&this->ObjectsLock))
				throw ExceptionInvalidAccess(L"wrong lock owner", 0, __WFUNCTION__, L"lock", L"mismatching lock and object");
		}

		//! The main lock facility, mutable for working with const functions
		//! \note Even though this is not protected it should not be abused
		//! \protected
		mutable boost::recursive_mutex ObjectsLock;
	};

}
#endif
