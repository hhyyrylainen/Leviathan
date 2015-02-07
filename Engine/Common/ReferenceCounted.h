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
    //! \todo Make sure that all functions using intrusive pointers use the MakeIntrusive function
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

        //! \brief Creates an intrusive_ptr from raw pointer
        template<class ActualType>
        DLLEXPORT static boost::intrusive_ptr<ActualType> MakeShared(ActualType* ptr){

            if(!ptr)
                return nullptr;

            boost::intrusive_ptr<ActualType> newptr(ptr);
            ptr->Release();

            return newptr;
        }

        //! \brief Returns the reference count
        //! \todo Make sure that the right memory order is used
        DLLEXPORT int32_t GetRefCount() const{
            
            return RefCount.load(boost::memory_order_acquire);
        }


	protected:
        
        DLLEXPORT friend void intrusive_ptr_add_ref(const ReferenceCounted * obj){
            
            obj->RefCount.fetch_add(1, boost::memory_order_relaxed);
        }
        
        DLLEXPORT friend void intrusive_ptr_release(const ReferenceCounted * obj){
            
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
