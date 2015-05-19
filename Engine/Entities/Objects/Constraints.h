#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "../../Common/Types.h"
#include "../Components.h"

namespace Leviathan{ namespace Entity{

        //! Holds built-in types of constraints, used to identify types over the network
        enum ENTITY_CONSTRAINT_TYPE : int32_t
        {
            //! Type is SliderConstraint
            ENTITY_CONSTRAINT_TYPE_SLIDER = 1,

            
            //! This type is used to connect entities to various controllers
            //! like TrackEntityController
            ENTITY_CONSTRAINT_TYPE_CONTROLLERCONSTRAINT,
                
            ENTITY_CONSTRAINT_TYPE_CUSTOM = 1000
        };

        
        //! Base class for all different types of constraints to inherit
        //! \todo Make this actually ThreadSafe
        class BaseConstraint : public ThreadSafe{
        public:
            DLLEXPORT BaseConstraint(ENTITY_CONSTRAINT_TYPE type, GameWorld* world,
                Constraintable &first, Constraintable &second);
            DLLEXPORT virtual ~BaseConstraint();
            
            //! \brief Actually creates the Newton joint.
            //!
            //! The Constraint won't work without calling this
            //! \pre The SetParameters method of the child class is called
            //! \todo Allow the error messages to be silenced
            DLLEXPORT virtual bool Init();
            
            //! \brief Calls the Newton destroy function
            DLLEXPORT void Release();

            //! Called when either one of constraint parts wants to disconnect
            //!
            //! Destroys the entire constraint
            //! \param callinginstance Ptr to either the parent or child is used to skip call to it
            //! (the destructor there is already running)
            DLLEXPORT void ConstraintPartUnlinkedDestroy(Constraintable* callinginstance);
            
            DLLEXPORT inline ENTITY_CONSTRAINT_TYPE GetType() const{
                return Type;
            }

            //! \brief Gets the first object
            DLLEXPORT Constraintable& GetFirstEntity() const;

            //! \brief Gets the second object
            DLLEXPORT Constraintable& GetSecondEntity() const;

            
        protected:
            // called to verify params before init proceeds //
            DLLEXPORT virtual bool _CheckParameters();
            DLLEXPORT virtual bool _CreateActualJoint();

            // ------------------------------------ //
            Constraintable& FirstObject;
            Constraintable& SecondObject;
            
            //! \note World is a direct ptr since all joints MUST be destroyed before the
            //! world is released
            GameWorld* OwningWorld;
            NewtonJoint* Joint;

            //! Constraint type used by serializers to identify the type
            ENTITY_CONSTRAINT_TYPE Type;
        };

        // Different types of constraints //

        //! Constraint that allows motion along a single axis
        //! \note The axis has to be normalized otherwise Init fails
        class SliderConstraint : public BaseConstraint{
        public:
            DLLEXPORT SliderConstraint(GameWorld* world, Constraintable &first,
                Constraintable &second);
            DLLEXPORT virtual ~SliderConstraint();


            //! Call this before init to set the right parameters
            //! \param slidingaxis Is a normalized axis in global coordinates and defines
            //! the axis along which
            //! the object can move
            DLLEXPORT SliderConstraint* SetParameters(const Float3 &slidingaxis);


            //! \brief Returns the axis along which this joint can move
            DLLEXPORT Float3 GetAxis() const;
            
        protected:
            virtual bool _CheckParameters() override;
            virtual bool _CreateActualJoint() override;
            // ------------------------------------ //

            // Stored parameters //
            Float3 Axis;

        };
        
    }
}

