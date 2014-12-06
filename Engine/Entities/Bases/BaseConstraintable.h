#ifndef LEVIATHAN_BASECONSTRAINTABLE
#define LEVIATHAN_BASECONSTRAINTABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "../Objects/Constraints.h"
#include "BaseObject.h"


namespace Leviathan{

    struct EntitysConstraintEntry{

        EntitysConstraintEntry(weak_ptr<Entity::BaseConstraint> ischild, BaseConstraintable* owner) :
            IsParent(false), ChildPartPtr(ischild), OwningInstance(owner)
        {
        };
        EntitysConstraintEntry(shared_ptr<Entity::BaseConstraint> isparent, BaseConstraintable* owner) :
            IsParent(true), ParentPtr(isparent), OwningInstance(owner)
        {
        };
        // destructor notifies the constraint object to unlink it's references (because might as well
        // destroy constraint when either one disconnects)
        ~EntitysConstraintEntry(){
            if(IsParent){
                if(ParentPtr)
                    ParentPtr->ConstraintPartUnlinkedDestroy(OwningInstance);
            } else {
                auto tmp = ChildPartPtr.lock();
                if(tmp){
                    tmp->ConstraintPartUnlinkedDestroy(OwningInstance);
                }
            }
        }

        BaseConstraintable* OwningInstance;
        bool IsParent;
        // above flag determines which one is set //
        weak_ptr<Entity::BaseConstraint> ChildPartPtr;
        shared_ptr<Entity::BaseConstraint> ParentPtr;

    };

    //! \brief Entities that inherit this can be a part of a constraint
    //!
    //! Classes should also inherit BasePhysicsObject, but it isn't required for certain constraints
    class BaseConstraintable : virtual public BaseObject{
        friend Entity::BaseConstraint;
    public:
        DLLEXPORT BaseConstraintable();
        DLLEXPORT virtual ~BaseConstraintable();

        // only callable on the parent, which then calls child to unlink it's reference //
        DLLEXPORT bool UnlinkConstraint(shared_ptr<Entity::BaseConstraint> constraintptr);
            
        // unlinks all constraints which this object has whether it is parent or child
        // will also release all the constraints
        DLLEXPORT void AggressiveConstraintUnlink();

        DLLEXPORT shared_ptr<Entity::BaseConstraint> GetConstraintPtr(Entity::BaseConstraint*
            unsafeptr);

        //! Creates a constraint between this and another object
        //! \warning DO NOT store the returned value (since that reference isn't counted)
        //! \note Before the constraint is actually finished, you need to call ->SetParameters() on the returned ptr
        //! and then ->Init() and then let go of the ptr
        //! \note If you do not want to allow constraints where child is NULL you have to check if child is
        //! NULL before calling this function
        template<class ConstraintClass>
        DLLEXPORT ConstraintClass* CreateConstraintWith(BaseConstraintable* child){

            // there might be some other way to do this, but we can dynamic cast to an object
            // that has the world ptr stored
            BaseObject* tmpbase = dynamic_cast<BaseObject*>(this);
            assert(tmpbase != NULL && "constraintable must be in an inherited class in a class that also inherits "
                "BaseObject");


            shared_ptr<ConstraintClass> tmpconstraint(new ConstraintClass(tmpbase->GetWorld(), this, child));
            
            // Set own parent ptr //
            AddConstraintWhereThisIsParent(tmpconstraint);
            
            // Add a reference to the child object (if there is one) //
            if(child)
                child->AddConstraintWhereThisIsChild(tmpconstraint);

            // Return direct ptr for the caller to actually create the joint //
            return tmpconstraint.get();
        }

        //! \brief Returns the number of connections this object is part in
        DLLEXPORT size_t GetConstraintCount() const;

        //! \brief Returns the constraint at index
        DLLEXPORT shared_ptr<Entity::BaseConstraint> GetConstraint(size_t index) const;
        
        
        // notify functions //
        DLLEXPORT void AddConstraintWhereThisIsChild(weak_ptr<Entity::BaseConstraint>
            constraintptr);
        DLLEXPORT void AddConstraintWhereThisIsParent(shared_ptr<Entity::BaseConstraint>
            constraintptr);
        DLLEXPORT void OnRemoveConstraint(Entity::BaseConstraint* tmpptr);

    protected:
        // over loadable notify functions //
        virtual void _OnConstraintAdded(Entity::BaseConstraint* ptr);
        virtual void _OnConstraintUnlink(Entity::BaseConstraint* ptr);

        // called by the joint object //
        void ConstraintDestroyedRemove(Entity::BaseConstraint* ptr);
        // ------------------------------------ //
        
        // for listing of constraints //
        std::vector<EntitysConstraintEntry*> PartInConstraints;
    };
}
#endif
