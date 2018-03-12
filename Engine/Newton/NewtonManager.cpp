// ------------------------------------ //
#include "NewtonManager.h"

#include <Newton.h>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
// ------------------ newton system functions ------------------ //

void* AllocMemory(int size){
	return malloc(size);
}

void FreeMemory(void* ptr, int size){
	free(ptr);
}


// ------------------ NewtonManager ------------------ //
DLLEXPORT Leviathan::NewtonManager::NewtonManager(){
	Staticaccess = this;

	// initialize newton library //
	NewtonSetMemorySystem(AllocMemory, FreeMemory);
}

DLLEXPORT Leviathan::NewtonManager::~NewtonManager(){
	// reset static variable to get nice errors with pointers //
    if(Staticaccess == this)
        Staticaccess = NULL;

	// release newton library //
    

}

NewtonManager* Leviathan::NewtonManager::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT std::shared_ptr<PhysicalWorld> Leviathan::NewtonManager::CreateWorld(GameWorld* owningworld){
	// we are probably initialized at this point so it should be safe to just call the constructor //
	return std::shared_ptr<PhysicalWorld>(new PhysicalWorld(owningworld));
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


