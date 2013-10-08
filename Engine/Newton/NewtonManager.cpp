#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NEWTONMANAGER
#include "NewtonManager.h"
#endif
#include <Newton.h>
using namespace Leviathan;
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
	Staticaccess = NULL;

	// release newton library //


}

NewtonManager* Leviathan::NewtonManager::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT shared_ptr<PhysicalWorld> Leviathan::NewtonManager::CreateWorld(){
	// we are probably initialized at this point so it should be safe to just call the constructor //
	return shared_ptr<PhysicalWorld>(new PhysicalWorld());
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


