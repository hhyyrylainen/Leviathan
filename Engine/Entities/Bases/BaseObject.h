#pragma once
// ------------------------------------ //
#include "Include.h"

// This is required for ObjectPtr
#include <boost/intrusive_ptr.hpp>


#include "../../Common/ReferenceCounted.h"
#include "../../Common/ThreadSafe.h"
#include "../../Common/Visitor.h"

namespace Leviathan{

    using ObjectPtr = boost::intrusive_ptr<BaseObject>;

    //! Represents an entity that can be in a world
	class BaseObject : public ReferenceCounted, public virtual ThreadSafe, public Visitable{
        friend GameWorld;
	public:
		//! \brief Default constructor that should never be used for actual objects 
		//!
		//! This is used by classes that aren't actually objects, but virtually inherit this
		DLLEXPORT BaseObject();
        
		//! \brief Use this constructor
		DLLEXPORT BaseObject(int id, GameWorld* worldptr);
		DLLEXPORT virtual ~BaseObject();
		
		//! Called before deletion and should release objects that need to be deleted during world release phase
        //! (like graphical nodes)
		DLLEXPORT virtual void ReleaseData();

		DLLEXPORT inline int GetID(){
			return ID;
		}
		DLLEXPORT inline GameWorld* GetWorld(){
			return OwnedByWorld;
		}

        //! \brief The new custom message function implemented as a visitor
        DLLEXPORT virtual void Accept(Visitor &visitor) = 0;
        
		//! This function is used to avoid explicit dynamic casts when trying to call features on entities
        //! that they might not have
		//! \returns True if the message is acknowledged so that the caller can avoid calling more general types
		DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr) = 0;


		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(BaseObject);

	protected:

        //! \brief Called by GameWorld when it is about to be deleted to stop this object
        //! from accessing it
        void Disown();

        //! \brief Callback for constraintable objects (or other containers) which need to disown their own children
        //! \note This object will be locked before this call
        virtual void _OnDisowned();

        // ------------------------------------ //
        
		int ID;
		// All objects should be in some world (even if not really in a world, then a dummy world) //
		GameWorld* OwnedByWorld;
	};

}

// Bring this common type to global namespace //
using Leviathan::ObjectPtr;


