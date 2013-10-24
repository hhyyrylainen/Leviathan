#ifndef LEVIATHAN_BASECONSTRAINTABLE
#define LEVIATHAN_BASECONSTRAINTABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "..\Objects\Constraints.h"
#include "BasePhysicsObject.h"


namespace Leviathan{ namespace Entity{

	class BaseContraintable;

	struct EntitysContraintEntry{

		EntitysContraintEntry(weak_ptr<BaseContraint> ischild, BaseContraintable* owner) : IsParent(false), ChildPartPtr(ischild), OwningInstance(owner){
		};
		EntitysContraintEntry(shared_ptr<BaseContraint> isparent, BaseContraintable* owner) : IsParent(true), ParentPtr(isparent), OwningInstance(owner){
		};
		// destructor notifies the constraint object to unlink it's references (because might as well destroy constraint when either one disconnects) // 
		~EntitysContraintEntry(){
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

		BaseContraintable* OwningInstance;
		bool IsParent;
		// above flag determines which one is set //
		weak_ptr<BaseContraint> ChildPartPtr;
		shared_ptr<BaseContraint> ParentPtr;

	};

	class BaseContraintable : public BasePhysicsObject{
		friend BaseContraint;
	public:
		DLLEXPORT BaseContraintable();
		DLLEXPORT virtual ~BaseContraintable();

		// only callable on the parent, which then calls child to unlink it's reference //
		DLLEXPORT bool UnlinkContraint(shared_ptr<BaseContraint> constraintptr);
		// unlinks all constraints which this object has whether it is parent or child, will also release all constraints //
		DLLEXPORT void AggressiveConstraintUnlink();

		DLLEXPORT shared_ptr<BaseContraint> GetConstraintPtr(BaseContraint* unsafeptr);

		// constraint creation function that is templated to allow different constraint types, warning DO NOT store the returned value (since that reference isn't counted) //
		// NOTE: before the constraint is actually finished, you need to call ->SetParameters() on the returned ptr and then ->Init() and then let go of
		// the ptr (I recommend doing it on the same line as the function so that you don't accidentally store the ptr)
		template<class ConstraintClass>
		DLLEXPORT ConstraintClass* CreateConstraintWith(BaseContraintable* child){

			// there might be some other way to do this, but we can dynamic cast to an object that has the world ptr stored //
			BaseObject* tmpbase = dynamic_cast<BaseObject*>(this);
			assert(tmpbase != NULL && "constraintable must be a inherited class in a class that also inherits BaseObject");


			shared_ptr<ConstraintClass> tmpconstraint(new ConstraintClass(tmpbase->GetWorld(), this, child));
			// set own parent ptr //
			AddContraintWhereThisIsParent(tmpconstraint);
			// add reference to child object //
			child->AddConstraintWhereThisIsChild(tmpconstraint);

			// return direct ptr for actually creating the joint //
			return tmpconstraint.get();
		}

		// notify functions //
		DLLEXPORT void AddConstraintWhereThisIsChild(weak_ptr<BaseContraint> constraintptr);
		DLLEXPORT void AddContraintWhereThisIsParent(shared_ptr<BaseContraint> constraintptr);
		DLLEXPORT void OnRemoveConstraint(BaseContraint* tmpptr);

	protected:
		// over loadable notify functions //
		virtual void _OnConstraintAdded(BaseContraint* ptr);
		virtual void _OnConstraintUnlink(BaseContraint* ptr);

		// called by the joint object //
		void ConstraintDestroyedRemove(BaseContraint* ptr);
		// ------------------------------------ //
		// for listing of constraints //
		std::vector<EntitysContraintEntry*> PartInConstraints;
	};

}}
#endif