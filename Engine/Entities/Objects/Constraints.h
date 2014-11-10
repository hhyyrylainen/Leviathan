#ifndef LEVIATHAN_CONSTRAINTS
#define LEVIATHAN_CONSTRAINTS
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/ThreadSafe.h"

namespace Leviathan{ namespace Entity{

        //! Holds built-in types of constraints, used to identify types over the network
        enum ENTITY_CONSTRAINT_TYPE : int32_t
        {
            //! Type is SliderConstraint
            ENTITY_CONSTRAINT_TYPE_SLIDER = 1,

                
            ENTITY_CONSTRAINT_TYPE_CUSTOM = 1000
        };

        
        //! Base class for all different types of constraints to inherit
        //! \todo Make this actually ThreadSafe
        class BaseConstraint : public ThreadSafe{
        public:
            DLLEXPORT BaseConstraint(GameWorld* world, BaseConstraintable* parent, BaseConstraintable* child);
            DLLEXPORT virtual ~BaseConstraint();

            //! \brief Actually creates the Newton joint.
            //!
            //! The Constraint won't work without calling this
            //! \pre The SetParameters method of the child class is called
            DLLEXPORT bool Init();
            
            //! \brief Calls the Newton destroy function
            DLLEXPORT void Release();

            //! Called when either one of constraint parts wants to disconnect
            //!
            //! Destroys the entire constraint
            //! \param callinginstance Ptr to either the parent or child is used to skip call to it
            //! (the destructor there is already running)
            DLLEXPORT void ConstraintPartUnlinkedDestroy(BaseConstraintable* callinginstance);

            DLLEXPORT inline ENTITY_CONSTRAINT_TYPE GetType() const{
                return Type;
            }

            //! \brief Gets the first object
            //! \return The object, this is always non-NULL while not being destructed
            DLLEXPORT BaseConstraintable* GetFirstEntity() const;

            //! \brief Gets the second object
            //! \return The object, this is may be NULL in special constraints
            DLLEXPORT BaseConstraintable* GetSecondEntity() const;

            
        protected:
            // called to verify params before init proceeds //
            virtual bool _CheckParameters() = 0;
            virtual bool _CreateActualJoint() = 0;
            
            // ------------------------------------ //
            BaseConstraintable* ParentObject;
            BaseConstraintable* ChildObject;
            
            //! \note World is a direct ptr since all joints MUST be destroyed before the world is released
            GameWorld* OwningWorld;
            NewtonJoint* Joint;

            //! Constraint type used by serializers to identify the type
            ENTITY_CONSTRAINT_TYPE Type;
        };

        // Different types of constraints //

        //! Constraint that allows motion along a single axis
        class SliderConstraint : public BaseConstraint{
        public:
            DLLEXPORT SliderConstraint(GameWorld* world, BaseConstraintable* parent, BaseConstraintable* child);
            DLLEXPORT virtual ~SliderConstraint();


            //! Call this before init to set the right parameters
            //! \param slidingaxis Is a normalized axis in global coordinates and defines the axis along which
            //! the object can move
            DLLEXPORT BaseConstraint* SetParameters(const Float3 &slidingaxis);


        protected:
            virtual bool _CheckParameters() override;
            virtual bool _CreateActualJoint() override;
            // ------------------------------------ //

            // stored parameters //
            Float3 Axis;

        };




    }
}
#endif
