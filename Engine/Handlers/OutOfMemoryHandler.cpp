#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OUTOFMEMORYHANDLER
#include "OutOfMemoryHandler.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::OutOfMemoryHandler::OutOfMemoryHandler(){
	// allocate memory to use when handling //
	try{
		ReservedMemory = new char[OUTOFMEMORY_REQUIREDMEMORY_AMOUNT];
	}
	catch (bad_alloc&){
		// quite embarrassing, really //
		assert(false);
	}
	// set static instance //
	staticinstance = this;
}

DLLEXPORT Leviathan::OutOfMemoryHandler::~OutOfMemoryHandler(){
	// release allocated memory //
	delete[] ReservedMemory;
	ReservedMemory = NULL;
}

OutOfMemoryHandler* Leviathan::OutOfMemoryHandler::staticinstance = NULL;
// ------------------------------------ //
DLLEXPORT  OutOfMemoryHandler* Leviathan::OutOfMemoryHandler::Get(){
	return staticinstance;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::OutOfMemoryHandler::HandleException(std::bad_alloc &except){
	// release reserved memory //
	if(ReservedMemory == NULL){
		// this is rather bad //
		DEBUG_BREAK;
	}

	SAFE_DELETE(ReservedMemory);

	// we probably now have enough memory to write to log and close and pop up a error window //
	Logger::Get()->Error(L"Out of memory! "+Convert::StringToWstring(except.what()), true);
	Logger::Get()->Save();

	// maybe tell engine to try to free up memory //

	// debug build should never continue out of here
	DEBUG_BREAK;
}

// ------------------------------------ //

// ------------------------------------ //







