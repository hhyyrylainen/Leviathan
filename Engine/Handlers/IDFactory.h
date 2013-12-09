#ifndef LEVIATHAN_IDFACTORY
#define LEVIATHAN_IDFACTORY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "boost/thread/lockable_adapter.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "Exceptions/ExceptionInvalidAccess.h"

namespace Leviathan{

	class IDFactory : public boost::basic_lockable_adapter<boost::recursive_mutex>{
	public:
		DLLEXPORT IDFactory();
		DLLEXPORT ~IDFactory();


		DLLEXPORT static inline int GetID(){
			// call on instance pointer //
			return Get()->ProduceID();
		}
		DLLEXPORT static inline int GetSystemID(){
			// call on instance pointer //
			return Get()->ProduceSystemID();
		}

		DLLEXPORT FORCE_INLINE int ProduceID(){
			// we need to lock this object to ensure thread safety //
			boost::strict_lock<IDFactory> guard(*this);
			return ProduceID(guard);
		}
		DLLEXPORT FORCE_INLINE int ProduceSystemID(){
			// we need to lock this object to ensure thread safety //
			boost::strict_lock<IDFactory> guard(*this);
			return ProduceSystemID(guard);
		}

		DLLEXPORT int ProduceID(boost::strict_lock<IDFactory> &guard);
		DLLEXPORT int ProduceSystemID(boost::strict_lock<IDFactory> &guard);


		DLLEXPORT static IDFactory* Get();

	private:
		FORCE_INLINE void VerifyLock(boost::strict_lock<IDFactory> &guard) THROWS{
			// ensure that lock is for this //
			if(!guard.owns_lock(this))
				throw ExceptionInvalidAccess(L"wrong lock owner", 0, __WFUNCTION__, L"lock", L"mismatching lock and object");
		}
		// ------------------------------------ //
		int SystemID;
		int GlobalID;



		static IDFactory* Instance;
	};

}
#endif
