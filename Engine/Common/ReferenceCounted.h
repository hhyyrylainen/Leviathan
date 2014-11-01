#ifndef LEVIATHAN_REFERENCECOUNTED
#define LEVIATHAN_REFERENCECOUNTED
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <boost/intrusive_ptr.hpp>
#include <boost/atomic.hpp>

namespace Leviathan{

	// macro for adding proxies to hopefully work with scripts //
#define REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(classname) void AddRefProxy(){ \
        this->AddRef(); }; void ReleaseProxy(){ this->Release(); };


	template<class TypeName>
	struct SharedPtrReleaseDeleter{

		static void DoRelease(TypeName* obj){
			obj->Release();
		}
	};


    //! Reference counted object which will be deleted when all references are gone
    //! \note Pointers can be used with ReferenceCounted::pointer ptr = new Object();
    //! \todo Make the reference count be zero in the beginning
	class ReferenceCounted{
	public:

        typedef boost::intrusive_ptr<ReferenceCounted> pointer;
        
		DLLEXPORT inline ReferenceCounted() : RefCount(1){}
		DLLEXPORT virtual ~ReferenceCounted(){}

		DLLEXPORT FORCE_INLINE void AddRef(){

            intrusive_ptr_add_ref(this);
		}
        
		//! removes a reference and deletes the object if reference count reaches zero
		DLLEXPORT FORCE_INLINE void Release(){

            intrusive_ptr_release(this);
        }


	protected:
        
        friend void intrusive_ptr_add_ref(const ReferenceCounted * obj){
            
            obj->RefCount.fetch_add(1, boost::memory_order_relaxed);
        }
        
        friend void intrusive_ptr_release(const ReferenceCounted * obj){
            
            if(obj->RefCount.fetch_sub(1, boost::memory_order_release) == 1){
                boost::atomic_thread_fence(boost::memory_order_acquire);
                delete obj;
            }
        }
        
    private:

        mutable boost::atomic<int32_t> RefCount;
	};

}
#endif
