#pragma once
// ------------------------------------ //
#include "Include.h"
#include "../../Exceptions.h"
#include "../../Common/ReferenceCounted.h"
#include "../../Common/ThreadSafe.h"

namespace Leviathan{

    //! \brief Class for Scripts to receive and pass on references to locks in native or script
    //! stacks
    template<class LockType>
	class ScriptLockPasser : public ReferenceCounted{
	public:

        DLLEXPORT ScriptLockPasser(LockType &objectsguard) : OurLockPtr(&objectsguard){

            
        }

        //! \brief Creates a passer that doesn't have a lock
        DLLEXPORT ScriptLockPasser() : OurLockPtr(nullptr){

        }

        DLLEXPORT ~ScriptLockPasser(){

            OurLockPtr = nullptr;
        }

        //! \brief Returns a reference to held lock
        //! \exception InvalidState if this object doesn't hold a reference to a lock
        //!
        //! Calling this while MakeInvalid is called or this is being destructed is invalid
        DLLEXPORT auto GetLock(){

            auto ptr = OurLockPtr;
            if(!ptr)
                throw InvalidState("ScriptLockPasser doesn't have a lock pointer");
            
            return std::move(*ptr);
        }

        //! \brief Makes this invalid
        //!
        //! Call before destructing the lock to which OurLockPtr points to
        DLLEXPORT void MakeInvalid(){

            OurLockPtr = nullptr;
        }

    private:

        //! Our pointer to the lock that is upper in the callstack 
        //! This has to be valid while this object is active
        LockType* OurLockPtr;
	};

    //! \brief Allows scripts to create locks and easily convert them to LockPasser types
    template<class LockType>
    class ScriptLockHolder : public ReferenceCounted, public ThreadSafe{

        template<class MutexType>
        DLLEXPORT ScriptLockHolder(MutexType &mutex){

            OurLock = std::move(LockType(mutex));
        }

        DLLEXPORT ~ScriptLockHolder(){

            if(CreatedPassers.empty())
                return;

            // Release our created passers and mark ones that have external references invalid
            for(auto& passptr : CreatedPassers){

                if(passptr->GetRefCount() > 1)
                    passptr->MakeInvalid();

                passptr->Release();
            }

            CreatedPassers.clear();
        }

        //! \brief Creates a new passer
        //!
        //! If this is destructed the passer will be made invalid
        DLLEXPORT ScriptLockPasser<LockType>* CreateLockReference(){

            auto obj = std::move(std::make_shared<ScriptLockPasser<LockType>>(OurLock));

            // We will also keep a reference
            obj->AddRef();

            {
                GUARD_LOCK();

                CreatedPassers.push_back(obj.get());
            }

            return obj.release();
        }

    private:

        //! Our actual lock object to which ScriptLockPassers can reference
        LockType OurLock;

        //! List of created passers that need to be reset if we are destructed
        //! \todo Find a better way to do this
        std::vector<ScriptLockPasser<LockType>*> CreatedPassers;
    };
}
