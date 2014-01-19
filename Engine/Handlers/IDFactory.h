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
		void VerifyLock(boost::strict_lock<IDFactory> &guard) THROWS;
		// ------------------------------------ //
		int SystemID;
		int GlobalID;



		static IDFactory* Instance;
	};

}
#endif
