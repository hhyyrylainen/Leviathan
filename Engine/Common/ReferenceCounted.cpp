#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_REFERENCECOUNTED
#include "ReferenceCounted.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Leviathan::ReferenceCounted::ReferenceCounted() : ToDelete(false), RefCount(1){

}

Leviathan::ReferenceCounted::~ReferenceCounted(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::ReferenceCounted::Release(){
	// this scope so that we will delete AFTER mutex has released //
	{
		// we need to lock this object to ensure thread safety //
		ObjectLock guard(*this);

		if(ToDelete){
			// we really shouldn't be here //
			// abandon object before it is deleted //
			Logger::Get()->Error(L"Object handle is corrupted, decrementing while being deleted!");
			return;
		}

		// we can now decrement reference count //
		if(--RefCount <= 0){
			// we want to fire a NULL error if we aren't at 0 //
			if(RefCount < 0){
				DEBUG_BREAK;
				Logger::Get()->Error(L"Object handle is corrupted, reference count is under 0!");
				return;
			}
			// set to be deleted //
			ToDelete = true;

			goto guibaseselfcheckfordelete;
		}
	}
	return;
guibaseselfcheckfordelete:
	// deleting here isn't without it's problems, but this might be the best alternative opposed to having destructor run before releasing lock //
	if(ToDelete){
		// we need to cleanup ourselves, so here goes //
		delete this;
	}
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //



