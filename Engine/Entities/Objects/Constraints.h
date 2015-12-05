#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "../../Common/Types.h"
#include "../Components.h"

namespace Leviathan{

    //! Holds built-in types of constraints, used to identify types over the network
    enum ENTITY_CONSTRAINT_TYPE : int32_t
    {
        //! Type is SliderConstraint
        ENTITY_CONSTRAINT_TYPE_SLIDER = 1,
            
            
        //! This type is used to connect entities to various controllers
        //! like TrackEntityController
        ENTITY_CONSTRAINT_TYPE_CONTROLLERCONSTRAINT,

        
         
        //! Constraint has been destructed, used to not allow
        //! destruction multiple times
        ENTITY_CONSTRAINT_TYPE_DESTRUCTED,
            
        ENTITY_CONSTRAINT_TYPE_CUSTOM = 1000
    };

    //! \brief Base class for constraint data types, used to serialize constraint states to
    //! state objects
    class ConstraintData{
    public:
            

    };

        
    //! Base class for all different types of constraints to inherit
    //! \todo Make this actually ThreadSafe
    class BaseConstraint : public ThreadSafe{
    public:
        DLLEXPORT BaseConstraint(ENTITY_CONSTRAINT_TYPE type, GameWorld* world,
            Constraintable &first, Constraintable &second);

        //! \brief Variant for forcing id
        //!
        //! Used for receiving from network
        DLLEXPORT BaseConstraint(ENTITY_CONSTRAINT_TYPE type, GameWorld* world,
            Constraintable &first, Constraintable &second, int id);
        
        
        DLLEXPORT virtual ~BaseConstraint();
            
        //! \brief Actually creates the Newton joint.
        //!
        //! The Constraint won't work without calling this
        //! \pre The SetParameters method of the child class is called
        //! \todo Allow the error messages to be silenced
        DLLEXPORT virtual bool Init();
            
        //! \brief Calls the Newton destroy function
        DLLEXPORT void Release();

        //! \brief Call to destroy this constraint
        //! \param skipthis Skips removing from this side, avoid
        //! unlinking multiple times from one side when it is being
        //! destructed
        DLLEXPORT void Destroy(Constraintable* skipthis = nullptr);
            
        DLLEXPORT inline ENTITY_CONSTRAINT_TYPE GetType() const{
            return Type;
        }

        //! \brief Gets the first object
        DLLEXPORT Constraintable& GetFirstEntity() const;

        //! \brief Gets the second object
        DLLEXPORT Constraintable& GetSecondEntity() const;

        //! \brief Returns ID
        DLLEXPORT int GetID() const;

    protected:
        // called to verify params before init proceeds //
        DLLEXPORT virtual bool _CheckParameters();
        DLLEXPORT virtual bool _CreateActualJoint();

        // ------------------------------------ //
        Constraintable& FirstObject;
        Constraintable& SecondObject;

        //! Server and client matching identifier for deleting constraints
        int ID;
            
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

        //! \brief Variant for forcing id
        //!
        //! Used for receiving from network
        DLLEXPORT SliderConstraint(GameWorld* world, Constraintable &first,
            Constraintable &second, int id);
        
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

